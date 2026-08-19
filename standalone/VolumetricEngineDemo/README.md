# Volumetric Engine Demo

`VolumetricEngineDemo.ino` is a **standalone Arduino developer tool** for a physical WS2812B cube. It is not part of CubeFXWeb’s 41-mode gallery. The sketch demonstrates how to make readable 3-D scenes from a compact, editable timeline while keeping the renderer entirely `millis()`-driven and non-blocking.

> **Design rule:** a step describes *what should be visible at a given time*. The engine derives its state from the current `millis()` age, renders a frame, and returns immediately. There are no `delay()` calls and no blocking animation loops.

## Included ESP32-S3 Profile

The supplied configuration targets the current CubeFX enclosure: GPIO6 drives **125 cube LEDs**, then **12 rear-ring LEDs**, for **137 LEDs total**. The coordinate origin is bottom-rear-left and the physical index model is `z × (xSize × ySize) + y × xSize + x`.

| Setting | Default | Purpose |
|---|---:|---|
| `VOLUME_X`, `VOLUME_Y`, `VOLUME_Z` | `5`, `5`, `5` | Matrix dimensions; change these three values to scale the engine. |
| `RING_LEDS` | `12` | External LEDs after the matrix; set to `0` for a cube without a ring. |
| `DATA_PIN` | `6` | ESP32-S3 Zero WS2812B data pin. |
| `GLOBAL_BRIGHTNESS` | `100` | Overall FastLED brightness limit. |
| `RING_LOCAL_BRIGHTNESS` | `160` | Local ring ceiling before global brightness is applied. |
| `TARGET_FPS` | `100` | Rendering cadence only; it does not alter step timing. |

The mapper also includes `SWAP_XY`, `FLIP_X`, `FLIP_Y`, `FLIP_Z`, `SERPENTINE_ROWS`, and `SERPENTINE_LAYERS`. Set these only when a physical cube’s wiring differs from the documented non-serpentine map.

## Step Editor

The editable scene lives in the `DEMO_STEPS[]` table. A row follows this format:

```cpp
STEP(durationMs, primitive, hue, saturation, value, ringMode, a, b, c)
```

| Field | Meaning |
|---|---|
| `durationMs` | Step duration in milliseconds. The next row begins automatically. |
| `primitive` | The cube operation to render. |
| `hue`, `saturation`, `value` | Standard FastLED `CHSV` colour values. |
| `ringMode` | Independent rear-ring treatment for the same step. |
| `a`, `b`, `c` | Small primitive-specific parameters. Unused fields remain `0`. |

### Primitive Reference

| Primitive | Parameters | Result |
|---|---|---|
| `PRIM_VOXEL` | `a=x`, `b=y`, `c=z` | A held single voxel. |
| `PRIM_WIRE_BOX` | `a=margin` | A cube-edge wireframe. Use `0` for the outer edge. |
| `PRIM_SWEEP` | `a=axis` (`0=x`, `1=y`, `2=z`) | A plane moving across the selected axis. |
| `PRIM_ORBIT` | None | A moving point and two-pixel tail on a setup-time orbit LUT. |
| `PRIM_SPHERE` | `a=max radius in voxel units` | A shell expanding from the volume centre. |
| `PRIM_BOUNCE` | None | A full-volume tri-wave bouncer with a faint two-sided glow. |
| `PRIM_SPARKLES` | `a=density` (`1–255`) | Deterministic, frame-rate-independent volumetric sparkles. |

### Ring Modes

| Ring mode | Result |
|---|---|
| `RING_OFF` | Ring black. |
| `RING_STEADY` | Uniform enclosure light. |
| `RING_BREATHE` | Slow shared breathing glow. |
| `RING_CHASE` | A travelling bright point around the ring. |
| `RING_PROJECTOR` | Cyan-blue projector treatment, useful for holograms and scans. |

## Make a New Pattern

Copy the existing `DEMO_STEPS[]` rows, change one row at a time, and upload. For example, this short sequence creates a violet vertical sweep followed by a gold central beacon:

```cpp
const PatternStep DEMO_STEPS[] = {
  STEP(2400, PRIM_SWEEP, 180, 255, 255, RING_CHASE, 2, 0, 0),
  STEP(1800, PRIM_VOXEL,  32, 255, 255, RING_STEADY, 2, 2, 2)
};
```

Every step loops after the final row. The `while` loop in `advanceTimeline()` prevents a temporary slow frame from making the sequence drift behind real time.

## Performance Policy

The engine stores only the LED buffer, one radial-distance byte per voxel, and a 24-point orbit table. `sqrtf()`, `sinf()`, and `cosf()` are used **only in `buildLUTs()` during setup**. The frame renderer relies on integer math, FastLED’s `sin8()`/`triwave8()`, and direct lookup tables.

For a larger matrix, check the static RAM report after compiling. The main cost is approximately **3 bytes per LED** for the `CRGB` buffer, plus **1 byte per matrix voxel** for `radius16`. Keep the number of timeline rows modest—each row is a small fixed record in flash/static data.

## Upload Settings

Use **ESP32S3 Dev Module**, **4 MB flash**, **Huge APP (3 MB No OTA / 1 MB SPIFFS)**, and **QSPI PSRAM enabled**. Connect GPIO6 to the first cube LED’s DIN through the usual resistor/level-shift arrangement. The 12-pixel rear ring follows cube LED 124 in the same data chain.

Created by **Dad (MysterEon) & Manus**.
