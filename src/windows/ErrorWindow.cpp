#include "ErrorWindow.h"

#include <Arduino.h>

namespace {
constexpr unsigned long RETRY_DELAY_MS = 5000;
}

void ErrorWindow::onEnter() {
    Serial.println(F("[Error] Connection failed, retrying shortly..."));
    enteredAtMs_ = millis();
}

AppState ErrorWindow::update(AppState current) {
    if (millis() - enteredAtMs_ >= RETRY_DELAY_MS) {
        return AppState::WifiConnecting;
    }
    return current;
}
