#include "AppStateMachine.h"

AppStateMachine::AppStateMachine(AppState initialState, Window& wifiConnectWindow, Window& serverDiscoveryWindow,
                                  Window& serverConnectingWindow, Window& mainControlWindow, Window& errorWindow)
    : state_(initialState),
      wifiConnectWindow_(wifiConnectWindow),
      serverDiscoveryWindow_(serverDiscoveryWindow),
      serverConnectingWindow_(serverConnectingWindow),
      mainControlWindow_(mainControlWindow),
      errorWindow_(errorWindow) {}

Window& AppStateMachine::windowFor(AppState state) {
    switch (state) {
        case AppState::WifiConnecting:
            return wifiConnectWindow_;
        case AppState::ServerDiscovery:
            return serverDiscoveryWindow_;
        case AppState::ServerConnecting:
            return serverConnectingWindow_;
        case AppState::MainControl:
            return mainControlWindow_;
        case AppState::Error:
        default:
            return errorWindow_;
    }
}

void AppStateMachine::begin() {
    windowFor(state_).onEnter();
}

void AppStateMachine::update() {
    AppState next = windowFor(state_).update(state_);
    if (next != state_) {
        windowFor(state_).onExit();
        state_ = next;
        windowFor(state_).onEnter();
    }
}
