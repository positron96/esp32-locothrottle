#pragma once

#include <lvgl.h>

#include "../AppState.h"

// Base class for the application's top-level "screens".
//
// Each window owns an LVGL screen object holding its (currently bare
// minimum) UI. No real display is attached yet (see main.cpp's no-op flush
// callback), so screens don't render anywhere visible yet; Serial is still
// used for status/logging. The rotary encoder used for menu navigation
// will be wired directly as an LVGL input device once the display exists,
// so it is not part of this class.
class Window {
public:
    virtual ~Window() = default;

    // Called once when the state machine switches to this window.
    virtual void onEnter() {}

    // Called once when the state machine switches away from this window.
    virtual void onExit() {}

    // Called every loop() iteration while this window is active.
    // Returns the state to transition to, or `current` to stay.
    virtual AppState update(AppState current) = 0;

protected:
    // Creates screen_ on first use and makes it the active LVGL screen.
    //
    // Screen creation is deliberately lazy: Window subclasses are
    // constructed as global statics before setup() runs lv_init(), so
    // LVGL objects can't be created in their constructors.
    void loadScreen();

    lv_obj_t* screen_ = nullptr;
};
