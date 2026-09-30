#pragma once

#include <cstddef>
#include <cstdint>

constexpr size_t THROTTLE_FUNCTION_COUNT = 8;

enum class ThrottleDirection : uint8_t {
    Reverse,
    Neutral,
    Forward,
};

struct ThrottleState {
    uint8_t speed = 0; // 0-100
    ThrottleDirection direction = ThrottleDirection::Neutral;
    bool functions[THROTTLE_FUNCTION_COUNT] = {};
};

// Abstraction over the physical speed/direction potentiometers and the
// function buttons+LEDs.
//
// The rotary encoder used for display/menu navigation is not part of this
// interface: it will be wired directly as an LVGL input device once the
// display is implemented.
class ThrottleInputs {
public:
    virtual ~ThrottleInputs() = default;

    virtual void begin() = 0;

    // Polls the underlying hardware/input source. Should be called every loop.
    virtual void update() = 0;

    // True if the state changed since the last consumeState() call.
    virtual bool hasChanged() const = 0;

    // Returns the current throttle state and clears the changed flag.
    virtual const ThrottleState& consumeState() = 0;
};
