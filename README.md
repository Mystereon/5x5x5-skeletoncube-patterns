# SkeletonCube Patterns

> A self-playing **5×5×5 WS2812B / NeoPixel skeletal-frame cube** pattern library for Arduino and FastLED.

**Created by Dad (MysterEon) & Manus, 2026.** It began with a physical 125-pixel cube and a simple question—how do you make voxel shapes tumble? It is released as a public starting point for anyone building or extending small LED cubes.

![Platform](https://img.shields.io/badge/platform-Arduino-00979D) ![LEDs](https://img.shields.io/badge/LEDs-125%20WS2812B-brightgreen) ![License](https://img.shields.io/badge/license-MIT-blue)

## What it does

The sketch runs a sequenced gallery of 3-D effects with no blocking frame delay. Every effect uses a shared `setVoxel(x, y, z, colour)` coordinate interface, so new patterns do not need to care about the physical position of LEDs in the serial chain.

| Pattern | Description |
|---|---|
| Red vector cube | A red, rotating **3×3×3 wireframe** cube. |
| Solid cube, sphere, octahedron | Inverse-rotated 3-D mathematical solids. |
| Voxel model | A hand-authored rotating voxel object. |
| Bouncing block | A 2×2×2 cube bouncing independently on all three axes. |
| Blue rain | Sparse, coloured falling drops. |
| Column sweep | A rainbow vertical-column sweep. |
| Matrix rain | Dense green 3-D Matrix-style streams with bright heads and trails. |
| Corner cubes | Eight 2×2×2 corner cubes with independent hue offsets. |
| White glitter | Random bright voxel sparkles and decay. |
| Autoplay Pong | A ball and two self-tracking 2×2 paddles across the x axis. |
| 3-D Tetris | Falling, settling four-voxel polycubes in a 5×5×5 well. |
| Blinking eye | A 3-D eye with moving pupil and periodic blink. |
| DNA helix | Two coloured helical strands. |
| Meteors | A warm-colour wrapped diagonal meteor stream. |

## Hardware assumptions

This project is configured for the cube that inspired it:

| Setting | Default | Change it here |
|---|---:|---|
| LEDs | 125 × WS2812B | `NUM_LEDS` is derived from `N = 5`. |
| Colour order | GRB | `#define COLOR_ORDER GRB` |
| Data pin | GPIO / Arduino pin 2 | `#define DATA_PIN 2` |
| FastLED brightness | 100 | `BRIGHTNESS` |
| Logical origin | Bottom–rear–left | Documented in the source header. |
| Row order | Left → right | `SERPENTINE_ROWS = false` |

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

Install the [FastLED library](https://fastled.io/docs/) in Arduino IDE. Then download or clone this repository, open:

```text
SkeletonCubePatterns/SkeletonCubePatterns.ino
```

and upload it. Keep `CubeTypes.h` in the **same Arduino sketch folder** as the `.ino` file; Arduino needs this header before it generates function prototypes.

The sketch starts with automatic cycling enabled:

```cpp
const bool AUTO_CYCLE_PATTERNS = true;
constexpr uint32_t PATTERN_DURATION_MS = 10000;
```

Each pattern shows for ten seconds. To hold on one pattern while developing it, change the first line to `false`, then select the desired item in the second line:

```cpp
const bool AUTO_CYCLE_PATTERNS = false;
const Pattern FIXED_PATTERN = PATTERN_MATRIX_RAIN;
```

## Calibrate the physical map first

Set the following switch to `true` before experimenting with complex patterns:

```cpp
const bool SHOW_MAPPING_MARKERS = true;
```

You should see the following four locations.

| Physical point | Expected colour |
|---|---|
| Bottom–rear–left `(0,0,0)` | Red |
| Bottom–rear–right `(4,0,0)` | Green |
| Bottom–front–left `(0,4,0)` | Blue |
| Top–rear–left `(0,0,4)` | White |

If the locations are wrong, correct only the map flags near the top of the sketch: `SWAP_XY`, `FLIP_X`, `FLIP_Y`, `FLIP_Z`, `SERPENTINE_ROWS`, and `SERPENTINE_LAYERS`. When correct, restore `SHOW_MAPPING_MARKERS = false`.

## Add a pattern

A new pattern needs three small changes:

1. Add a name to `enum Pattern` in `CubeTypes.h`, before `PATTERN_COUNT`.
2. Write a `renderYourPattern()` function that calls `setVoxel()` or `addVoxel()`.
3. Add a case to the `switch (pattern)` in `loop()`.

A minimal static example is:

```cpp
void renderCentreMarker() {
  setVoxel(2, 2, 2, CRGB::Purple);
}
```

Patterns that need state should use `millis()` timestamps instead of `delay()`. This lets every effect fit cleanly into the auto-cycle and keeps the cube responsive.

## Electrical notes

Use a 5 V supply sized for the LED load and connect its ground to Arduino ground. FastLED includes a software power-management function; this sketch conservatively sets it to 1.5 A:

```cpp
FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
```

Adjust that value only after checking your supply, wire gauge, and connector ratings. FastLED's power guidance gives a common 60 mA-per-pixel full-white planning bound, which would be 7.5 A for 125 pixels.[1]

## Contributing

This is intentionally a **pattern playground**, not a polished framework. Please open an issue or pull request with new effects, mapping variants, hardware notes, or small-cube optimisations. If you build something fun, share a video or GIF in the discussion.

## License

Released under the [MIT License](LICENSE). Keep the Dad & Manus origin credit in derived versions where practical, and then make it your own.

## Reference

[1]: https://github.com/FastLED/FastLED/blob/master/cookbook/core-concepts/power.md "FastLED power considerations"
