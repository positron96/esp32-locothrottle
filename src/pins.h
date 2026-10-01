#pragma once

#include <cstdint>

// Hardware pin assignments and peripheral addresses live here, per the
// project convention described in the README.
//
// Encoder wiring: A, B and the push switch connect between the GPIO and GND.
// All three inputs use the ESP32's internal pull-ups.
constexpr uint8_t ENCODER_A_PIN = 32;
constexpr uint8_t ENCODER_B_PIN = 33;
constexpr uint8_t ENCODER_BUTTON_PIN = 25;

// On the LOLIN32, generally usable GPIOs include 4, 13, 14, 16-19, 21-23,
// 25-27 and 32-33. GPIOs 34-39 are input-only and have no internal pull-ups.
// Avoid GPIOs 6-11 because they are connected to flash, GPIOs 1 and 3 when
// using the USB serial port, and boot-strapping GPIOs 0, 2, 5, 12 and 15
// unless their boot-time levels are handled by the circuit.
