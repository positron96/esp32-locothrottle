/**
 * Checks whether LVGL's I1 buffer layout can be handed straight to U8g2's bitmap
 * drawing calls, by running the real U8g2 library on the host against a dummy panel.
 *
 * This deliberately links the actual U8g2 sources rather than the simulator's stand-in,
 * so the answer reflects what the library really does rather than a model of it.
 */

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <vector>

#include "lvgl.h"
#include "display/i1_blit.h"

extern "C" {
#include "u8g2.h"
}

namespace {

constexpr int PANEL_W = 128;
constexpr int PANEL_H = 64;

// The area under test: LVGL always byte-aligns I1 areas horizontally, so x1 and width
// are multiples of 8. y is deliberately not tile aligned, which is the awkward case.
constexpr int32_t AX = 8, AY = 3, AW = 32, AH = 13;

u8g2_t g_u8g2;

size_t buffer_bytes() {
    return static_cast<size_t>(u8g2_GetBufferTileWidth(&g_u8g2)) * 8 *
           u8g2_GetBufferTileHeight(&g_u8g2);
}

std::vector<uint8_t> snapshot() {
    const uint8_t * buf = u8g2_GetBufferPtr(&g_u8g2);
    return std::vector<uint8_t>(buf, buf + buffer_bytes());
}

void clear_buffer() {
    std::memset(u8g2_GetBufferPtr(&g_u8g2), 0, buffer_bytes());
}

/** Reverses the bit order within each byte, MSB-first <-> LSB-first. */
uint8_t reverse_bits(uint8_t v) {
    v = static_cast<uint8_t>(((v & 0xF0) >> 4) | ((v & 0x0F) << 4));
    v = static_cast<uint8_t>(((v & 0xCC) >> 2) | ((v & 0x33) << 2));
    v = static_cast<uint8_t>(((v & 0xAA) >> 1) | ((v & 0x55) << 1));
    return v;
}

/** Thin adapter so the shipped blitters can drive the real u8g2_t. */
struct RealCanvas {
    void setDrawColor(uint8_t c) { u8g2_SetDrawColor(&g_u8g2, c); }
    void drawHLine(int32_t x, int32_t y, int32_t len) {
        u8g2_DrawHLine(&g_u8g2, static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                       static_cast<u8g2_uint_t>(len));
    }
    uint8_t * getBufferPtr() { return u8g2_GetBufferPtr(&g_u8g2); }
    uint8_t getBufferTileWidth() const { return u8g2_GetBufferTileWidth(&g_u8g2); }
};

/** Per-pixel blit, kept only as the benchmark baseline the other options are judged against. */
void blit_pixels(const lv_area_t * area, const uint8_t * bits) {
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);
    const uint32_t stride = lv_draw_buf_width_to_stride(w, LV_COLOR_FORMAT_I1);

    uint8_t draw_color = 2;
    for(int32_t y = 0; y < h; ++y) {
        for(int32_t x = 0; x < w; ++x) {
            const uint8_t color = (bits[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1;
            if(color != draw_color) {
                u8g2_SetDrawColor(&g_u8g2, color);
                draw_color = color;
            }
            u8g2_DrawPixel(&g_u8g2, static_cast<u8g2_uint_t>(area->x1 + x),
                           static_cast<u8g2_uint_t>(area->y1 + y));
        }
    }
    u8g2_SetDrawColor(&g_u8g2, 1);
}

int report(const char * name, const std::vector<uint8_t> & ref,
           const std::vector<uint8_t> & got) {
    size_t diff = 0;
    for(size_t i = 0; i < ref.size(); ++i)
        if(ref[i] != got[i]) ++diff;
    std::printf("  %-34s %s", name, diff == 0 ? "MATCH" : "DIFFERS");
    if(diff) std::printf("  (%zu/%zu bytes)", diff, ref.size());
    std::printf("\n");
    return diff == 0 ? 1 : 0;
}

/** Lookup table for MSB<->LSB bit reversal, the cost XBM has to pay. */
uint8_t g_reverse[256];

void build_reverse_table() {
    for(int i = 0; i < 256; ++i) g_reverse[i] = reverse_bits(static_cast<uint8_t>(i));
}

/** Reads one pixel out of an LVGL I1 render (rows packed MSB first). */
inline uint8_t src_pixel(const uint8_t * bits, uint32_t stride, int32_t x, int32_t y) {
    return (bits[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1;
}

/**
 * Run-length blit along rows, a local copy of the shipped blit_i1_hline() kept here only
 * so the u8g2 free function form can be timed identically to the other candidates.
 * Correctness of the real one is covered by the RealCanvas case below.
 */
void blit_runs_hline(const lv_area_t * area, const uint8_t * bits) {
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);
    const uint32_t stride = lv_draw_buf_width_to_stride(w, LV_COLOR_FORMAT_I1);

    uint8_t draw_color = 2;
    for(int32_t y = 0; y < h; ++y) {
        int32_t x = 0;
        while(x < w) {
            const uint8_t color = src_pixel(bits, stride, x, y);
            int32_t run = 1;
            while(x + run < w && src_pixel(bits, stride, x + run, y) == color) ++run;
            if(color != draw_color) {
                u8g2_SetDrawColor(&g_u8g2, color);
                draw_color = color;
            }
            u8g2_DrawHLine(&g_u8g2, static_cast<u8g2_uint_t>(area->x1 + x),
                           static_cast<u8g2_uint_t>(area->y1 + y),
                           static_cast<u8g2_uint_t>(run));
            x += run;
        }
    }
    u8g2_SetDrawColor(&g_u8g2, 1);
}
/**
 * Run-length blit along columns. A vertical run lands in consecutive bits of a single
 * frame buffer byte in the vertical-LSB layout, so u8g2 can service it with a masked
 * read-modify-write per page rather than per pixel. The catch is that reading a column
 * out of row-packed source touches a different byte for every pixel.
 */
void blit_runs_vline(const lv_area_t * area, const uint8_t * bits) {
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);
    const uint32_t stride = lv_draw_buf_width_to_stride(w, LV_COLOR_FORMAT_I1);

    uint8_t draw_color = 2;
    for(int32_t x = 0; x < w; ++x) {
        int32_t y = 0;
        while(y < h) {
            const uint8_t color = src_pixel(bits, stride, x, y);
            int32_t run = 1;
            while(y + run < h && src_pixel(bits, stride, x, y + run) == color) ++run;
            if(color != draw_color) {
                u8g2_SetDrawColor(&g_u8g2, color);
                draw_color = color;
            }
            u8g2_DrawVLine(&g_u8g2, static_cast<u8g2_uint_t>(area->x1 + x),
                           static_cast<u8g2_uint_t>(area->y1 + y),
                           static_cast<u8g2_uint_t>(run));
            y += run;
        }
    }
    u8g2_SetDrawColor(&g_u8g2, 1);
}

}  // namespace

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    // lv_draw_buf_width_to_stride() dispatches through handlers that only exist after
    // lv_init(); without this the blitters call a null pointer.
    lv_init();

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&g_u8g2, U8G2_R0, u8x8_byte_empty,
                                           u8x8_dummy_cb);
    // Deliberately no u8g2_InitDisplay(): there is no panel to initialise, and the
    // SSD1306 init sequence crashes against the dummy callbacks. Only the frame buffer
    // and the clip window matter here, and u8g2_ClearBuffer() establishes those.
    u8g2_ClearBuffer(&g_u8g2);

    std::printf("u8g2 buffer: tile_w=%u tile_h=%u (%zu bytes)\n",
                u8g2_GetBufferTileWidth(&g_u8g2), u8g2_GetBufferTileHeight(&g_u8g2),
                buffer_bytes());
    std::printf("area: x=%d y=%d w=%d h=%d\n", AX, AY, AW, AH);

    // Guard against a vacuous result: if the clip window were empty, every method would
    // draw nothing and all comparisons would "match" as identical all-zero buffers.
    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    u8g2_DrawPixel(&g_u8g2, AX, AY);
    size_t lit = 0;
    for(uint8_t b : snapshot()) lit += (b != 0);
    if(lit == 0) {
        std::printf("\nABORT: u8g2 drawing is being clipped away, results would be "
                    "meaningless\n");
        return 1;
    }
    std::printf("sanity: u8g2_DrawPixel reaches the buffer\n\n");

    // Build an LVGL style I1 source: 8 palette bytes, then rows packed MSB first.
    const uint32_t stride = lv_draw_buf_width_to_stride(AW, LV_COLOR_FORMAT_I1);
    std::vector<uint8_t> px_map(mono_display::I1_PALETTE_BYTES + stride * AH, 0);
    uint32_t seed = 12345;
    for(int32_t row = 0; row < AH; ++row) {
        for(uint32_t b = 0; b < stride; ++b) {
            seed = seed * 1103515245u + 12345u;
            px_map[mono_display::I1_PALETTE_BYTES + row * stride + b] =
                static_cast<uint8_t>(seed >> 16);
        }
    }
    const uint8_t * bits = px_map.data() + mono_display::I1_PALETTE_BYTES;

    lv_area_t area;
    area.x1 = AX;
    area.y1 = AY;
    area.x2 = AX + AW - 1;
    area.y2 = AY + AH - 1;

    std::printf("LVGL I1 stride for w=%d: %u bytes (tightly packed = %d)\n\n", AW, stride,
                (AW + 7) / 8);

    // Reference: the direct-to-buffer blitter, which the simulator cross-checks too.
    RealCanvas canvas;
    clear_buffer();
    mono_display::blit_i1_to_vlsb(canvas, &area, bits);
    const std::vector<uint8_t> ref = snapshot();

    size_t ref_lit = 0;
    for(uint8_t b : ref) ref_lit += (b != 0);
    if(ref_lit == 0) {
        std::printf("ABORT: the reference blit produced an empty buffer\n");
        return 1;
    }

    int pass = 0, total = 0;
    std::printf("against blit_i1_to_vlsb() as reference:\n");

    // Sanity check that the reference itself is trustworthy: the shipped blitter,
    // driven through the real u8g2 API, must agree.
    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    mono_display::blit_i1_hline(canvas, &area, bits);
    total++; pass += report("blit_i1_hline (shipped, real u8g2)", ref, snapshot());

    // u8g2_DrawBitmap: MSB first, same as LVGL.
    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    u8g2_SetBitmapMode(&g_u8g2, 0);  // opaque, so clear bits are drawn too
    for(int32_t row = 0; row < AH; ++row) {
        u8g2_DrawBitmap(&g_u8g2, AX, AY + row, AW / 8, 1, bits + row * stride);
    }
    total++; pass += report("u8g2_DrawBitmap, as-is", ref, snapshot());

    // u8g2_DrawXBM: LSB first, so feeding it LVGL data unchanged should NOT match.
    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    u8g2_SetBitmapMode(&g_u8g2, 0);
    u8g2_DrawXBM(&g_u8g2, AX, AY, AW, AH, bits);
    total++; pass += report("u8g2_DrawXBM, as-is", ref, snapshot());

    // u8g2_DrawXBM with each byte's bits reversed should match.
    std::vector<uint8_t> flipped(stride * AH);
    for(size_t i = 0; i < flipped.size(); ++i) flipped[i] = reverse_bits(bits[i]);
    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    u8g2_SetBitmapMode(&g_u8g2, 0);
    u8g2_DrawXBM(&g_u8g2, AX, AY, AW, AH, flipped.data());
    total++; pass += report("u8g2_DrawXBM, bits reversed", ref, snapshot());

    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    blit_runs_hline(&area, bits);
    total++; pass += report("run-length drawHLine", ref, snapshot());

    clear_buffer();
    u8g2_SetDrawColor(&g_u8g2, 1);
    blit_runs_vline(&area, bits);
    total++; pass += report("run-length drawVLine", ref, snapshot());

    std::printf("\n%d of %d comparisons matched\n", pass, total);

    // ---- throughput -------------------------------------------------------------
    // Layout compatibility is only half the question; the two calls have very different
    // inner loops. DrawBitmap emits one DrawHVLine per pixel, while DrawXBM coalesces
    // runs of equal bits into a single DrawHVLine, so content matters. UI screens are
    // mostly long runs; random noise is the adversarial case for run-length coding.
    build_reverse_table();

    constexpr int BW = 128, BH = 64;
    const uint32_t bstride = lv_draw_buf_width_to_stride(BW, LV_COLOR_FORMAT_I1);
    lv_area_t full;
    full.x1 = 0; full.y1 = 0; full.x2 = BW - 1; full.y2 = BH - 1;

    std::vector<uint8_t> ui(mono_display::I1_PALETTE_BYTES + bstride * BH, 0);
    std::vector<uint8_t> noise = ui;
    for(int32_t row = 0; row < BH; ++row) {
        for(uint32_t b = 0; b < bstride; ++b) {
            const size_t i = mono_display::I1_PALETTE_BYTES + row * bstride + b;
            // UI-like: a border plus a couple of filled bars, i.e. long uniform runs.
            const bool edge = (row < 2 || row >= BH - 2);
            const bool bar = (row > 20 && row < 30) || (row > 40 && row < 46);
            ui[i] = (edge || bar) ? 0xFF : (b == 0 ? 0xC0 : 0x00);
            seed = seed * 1103515245u + 12345u;
            noise[i] = static_cast<uint8_t>(seed >> 16);
        }
    }

    const int REPS = 2000;
    enum class Method { Direct, Pixels, Bitmap, Xbm, HLine, VLine };
    struct Bench { const char * name; Method method; };
    const Bench benches[] = {
        {"blit_i1_to_vlsb (direct buffer)", Method::Direct},
        {"blit_i1_hline (shipped)", Method::HLine},
        {"per-pixel drawPixel", Method::Pixels},
        {"u8g2_DrawBitmap", Method::Bitmap},
        {"u8g2_DrawXBM + bit reversal", Method::Xbm},
        {"run-length drawVLine", Method::VLine},
    };

    for(int content = 0; content < 2; ++content) {
        const std::vector<uint8_t> & src = content == 0 ? ui : noise;
        const uint8_t * sbits = src.data() + mono_display::I1_PALETTE_BYTES;
        std::printf("\n%s content, %d reps of a full %dx%d blit:\n",
                    content == 0 ? "UI-like" : "random-noise", REPS, BW, BH);

        for(const Bench & bench : benches) {
            std::vector<uint8_t> flip(bstride * BH);
            const auto t0 = std::chrono::steady_clock::now();
            for(int r = 0; r < REPS; ++r) {
                clear_buffer();
                u8g2_SetDrawColor(&g_u8g2, 1);
                switch(bench.method) {
                    case Method::Xbm:
                        for(size_t i = 0; i < flip.size(); ++i)
                            flip[i] = g_reverse[sbits[i]];
                        u8g2_DrawXBM(&g_u8g2, 0, 0, BW, BH, flip.data());
                        break;
                    case Method::Bitmap:
                        for(int32_t row = 0; row < BH; ++row)
                            u8g2_DrawBitmap(&g_u8g2, 0, row, BW / 8, 1,
                                            sbits + row * bstride);
                        break;
                    case Method::Pixels:
                        blit_pixels(&full, sbits);
                        break;
                    case Method::HLine:
                        blit_runs_hline(&full, sbits);
                        break;
                    case Method::VLine:
                        blit_runs_vline(&full, sbits);
                        break;
                    case Method::Direct:
                        mono_display::blit_i1_to_vlsb(canvas, &full, sbits);
                        break;
                }
            }
            const auto t1 = std::chrono::steady_clock::now();
            const double us =
                std::chrono::duration<double, std::micro>(t1 - t0).count() / REPS;
            std::printf("  %-34s %8.1f us/frame\n", bench.name, us);
        }
    }

    return 0;
}
