# Changelog

All notable changes to SkeletonCube Patterns are recorded here.

## v0.7.8 — Control-Path Repair

Repaired the full CubeFX control path after a regression audit. CubeFXWeb’s Pattern Gallery now has both immediate selection and a deliberate **PLAY SELECTED PATTERN** route, so replaying the current effect and selecting a new effect each have a visible command path. The page now shows a command acknowledgement line and checks HTTP responses rather than silently treating every browser request as successful.

The browser controller now includes the same five direct **Easter Egg Control Deck** triggers as the Android controller. Firmware accepts the guarded `/api/control?secret=0..4` route while preserving every physical owner ritual. The direct scenes remain five-second, intentional effects and do not alter the normal pattern catalogue.

CubeFXPhone now presents its **56 firmware-embedded modes** as explicit `PLAY` buttons. The remaining **17 standalone .ino references** are visibly separated and linked to their source rather than behaving as silent, non-playing rows. Android subscribes to the firmware’s BLE status characteristic, so the header distinguishes a sent command from a firmware acknowledgement or rejection.

Added two source-level regression checks: the command-contract verifier confirms all 56 browser/Android modes, five browser/Android secret routes, canonical IDs, shared BLE UUIDs, and embedded browser JavaScript syntax; the low-memory firmware syntax check reuses the Arduino-generated ESP32-S3 command. The Android debug APK compiled successfully after these checks.

## v0.7.7 — Owner Controls, Gold O, and Next-Wave Instruments

Restored CubeFXPhone as a rich, muggle-proof controller rather than a stripped utility: its side navigation now leads to a rebuilt LIVE tab with direct **Next Pattern**, engine, Auto/Manual, Banner, context-action, Audio Link, firmware, ESP32 setup, Watch, repository, wiring, and support controls. The Owner Easter Eggs tab offers one-tap deliberate triggers for all five hidden scenes and an in-app Owner’s Guide for their backup routes. Firmware now accepts the concise connected-app BLE command `{"op":"secret","scene":0..4}` without replacing the original button and timed paths.

Replaced the unreadable sparse torus with **Rotating Gold O**, a centred discrete 5×5 gold O that turns cleanly face-on to edge-on around one upright visual axis against a violet/cyan acrylic halo.

Added the first ten original next-wave scenes as CubeFXWeb modes **47–56** / BLE IDs **64–73**: **Reactor Core**, **Reactor Core Meltdown**, **Targeting System**, **Phosphor Green Radar**, **Ghost Detector**, **Alert**, **Intersecting Planes**, **Oscillating Wave Field**, **Rainbow Spiral**, and **Plasma Entity Containment**. Each includes a rear-ring treatment intended to activate the acrylic enclosure as a distinct visual layer.

The ESP32-S3 Huge APP build verifies at **1,328,035 bytes / 42% flash** and **54,208 bytes / 16% global RAM**. CubeFXPhone builds successfully with **73** selectable entries and a refreshed debug APK.

## v0.7.6 — Physical Direction Fixes, Acrylic Contrast, Cloud Rain, and Gold Ring

Corrected CubeFXWeb’s physical coordinate interpretation. The 3×5 and 5×5 Banner now samples glyph columns in the correct perimeter order, so entered words read forward instead of requiring reversed text. Matrix Rain, Matrix Drift, and Red Matrix Rain now move from the physical top layer (`z = 4`) down to the physical base (`z = 0`), with their trails remaining above the descending heads.

Added **Cloud-Top Rain** as CubeFXWeb mode **45** / BLE ID **62**. A pale drifting cloud deck remains on physical layer five while blue rain descends below it. Short GPIO2 cycles rain hue and short GPIO4 cycles the rear-ring hue; its default blue rain and amber ring deliberately use opposing chroma to show the acrylic enclosure.

Added **Rotating Gold O** as CubeFXWeb mode **46** / BLE ID **63**. It is a deliberately readable discrete 5×5 gold O, centred in the cube, which turns only around the cube’s upright visual axis so it alternates between a full face and a slim edge. Short GPIO2 changes gold tone; short GPIO4 changes turn speed. A deep violet rear-ring halo with a cyan travelling accent creates the intended high-contrast acrylic reveal.

Fish Tank and Fairies are now genuinely open-volume scenes. Fish Tank uses only orange fish, tails, and sparse bright bubbles inside black cube space; the bright blue rear ring makes the acrylic enclosure read as water. Fairies use only floating bodies and wings inside black space, while a bright green rear ring forms their implied magical habitat. High-contrast Matrix and fire-family effects now use complementary rear-ring hues—green versus violet, red versus cyan, orange versus blue, and blue versus amber—rather than same-colour enclosure light.

CubeFXWeb now exposes **46 modes**. CubeFXPhone exposes controller-specific IDs **57–63**, including the new rain and gold-ring modes; a fresh debug APK build is attached. The ESP32-S3 Huge APP build verifies at **1,323,923 bytes / 42% flash** and **54,256 bytes / 16% global RAM**.

## v0.7.5 — Phone Audio Link, VU Meter, and 3-D Spectrum

CubeFXWeb now adds two phone-driven audio visualiser modes: **Phone VU Meter** (BLE ID **60**) and **Phone Spectrum 3-D** (BLE ID **61**). The Android controller requests microphone permission only when the user explicitly starts Audio Link. It captures and analyses a local 256-sample PCM window, combines the result into eight visible bands, then streams a tiny 13-byte spectrum envelope at no more than 25 updates per second. The cube never receives, records, or stores raw microphone audio.

The ESP32-S3 retains only eight band values, loudness, a beat estimate, and a timestamp. It smooths packet cadence, decays to black after 550 ms without packets, renders eight vertical VU columns or a raised 3-D spectrum field, and uses the 12-pixel rear ring for bass-weighted colour and white beat accents. The binary protocol, consent boundary, packet format, and audio-link behaviour are documented in [`AUDIO_SPECTRUM_PROTOCOL.md`](standalone/CubeFXWeb/AUDIO_SPECTRUM_PROTOCOL.md).

CubeFXWeb now exposes **44 selectable modes**. The native CubeFXPhone app now has an opt-in Audio Link panel and a fresh debug APK build. The ESP32-S3 Huge APP build verifies at **1,322,495 bytes / 42% flash** and **54,192 bytes / 16% global RAM**.

## v0.7.4 — Voxel World Explorer and Volumetric Engine Demo

CubeFXWeb mode **42**, **Voxel World Explorer**, uses the physical 5×5×5 matrix as a live window into an original procedural **48×48×14** block world. Rather than allocate a 32 KB-plus world-colour framebuffer, every requested terrain block is derived from its virtual coordinates. The self-playing camera follows a long rectangular flight path and moves through the vertical terrain range, making water, grass, dirt, stone, trees, cave gaps, and mineral cues appear and disappear through the physical viewport.

The 12-pixel rear ring now reports the virtual camera state: water-blue at low altitude, green terrestrial travel at middle height, and pale airborne scan light at high altitude. The browser selector lists the mode, BLE ID **59** selects it, GPIO2 changes its mineral palette, and GPIO4 changes flight speed. CubeFXPhone’s catalog and firmware page have been updated and the native Android debug build completed successfully.

Added [`standalone/VolumetricEngineDemo`](standalone/VolumetricEngineDemo/), a direct-upload ESP32-S3 developer tool with a scalable volume mapper, setup-time LUTs, reusable voxel primitives, a non-blocking `millis()` timeline, and a concise editable `DEMO_STEPS[]` pattern editor. CubeFXWeb verifies at **1,321,075 bytes / 41% flash** and **54,168 bytes / 16% global RAM**. The standalone tool verifies at **432,343 bytes / 13% flash** and **27,940 bytes / 8% global RAM**.

## v0.7.3 — Help Me Obi-Wan Hologram

CubeFXWeb mode **41** is a compact blue hologram projection. A perimeter projector base and sparse upward light cone frame a small cyan humanoid silhouette with head, shoulders, outstretched arms, torso, and widening skirt. A moving scanline brightens one layer at a time while deterministic tiny dropouts make the transmission shimmer rather than look like solid sculpture.

The 12-pixel rear ring now behaves as its projector: cyan-blue illumination circles the enclosure with a blue-white head and occasional brief transmission dropout. The browser selector lists the scene, BLE pattern ID **58** selects it, and the native Android CubeFXPhone catalog has been updated and built successfully with the new entry. The verified ESP32-S3 Huge APP build measures **1,317,231 bytes / 41% flash** and **53,784 bytes / 16% global RAM**.

## v0.7.2 — Wild full-volume Ring Bouncer

Ring Bouncer no longer follows a fixed three-axis diagonal between two opposite corners. At each wall impact—and after a short run of moves—it chooses a fresh, safe heading with any combination of X, Y, and Z travel. The voxel therefore explores lines, planes, and diagonals across the full 5×5×5 volume while never pointing through a wall.

The physical controls and 200 / 255 local rear-ring scale are unchanged: short GPIO2 changes ring colour, short GPIO4 changes voxel colour, and long presses remain Banner / Next Pattern. The verified ESP32-S3 Huge APP build measures **1,316,251 bytes / 41% flash** and **53,784 bytes / 16% global RAM**.

## v0.7.1 — Android Ring Bouncer controller alignment

The native Android **CubeFXPhone** catalog now exposes controller-specific BLE pattern ID **57**, Ring Bouncer, alongside the existing 56 canonical patterns. Its pattern screen explains that short **GPIO2** changes the rear-ring colour and short **GPIO4** changes the bouncing-voxel colour. The Android firmware page now documents the ESP32-S3 Zero profile: GPIO6 data, GPIO2/GPIO4 buttons, 125 matrix LEDs plus the 12-pixel rear ring, 40 embedded modes, and the Huge APP no-OTA setup.

The updated native phone app assembled successfully as a debug APK with Android SDK tooling, confirming the Kotlin catalog and controller UI compile against the revised BLE protocol.

## v0.7.0 — Ring Bouncer interactive enclosure mode

CubeFXWeb now has **40 selectable modes**. The new controller-specific **Ring Bouncer** leaves the entire 5×5×5 matrix free for one voxel that ricochets on all three axes. Its 12-pixel rear ring is a solid selected colour at a local **200 / 255** cap before CubeFX’s global brightness limiter.

Short **GPIO2** presses step the rear-ring hue; short **GPIO4** presses step the bouncing voxel hue. The long-press actions are unchanged: GPIO2 opens Banner mode and GPIO4 advances to the next pattern. The browser selector presents the new mode, while BLE canonical ID **57** selects it for controllers that expose controller-specific modes. The ESP32-S3 Huge APP build measured **1,315,939 bytes / 41% flash** and **53,784 bytes / 16% global RAM**.

## v0.6.9 — Reactive rear ring and hidden ring scenes

The 12-pixel rear ring is now a **second reactive visual plane** across the CubeFX gallery. Matrix rain has moving heads, plasma and shape scenes use rainbow flow, arcade scenes use a bright tracer, Life and landscape scenes breathe, and impact scenes carry a stronger orange alert. Fish Tank and Fairies remain deliberately frameless inside the cube: the acrylic enclosure and its ring now provide their visible frame.

Four existing hidden-scene activation routes now lead to **ring-only** effects while all 125 matrix voxels stay black: a slow warm pulse, a compact blue-and-amber clock face, a five-step countdown, and outward orange explosion waves. The Red Eye remains the lone cube apparition. The secret-ring strobe is deliberately slow and warm rather than a high-frequency white flash.

The rear-ring scene cap is now `CUBEFX_MOOD_RING_BRIGHTNESS = 160`, before the existing FastLED global brightness of 100. The verified ESP32-S3 Huge APP build measures **1,315,335 bytes / 41% flash** and **53,760 bytes / 16% global RAM**.

## v0.6.8 — 12-pixel rear mood ring

The enclosure output after the 125-voxel cube is now a **12-pixel rear-facing WS2812B mood ring**, producing a 137-output FastLED chain. Outputs 125–136 are assigned after every matrix scene, so they cannot be consumed by a cube renderer. Fish Tank, Fairies, Zarch, Stargate, Black Hole, and the fire-family scenes retain their matching enclosure mood colours across the full ring.

The ring starts with `CUBEFX_MOOD_RING_BRIGHTNESS = 48` before the existing FastLED global brightness of 100 is applied. This is intentionally cautious for the first physical power-up; check supply voltage, enclosure illumination, and temperature before increasing the macro. The ESP32-S3 Huge APP build verified at **1,314,127 bytes / 41% flash** and **53,760 bytes / 16% global RAM**.

## v0.6.7 — Standalone Zarch terrain-cache synchronisation

The source sketch in `standalone/CubeFXPatternDemos/ZarchVoxelDefender` and its generated canonical counterpart in `patterns/56_ZarchVoxelDefender` now match CubeFXWeb’s compact terrain representation. The 125-voxel `CRGB` terrain frame has been replaced with 25 `{ height, tint }` terrain columns, reducing fixed terrain storage from **375 bytes to 50 bytes** while retaining the same seeded low-poly terrain appearance.

Pattern 56’s standalone README now correctly specifies its ESP32-S3 Zero defaults: **GPIO6** LED data, **GPIO2** primary reset button, and **GPIO4** secondary support-shot button. The canonical direct-upload sketch was compiled successfully with the ESP32-S3 Huge APP profile at **422,323 bytes / 13% flash** and **27,948 bytes / 8% global RAM**.

## v0.6.6 — ESP32-S3 Zero enclosure profile and mood light

CubeFXWeb now targets the Waveshare **ESP32-S3-Zero** enclosure profile: **GPIO6** is WS2812B data, **GPIO2** is the primary button, and **GPIO4** is the secondary button. The serial LED chain now contains **126 outputs**. Indices 0–124 remain the 5×5×5 matrix, while index 125 is an independent external NeoPixel for acrylic-edge mood lighting.

Fish Tank and Fairies now use all 125 cube voxels; their former internal visual frames have moved into the enclosure as water-blue and green ambient light. Zarch responds with alert/recovery mood beats, while Stargate, Black Hole, and the fire-family scenes each receive a matching external colour. Fish Tank and Fairies hold the automatic cycle for at least **90 seconds**; Zarch keeps its two-minute cinematic minimum.

Zarch’s fixed terrain cache was reduced from a 125-element `CRGB` frame (**375 bytes**) to a 25-column terrain description (**50 bytes**), saving **325 bytes** of global RAM. Its terrain colour is reconstructed with compact integer arithmetic during the render, while the fixed geometric LUTs remain setup-time values. Compiled with **ESP32S3 Dev Module**, **Huge APP (3 MB No OTA / 1 MB SPIFFS)**, and **QSPI PSRAM**, CubeFXWeb measures **1,314,075 bytes / 41% flash** and **53,728 bytes / 16% global RAM**.

## v0.6.5 — Long-form self-playing Zarch

Pattern 56 / CubeFXWeb mode 39 has grown into a cinematic self-playing Zarch scene. It now moves through a patrol, initial contact, crossfire, finale, and recovery sequence; the lime craft patrols and aligns itself with active landers, autonomous shots deliberately include atmospheric misses, attacks create short orange impact clusters, and a recovery pause returns the scene to calm cyan navigation lights.

Zarch now holds the auto-cycle for at least **120 seconds**—or the user-selected longer cycle duration—so the complete miniature story can play. Manual selection remains unlimited; short primary resets the full scene and short secondary contributes a support shot. The 125-voxel terrain lookup table is still generated only on reseed, and the new director adds only compact byte/timer state.

## v0.6.4 — Zarch: Voxel Defender

Added **Zarch: Voxel Defender** as canonical pattern 56 and CubeFXWeb mode 39. The compact scene flies a lime/cyan craft over green voxel terrain while two red landers descend, forward shots intercept them, and short orange sparks mark impacts. Short primary reseeds the terrain; short secondary fires. Global long-press Banner and Next actions remain unchanged.

The terrain renderer uses a 125-element `CRGB` lookup table. Terrain heights and colours are generated only when the scene resets; each frame begins with a direct cached-buffer copy before moving entities are overlaid. CubeFXWeb compiles for ESP32-C3 at **1,464,756 bytes** of program storage with **45,244 bytes** of global RAM. The standalone Zarch sketch compiles at **433,164 bytes** and **20,224 bytes** of global RAM. Android phone and Galaxy Watch catalogs now expose canonical ID 56.

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
