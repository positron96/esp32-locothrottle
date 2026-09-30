#pragma once

#include <WiFiClient.h>

#include "Window.h"
#include "../inputs/ThrottleInputs.h"

// Placeholder main control screen.
//
// Reads throttle inputs (speed/direction/functions) and prints what would be
// sent to the WiThrottle server, once that protocol is implemented. The
// user can quit back to server discovery (typing 'q' + Enter), which
// properly finalizes/closes the current server connection first.
//
// Also watches for WiFi and server disconnects: since the WiThrottle
// protocol itself isn't implemented yet, serverLink (a plain TCP socket
// opened by ServerConnectWindow) is used as a stand-in "is the server still
// there" heartbeat until the real protocol's keep-alive/heartbeat replaces
// it.
class MainControlWindow : public Window {
public:
    MainControlWindow(ThrottleInputs& inputs, WiFiClient& serverLink);

    void onEnter() override;
    void onExit() override;
    AppState update(AppState current) override;

private:
    void build();
    static void exitClickedHandler(lv_event_t* e);

    ThrottleInputs& inputs_;
    WiFiClient& serverLink_;
    bool built_ = false;
    bool quitRequested_ = false;
    lv_obj_t* statusLabel_ = nullptr;
    lv_obj_t* exitButton_ = nullptr;
};
