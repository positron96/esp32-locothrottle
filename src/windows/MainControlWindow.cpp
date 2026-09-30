#include "MainControlWindow.h"

#include <Arduino.h>
#include <WiFi.h>

namespace {

const char* directionName(ThrottleDirection direction) {
    switch (direction) {
        case ThrottleDirection::Forward:
            return "forward";
        case ThrottleDirection::Reverse:
            return "reverse";
        case ThrottleDirection::Neutral:
        default:
            return "neutral";
    }
}

} // namespace

MainControlWindow::MainControlWindow(ThrottleInputs& inputs, WiFiClient& serverLink)
    : inputs_(inputs), serverLink_(serverLink) {}

void MainControlWindow::onEnter() {
    Serial.println(F("[MainControl] Ready (type 'q' + Enter to quit to server discovery)"));
    inputs_.begin();
}

void MainControlWindow::onExit() {
    // Finalize the connection properly, whether we're leaving because the
    // user quit or because WiFi/server was lost.
    serverLink_.stop();
    Serial.println(F("[MainControl] Connection closed"));
}

AppState MainControlWindow::update(AppState current) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[MainControl] WiFi disconnected"));
        return AppState::Error;
    }

    if (!serverLink_.connected()) {
        Serial.println(F("[MainControl] Server connection lost"));
        return AppState::Error;
    }

    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line == "q") {
            Serial.println(F("[MainControl] Quitting to server discovery"));
            return AppState::ServerDiscovery;
        }
    }

    inputs_.update();

    if (inputs_.hasChanged()) {
        const ThrottleState& state = inputs_.consumeState();
        Serial.printf("[MainControl] speed=%u dir=%s functions=", state.speed, directionName(state.direction));
        for (size_t i = 0; i < THROTTLE_FUNCTION_COUNT; ++i) {
            Serial.printf("%d", state.functions[i] ? 1 : 0);
        }
        Serial.println();
    }

    return current;
}
