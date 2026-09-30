#pragma once

// Top level states of the application's explicit state machine.
enum class AppState {
    WifiConnecting,
    ServerDiscovery,
    ServerConnecting,
    MainControl,
    Error,
};
