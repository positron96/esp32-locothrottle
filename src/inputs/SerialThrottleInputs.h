#pragma once

#include "ThrottleInputs.h"

#include <WString.h>

// Temporary stand-in for the real buttons/potentiometers hardware.
// Reads simple text commands from Serial, one per line:
//   s<0-100>  set speed percentage, e.g. "s75"
//   d<f|n|r>  set direction: forward/neutral/reverse, e.g. "df"
//   t<0-7>    toggle a function, e.g. "t3"
class SerialThrottleInputs : public ThrottleInputs {
public:
    void begin() override;
    void update() override;
    bool hasChanged() const override;
    const ThrottleState& consumeState() override;

private:
    void processLine(const String& line);

    ThrottleState state_;
    bool changed_ = false;
};
