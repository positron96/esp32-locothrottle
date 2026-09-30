#include "ErrorWindow.h"

#include <Arduino.h>

namespace {
constexpr unsigned long RETRY_DELAY_MS = 5000;
}

void ErrorWindow::build() {
    statusLabel_ = lv_label_create(screen_);
    lv_obj_center(statusLabel_);
}

void ErrorWindow::onEnter() {
    loadScreen();
    if (!built_) {
        build();
        built_ = true;
    }

    lv_label_set_text(statusLabel_, "Connection failed\nretrying shortly...");
    Serial.println(F("[Error] Connection failed, retrying shortly..."));
    enteredAtMs_ = millis();
}

AppState ErrorWindow::update(AppState current) {
    if (millis() - enteredAtMs_ >= RETRY_DELAY_MS) {
        return AppState::WifiConnecting;
    }
    return current;
}
