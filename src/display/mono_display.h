#pragma once

#include <lvgl.h>

#include <cstdint>

/**
 * LVGL display backend for a monochrome SSD1306 128x64 OLED driven over I2C.
 *
 * LVGL renders into an I1 (1 bit per pixel) buffer; the flush callback draws it into the
 * U8g2 frame buffer and pushes only the affected 8x8 tiles, so any panel supported by
 * U8g2 can be used by changing the U8g2 class in mono_display.cpp.
 *
 * Two blitters are available, selected at build time:
 *   (default)              blit_i1_hline(), public U8g2 API, works with any panel
 *   -D MONO_BLITTER_VLSB   blit_i1_to_vlsb(), writes the frame buffer directly, faster
 *                          but only valid for the "vertical top LSB" layout
 * Build with -D MONO_BLIT_PROFILE=0 to compile the timing instrumentation out.
 */
namespace mono_display {

constexpr int32_t WIDTH = 128;
constexpr int32_t HEIGHT = 64;

/** Cumulative flush cost, split into CPU-bound blitting and I2C-bound transfer. */
struct Stats {
    uint32_t flushes;  ///< flush_cb() calls
    uint32_t blit_us;  ///< microseconds spent converting LVGL output to U8g2 pixels
    uint32_t xfer_us;  ///< microseconds spent in updateDisplayArea()
    uint32_t tiles;    ///< 8x8 tiles sent, i.e. bytes/8 of I2C payload
    uint32_t pixels;   ///< pixels blitted
};

/**
 * Initialize I2C, the panel and LVGL (tick source, display, mono theme).
 * @return false if the panel did not answer on the I2C bus.
 */
bool begin();

/** The LVGL display created by begin(), or nullptr before initialization. */
lv_display_t * display();

/** Flush cost accumulated since the last reset_stats(). Zeroed if profiling is off. */
Stats stats_snapshot();

/** Start a fresh measurement window. */
void reset_stats();

/** Human-readable name of the blitter this build was compiled with. */
const char * blitter_name();

/** Panel contrast/brightness, 0..255. */
void set_contrast(uint8_t contrast);

/** Turn the panel on/off while keeping the LVGL state intact. */
void set_power(bool on);

}  // namespace mono_display
