# Changelog

All notable changes to SkeletonCube Patterns are recorded here.

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
