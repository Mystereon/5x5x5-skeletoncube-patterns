# Changelog

All notable changes to SkeletonCube Patterns are recorded here.

## v0.6.2 — Lissajous ripple and articulated Walking Legs

Added **Lissajous Layer Ripple** as canonical pattern 55 and as an embedded CubeFXWeb mode. Its bright trace lives on layer 3 at rest and only ripples into layers 2 and 4, retaining deliberate negative space. Reworked **Running Legs** into a fluid hip–thigh–knee–shin–foot stride with opposing arm swing, so knees visibly rise during the swing phase rather than jumping between two static poses. CubeFXWeb now exposes 38 selectable modes and the canonical standalone library contains 55 effects.

Added a setup-time 125-voxel geometry cache for fixed centre radii, moon distances/shading, Black Hole polar angles, explosion distances, and Stargate ring radii. Plasma now uses FastLED `sin8()` lookup math rather than per-voxel floating-point sine calls; fixed-scene geometry renderers no longer calculate square roots or polar angles in their frame loops. The ESP32-C3 compile completed successfully at **1,462,952 bytes** of program storage and **44,844 bytes** of global RAM; the supplied `partitions.csv` keeps a 0x3f0000-byte application slot for the 4 MB C3 flash layout.

## v0.6.1 — Android firmware route

Added a dedicated **Firmware** page to the Android CubeFX Controller source. The page links directly to the current `standalone/CubeFXWeb` firmware folder and the latest GitHub release, then presents a concise Arduino IDE flashing checklist including the supplied maximum-app `partitions.csv` profile and the distinction between persistent BLE button-pin changes versus reflash-required data-pin/dimension changes.

## v0.6.0 — Configurable CubeFX, patterns 49–54, and rumoured hidden scenes

Expanded the canonical standalone library to **54 effects**. Added Black Hole Vortex, Stargate Dial-Up, 3-D Defender, 3-D Chequerboard, Hellraiser Puzzle Cube, and 3-D Rubik’s Cube. CubeFXWeb now exposes **37 selectable modes** and keeps the new renderers procedural, without SPIFFS assets or frame buffers. The public library generator produces directly uploadable folders 49–54 alongside the previous catalogue.

CubeFXWeb now has `CubeFXConfig.h` for LED data pin, primary/secondary button pins, columns, rows, layers, and product-derived LED total. The two button pins can be changed through BLE and saved to ESP32 NVS; data-pin and physical-dimension changes remain explicit compile-and-flash operations. The recommended 4 MB ESP32-C3 profile is a custom maximum-app, no-OTA, no-SPIFFS partition table: compiled CubeFXWeb is 1,461,578 bytes, leaving 2,667,190 bytes inside the 0x3f0000-byte app partition.

Added the CubeFX phone setup source with safe ESP32-C3 pin selection, rows × columns × layers LED calculation, generated configuration profile, and a Galaxy Watch controller route. Added the native Watch8 Classic controller source with a rotating-bezel pattern browser, BLE GATT writes, brightness/speed touch controls, Banner/Auto actions, and single-player Pong left/right commands.

Five temporary hidden scenes now have distinct activation paths through a banner phrase, button rhythm, mode sequence, control coincidence, and patient observation. Their public treatment is deliberately spoiler-light in `EASTER_EGG_RUMOURS.md`.

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
