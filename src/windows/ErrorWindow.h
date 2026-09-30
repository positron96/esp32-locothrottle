#pragma once

#include "Window.h"

// Shown when WiFi or server connection fails. Waits a bit then retries from
// the WiFi connection step.
class ErrorWindow : public Window {
public:
    void onEnter() override;
    AppState update(AppState current) override;

private:
    void build();

    unsigned long enteredAtMs_ = 0;
    bool built_ = false;
    lv_obj_t* statusLabel_ = nullptr;
};
