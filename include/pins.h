#pragma once

#include <cstdint>

// Hardware pin assignments and peripheral addresses.

namespace pins {

// I2C bus shared by the display and the TCA9535 port expander.
constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;
constexpr uint32_t I2C_FREQUENCY = 400000;

// SSD1306 128x64 monochrome OLED, 0x3C is the usual address (0x3D on some modules).
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

}  // namespace pins
