# ESP32-C3 SuperMini dual-button controller

This variant gives the cube two momentary controls while retaining the long automatic gallery.

| Function | Pin | Wiring | Behaviour |
|---|---:|---|---|
| **Next pattern** | GPIO3 | GPIO3 → button → GND | In manual mode, one stable press advances exactly one pattern. |
| **Auto/manual** | GPIO4 | GPIO4 → button → GND | One stable press toggles between the automatic gallery and manual selection. |

Both inputs use `INPUT_PULLUP`, so an external pull-up resistor is unnecessary. A 35 ms debounce interval ensures that one physical press registers only once.

```text
GPIO3 ────[ NEXT ]──── GND
GPIO4 ────[ AUTO / MANUAL ]──── GND
```

The sketch begins in automatic mode. Press the GPIO4 button once to freeze the currently selected effect in manual mode. Then use GPIO3 to step through effects. Press GPIO4 again to resume automatic cycling, beginning a fresh dwell interval on the current effect.

> **Why GPIO4 rather than GPIO9?** GPIO9 is an ESP32-C3 boot strapping pin. A normal-function GPIO4 button avoids any chance that a held switch changes the board’s boot-time configuration. Espressif identifies GPIO2, GPIO8, and GPIO9 as strapping pins, while GPIO12–GPIO17 are normally associated with SPI flash and GPIO18–GPIO19 with USB-JTAG.[1] [2]

## Use it

Clone or download the whole repository, then open:

```text
patterns/30_ESP32C3_DualButtonController/30_ESP32C3_DualButtonController.ino
```

Both source and reference sketch retain the project support message and Dad (MysterEon) & Manus credit.

## References

[1]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
[2]: https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html#strapping-pins "Espressif ESP32-C3 strapping-pin guidance"
