# SkeletonCube Patterns

> A self-playing **5×5×5 WS2812B / NeoPixel skeletal-frame cube** pattern library for Arduino and FastLED.

**Created by Dad (MysterEon) & Manus, 2026.** It began with a physical 125-pixel cube and a simple question—how do you make voxel shapes tumble? It is released as a public starting point for anyone building, extending, or remixing small LED cubes.

![Platform](https://img.shields.io/badge/platform-Arduino-00979D) ![LEDs](https://img.shields.io/badge/LEDs-125%20WS2812B-brightgreen) ![Patterns](https://img.shields.io/badge/patterns-28-purple) ![License](https://img.shields.io/badge/license-MIT-blue)

## The pattern gallery

The master sketch contains a sequence of **28 effects**. Each receives a deliberately long, pattern-specific dwell time; the shortest is 20 seconds, and simulations such as Pong, Tetris, Snake, and Life stay visible longer so they read as scenes rather than brief flashes.

| # | Pattern | Dwell | Description |
|---:|---|---:|---|
| 01 | Red vector cube | 24 s | Rotating red **3×3×3 wireframe** cube. |
| 02 | Solid cube | 22 s | Tumbling analytic voxel cube. |
| 03 | Sphere | 22 s | Tumbling analytic voxel sphere. |
| 04 | Octahedron | 22 s | Tumbling analytic diamond. |
| 05 | Voxel model | 26 s | Hand-authored rotating voxel object. |
| 06 | Bouncing block | 24 s | 2×2×2 cube independently bouncing on all axes. |
| 07 | Blue rain | 26 s | Sparse falling blue drops. |
| 08 | Column sweep | 20 s | Rainbow vertical x-column sweep. |
| 09 | Matrix rain | 36 s | Dense green 3-D streams with bright heads and trails. |
| 10 | Corner cubes | 26 s | Eight 2×2×2 colour-shifting corner cubes. |
| 11 | White glitter | 22 s | Fast-fading random white voxel sparkle. |
| 12 | Pong | 40 s | Self-playing 3-D Pong with two tracking paddles. |
| 13 | 3-D Tetris | 50 s | Falling, settling four-voxel polycubes. |
| 14 | Blinking eye | 26 s | Volumetric eye with a moving pupil and periodic blink. |
| 15 | DNA helix | 28 s | Twin-colour 3-D helix. |
| 16 | Meteors | 26 s | Wrapped diagonal meteor stream. |
| 17 | Snake | 42 s | Autonomous wrapping 3-D snake. |
| 18 | Space Invaders | 36 s | Marching 3-D invader scene and moving laser. |
| 19 | Conway-style Life | 46 s | 26-neighbour cellular automaton in a 5×5×5 volume. |
| 20 | Clouds | 32 s | Slow volumetric noise clouds. |
| 21 | Neon plasma | 30 s | Saturated trigonometric 3-D plasma field. |
| 22 | Fire | 32 s | Rising volumetric noise-driven fire. |
| 23 | Hourglass | 42 s | Tapered cyan glass and animated gold sand. |
| 24 | Pulse cube | 26 s | Reducing and expanding ping-pong wire cube. |
| 25 | Upward red rain | 26 s | Dense rising red streams with fast fade-out. |
| 26 | Spirals | 34 s | Twin animated 3-D spirals. |
| 27 | Point bouncer | 24 s | A single white voxel bouncing in all dimensions. |
| 28 | Wrapping comets | 32 s | Three multicolour comets wrapping through the cube. |

## Repository layout

```text
SkeletonCubePatterns/                 # The self-playing master Arduino sketch
  SkeletonCubePatterns.ino
  CubeTypes.h
patterns/                             # 28 fixed-pattern sketches plus an ESP32-C3 controller
  01_RedVectorCube/
  02_SolidCube/
  ...
  28_WrappingComets/
  29_ESP32C3_NextPatternButton/
  30_ESP32C3_DualButtonController/
tools/generate_pattern_examples.sh    # Rebuilds the reference-wrapper folders
```

Every folder under [`patterns/`](patterns/) contains an uploadable `.ino` file and a short README. These reference sketches intentionally share the master implementation rather than carrying 28 divergent copies: each one disables auto-cycling, selects exactly one pattern, and includes the common code. **Clone or download the entire repository before opening a reference sketch**, so its relative include path remains valid.

## Hardware assumptions

This project is configured for the cube that inspired it.

| Setting | Default | Change it here |
|---|---:|---|
| LEDs | 125 × WS2812B | `NUM_LEDS` is derived from `N = 5`. |
| Colour order | GRB | `#define COLOR_ORDER GRB` |
| Data pin | Arduino pin 2 | `#define DATA_PIN 2` |
| FastLED brightness | 100 | `BRIGHTNESS` |
| Logical origin | Bottom–rear–left | Documented in the source header. |
| Row order | Left → right | `SERPENTINE_ROWS = false` |
| Frame delay | None | The project uses `millis()` timers instead. |

The intended logical coordinate system is:

```text
(0,0,0) = bottom, rear, left
x: left → right
y: rear → front
z: bottom → top
```

With non-serpentine rows, bottom-to-top layers, and rear-to-front rows, the data index is:

```cpp
index = z * 25 + y * 5 + x;
```

## Quick start

Install the [FastLED library](https://fastled.io/docs/) in Arduino IDE. Then clone or download this repository, open:

```text
SkeletonCubePatterns/SkeletonCubePatterns.ino
```

and upload it. Keep `CubeTypes.h` in the **same Arduino sketch folder** as the `.ino` file; Arduino needs this header before it generates function prototypes.

Automatic cycling is on by default. The `dwellForPattern()` function contains the per-pattern durations. To hold on one pattern while experimenting, set the first line below to `false`, then select an item from `CubeTypes.h` in the second:

```cpp
const bool AUTO_CYCLE_PATTERNS = false;
const Pattern FIXED_PATTERN = PATTERN_MATRIX_RAIN;
```

## Calibrate the physical map first

Set the following switch to `true` before experimenting with complex patterns:

```cpp
const bool SHOW_MAPPING_MARKERS = true;
```

| Physical point | Expected colour |
|---|---|
| Bottom–rear–left `(0,0,0)` | Red |
| Bottom–rear–right `(4,0,0)` | Green |
| Bottom–front–left `(0,4,0)` | Blue |
| Top–rear–left `(0,0,4)` | White |

If the locations are wrong, correct only the map flags near the top of the sketch: `SWAP_XY`, `FLIP_X`, `FLIP_Y`, `FLIP_Z`, `SERPENTINE_ROWS`, and `SERPENTINE_LAYERS`. When correct, restore `SHOW_MAPPING_MARKERS = false`.

## Add a pattern

A new effect needs three small changes:

1. Add a name to `enum Pattern` in `CubeTypes.h`, before `PATTERN_COUNT`.
2. Write a `renderYourPattern()` function that calls `setVoxel()` or `addVoxel()`.
3. Add its dwell time and `switch (pattern)` case in the master sketch.

A minimal static starting point is:

```cpp
void renderCentreMarker() {
  setVoxel(2, 2, 2, CRGB::Purple);
}
```

Use `millis()` timestamps rather than `delay()` for moving effects. This keeps the pattern gallery fluid, allows trails where wanted, and makes an effect easy to use both in the cycle and as a reference sketch.

## ESP32-C3 SuperMini button controllers

For a physical one-button version on an **ESP32-C3 SuperMini**, use **GPIO3** and wire the momentary switch directly between GPIO3 and GND:

```text
GPIO3 ────[ momentary push button ]──── GND
```

Open [`patterns/29_ESP32C3_NextPatternButton/29_ESP32C3_NextPatternButton.ino`](patterns/29_ESP32C3_NextPatternButton/29_ESP32C3_NextPatternButton.ino). This variant disables automatic cycling and advances one pattern on each debounced press. It uses the internal pull-up, so no external resistor is needed. The full wiring and pin-selection rationale are in its [reference note](patterns/29_ESP32C3_NextPatternButton/README.md).

GPIO3 is a normal GPIO on the ESP32-C3. Avoid GPIO2, GPIO8, and GPIO9 for this switch because they are boot-strapping pins; avoid GPIO12–GPIO17, which are normally used by SPI flash; and leave GPIO18–GPIO19 available if you rely on USB-JTAG.[2] [3]

### Dual-button auto/manual controller

Use the second controller when you want both long automatic playback and hands-on browsing:

```text
GPIO4 ────[ NEXT ]─────────── GND
GPIO8 ────[ AUTO / MANUAL ]── GND
```

Open [`patterns/30_ESP32C3_DualButtonController/30_ESP32C3_DualButtonController.ino`](patterns/30_ESP32C3_DualButtonController/30_ESP32C3_DualButtonController.ino). The cube starts in **auto** mode. Press GPIO8 to pause the current pattern and enter **manual** mode; then press GPIO4 to advance one pattern at a time. Press GPIO8 again to restart **auto** playback from the pattern currently on display.

GPIO8 is an ESP32-C3 boot strapping pin. The mode switch is usable after startup but it must be **released** at reset and power-up. If the SuperMini board does not already keep GPIO8 high at boot, add a 10 kΩ pull-up resistor from GPIO8 to 3V3. If booting or uploading becomes unreliable, temporarily disconnect the GPIO8 switch and substitute a normal GPIO when feasible.[2] [3]

## Electrical notes

Use a 5 V supply sized for the LED load and connect its ground to Arduino ground. FastLED includes a software power-management function; this sketch conservatively sets it to 1.5 A:

```cpp
FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
```

Adjust that value only after checking your supply, wire gauge, and connector ratings. FastLED's power guidance gives a common 60 mA-per-pixel full-white planning bound, which would be 7.5 A for 125 pixels.[1]

## Contributing

This is intentionally a **pattern playground**, not a polished framework. Pull requests for new effects, mapping variants, hardware notes, or small-cube optimisations are welcome. If you build something fun, share a video or GIF in a discussion or issue.

## License

Released under the [MIT License](LICENSE). Keep the Dad & Manus origin credit in derived versions where practical, and then make it your own.

## Reference

[1]: https://github.com/FastLED/FastLED/blob/master/cookbook/core-concepts/power.md "FastLED power considerations"
[2]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
[3]: https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html#strapping-pins "Espressif ESP32-C3 strapping-pin guidance"
