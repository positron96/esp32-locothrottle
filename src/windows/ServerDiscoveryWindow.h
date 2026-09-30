#pragma once

#include "Window.h"
#include "../network/ServerDiscovery.h"

// Scans for WiThrottle servers via mDNS and lets the user pick one.
//
// The scan itself is non-blocking (polled via ServerDiscovery::update()) and
// cancellable by typing 'c' over Serial. Selection is done by typing the
// server's list index for now; once the display and encoder are wired up,
// this will become an on-screen list navigated with the encoder.
class ServerDiscoveryWindow : public Window {
public:
    ServerDiscoveryWindow(ServerDiscovery& discovery, DiscoveredServer& selectedServer);

    void onEnter() override;
    AppState update(AppState current) override;

private:
    void printResults();

    ServerDiscovery& discovery_;
    DiscoveredServer& selectedServer_;
    DiscoveredServerList servers_;
    bool resultsPrinted_ = false;
    bool hasSelection_ = false;
};
