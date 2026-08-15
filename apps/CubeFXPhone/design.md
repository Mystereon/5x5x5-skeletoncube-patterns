# CubeFX Controller — Phone Setup Extension

## Product intent

The phone controller retains CubeFX’s dark neon control identity while providing two practical additions: a direct route to the Galaxy Watch controller and an ESP32 Setup screen. The setup screen makes the electrical configuration visible and safe rather than treating pin and LED-count changes as hidden source edits.

## Screen list

| Screen | Purpose |
|---|---|
| **Live** | Current CubeFX state, engine controls, and next-pattern action. |
| **Patterns** | Searchable 48-pattern catalogue, clearly marked where a mode is standalone-only versus embedded in CubeFXWeb. |
| **Banner** | Message, font, hue, and scroll speed. |
| **ESP32 Setup** | LED data pin, primary/secondary button pins, columns, rows, layers, calculated LED total, safe-pin warnings, and generated header snippet. |
| **Galaxy Watch** | Watch8 Classic companion summary, bezel interaction guide, and a link to the signed watch APK release when published. |

## Key interaction

The ESP32 Setup screen calculates **total LEDs = columns × rows × layers** as each value changes. It distinguishes between the two persistent button pins, which can be sent to a connected cube, and the FastLED data pin/dimensions, which must be written to `CubeFXConfig.h` and reflashed. The Galaxy Watch card never claims a non-existent download; it opens the GitHub release page that will host the Wear OS APK.

## Colours

Graphite `#0B0D0B` carries the background. Lime `#C8FF20` identifies ready/apply states, cyan `#32C8FF` identifies Bluetooth and Galaxy Watch features, orange `#FF7A16` identifies installation and configuration attention, and red `#FF432E` identifies unsafe or reflash-required changes.
