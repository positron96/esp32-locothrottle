#include "WifiConnectWindow.h"

#include <Arduino.h>

WifiConnectWindow::WifiConnectWindow(WiFiSetup& wifiSetup) : wifiSetup_(wifiSetup) {}

void WifiConnectWindow::onEnter() {
    portalAnnounced_ = false;
    Serial.println(F("[WiFi] Connecting... (type 'c' + Enter to cancel)"));
    wifiSetup_.begin();
}

AppState WifiConnectWindow::update(AppState current) {
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line == "c") {
            Serial.println(F("[WiFi] Cancelled by user"));
            wifiSetup_.cancel();
            return AppState::Error;
        }
    }

    WiFiSetup::Status status = wifiSetup_.update();
    switch (status) {
        case WiFiSetup::Status::Connected:
            Serial.println(F("[WiFi] Connected"));
            return AppState::ServerDiscovery;

        case WiFiSetup::Status::ConfigPortal:
            if (!portalAnnounced_) {
                Serial.printf("[WiFi] No saved network; connect to AP \"%s\" to configure (type 'c' to cancel)\n",
                              wifiSetup_.configPortalApName().c_str());
                portalAnnounced_ = true;
            }
            return current;

        case WiFiSetup::Status::Cancelled:
        case WiFiSetup::Status::Failed:
            return AppState::Error;

        case WiFiSetup::Status::Connecting:
        default:
            return current;
    }
}
