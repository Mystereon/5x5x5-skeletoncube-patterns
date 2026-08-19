# SkeletonCube Patterns

> A **5×5×5 WS2812B / NeoPixel skeletal-frame cube** pattern library for Arduino and FastLED.

**Created by Dad (MysterEon) & Manus, 2026.** It began with a physical 125-pixel cube and a simple question—how do you make voxel shapes tumble? It is released as a public starting point for anyone building, extending, or remixing small LED cubes.

![Platform](https://img.shields.io/badge/platform-Arduino-00979D) ![LEDs](https://img.shields.io/badge/LEDs-125%20WS2812B-brightgreen) ![Patterns](https://img.shields.io/badge/patterns-56-purple) ![License](https://img.shields.io/badge/license-MIT-blue)

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
patterns/                             # Canonical 56 standalone visual effects
  01_RedVectorCube/
  ...
  28_WrappingComets/
  29_PerimeterBanner/
  ...
  39_MatrixDrift/
  ...
  45_RedMatrixRain/
  46_VoxelMinesweeper/
  47_BigMoonStars/
  48_NixieTube/
  49_BlackHoleVortex/
  50_StargateDialUp/
  51_Defender3D/
  52_Chequerboard3D/
  53_HellraiserPuzzleCube/
  54_RubiksCube3D/
  55_LissajousRipple/
  56_ZarchVoxelDefender/
controllers/                           # ESP32-C3 hardware control examples
  29_ESP32C3_NextPatternButton/
  30_ESP32C3_DualButtonController/
standalone/                            # Standalone application-style sketches
  Conway3DLife/
  CubeFXWeb/
  VolumetricEngineDemo/                 # millis()-driven scalable-volume developer tool
  CubeFXPatternDemos/                  # Alternate CubeFX demo source collection
apps/                                  # Native Android companion source projects
  CubeFXPhone/                         # 56-pattern Android phone controller
  CubeFXWatch/                         # Galaxy Watch8 Classic Wear OS controller
tools/build_canonical_standalone_patterns.py # Rebuilds patterns/ 01–56
```

[`patterns/`](patterns/) is now the canonical visual-effect library: it contains **56 distinct, directly uploadable standalone Arduino projects**. Every numbered folder has a matching `.ino` file and its own README. Patterns 01–28 use a same-folder `DemoTypes.h` only to make Arduino’s auto-prototype process reliable; patterns 29–56 are single-file CubeFX projects. No effect project depends on the master sketch, the browser controller, or a relative include outside its own folder.

The [`standalone/Conway3DLife`](standalone/Conway3DLife/) folder is different: it contains a full single-file **Conway3DLife.ino** implementation with its own mapper, Life simulation, and GPIO4/GPIO8 controls. It can be copied to an Arduino sketchbook without any dependency on the master pattern library.

[`standalone/CubeFXWeb`](standalone/CubeFXWeb/) is a complete ESP32 Wi-Fi and BLE controller inspired by the fast, friendly control approach of [WS2812FX](https://github.com/kitesurfer1404/WS2812FX). It starts as a local Wi-Fi access point, serves its own cube-aware web UI, displays a live isometric voxel preview, and exposes **44 selectable CubeFX modes**. The current enclosure profile targets the Waveshare ESP32-S3-Zero. It drives **137 LEDs** over one data chain: **0–124 remain the 5×5×5 cube, while 125–136 are a rear-facing 12-pixel enclosure mood ring**. The ring uses a bright `160/255` scene-colour cap before FastLED’s global brightness is applied and acts as a second reactive visual plane for the whole gallery. The controller-specific **Ring Bouncer** mode lets one voxel boing through the whole volume in varied wall-reflecting headings while the rear ring runs at a local `200/255` colour cap; GPIO2 cycles ring colour and GPIO4 cycles bouncer colour independently. **Help Me Obi-Wan Hologram** projects a sparse cyan messenger with a perimeter projector base, broken scanline transmission, and a moving blue-white glow around the rear ring; BLE ID 58 selects it. **Voxel World Explorer** uses the physical 5×5×5 cube as a moving viewport through a much larger procedural 48×48×14 block world. It flies across water, grassy terrain, trees, small caves, and bright mineral cues while its camera window rises into the air and returns to ground; BLE ID 59 selects it, GPIO2 switches the mineral palette, and GPIO4 changes flight speed. **Phone VU Meter** and **Phone Spectrum 3-D** are audio-reactive modes selected with BLE IDs 60 and 61. The Android controller performs local microphone analysis and sends only eight band levels, loudness, and a beat flag—not recordings or raw audio—to CubeFX over BLE. Zarch: Voxel Defender is a long-form self-playing miniature battle: a lime craft patrols a compact cached terrain-column landscape, staged red-lander waves arrive, support shots create misses and hits, orange impact clusters flash, and short recovery pauses reset the mood. It receives at least a two-minute automatic dwell, while manual selection can run indefinitely. Lissajous Layer Ripple rests on layer 3 and makes restrained wave crests on layers 2 and 4, while Running Legs uses a hip–knee–ankle gait rather than angular alternating poses. Fixed 5×5×5 geometry—central radii, moon shading, Black Hole polar angles, explosion radii, and Stargate ring radii—is calculated once in `setup()` and reused during every frame, so heavy render loops avoid repeated square roots and polar-angle work.

[`standalone/CubeFXPatternDemos`](standalone/CubeFXPatternDemos/) remains as an alternate CubeFX source collection. The non-duplicated official effect catalog is [`patterns/`](patterns/): 01–28 are the original effects, 29–39 are the initial CubeFX-only additions, 40–45 add high-impact scenes, 46–48 add Voxel Minesweeper, Big Moon & Stars, and Nixie Tube, 49–54 explore negative space, sci-fi, retro arcade, and puzzle-cube scenes, 55 is the restrained layer-3 Lissajous ripple, and 56 is Zarch: Voxel Defender.

[`standalone/VolumetricEngineDemo`](standalone/VolumetricEngineDemo/) is a separate, directly uploadable developer tool. It exposes a scalable `VOLUME_X × VOLUME_Y × VOLUME_Z` mapper, setup-time radial/orbit lookup tables, non-blocking primitive rendering, and an editable `DEMO_STEPS[]` timeline. It is the foundation for future 10×10×5 and larger physical viewport hardware without forcing today’s CubeFXWeb gallery to carry generic-volume complexity.

The native Kotlin/Jetpack Compose source for the Android phone controller and the Galaxy Watch8 Classic companion is in [`apps/`](apps/). The phone app offers the 56-pattern browser plus controller-specific **Ring Bouncer** (BLE ID 57), **Help Me Obi-Wan Hologram** (BLE ID 58), **Voxel World Explorer** (BLE ID 59), **Phone VU Meter** (BLE ID 60), and **Phone Spectrum 3-D** (BLE ID 61). Its explicit opt-in **Audio Link** requests microphone permission, analyses audio locally, and streams only a small spectrum envelope while active. It also supplies live engine/Banner controls, an ESP32 wiring-and-dimensions setup screen, and a Watch route. The Watch app targets the rotating bezel as its primary pattern selector and includes Zarch in its embedded-mode list. Build and installation notes are in [`apps/README.md`](apps/README.md).

## Hardware assumptions

This project is configured for the cube that inspired it.

| Setting | Default | Change it here |
|---|---:|---|
| LEDs | 125 × WS2812B matrix + 12-pixel rear mood ring | `CUBEFX_MATRIX_LEDS` plus `CUBEFX_MOOD_LED_COUNT` = 137 outputs. |
| Rear-ring scene cap | 160 / 255, before global FastLED brightness | `CUBEFX_MOOD_RING_BRIGHTNESS`; the brighter ring is still subject to global brightness 100. |
| Ring Bouncer ring cap | 200 / 255, before global FastLED brightness | Fixed local value for Mode 40 only. |
| Colour order | GRB | `#define COLOR_ORDER GRB` |
| ESP32-S3 Zero data pin | GPIO6 | `CUBEFX_LED_DATA_PIN` |
| Primary / secondary buttons | GPIO2 / GPIO4 | `CUBEFX_PRIMARY_BUTTON_PIN` / `CUBEFX_SECONDARY_BUTTON_PIN` |
| FastLED brightness | 100 | `BRIGHTNESS` |
| Logical origin | Bottom–rear–left | Documented in the source header. |
| Row order | Left → right | `SERPENTINE_ROWS = false` |
| Frame delay | None | The project uses `millis()` timers instead. |

CubeFXWeb has its own [`CubeFXConfig.h`](standalone/CubeFXWeb/CubeFXConfig.h) for the WS2812B data pin, both physical button pins, columns, rows, layers, and the independent enclosure mood ring. The renderer deliberately keeps matrix operations at 125 voxels and writes the shared mood colour only to outputs **125–136** after every scene. Connect the last cube LED’s **DOUT** to the rear ring’s **DIN**; do not run it in parallel. Fish Tank and Fairies now use every matrix voxel, while the rear ring supplies their blue-water and green-fairy ambient glow. In **Ring Bouncer**, short GPIO2 presses advance the ring hue and short GPIO4 presses advance the lone bouncing voxel’s hue; the usual long presses remain Banner and Next Pattern. The supplied visual renderer explicitly remains 5×5×5, so another dimension can be planned and calculated in the Android setup screen but must wait for the generic-volume renderer before it is flashed.

### Recommended ESP32-S3 Zero profile

The Waveshare ESP32-S3-Zero uses an ESP32-S3FH4R2 with **4 MB flash, 2 MB PSRAM, and 512 KB internal SRAM**. Select **ESP32S3 Dev Module**, **4 MB flash**, **Huge APP (3 MB No OTA / 1 MB SPIFFS)**, and **QSPI PSRAM** in Arduino IDE. The enclosure mapping is GPIO6 data, GPIO2 primary button, and GPIO4 secondary button. The present **137-output** build compiles to **1,322,495 bytes flash (42% of the 3 MB app slot)** and **54,192 bytes static RAM (16% of the reported internal-RAM allocation)**. It retains more than 1.8 MB of application flash for future pattern code. The standalone VolumetricEngineDemo compiles separately at **432,343 bytes flash (13%)** and **27,940 bytes global RAM (8%)**.

The 2 MB PSRAM is available for future slow-changing cinematic scripts, diagnostics, or optional replay buffers. Keep the FastLED output buffer, active lookup tables, and timing-sensitive data in internal RAM; full PSRAM guidance is in [`ESP32_S3_MEMORY_NOTES.md`](standalone/CubeFXWeb/ESP32_S3_MEMORY_NOTES.md). [5]

### Recommended ESP32-C3 partition profile

For the 4 MB ESP32-C3 SuperMini, CubeFXWeb ships with a custom **maximum-app, no-OTA, no-SPIFFS** `partitions.csv`. The cube uses NVS for small Wi-Fi and button-pin settings but stores no runtime assets, so filesystem flash is better spent on the expanding firmware. The application partition is **0x3f0000 bytes / about 3.94 MiB**. The expanded 54-pattern build measured **1,461,578 bytes**, leaving **2,667,190 bytes** of headroom; it also fits the former 2 MiB application partition at 69.69% should that profile be retained. Full rationale and an optional 1 MB SPIFFS alternative are in [`PARTITION_SCHEMES.md`](standalone/CubeFXWeb/PARTITION_SCHEMES.md).[5]

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

Open [`controllers/29_ESP32C3_NextPatternButton/29_ESP32C3_NextPatternButton.ino`](controllers/29_ESP32C3_NextPatternButton/29_ESP32C3_NextPatternButton.ino). This variant disables automatic cycling and advances one pattern on each debounced press. It uses the internal pull-up, so no external resistor is needed. The full wiring and pin-selection rationale are in its [reference note](controllers/29_ESP32C3_NextPatternButton/README.md).

GPIO3 is a normal GPIO on the ESP32-C3. Avoid GPIO2, GPIO8, and GPIO9 for this switch because they are boot-strapping pins; avoid GPIO12–GPIO17, which are normally used by SPI flash; and leave GPIO18–GPIO19 available if you rely on USB-JTAG.[2] [3]

### CubeFXWeb dual-button and BLE controller

CubeFXWeb starts in automatic playback. Its two physical buttons retain global long-press behaviour while exposing useful short-press pattern actions:

```text
GPIO4 ────[ PRIMARY / BANNER ]── GND
GPIO8 ────[ SECONDARY / NEXT ]── GND
```

| Input | Short press | Long press |
|---|---|---|
| **GPIO4 / Button 1** | Pattern primary action. In Single-player Pong, move the full-height paddle **left**. | Enter **Banner** mode. |
| **GPIO8 / Button 2** | Pattern secondary action. In Single-player Pong, move the full-height paddle **right**. | Select the **next** pattern and enter manual mode. |

Examples of short-press actions include Banner hue/font control, Life reseed/step, Minesweeper target/probe reset, and intensity/speed adjustments for compatible effects. The legacy standalone dual-button example remains at [`controllers/30_ESP32C3_DualButtonController/`](controllers/30_ESP32C3_DualButtonController/), while CubeFXWeb contains this current control model.

GPIO8 is an ESP32-C3 boot strapping pin. The mode switch is usable after startup but it must be **released** at reset and power-up. If the SuperMini board does not already keep GPIO8 high at boot, add a 10 kΩ pull-up resistor from GPIO8 to 3V3. If booting or uploading becomes unreliable, temporarily disconnect the GPIO8 switch and substitute a normal GPIO when feasible.[2] [3]

### Android BLE control

CubeFXWeb advertises as **`CubeFX-5x5x5`** and exposes the CubeFX service UUID `6c75a300-7b1d-4f29-a221-000000000001`. The accompanying Android controller scans for this service and sends compact JSON pattern, engine, banner, next-pattern, and Life-reseed commands through its write-without-response characteristic. The firmware also publishes concise success/error status messages through a notify characteristic. This standard ESP32 Arduino BLE server structure uses a writable characteristic and advertising service UUID as documented by Espressif.[4]

The Android setup workflow also distinguishes **persistent button-pin choices** from **compile-and-flash choices**. A BLE `pins` command can save distinct primary and secondary button pins in NVS; changing the FastLED data pin or physical dimensions generates a new `CubeFXConfig.h` profile and requires a USB reflash. The Android phone controller includes a Galaxy Watch8 Classic companion-download route and an ESP32 setup screen; the native Watch companion uses the physical bezel to browse modes and touch controls for live actions.

## CubeFX rumours

Some builders say a skeletal cube has stranger moods than its normal selector suggests: a red eye, an impossible extra light, a green door, a cold signal, and a small wandering lantern. The five alleged activation methods are different, temporary, and not described as instructions. Read [`EASTER_EGG_RUMOURS.md`](EASTER_EGG_RUMOURS.md) only if you are comfortable with spoilers that are not quite spoilers.

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
[4]: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ble.html "Arduino-ESP32 BLE API"
[5]: https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/partition_table.html "Arduino-ESP32 partition tables"
