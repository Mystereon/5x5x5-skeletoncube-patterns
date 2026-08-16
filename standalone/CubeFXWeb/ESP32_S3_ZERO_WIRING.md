# CubeFX on the Waveshare ESP32-S3-Zero

> **Recommended CubeFX assignment for the photographed board:** LED data = **GPIO14**; Button 1 = **GPIO15**; Button 2 = **GPIO16**.

This guidance applies to the compact **Waveshare ESP32-S3-Zero** board, including the S3-Zero-M header version. It is not the ESP32-C3 SuperMini used by the default CubeFXWeb configuration. The S3-Zero has 4 MB flash, 2 MB PSRAM, a native USB-C connection, GPIO21 occupied by its onboard WS2812 indicator, and GPIO33–GPIO37 reserved for its Octal PSRAM. [1]

## Recommended wiring

| CubeFX function | ESP32-S3-Zero GPIO | Wiring | Why |
|---|---:|---|---|
| WS2812B cube DIN | **GPIO14** | GPIO14 → 330–470 Ω series resistor → level shifter → cube DIN | A normal bidirectional GPIO, clear of boot strapping, USB/JTAG, onboard RGB, UART labels, and S3-Zero PSRAM wiring. It is the upper pin of the convenient 14/15/16 group visible at the board's lower-right corner. |
| Button 1 / primary | **GPIO15** | Momentary switch from GPIO15 to GND | Normal GPIO using CubeFX's `INPUT_PULLUP`; short press is pattern-primary, long press opens Banner mode. |
| Button 2 / secondary | **GPIO16** | Momentary switch from GPIO16 to GND | Normal GPIO using CubeFX's `INPUT_PULLUP`; short press is pattern-secondary, long press advances to the next pattern and enters manual mode. |
| Cube ground | **GND** | Connect to the cube power supply ground | Required common reference for the data signal. |

Use a **74AHCT125**, **74HCT245**, or similar 5 V-tolerant logic level shifter between GPIO14 and a 5 V WS2812B cube. Do not power 125 LEDs from the S3-Zero board: Waveshare rates its onboard regulator at 800 mA, whereas the cube needs its own properly sized 5 V supply. [1]

## `CubeFXConfig.h` change

For an S3-Zero build, replace only the board-pin definitions in `CubeFXConfig.h`:

```cpp
// Waveshare ESP32-S3-Zero CubeFX wiring
#define CUBEFX_LED_DATA_PIN       14
#define CUBEFX_PRIMARY_BUTTON_PIN 15
#define CUBEFX_SECONDARY_BUTTON_PIN 16
```

Leave the cube geometry at 5 × 5 × 5 and `CUBEFX_TOTAL_LEDS` at 125 for the present cube. Select an **ESP32-S3** board in Arduino IDE; the S3-Zero’s native USB design may require holding **BOOT** (GPIO0) while connecting USB, then using RESET as described by Waveshare. [1]

## Pins intentionally avoided

| Pins | Reason |
|---|---|
| GPIO0, GPIO3, GPIO45, GPIO46 | ESP32-S3 strapping pins; GPIO0 controls download boot mode, so an external CubeFX switch or LED load can interfere with flashing. [2] [3] |
| GPIO19, GPIO20 | USB-JTAG pins by default; preserve them for reliable native USB debugging. [3] |
| GPIO21 | Used by the S3-Zero’s onboard WS2812 RGB LED. [1] |
| GPIO26–GPIO32 | Normally occupied by flash/PSRAM interfaces on ESP32-S3 designs. [3] |
| GPIO33–GPIO37 | Not broken out on the S3-Zero and reserved for its Octal PSRAM. [1] |
| GPIO43, GPIO44 | Board UART0 TX/RX labels; reserve them for serial diagnosis. [1] |

## References

[1]: https://docs.waveshare.com/ESP32-S3-Zero "Waveshare: ESP32-S3-Zero"

[2]: https://docs.espressif.com/projects/esptool/en/latest/esp32s3/advanced-topics/boot-mode-selection.html "Espressif: ESP32-S3 boot mode selection"

[3]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html "Espressif: ESP32-S3 GPIO"
