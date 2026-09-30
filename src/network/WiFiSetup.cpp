#include "WiFiSetup.h"

#include <Arduino.h>
#include <WiFi.h>

void WiFiSetup::begin() {
    status_ = Status::Connecting;
    attemptStartMs_ = millis();

    WiFi.mode(WIFI_STA);
    WiFi.begin(); // (re)try last saved credentials, non-blocking
}

WiFiSetup::Status WiFiSetup::update() {
    switch (status_) {
        case Status::Connecting:
            if (WiFi.status() == WL_CONNECTED) {
                status_ = Status::Connected;
            } else if (millis() - attemptStartMs_ > SAVED_CREDENTIALS_TIMEOUT_MS) {
                // Saved credentials didn't work (or none exist): bring up the
                // config portal without blocking the rest of the app.
                wifiManager_.setConfigPortalBlocking(false);
                wifiManager_.startConfigPortal(apName_.c_str());
                status_ = Status::ConfigPortal;
            }
            break;

        case Status::ConfigPortal:
            wifiManager_.process();
            if (WiFi.status() == WL_CONNECTED) {
                wifiManager_.stopConfigPortal();
                status_ = Status::Connected;
            }
            break;

        case Status::Connected:
        case Status::Cancelled:
        case Status::Failed:
            break;
    }

    return status_;
}

void WiFiSetup::cancel() {
    if (status_ == Status::ConfigPortal) {
        wifiManager_.stopConfigPortal();
    }
    WiFi.disconnect(true);
    status_ = Status::Cancelled;
}
