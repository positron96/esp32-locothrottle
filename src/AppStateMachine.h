#pragma once

#include "AppState.h"
#include "windows/Window.h"

// Explicit state machine driving the application's top-level screens
// (WiFi setup, server discovery, server connection, main control).
class AppStateMachine {
public:
    AppStateMachine(AppState initialState, Window& wifiConnectWindow, Window& serverDiscoveryWindow,
                    Window& serverConnectingWindow, Window& mainControlWindow, Window& errorWindow);

    // Enters the initial state's window. Call once from setup().
    void begin();

    // Advances the current window and handles state transitions.
    // Call once per loop() iteration.
    void update();

private:
    Window& windowFor(AppState state);

    AppState state_;
    Window& wifiConnectWindow_;
    Window& serverDiscoveryWindow_;
    Window& serverConnectingWindow_;
    Window& mainControlWindow_;
    Window& errorWindow_;
};
