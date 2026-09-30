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
     - Potentially: allow to connect to wifi with display and buttons (needs on-screen keyboard for password)
 - After connecting to WiFi, scans mDNS for WiThrottle servers, shows the list and allows user to choose.
   (similar to EngineDriver phone app)
      - Potentially: allow to enter IP address/port manually if mDNS fails (needs on-screen keyboard)
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
  - LVGL renders in the I1 (1 bit per pixel) color format and is configured with the
    built-in mono theme, zero animation times and the pixel perfect `unscii` fonts;
    anti-aliased fonts such as Montserrat look muddy when thresholded to 1 bit.
  - U8g2 is the panel driver. `src/display/i1_blit.h` converts LVGL's row-packed I1
    render into U8g2 drawing calls, and only the 8x8 tiles that changed are pushed with
    `updateDisplayArea()`, which keeps the UI responsive over I2C. Any monochrome panel
    U8g2 supports can be used by changing the U8g2 class in
    `src/display/mono_display.cpp`.
  - Two blitters live in `i1_blit.h`. `blit_i1_hline()` is the one in use: it collapses
    runs of equal pixels into `drawHLine()` calls, so it stays on the public U8g2 API and
    works with any panel or rotation. `blit_i1_to_vlsb()` is about 4x faster because it
    writes the frame buffer directly, but only suits the `vertical_top_lsb` layout of
    SSD13xx and friends; it is currently unused and kept as a drop-in upgrade if blitting
    ever matters. The simulator runs both on every flush and fails if they disagree, so
    the unused one cannot silently rot.
  - `make -C tools/sim probe` answers layout questions against the real U8g2 library,
    linked on the host against a dummy panel. It verifies that LVGL's I1 rows are
    bit-for-bit compatible with `u8g2_DrawBitmap()` (both MSB first) but not with
    `u8g2_DrawXBM()` (LSB first, needs the bits in each byte reversed), and benchmarks
    the blitting options against each other. Of the public-API options, run-length
    `drawHLine()` is the fastest: `u8g2_DrawHVLine()` writes a pixel per loop iteration
    in both directions, but the horizontal loop just walks a pointer with a fixed mask,
    while the vertical one re-derives the bit position every pixel.
- The application uses an explicit state machine for WiFi setup, server discovery,
  server connection.
- Buttons are debounced, potentiometers are calibrated, and commands are sent
  only when values change significantly.
- A lost connection causes the controller to enter a safe state and issue a stop
  command when possible.

Building:

    pio run                 # build the firmware
    pio run -t upload       # flash it

The display code can also be exercised on a PC, without hardware: `tools/sim` builds the
real UI code against a host build of LVGL and prints the rendered screens as ASCII art.
It needs MSYS2 (gcc and make):

    $env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
    make -C tools/sim -j8 run
