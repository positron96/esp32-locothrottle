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

void MainControlWindow::exitClickedHandler(lv_event_t* e) {
    auto* self = static_cast<MainControlWindow*>(lv_event_get_user_data(e));
    self->quitRequested_ = true;
}

void MainControlWindow::build() {
    statusLabel_ = lv_label_create(screen_);
    lv_label_set_text(statusLabel_, "Main control");
    lv_obj_align(statusLabel_, LV_ALIGN_TOP_MID, 0, 5);

    // Only an exit button for now; throttle input UI (speed/direction/
    // functions) will be added once the WiThrottle protocol is wired up.
    exitButton_ = lv_button_create(screen_);
    lv_obj_align(exitButton_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(exitButton_, exitClickedHandler, LV_EVENT_CLICKED, this);

    lv_obj_t* exitLabel = lv_label_create(exitButton_);
    lv_label_set_text(exitLabel, "Exit");
    lv_obj_center(exitLabel);
}

void MainControlWindow::onEnter() {
    loadScreen();
    if (!built_) {
        build();
        built_ = true;
    }

    quitRequested_ = false;
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
            // Route through the same click event a real touch/encoder would
            // send, so the button's handler is the single source of truth.
            lv_obj_send_event(exitButton_, LV_EVENT_CLICKED, nullptr);
        }
    }

    if (quitRequested_) {
        Serial.println(F("[MainControl] Quitting to server discovery"));
        return AppState::ServerDiscovery;
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
