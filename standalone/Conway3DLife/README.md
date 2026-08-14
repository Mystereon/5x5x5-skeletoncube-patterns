# Standalone Conway-style 3-D Life

`Conway3DLife.ino` is a **single, self-contained FastLED sketch** for a 5×5×5, 125-pixel WS2812B cube. It does not include or depend on the SkeletonCube master gallery. Copy the entire `Conway3DLife` folder into your Arduino sketchbook, open `Conway3DLife.ino`, install FastLED, and upload.

## What it does

The sketch evolves a 5×5×5 cellular automaton using the compact 3-D rule **B5/S45**. A dead voxel is born with exactly five live neighbours; a live voxel survives with four or five. Each voxel has up to 26 neighbours. New cells display white, then age into bright emerald/cyan green. Empty or long-running worlds automatically reseed so the cube does not remain dark.

| Setting | Default | Change in sketch |
|---|---:|---|
| Cube dimensions | 5×5×5 | `N` |
| LED count | 125 | Derived from `N` |
| Data pin | GPIO2 | `DATA_PIN` |
| LED order | GRB | `COLOR_ORDER` |
| Brightness | 100 | `BRIGHTNESS` |
| Generation interval | 280 ms | `GENERATION_MS` |
| Rule | B5/S45 | `BIRTH_NEIGHBOURS`, `SURVIVE_MIN`, `SURVIVE_MAX` |

## Confirmed cube mapping

The default logical origin is **bottom–rear–left**, and rows are non-serpentine left-to-right:

```text
(0,0,0) = bottom, rear, left
x: left → right
y: rear → front
z: bottom → top
index = z * 25 + y * 5 + x
```

Set `SHOW_MAPPING_MARKERS = true` before first use if you want to verify the physical map. The cube should show red at bottom–rear–left, green at bottom–rear–right, blue at bottom–front–left, and white at top–rear–left.

## Optional ESP32-C3 SuperMini controls

The sketch is already configured for Dad’s wired buttons.

```text
GPIO4 ────[ SEED NEW WORLD ]── GND
GPIO8 ────[ PAUSE / RESUME ]── GND
```

GPIO4 reseeds the simulation immediately. GPIO8 pauses or resumes evolution; while paused, the centre voxel receives a subtle red indicator. Both buttons use `INPUT_PULLUP` and a 35 ms software debounce, so the switches are normally open and wire directly to ground.

> **GPIO8 warning:** GPIO8 is an ESP32-C3 strapping pin. Release its button while resetting or powering the board. If your SuperMini does not already pull GPIO8 high during reset, add a 10 kΩ pull-up from GPIO8 to 3V3. If that switch affects booting, uploading, or the onboard status LED, disconnect it and move pause/resume to a normal GPIO.[1] [2]

## Electrical note

Use a proper 5 V LED supply, share its ground with the controller, and do not rely on the SuperMini’s 3.3 V rail to power the cube. The sketch keeps FastLED’s conservative 5 V, 1.5 A software limit; adjust only after confirming the supply, cable, and connector ratings. FastLED’s common full-white planning rule is 60 mA per pixel, which would be 7.5 A for 125 pixels.[3]

## Project origin

> **FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon**

Created by Dad (MysterEon) & Manus, 2026. Released under the project’s MIT License.

## References

[1]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
[2]: https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html "Espressif ESP32-C3 boot-mode selection"
[3]: https://github.com/FastLED/FastLED/blob/master/cookbook/core-concepts/power.md "FastLED power considerations"
