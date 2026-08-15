# CubeFX ESP32-C3 pin configuration

`CubeFXConfig.h` is the single place to configure CubeFX wiring before an Arduino upload. The **LED data pin is compile-time** because FastLED binds the WS2812B output during initialisation; changing it in the Android app cannot move an already-running LED output. Select the desired data pin in the phone app as a configuration record, edit the matching `CUBEFX_LED_DATA_PIN` value in `CubeFXConfig.h`, and then compile/upload the sketch.

The same header explicitly defines **columns**, **rows**, and **layers**. Its `CUBEFX_TOTAL_LEDS` macro always calculates the physical count as `columns × rows × layers`; for the supplied skeletal cube this is `5 × 5 × 5 = 125`. The Android setup screen uses the exact same multiplication so the displayed LED total and generated header agree. The current CubeFXWeb visual effects deliberately enforce 5×5×5, because their 3-D geometry, fonts, and layer maps are designed for that volume. A non-5×5×5 configuration is recorded and calculated safely, but requires a future generic renderer before it can be compiled as CubeFXWeb.

The two momentary button pins are different. The initial defaults are loaded from the same header, and a connected CubeFX Android controller can replace the **primary** and **secondary** button pins through BLE. The firmware stores those button choices in ESP32 preferences and restores them on the next boot.

| Signal | Default | Safe practical choices on an ESP32-C3 SuperMini | Important restriction |
|---|---:|---|---|
| WS2812B data | GPIO2 | GPIO0, GPIO1, GPIO3–GPIO7, GPIO10, GPIO11, GPIO20, GPIO21 | Requires a recompile/reflash. GPIO2 is a strapping pin, so the LED input must not pull it low during reset. |
| Primary button | GPIO4 | GPIO0, GPIO1, GPIO3–GPIO7, GPIO10, GPIO11, GPIO20, GPIO21 | Input-pull-up button to GND; changing it through BLE persists. |
| Secondary button | GPIO8 | GPIO0, GPIO1, GPIO3–GPIO7, GPIO10, GPIO11, GPIO20, GPIO21 | GPIO8 is a strapping pin: keep its button released at reset and add a 10 kΩ pull-up where required. |

GPIO2, GPIO8, and GPIO9 are ESP32-C3 strapping pins. GPIO12–GPIO17 are normally used by SPI flash, while GPIO18–GPIO19 are USB-JTAG pins. The Android setup screen labels all of these choices rather than silently allowing a risky configuration. [1]

## References

[1]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
