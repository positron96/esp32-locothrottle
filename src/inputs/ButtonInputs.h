
// Default I2C address for the TCA9535 (with pins A0, A1, A2 tied to GND)
#define TCA9535_ADDR 0x20

// TCA9535 Register Commands
#define REG_OUTPUT_P0  0x02  // Output Port 0 register
#define REG_CONFIG_P0  0x06  // Configuration Port 0 register (0 = Output, 1 = Input)

// Helper function to send the 16-bit state to both ports
void writePins(uint16_t state) {
  Wire.beginTransmission(TCA9535_ADDR);
  Wire.write(REG_OUTPUT_P0);
  Wire.write(state & 0xFF);         // Lower 8 bits to Port 0
  Wire.write((state >> 8) & 0xFF);  // Upper 8 bits to Port 1
  Wire.endTransmission();
}

void setup() {
  Wire.begin(); // Initialize I2C bus

  // Initialize all outputs to LOW first
  writePins(0x0000);

  // Configure all 16 pins as OUTPUTS
  Wire.beginTransmission(TCA9535_ADDR);
  Wire.write(REG_CONFIG_P0);
  Wire.write(0x00); // Port 0: Set all 8 pins as OUTPUT
  Wire.write(0x00); // Port 1: Set all 8 pins as OUTPUT
  Wire.endTransmission();
}

void loop() {
  // Cycle through each of the 16 pins (0 to 15)
  for (int pin = 4; pin < 12; pin++) {
    // Shift a '1' into the position of the current active pin
    uint16_t pinState = (1 << pin);

    writePins(pinState); // Turn only this pin HIGH
    delay(100);          // Wait for 200ms
  }
}
