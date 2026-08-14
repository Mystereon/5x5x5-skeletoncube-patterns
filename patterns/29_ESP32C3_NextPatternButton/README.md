# ESP32-C3 SuperMini next-pattern button

This is a dedicated **manual pattern-selector** version of SkeletonCube Patterns for an ESP32-C3 SuperMini. Automatic cycling is disabled. Each stable press advances to the next effect and holds it there until the next press.

## Recommended pin and wiring

Use **GPIO3** for the momentary switch.

```text
ESP32-C3 SuperMini GPIO3 ────[ momentary push button ]──── GND
```

No external pull-up resistor is needed: the sketch configures GPIO3 as `INPUT_PULLUP`, which means the input is normally `HIGH` and reads `LOW` when pressed. A software debounce interval of 35 ms prevents one physical press from advancing through several patterns.

| Pin family | Recommendation | Reason |
|---|---|---|
| **GPIO3** | **Use for the button** | Ordinary GPIO, supports input and internal pull-up. |
| GPIO2, GPIO8, GPIO9 | Avoid for this button | ESP32-C3 boot-strapping pins; their state is sampled at reset. |
| GPIO18, GPIO19 | Avoid when USB-JTAG matters | Used by USB-JTAG by default. |
| GPIO12–GPIO17 | Avoid | Usually connected to SPI flash. |

ESP32-C3 documentation lists GPIO2, GPIO8, and GPIO9 as strapping pins; it also notes that GPIO12–GPIO17 are normally used by SPI flash and GPIO18–GPIO19 by USB-JTAG.[1] [2]

## Use it

Clone or download the complete repository, preserving the folder layout. Open:

```text
patterns/29_ESP32C3_NextPatternButton/29_ESP32C3_NextPatternButton.ino
```

The wrapper sets:

```cpp
#define SKELETONCUBE_AUTO_CYCLE 0
#define SKELETONCUBE_PATTERN_BUTTON 1
```

The master sketch starts at the red vector cube. Each button press advances one entry through the complete pattern list and wraps back to the first pattern after the last.

## References

[1]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
[2]: https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html#strapping-pins "Espressif ESP32-C3 strapping-pin guidance"
