#include <Arduino.h>
#include <WiFiClient.h>
#include <lvgl.h>

#include "AppState.h"
#include "AppStateMachine.h"
#include "inputs/SerialThrottleInputs.h"
#include "network/ServerDiscovery.h"
#include "network/WiFiSetup.h"
#include "windows/ErrorWindow.h"
#include "windows/MainControlWindow.h"
#include "windows/ServerConnectWindow.h"
#include "windows/ServerDiscoveryWindow.h"
#include "windows/WifiConnectWindow.h"

namespace {

// No physical display is wired up yet, so LVGL just renders into an
// in-memory buffer that nothing ever reads; this flush callback discards
// the pixels and immediately reports the buffer as consumed.
void lvglFlushNoop(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    (void)area;
    (void)px_map;
    lv_display_flush_ready(disp);
}

constexpr uint32_t DISPLAY_HOR_RES = 128;
constexpr uint32_t DISPLAY_VER_RES = 64;

void lvglInit() {
    lv_init();

    static lv_color_t drawBuffer[DISPLAY_HOR_RES * 10];
    lv_display_t* display = lv_display_create(DISPLAY_HOR_RES, DISPLAY_VER_RES);
    lv_display_set_buffers(display, drawBuffer, nullptr, sizeof(drawBuffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, lvglFlushNoop);
}

unsigned long lastTickMs = 0;

void lvglTick() {
    unsigned long now = millis();
    lv_tick_inc(now - lastTickMs);
    lastTickMs = now;
    lv_timer_handler();
}

WiFiSetup wifiSetup;
ServerDiscovery serverDiscovery;
DiscoveredServer selectedServer;
SerialThrottleInputs throttleInputs;
WiFiClient serverLink;

WifiConnectWindow wifiConnectWindow(wifiSetup);
ServerDiscoveryWindow serverDiscoveryWindow(serverDiscovery, selectedServer);
ServerConnectWindow serverConnectWindow(selectedServer, serverLink);
MainControlWindow mainControlWindow(throttleInputs, serverLink);
ErrorWindow errorWindow;

AppStateMachine stateMachine(AppState::WifiConnecting, wifiConnectWindow, serverDiscoveryWindow, serverConnectWindow,
                             mainControlWindow, errorWindow);

} // namespace

void setup() {
    Serial.begin(115200);
    Serial.println("Starting WiThRemote");

    lvglInit();
    lastTickMs = millis();

    stateMachine.begin();
}

void loop() {
    lvglTick();
    stateMachine.update();
}
