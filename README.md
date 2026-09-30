# WiThRemote - a ESP32-based model train controller

Works over WiFi, uses WiThrottle protocol, so will work with any WiThrottle server (e.g. JMRI, DCC-EX)

Status: work-in-progress

Hardware:

 * ESP32 lolin32 lite (with battery charger),
 * 8 buttons for function selection, 8 leds for function indication, connected over TCA9535 i2c expander,
 * potentiometer for speed (lateched into 5 or 7 positions mechanically, read as analog),
 * potentiometer for direction (latched into 3 positions, fwd/neutral/rev, read as analog),
 * display, either OLED/LCD (I2C/SPI) or IPS (SPI), to be used with lvgl and/or u8g2,
 * encoder for display navigation

Operation:

 - Upon poweron, connects to saved WiFi network or uses WiFiManager library to choose new network,
   status is shown on display
     - Potentially: allow to connect to wifi with display and buttons (with LVGL on-screen keyboard for password)
 - After connecting to WiFi, scans mDNS for WiThrottle servers, shows the list and allows user to choose.
   (similar to EngineDriver phone app)
      - Potentially: allow to enter IP address/port manually if mDNS fails (with LVGL on-screen keyboard)
 - Connects to WiThrottle server, acquires server info, rosters, etc.
 - Switches to main control screen:
    - Shown: Current loco chosen (with option to choose), speed/dir (functions indicated by LEDs)
    - Later: wiThrottle-based consist control (add/remove locos)
    - Acquiring several locos at once, only one is displayed, others can be switched to
    - Reading speed/direction/functions and sending to server
 - Switchable to turnout control screen (either with turnout address or from server list)

Implementation details:

- Hardware pin assignments and peripheral addresses are kept in pins.h.
- Speed/Direction/Functions hardware can be replaced with other input methods.
- ETLCPP library is used for containers, algorithms, and other utilities.
- LVGL is used for display management, even for monochrome OLED/LCD displays.
- The application uses an explicit state machine for WiFi setup, server discovery,
  server connection.
- Application tracks errors like disconnects from server or WiFi and does
  appropriate recovery with indications.
- Buttons are debounced, potentiometers are calibrated, and commands are sent
  only when values change significantly.
- ETL library is used for containers, algorithms etc.
