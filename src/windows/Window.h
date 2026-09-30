#pragma once

#include "../AppState.h"

// Base class for the application's top-level "screens".
//
// For now these only print status to Serial. Later they will render with
// LVGL; the rotary encoder used for menu navigation will be wired directly
// as an LVGL input device at that point, so it is not part of this class.
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
};
