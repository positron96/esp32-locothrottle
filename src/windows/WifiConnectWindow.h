#pragma once

#include "Window.h"
#include "../network/WiFiSetup.h"

// Connects to WiFi without blocking the rest of the app; the user can
// cancel an in-progress attempt (e.g. while the config portal is up) by
// typing 'c' over Serial. Rendered via Serial for now; will show status on
// the display once implemented.
class WifiConnectWindow : public Window {
public:
    explicit WifiConnectWindow(WiFiSetup& wifiSetup);

    void onEnter() override;
    AppState update(AppState current) override;

private:
    void build();

    WiFiSetup& wifiSetup_;
    bool portalAnnounced_ = false;
    bool built_ = false;
    lv_obj_t* statusLabel_ = nullptr;
};
