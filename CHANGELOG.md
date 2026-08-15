# Changelog

All notable changes to SkeletonCube Patterns are recorded here.

## Unreleased — Interactive Pong, BLE CubeFX, and patterns 46–48

Expanded the canonical standalone library to **48 effects**. Added Voxel Minesweeper with falling probes, green base targets, and orange 3×3×3 hit bursts; Big Moon & Stars with a large shaded moon and twinkling sky; and a Nixie Tube that cycles 0–9 with dull orange inactive and bright orange active segments. Replaced the CubeFX self-playing Pong mode with actual single-player Pong: GPIO4 moves the full-height player paddle left and GPIO8 moves it right.

CubeFXWeb now has 31 selectable modes and a BLE service for the Android CubeFX controller. It advertises as `CubeFX-5x5x5`, accepts pattern, engine, banner, next, and Life-reseed commands, returns status notifications, and preserves the local Wi-Fi browser controller. GPIO4 and GPIO8 now use global long presses—Banner and Next—plus pattern-aware short presses.

## v0.4.0 — High-impact scenes and corrected banner motion

Expanded the canonical library from 39 to **45 distinct standalone patterns**. Added Intense Fire, Magical Blue Fire, Explosions, Launching Fireworks, Pixel Pasture, and Red Matrix Rain. Pixel Pasture places a green base on layer one, blocky brown cows on layer two, clouds on layers three and four, and a golden sun on layer five. Corrected CubeFXWeb banner travel while retaining the forward-facing 3×5 and 5×5 glyph fix.

## v0.3.9 — Canonical standalone patterns library

Reorganized `patterns/` as the canonical collection of **39 distinct visual effects**: original effects 01–28 plus CubeFX-only additions 29–39. Every numbered project is now directly uploadable and self-contained within its own folder. Moved the two ESP32-C3 hardware-controller examples out of the visual pattern sequence to `controllers/`, and removed duplicate CubeFX pattern numbering.

## v0.3.8 — CubeFX actual standalone demos

Added **22 complete, directly uploadable Arduino FastLED sketches** under `standalone/CubeFXPatternDemos/`. Each effect now has its own folder and self-contained `.ino`, with no browser, web-server, master-sketch, or local-header dependency.

## v0.3.7 — CubeFXWeb micro-world scenes

Expanded CubeFXWeb from 12 to 22 selectable effects. Added Bullet Wall particle impacts, Padded Cell, Block Run, Parallax Starfield, Trench Run, Running Legs, Fairies in Green Box, Orange Fish Tank, the requested three-layer Pyramid with a 3×3 base, 2×2 middle tier, and pulsing apex, plus Matrix Drift. Retuned 3-D Matrix Rain to independent straight-down deep-emerald streams with yellow-green heads and no white, while preserving the former diagonal wash as Matrix Drift.

## v0.3.6 — CubeFXWeb non-sticky controls

Added dirty-control tracking in the browser interface. Live refreshes no longer overwrite engine sliders, banner text, font selection, hue, or scroll speed while they are being edited; the controls synchronize only after **Apply** or **Send to Cube**.

## v0.3.5 — CubeFXWeb forward-reading banner

Corrected the exterior-banner glyph sampling direction. The physical perimeter path remains unchanged, while both compact 3×5 and bold 5×5 text now reads forward rather than mirrored around the cube.

## v0.3.4 — CubeFXWeb performance mode

Replaced the fixed 16 ms / roughly 60 FPS animation gate with a browser-configurable 30–120 FPS frame scheduler. Separated motion speed from rendering cadence, increased the motion-speed scale, and added the `fps` API parameter plus the Frame Rate Cap browser control.

## v0.3.3 — CubeFXWeb Arduino build fix

Moved CubeFXWeb’s browser page into `CubeFXWebPage.h`, preventing the Arduino sketch preprocessor from treating embedded JavaScript `function` declarations as C++. Restored the missing compact 3×5 `E` glyph so both font-table compile-time checks pass. The complete `CubeFXWeb` folder must now be kept together when opening the sketch.

## v0.3.2 — CubeFXWeb true 5×5 banner font

Added a selectable **5×5 / Bold full-face** bitmap font alongside the existing compact 3×5 option. CubeFXWeb now exposes the `bannerFont` API parameter and a browser font selector; it preserves message, hue, and independent scroll speed controls.

## v0.3.1 — CubeFXWeb 3×5 perimeter banner

Added a configurable 3×5 bitmap-font text banner to CubeFXWeb. The new pattern scrolls around the rear, right, front, and left exterior faces through a continuous 16-column perimeter. It begins with the default message `CUBE 4 3 2 1 0`. The browser UI and HTTP API now expose the message, FastLED hue, and dedicated banner-speed controls.

## v0.3.0 — CubeFXWeb browser controller

Added `standalone/CubeFXWeb/CubeFXWeb.ino`: an ESP32-C3 local Wi-Fi browser controller inspired by WS2812FX. It serves a cube-aware side-tab interface with a live isometric voxel preview, 11 3-D patterns, brightness/speed/dwell controls, auto/manual playback, a simple HTTP API, and GPIO4/GPIO8 hardware control.

## v0.2.5 — Standalone Conway 3-D Life

Added `standalone/Conway3DLife/Conway3DLife.ino`: a complete independent FastLED sketch with its own 5×5×5 map, B5/S45 3-D Life simulation, automatic reseeding, life-age colours, and optional GPIO4 reseed / GPIO8 pause controls. No master-gallery include is required.

## v0.2.4 — GPIO4/GPIO8 tailored controller

The ESP32-C3 dual-button controller now matches Dad's existing wiring: **GPIO4** selects the next pattern in manual mode and **GPIO8** toggles auto/manual playback. The master sketch exposes pin macros for alternate wiring. GPIO8 documentation now clearly requires a released button at reset and recommends an external 10 kΩ pull-up if the SuperMini board does not already maintain its boot-time high level.

## v0.2.3 — ESP32-C3 dual-button controller

This update adds a dual-button ESP32-C3 SuperMini controller. **GPIO3** steps to the next pattern in manual mode, while **GPIO4** safely toggles automatic and manual playback. GPIO4 replaces the proposed GPIO9 mode switch because GPIO9 is a boot strapping pin.

## v0.2.2 — ESP32-C3 next-pattern button

This update adds a debounced one-button manual pattern selector for the ESP32-C3 SuperMini. The new reference sketch uses **GPIO3** configured as `INPUT_PULLUP`; wire a momentary switch from GPIO3 to GND. Each stable press advances one pattern and disables automatic cycling. GPIO2, GPIO8, and GPIO9 are deliberately avoided because they are ESP32-C3 boot strapping pins.

## v0.2.0 — Expanded pattern library

This release turns the original self-playing gallery into a 28-pattern public playground. Every effect now has a tailored dwell time of at least 20 seconds, with longer sessions for simulations and scene-like animations.

| Area | Changes |
|---|---|
| Requested new effects | Added Snake, Space Invaders, Conway-style 3-D Life, clouds, neon plasma, fire, hourglass, pulse cube, upward red rain, spirals, point bouncer, and wrapping comets. |
| Playback | Replaced a global fixed duration with `dwellForPattern()`, providing effect-specific long dwell times. |
| Reference material | Added 28 fixed-pattern Arduino wrappers under `patterns/`, each with its own README. |
| Reuse | Added compile-time pattern-selection macros so the master sketch can be reused without code duplication. |
| Credits | Retains the Dad (MysterEon) & Manus project origin in source and documentation. |

## v0.1.0 — Initial public release

The first public release established the 5×5×5 coordinate mapper, manual-map validation markers, tumbling analytic solids, voxel model support, bouncing block, rain, Matrix rain, corner cubes, glitter, Pong, 3-D Tetris, blinking eye, DNA helix, and meteor stream.
