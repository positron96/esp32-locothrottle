#include "display/mono_display.h"

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "display/i1_blit.h"
#include "pins.h"

// Timing instrumentation costs a couple of micros() reads per flush; leave it on by
// default since flushes are rare, but allow -D MONO_BLIT_PROFILE=0 to compile it out.
#ifndef MONO_BLIT_PROFILE
#define MONO_BLIT_PROFILE 1
#endif

namespace mono_display {
namespace {

// Full frame buffer panel driver. Swapping this type (and the constructor arguments)
// is enough to move the whole UI to another U8g2-supported monochrome panel.
using Panel = U8G2_SSD1306_128X64_NONAME_F_HW_I2C;
Panel panel(U8G2_R0, /*reset=*/U8X8_PIN_NONE, /*clock=*/pins::I2C_SCL, /*data=*/pins::I2C_SDA);

lv_display_t * lv_disp = nullptr;

// Which blitter the flush callback uses. Both are in i1_blit.h and produce identical
// output; build with -D MONO_BLITTER_VLSB to pick the faster layout-dependent one.
#ifdef MONO_BLITTER_VLSB
#define BLIT blit_i1_to_vlsb
constexpr const char * BLITTER_NAME = "blit_i1_to_vlsb (direct buffer)";
#else
#define BLIT blit_i1_hline
constexpr const char * BLITTER_NAME = "blit_i1_hline (public API)";
#endif

Stats stats;
constexpr uint32_t STRIDE = (WIDTH + 7) / 8;

alignas(4) uint8_t draw_buffer[I1_PALETTE_BYTES + STRIDE * HEIGHT];

//lv_style_t style_no_anim;
lv_theme_t * theme = nullptr;

uint32_t tick_cb() {
    return millis();
}

/**
 * Draw the LVGL render into the U8g2 frame buffer and push only the affected tiles.
 *
 * The panel is page addressed, so the smallest transferable unit is an 8x8 tile; the
 * flushed area is expanded to tile boundaries before sending. Sending just the dirty
 * tiles instead of the whole 1 KiB buffer is what keeps the UI responsive over I2C.
 *
 * The blitter is selected at build time, see BLITTER_NAME above. Blit and transfer are
 * timed separately, because they are expected to differ by orders of magnitude: the blit
 * is CPU-bound on a 240 MHz core while the transfer is bound by the I2C clock.
 */
void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    // LVGL prepends a palette to the flush buffer; the blitters want pixel data.
    const uint8_t * src = px_map + I1_PALETTE_BYTES;

    const uint8_t tx = static_cast<uint8_t>(area->x1 >> 3);
    const uint8_t ty = static_cast<uint8_t>(area->y1 >> 3);
    const uint8_t tw = static_cast<uint8_t>((area->x2 >> 3) - tx + 1);
    const uint8_t th = static_cast<uint8_t>((area->y2 >> 3) - ty + 1);

#if MONO_BLIT_PROFILE
    // The barriers stop the compiler moving work across the timestamp reads.
    const uint32_t t0 = micros();
    asm volatile("" ::: "memory");
    BLIT(panel, area, src);
    asm volatile("" ::: "memory");
    const uint32_t t1 = micros();
    panel.updateDisplayArea(tx, ty, tw, th);
    asm volatile("" ::: "memory");
    const uint32_t t2 = micros();

    stats.flushes++;
    stats.blit_us += t1 - t0;
    stats.xfer_us += t2 - t1;
    stats.tiles += static_cast<uint32_t>(tw) * th;
    stats.pixels += static_cast<uint32_t>(lv_area_get_width(area)) *
                    lv_area_get_height(area);
#else
    BLIT(panel, area, src);
    panel.updateDisplayArea(tx, ty, tw, th);
#endif

    lv_display_flush_ready(disp);
}

void theme_apply_cb(lv_theme_t *, lv_obj_t * obj) {
    //lv_obj_add_style(obj, &style_no_anim, 0);
}

void init_theme() {
    //lv_style_init(&style_no_anim);
    //lv_style_set_anim_duration(&style_no_anim, 0);

    lv_theme_t * mono = lv_theme_mono_init(lv_disp, /*dark_bg=*/true, LV_FONT_DEFAULT);

    // Chain a theme on top of the mono theme that zeroes the animation time of every widget.
    // theme = lv_theme_create();
    // lv_theme_copy(theme, mono);
    // lv_theme_set_parent(theme, mono);
    // lv_theme_set_apply_cb(theme, theme_apply_cb);
    lv_display_set_theme(lv_disp, mono);
}

bool panel_responds() {
    Wire.beginTransmission(pins::OLED_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

}  // namespace

bool begin() {
    Wire.begin(pins::I2C_SDA, pins::I2C_SCL, pins::I2C_FREQUENCY);
    if(!panel_responds()) return false;

    panel.setI2CAddress(pins::OLED_I2C_ADDRESS << 1);
    panel.setBusClock(pins::I2C_FREQUENCY);
    panel.begin();
    panel.clearBuffer();
    panel.sendBuffer();

    lv_init();
    lv_tick_set_cb(tick_cb);

    lv_disp = lv_display_create(WIDTH, HEIGHT);
    lv_display_set_color_format(lv_disp, LV_COLOR_FORMAT_I1);
    lv_display_set_buffers(lv_disp, draw_buffer, nullptr, sizeof(draw_buffer),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(lv_disp, flush_cb);
    lv_display_set_default(lv_disp);

    init_theme();
    return true;
}

lv_display_t * display() {
    return lv_disp;
}

Stats stats_snapshot() {
    return stats;
}

void reset_stats() {
    stats = Stats{};
}

const char * blitter_name() {
    return BLITTER_NAME;
}

void set_contrast(uint8_t contrast) {
    panel.setContrast(contrast);
}

void set_power(bool on) {
    panel.setPowerSave(on ? 0 : 1);
}

}  // namespace mono_display
