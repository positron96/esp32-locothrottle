#include "ServerConnectWindow.h"

#include <Arduino.h>

ServerConnectWindow::ServerConnectWindow(const DiscoveredServer& selectedServer, WiFiClient& serverLink)
    : selectedServer_(selectedServer), serverLink_(serverLink) {}

void ServerConnectWindow::onEnter() {
    Serial.printf("[Connect] Connecting to %s:%u...\n", selectedServer_.ip.toString().c_str(),
                  selectedServer_.port);

    // Bounded, short blocking check: this is a much shorter, deterministic
    // wait than the WiFi connection step, so it isn't worth the complexity
    // of a fully async socket connect here.
    serverLink_.stop();
    connected_ = serverLink_.connect(selectedServer_.ip, selectedServer_.port, CONNECT_TIMEOUT_MS);

    Serial.println(connected_ ? F("[Connect] Reachable (protocol not implemented yet)")
                               : F("[Connect] Failed to connect"));
}

AppState ServerConnectWindow::update(AppState current) {
    if (connected_) {
        return AppState::MainControl;
    }
    return AppState::Error;
}
