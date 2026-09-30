#include "SerialThrottleInputs.h"

#include <Arduino.h>

void SerialThrottleInputs::begin() {
    Serial.println(F("[ThrottleInputs] Commands: s<0-100> speed, d<f|n|r> direction, t<0-7> toggle function"));
}

void SerialThrottleInputs::update() {
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
            continue;
        }
        processLine(line);
    }
}

bool SerialThrottleInputs::hasChanged() const {
    return changed_;
}

const ThrottleState& SerialThrottleInputs::consumeState() {
    changed_ = false;
    return state_;
}

void SerialThrottleInputs::processLine(const String& line) {
    char command = line[0];
    switch (command) {
        case 's': {
            int speed = constrain(line.substring(1).toInt(), 0, 100);
            if (state_.speed != speed) {
                state_.speed = static_cast<uint8_t>(speed);
                changed_ = true;
            }
            break;
        }
        case 'd': {
            String arg = line.substring(1);
            arg.trim();
            ThrottleDirection direction;
            if (arg == "f") {
                direction = ThrottleDirection::Forward;
            } else if (arg == "n") {
                direction = ThrottleDirection::Neutral;
            } else if (arg == "r") {
                direction = ThrottleDirection::Reverse;
            } else {
                Serial.println(F("[ThrottleInputs] Unknown direction, use f, n or r"));
                return;
            }
            if (state_.direction != direction) {
                state_.direction = direction;
                changed_ = true;
            }
            break;
        }
        case 't': {
            int index = line.substring(1).toInt();
            if (index < 0 || index >= static_cast<int>(THROTTLE_FUNCTION_COUNT)) {
                Serial.println(F("[ThrottleInputs] Function index out of range"));
                return;
            }
            state_.functions[index] = !state_.functions[index];
            changed_ = true;
            break;
        }
        default:
            Serial.println(F("[ThrottleInputs] Unknown command"));
            break;
    }
}
