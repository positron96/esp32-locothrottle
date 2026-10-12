#pragma once

#include <cstdint>
#include <Arduino.h>
#include <Wire.h>

#include <etl/optional.h>

#include "../pins.h"

class Keypad {
public:
    explicit Keypad(uint8_t interruptPin, TwoWire& wire = Wire,
                    uint8_t address = KEYPAD_I2C_ADDRESS)
        : wire(wire), address(address), isrPin(interruptPin) {}

    Keypad(const Keypad&) = delete;
    Keypad& operator=(const Keypad&) = delete;

    bool begin() {
        initialized = false;
        if (!writeRegister(REG_OUTPUT_P0, 0x0000)
            || !writeRegister(REG_POLARITY_P0, 0x0000)
            || !writeRegister(REG_CONFIG_P0, 0xF00F)) {
            return false;
        }
        ledsVal = 0;
        pinMode(isrPin, INPUT_PULLUP);
        changePending = true; // read actual values at next readButtons call;
        attachInterruptArg(isrPin, interruptHandler, this, FALLING);
        initialized = true;
        return true;
    }

    bool buttonInterruptPending() const {
        return changePending;
    }

    bool setLeds(uint8_t leds) {
        if (!initialized) {
            return false;
        }
        uint16_t outputs = 0;
        for (uint8_t led = 0; led < 8; ++led) {
            if (bitRead(leds, led) != 0) {
                outputs |= uint16_t{1} << (11 - led);
            }
        }
        if (!writeRegister(REG_OUTPUT_P0, outputs)) {
            return false;
        }
        ledsVal = leds;
        return true;
    }

    bool setLed(uint8_t ledNum, bool on) {
        if (ledNum >= 8) {
            return false;
        }
        const uint8_t mask = bit(ledNum);
        return setLeds(on ? (ledsVal | mask) : (ledsVal & ~mask));
    }

    etl::optional<uint8_t> readButtons() {
        if (!initialized) {
            return etl::nullopt;
        }
        if(!changePending) {
            return buttonsVal;
        }
        wire.beginTransmission(address);
        wire.write(REG_INPUT_P0);
        if (wire.endTransmission(false) != 0) {
            return etl::nullopt;
        }
        if (wire.requestFrom(address, 2) != 2 || wire.available() < 2) {
            return etl::nullopt;
        }
        const int port0 = wire.read();
        const int port1 = wire.read();
        if (port0 < 0 || port1 < 0) {
            return false;
        }
        const uint16_t inputs = static_cast<uint16_t>(port0)
            | (static_cast<uint16_t>(port1) << 8);
        buttonsVal = 0;
        for (uint8_t button = 0; button < 8; ++button) {
            const uint8_t pin = button < 4 ? (12 + button) : (7 - button);
            if (bitRead(inputs, pin) == 0) {
                buttonsVal |= bit(button);
            }
        }
        changePending = false;
        return buttonsVal;
    }

private:
    static constexpr uint8_t REG_INPUT_P0 = 0x00;
    static constexpr uint8_t REG_OUTPUT_P0 = 0x02;
    static constexpr uint8_t REG_POLARITY_P0 = 0x04;
    static constexpr uint8_t REG_CONFIG_P0 = 0x06;

    static void IRAM_ATTR interruptHandler(void* context) {
        static_cast<Keypad*>(context)->changePending = true;
    }

    bool writeRegister(uint8_t reg, uint16_t value) {
        wire.beginTransmission(address);
        wire.write(reg);
        wire.write(static_cast<uint8_t>(value));
        wire.write(static_cast<uint8_t>(value >> 8));
        return wire.endTransmission() == 0;
    }

    TwoWire& wire;
    uint8_t address;
    uint8_t isrPin;
    uint8_t ledsVal = 0;
    uint8_t buttonsVal = 0;
    bool initialized = false;
    volatile bool changePending = false;
};
