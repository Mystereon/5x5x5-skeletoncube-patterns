# ESP32-C3 SuperMini dual-button controller

This variant gives the cube two momentary controls while retaining the long automatic gallery. It is configured for the existing **GPIO4 and GPIO8** wiring.

| Function | Pin | Wiring | Behaviour |
|---|---:|---|---|
| **Next pattern** | GPIO4 | GPIO4 → button → GND | In manual mode, one stable press advances exactly one pattern. |
| **Auto/manual** | GPIO8 | GPIO8 → button → GND | One stable press toggles between the automatic gallery and manual selection. |

The sketch begins in automatic mode. Press **GPIO8** once to freeze the currently selected effect in manual mode. Then use **GPIO4** to step through effects. Press **GPIO8** again to resume automatic cycling, beginning a fresh dwell interval on the current effect. A 35 ms debounce interval ensures that one physical press registers only once.

```text
GPIO4 ────[ NEXT ]─────────── GND
GPIO8 ────[ AUTO / MANUAL ]── GND
```

GPIO4 uses `INPUT_PULLUP`, so it needs no external pull-up resistor. GPIO8 also becomes an `INPUT_PULLUP` after boot, but it is an ESP32-C3 **strapping pin**. For safe operation:

1. Do **not** press or hold the GPIO8 button while resetting or powering the SuperMini.
2. Use a normally-open momentary button, wired only to GND.
3. Add a **10 kΩ resistor from GPIO8 to 3V3** if your board does not already hold GPIO8 high at reset. This helps maintain a valid boot state before software has configured its internal pull-up.
4. If the board fails to boot or upload intermittently, disconnect the GPIO8 switch first; replace it with a normal GPIO when practical.
5. Some SuperMini variants also connect GPIO8 to an onboard status LED. If the mode switch appears to trigger by itself or the LED behaves oddly, move that function to a normal GPIO.

> **Why retain GPIO8?** You already have it wired. It can function as a mode input after the ESP32-C3 has booted; the caveat is solely its reset-time strapping role. Espressif identifies GPIO2, GPIO8, and GPIO9 as strapping pins, and documents the boot-mode relationship of GPIO8 and GPIO9.[1] [2]

## Use it

Clone or download the whole repository, then open:

```text
patterns/30_ESP32C3_DualButtonController/30_ESP32C3_DualButtonController.ino
```

Both source and reference sketch retain the project support message and Dad (MysterEon) & Manus credit.

## References

[1]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
[2]: https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html "Espressif ESP32-C3 boot-mode selection"
