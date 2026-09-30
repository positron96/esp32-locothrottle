#pragma once

#include <lvgl.h>

#include <cstdint>

namespace mono_display {

/** LVGL prepends a 2 entry palette (2 * sizeof(lv_color32_t)) to every I1 flush buffer. */
constexpr uint32_t I1_PALETTE_BYTES = 8;

/**
 * Blit an LVGL I1 render of `area` straight into a U8g2 "vertical top LSB" frame buffer
 * (SSD13xx, UC17xx, UC16xx and friends).
 *
 * Currently unused: the firmware flushes with blit_i1_hline(), which is portable across
 * every U8g2 panel. This one is roughly 4x faster (see tools/sim/bitmap_probe.cpp), so
 * it is worth switching to if blitting ever shows up in a profile; build with
 * -D MONO_BLITTER_VLSB to select it.
 *
 * Note the tradeoff: this reaches past the public API into U8g2's buffer, so it is only
 * valid for the "vertical top LSB" layout and would need revisiting if a future U8g2
 * release rearranged that buffer.
 *
 * The two formats are transposed with respect to each other:
 *   - LVGL I1 packs 8 horizontally adjacent pixels per byte, MSB leftmost;
 *   - U8g2 packs 8 vertically adjacent pixels per byte, LSB topmost, so the pixel at
 *     (x, y) lives in `fb[(y / 8) * page_stride + x]`, bit `1 << (y % 8)`.
 *
 * Pixels are written straight into the frame buffer, 8 columns at a time, instead of
 * going through the U8g2 drawing primitives. Bits of a partially covered page byte that
 * fall outside `area` are preserved, so this is safe for partial (chunked) rendering.
 *
 * @param dst   anything providing U8g2's getBufferPtr() and getBufferTileWidth()
 * @param area  the area being flushed, in display coordinates
 * @param src   start of the I1 pixel data, palette already skipped
 */
template <typename Dst>
void blit_i1_to_vlsb(Dst & dst, const lv_area_t * area, const uint8_t * src) {
    uint8_t * fb = dst.getBufferPtr();
    const uint32_t page_stride = static_cast<uint32_t>(dst.getBufferTileWidth()) * 8;

    const int32_t x1 = area->x1;
    const int32_t y1 = area->y1;
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);
    const uint32_t src_stride = lv_draw_buf_width_to_stride(w, LV_COLOR_FORMAT_I1);

    // LVGL rounds an invalidated I1 area out to byte boundaries, so x1 is a multiple of 8
    // and one source byte maps onto exactly 8 destination columns.
    for(int32_t y = 0; y < h;) {
        const int32_t dst_y = y1 + y;
        const int32_t first_bit = dst_y & 7;
        int32_t rows = 8 - first_bit;
        if(rows > h - y) rows = h - y;

        uint8_t * page = fb + static_cast<uint32_t>(dst_y >> 3) * page_stride + x1;
        // Bits of the destination byte that this pass must leave alone.
        const uint8_t keep = static_cast<uint8_t>(~(((1u << rows) - 1u) << first_bit));

        for(int32_t xb = 0; xb < w; xb += 8) {
            int32_t cols = w - xb;
            if(cols > 8) cols = 8;

            uint8_t col[8] = {};
            for(int32_t r = 0; r < rows; ++r) {
                const uint8_t byte = src[(y + r) * src_stride + (xb >> 3)];
                if(byte == 0) continue;  // common case on a mostly dark screen
                const uint8_t bit = static_cast<uint8_t>(1u << (first_bit + r));
                for(int32_t c = 0; c < cols; ++c) {
                    if(byte & (0x80u >> c)) col[c] |= bit;
                }
            }

            uint8_t * out = page + xb;
            for(int32_t c = 0; c < cols; ++c) out[c] = (out[c] & keep) | col[c];
        }

        y += rows;
    }
}

/**
 * Blit an LVGL I1 render of `area` through U8g2's public line drawing API.
 *
 * This is the blitter the firmware uses by default. Spans of equal pixels within a row
 * are collapsed into a single drawHLine() call, which suits UI content well: borders,
 * bars and glyph rows are long uniform runs, so a 128x64 frame costs far fewer calls
 * than one per pixel. Both colors are drawn, because a flush has to overwrite the
 * previous contents rather than blend with them.
 *
 * Unlike blit_i1_to_vlsb() this makes no assumption about how U8g2 arranges its frame
 * buffer, so it works with every panel and rotation U8g2 supports. Measured on the host
 * it is roughly a quarter the speed of the direct blit but around 7x faster than driving
 * one pixel at a time; see tools/sim/bitmap_probe.cpp.
 *
 * Runs are found along rows rather than columns on purpose. u8g2_DrawHVLine() writes one
 * pixel per loop iteration in both directions, but the horizontal path walks a pointer
 * with a mask fixed for the whole run, whereas the vertical path recomputes the bit
 * position and re-reads the same byte for every pixel.
 *
 * @param dst   anything providing U8g2's setDrawColor(uint8_t) and drawHLine(x, y, len)
 * @param area  the area being flushed, in display coordinates
 * @param src   start of the I1 pixel data, palette already skipped
 */
template <typename Dst>
void blit_i1_hline(Dst & dst, const lv_area_t * area, const uint8_t * src) {
    const int32_t x1 = area->x1;
    const int32_t y1 = area->y1;
    const int32_t w = lv_area_get_width(area);
    const int32_t h = lv_area_get_height(area);
    const uint32_t src_stride = lv_draw_buf_width_to_stride(w, LV_COLOR_FORMAT_I1);

    // setDrawColor() is not free, so only call it when the color actually changes.
    uint8_t draw_color = 2;  // neither 0 nor 1, forces the first set
    for(int32_t y = 0; y < h; ++y) {
        const uint8_t * row = src + y * src_stride;
        int32_t x = 0;
        while(x < w) {
            const uint8_t color = (row[x >> 3] >> (7 - (x & 7))) & 1;
            int32_t run = 1;
            while(x + run < w &&
                  (((row[(x + run) >> 3] >> (7 - ((x + run) & 7))) & 1) == color)) {
                ++run;
            }
            if(color != draw_color) {
                dst.setDrawColor(color);
                draw_color = color;
            }
            dst.drawHLine(x1 + x, y1 + y, run);
            x += run;
        }
    }
    dst.setDrawColor(1);
}

}  // namespace mono_display
