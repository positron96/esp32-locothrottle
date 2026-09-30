#include <Arduino.h>
#include <lvgl.h>

#include "display/mono_display.h"
#include "ui/demo_ui.h"

namespace {

bool display_ready = false;
uint32_t last_report_ms = 0;

constexpr uint32_t REPORT_PERIOD_MS = 5000;

/**
 * Print how the last window was spent, split into blitting and I2C transfer.
 *
 * Both are reported per flush and as a share of wall clock time, since what matters is
 * how much of the CPU the display backend is taking, not the absolute microseconds.
 */
void report_stats() {
    const mono_display::Stats s = mono_display::stats_snapshot();
    if(s.flushes == 0) {
        Serial.println("display: idle, no flushes");
        return;
    }

    const uint32_t total_us = s.blit_us + s.xfer_us;
    // A tile is 8 bytes of payload; the I2C frame adds address and control overhead.
    const uint32_t bytes = s.tiles * 8;

    Serial.printf("display: %s\n", mono_display::blitter_name());
    Serial.printf("  %lu flushes, %lu px, %lu tiles (%lu B) in %lu ms\n",
                  s.flushes, s.pixels, s.tiles, bytes, REPORT_PERIOD_MS);
    Serial.printf("  blit %lu us total, %lu us/flush\n", s.blit_us,
                  s.blit_us / s.flushes);
    Serial.printf("  i2c  %lu us total, %lu us/flush, %lu kB/s while active\n",
                  s.xfer_us, s.xfer_us / s.flushes,
                  s.xfer_us ? (bytes * 1000u) / s.xfer_us : 0);
    Serial.printf("  blit is %lu%% of flush cost, flushing is %lu%% of wall clock\n",
                  total_us ? (s.blit_us * 100u) / total_us : 0,
                  total_us / (REPORT_PERIOD_MS * 10u));

    mono_display::reset_stats();
}

}  // namespace

void setup() {
    Serial.begin(115200);
    Serial.println("Starting WiThRemote");

    display_ready = mono_display::begin();
    if(!display_ready) {
        Serial.println("SSD1306 not found on the I2C bus - check wiring/address");
        return;
    }

    mono_display::set_contrast(200);
    demo_ui::start();
    mono_display::reset_stats();
    last_report_ms = millis();
}

void loop() {
    if(!display_ready) {
        delay(1000);
        return;
    }

    uint32_t next_ms = lv_timer_handler();
    if(next_ms == LV_NO_TIMER_READY) next_ms = LV_DEF_REFR_PERIOD;

    if(millis() - last_report_ms >= REPORT_PERIOD_MS) {
        last_report_ms = millis();
        report_stats();
    }

    delay(next_ms);
}
