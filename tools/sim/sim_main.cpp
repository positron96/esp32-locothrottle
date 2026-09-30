/**
 * Host-side renderer for the monochrome demo UI.
 *
 * Builds the real demo screens with LVGL compiled for the host, flushes them through the
 * same blit_i1_hline() code the firmware uses into a frame buffer laid out exactly like
 * U8g2's, and prints each screen as ASCII art. Build and run with `make -C tools/sim run`.
 *
 * The unused blit_i1_to_vlsb() runs alongside it on every flush, into a separate buffer,
 * so the two are checked against each other and the unused one cannot silently rot.
 *
 * The fake panel only accepts pixels that were covered by an updateDisplayArea() call,
 * so the printed screens also prove that the partial tile updates cover everything drawn.
 */

#include <lvgl.h>

#include <cstdint>
#include <cstdio>

#include "display/i1_blit.h"
#include "display/mono_display.h"
#include "ui/demo_ui.h"

namespace {

constexpr int32_t W = mono_display::WIDTH;
constexpr int32_t H = mono_display::HEIGHT;
constexpr int32_t TILES_X = W / 8;
constexpr int32_t TILES_Y = H / 8;

/**
 * Stand-in for a U8g2 full-buffer device: holds the page-addressed frame buffer plus a
 * "panel" that only changes where updateDisplayArea() actually transferred tiles.
 */
class FakePanel {
public:
    uint8_t * getBufferPtr() { return fb_; }
    uint8_t getBufferTileWidth() const { return TILES_X; }

    void updateDisplayArea(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th) {
        if(tx + tw > TILES_X || ty + th > TILES_Y) {
            ++bad_updates_;
            return;
        }
        for(int32_t t = 0; t < th; ++t) {
            for(int32_t s = 0; s < tw; ++s) {
                for(int32_t c = 0; c < 8; ++c) {
                    panel_[(ty + t) * page_stride() + (tx + s) * 8 + c] =
                        fb_[(ty + t) * page_stride() + (tx + s) * 8 + c];
                }
            }
        }
        tiles_sent_ += static_cast<uint32_t>(tw) * th;
        ++updates_;
    }

    /** Decode the page-addressed panel content back into a pixel. */
    bool pixel(int32_t x, int32_t y) const {
        return (panel_[(y >> 3) * page_stride() + x] >> (y & 7)) & 1;
    }

    uint32_t lit() const {
        uint32_t n = 0;
        for(int32_t y = 0; y < H; ++y)
            for(int32_t x = 0; x < W; ++x) n += pixel(x, y) ? 1 : 0;
        return n;
    }

    /** True if the panel shows everything the frame buffer holds. */
    bool panel_matches_buffer() const {
        for(int32_t i = 0; i < H / 8 * page_stride(); ++i)
            if(panel_[i] != fb_[i]) return false;
        return true;
    }

    /* U8g2's public line API, which is what the firmware's blitter uses. This writes the
     * frame buffer that the panel is then updated from. */
    void setDrawColor(uint8_t color) { draw_color_ = color; }

    void drawHLine(int32_t x, int32_t y, int32_t len) {
        if(y < 0 || y >= H || x < 0 || len < 0 || x + len > W) {
            ++out_of_bounds_;
            return;
        }
        const uint8_t mask = static_cast<uint8_t>(1u << (y & 7));
        uint8_t * row = fb_ + (y >> 3) * page_stride();
        for(int32_t i = 0; i < len; ++i) {
            if(draw_color_) row[x + i] |= mask;
            else row[x + i] &= static_cast<uint8_t>(~mask);
        }
        ++hline_calls_;
    }

    /** Scratch buffer for cross-checking the unused blit_i1_to_vlsb() against the above. */
    uint8_t * getAltBufferPtr() { return alt_fb_; }

    /** True if the unused direct blitter produced exactly the same frame buffer. */
    bool alt_matches_buffer() const {
        for(int32_t i = 0; i < H / 8 * page_stride(); ++i)
            if(alt_fb_[i] != fb_[i]) return false;
        return true;
    }

    uint32_t updates() const { return updates_; }
    uint32_t tiles_sent() const { return tiles_sent_; }
    uint32_t bad_updates() const { return bad_updates_; }
    uint32_t out_of_bounds() const { return out_of_bounds_; }
    uint32_t hline_calls() const { return hline_calls_; }
    void reset_counters() {
        updates_ = 0;
        tiles_sent_ = 0;
    }

private:
    static constexpr int32_t page_stride() { return TILES_X * 8; }

    uint8_t fb_[H / 8 * TILES_X * 8] = {};
    uint8_t alt_fb_[H / 8 * TILES_X * 8] = {};
    uint8_t panel_[H / 8 * TILES_X * 8] = {};
    uint8_t draw_color_ = 1;
    uint32_t updates_ = 0;
    uint32_t tiles_sent_ = 0;
    uint32_t bad_updates_ = 0;
    uint32_t out_of_bounds_ = 0;
    uint32_t hline_calls_ = 0;
};

FakePanel panel;

/**
 * Presents the panel's scratch buffer through the same getBufferPtr()/getBufferTileWidth()
 * interface, so blit_i1_to_vlsb() can be run side by side with the shipped blitter.
 */
struct AltBuffer {
    FakePanel & panel;
    uint8_t * getBufferPtr() { return panel.getAltBufferPtr(); }
    uint8_t getBufferTileWidth() const { return panel.getBufferTileWidth(); }
};

uint32_t fake_tick = 0;
lv_style_t style_no_anim;
alignas(4) uint8_t draw_buffer[mono_display::I1_PALETTE_BYTES + ((W + 7) / 8) * H];

uint32_t tick_cb() {
    return fake_tick;
}

void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    // LVGL prepends a palette to the flush buffer; the blitters want pixel data.
    const uint8_t * src = px_map + mono_display::I1_PALETTE_BYTES;

    mono_display::blit_i1_hline(panel, area, src);

    // Run the unused direct blitter over the same render, into its own buffer, so that
    // every flush of the demo checks the two produce byte identical results.
    AltBuffer alt{panel};
    mono_display::blit_i1_to_vlsb(alt, area, src);

    const uint8_t tx = static_cast<uint8_t>(area->x1 >> 3);
    const uint8_t ty = static_cast<uint8_t>(area->y1 >> 3);
    const uint8_t tw = static_cast<uint8_t>((area->x2 >> 3) - tx + 1);
    const uint8_t th = static_cast<uint8_t>((area->y2 >> 3) - ty + 1);
    panel.updateDisplayArea(tx, ty, tw, th);

    lv_display_flush_ready(disp);
}

void theme_apply_cb(lv_theme_t *, lv_obj_t * obj) {
    lv_obj_add_style(obj, &style_no_anim, 0);
}

void rule() {
    std::printf("+");
    for(int32_t x = 0; x < W; ++x) std::printf("-");
    std::printf("+\n");
}

void dump(const char * title) {
    std::printf("\n%s (%u lit px, %u updates, %u/%u tiles sent)\n", title, panel.lit(),
                panel.updates(), panel.tiles_sent(), TILES_X * TILES_Y);
    rule();
    for(int32_t y = 0; y < H; ++y) {
        std::printf("|");
        for(int32_t x = 0; x < W; ++x) std::printf("%c", panel.pixel(x, y) ? '#' : ' ');
        std::printf("|\n");
    }
    rule();
    panel.reset_counters();
}

/** Advance the fake clock and let LVGL render everything that became due. */
void advance(uint32_t ms) {
    for(uint32_t i = 0; i < ms; i += 5) {
        fake_tick += 5;
        lv_timer_handler();
    }
}

}  // namespace

int main() {
    lv_init();
    lv_tick_set_cb(tick_cb);

    lv_display_t * disp = lv_display_create(W, H);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_I1);
    lv_display_set_buffers(disp, draw_buffer, nullptr, sizeof(draw_buffer),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_default(disp);

    lv_style_init(&style_no_anim);
    lv_style_set_anim_duration(&style_no_anim, 0);
    lv_theme_t * mono = lv_theme_mono_init(disp, /*dark_bg=*/true, LV_FONT_DEFAULT);
    lv_theme_t * theme = lv_theme_create();
    lv_theme_copy(theme, mono);
    lv_theme_set_parent(theme, mono);
    lv_theme_set_apply_cb(theme, theme_apply_cb);
    lv_display_set_theme(disp, theme);

    demo_ui::start();

    advance(500);
    dump("Screen 1: splash");

    advance(4000);
    dump("Screen 2: throttle");

    advance(4000);
    dump("Screen 3: menu");

    // A steady state redraw: only the speed readout and bars change, so the partial
    // update should touch far fewer tiles than a full frame.
    advance(4000);
    dump("Screen 1 again: splash");

    advance(1000);
    const uint32_t steady_tiles = panel.tiles_sent();
    const uint32_t steady_updates = panel.updates();
    // Every tile is 8 bytes on the wire, a full frame is the whole buffer.
    const uint32_t full_frames = 1000 / LV_DEF_REFR_PERIOD;
    std::printf("\nSteady state over 1 s: %u updates, %u tiles (%u bytes).\n", steady_updates,
                steady_tiles, steady_tiles * 8);
    std::printf("Redrawing full frames instead would cost %u tiles (%u bytes).\n",
                full_frames * TILES_X * TILES_Y, full_frames * TILES_X * TILES_Y * 8);

    if(panel.bad_updates() != 0) {
        std::printf("FAIL: %u updateDisplayArea() calls fell outside the panel\n",
                    panel.bad_updates());
        return 1;
    }
    if(panel.out_of_bounds() != 0) {
        std::printf("FAIL: the blitter drew %u spans outside the panel\n",
                    panel.out_of_bounds());
        return 1;
    }
    if(panel.hline_calls() == 0) {
        std::printf("FAIL: no drawHLine() calls were made\n");
        return 1;
    }
    if(!panel.alt_matches_buffer()) {
        std::printf("FAIL: blit_i1_to_vlsb() and blit_i1_hline() disagree\n");
        return 1;
    }
    if(!panel.panel_matches_buffer()) {
        std::printf("FAIL: the panel does not match the frame buffer, a dirty tile was "
                    "not sent\n");
        return 1;
    }
    if(steady_tiles >= full_frames * TILES_X * TILES_Y) {
        std::printf("FAIL: partial updates sent %u tiles, no better than full frames\n",
                    steady_tiles);
        return 1;
    }
    std::printf("OK: tile updates in bounds, panel matches buffer, partial updates active,\n"
                "    unused blit_i1_to_vlsb() agrees with blit_i1_hline()\n");
    return 0;
}
