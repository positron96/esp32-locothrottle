#pragma once

#include <cstdint>

#include <WiFiClient.h>

#include "Window.h"
#include "../network/ServerDiscovery.h"

// Opens a TCP connection to the selected WiThrottle server and keeps it
// open in serverLink, so MainControlWindow can poll it for disconnects.
//
// This only manages the connection; it does not speak the WiThrottle
// protocol itself (handshake, roster, etc.), which will be added separately.
class ServerConnectWindow : public Window {
public:
    ServerConnectWindow(const DiscoveredServer& selectedServer, WiFiClient& serverLink);

    void onEnter() override;
    AppState update(AppState current) override;

private:
    static constexpr int32_t CONNECT_TIMEOUT_MS = 5000;

    const DiscoveredServer& selectedServer_;
    WiFiClient& serverLink_;
    bool connected_ = false;
};
