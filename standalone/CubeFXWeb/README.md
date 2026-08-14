# CubeFXWeb — browser-controlled 3-D cube patterns

> **CubeFXWeb** is a standalone ESP32-C3 browser controller for the 5×5×5 WS2812B skeletal cube. It takes inspiration from the immediate, in-browser pattern selection style of [WS2812FX][1], but its renderer and interface are built around **3-D voxels**, not a one-dimensional strip.

The firmware has no cloud dependency and uses only FastLED plus the Arduino-ESP32 built-in `WiFi` and `WebServer` classes. The browser UI is served directly from the ESP32-C3; it includes a live isometric voxel preview of the firmware framebuffer.

## Features

| Area | Included |
|---|---|
| Pattern gallery | 12 cube-aware effects: vector cube, Matrix rain, plasma, fire, spirals, comets, Pong, 3-D Life, clouds, glitter, corner cubes, and a 3×5 perimeter banner. |
| Browser controls | Pattern selection, brightness, speed, automatic dwell time, auto/manual switch, next pattern, 3-D Life reseed, and banner text/colour/scroll-speed controls. |
| Networking | Access-point mode by default; optional connection to an existing Wi-Fi network. |
| Physical controls | GPIO4 advances a pattern in manual mode; GPIO8 toggles auto/manual. |
| Rendering | Non-blocking `millis()` scheduling; no frame delay. |
| Cube map | The confirmed bottom–rear–left origin and non-serpentine left-to-right rows. |

## Upload and connect

Install **FastLED** and the Espressif ESP32 Arduino core. Select the ESP32-C3 SuperMini board profile appropriate for your installation, open `CubeFXWeb.ino`, and upload.

On first boot the controller starts its own Wi-Fi access point:

| Setting | Value |
|---|---|
| Network name | `CubeFX-5x5x5` |
| Password | `cubecontrol` |
| Controller page | `http://192.168.4.1` |

Connect a phone, tablet, laptop, or desktop browser to that network, then open the address above. The page has a side-tab layout for **Live**, **Patterns**, **Control**, and **About**.

## Join home Wi-Fi instead

At the top of `CubeFXWeb.ino`, edit these values before upload:

```cpp
const char *WIFI_SSID = "YourNetwork";
const char *WIFI_PASSWORD = "YourPassword";
```

CubeFXWeb tries home Wi-Fi for 12 seconds. If it cannot join, it deliberately falls back to the local access point, so the cube remains controllable even when the home network changes.

## Cube defaults

| Setting | Default |
|---|---:|
| LEDs | 125 WS2812B / NeoPixel LEDs |
| LED data pin | GPIO2 |
| Colour order | GRB |
| Brightness | 100 |
| Logical origin | Bottom–rear–left `(0,0,0)` |
| Row order | Left to right; non-serpentine |
| Map equation | `index = z * 25 + y * 5 + x` |

The cube is powered separately at 5 V. Connect the LED supply ground to ESP32-C3 ground. The firmware calls FastLED's power-management limiter at 5 V and 1.5 A; review the supply, wires, and connector ratings before increasing it. FastLED’s common full-white planning estimate is 60 mA per pixel, which would be 7.5 A for 125 pixels.[2]

## Exterior text banner: 3×5 and 5×5 fonts

The **Perimeter Banner** pattern renders bitmap text across the cube’s five vertical voxels and scrolls it continuously around the **four outside faces**. Its 16-column ring path follows the rear, right, front, and left sides clockwise when viewed from above. Corners are assigned only once, so the text remains continuous as it turns each corner.

Two font modes are available in **Control → 3×5 Perimeter Banner**:

| Mode | Glyph size | Behaviour |
|---|---:|---|
| **3×5 / Compact** | 3 columns × 5 rows | Faster-reading, narrow characters with a one-voxel gap between letters. |
| **5×5 / Bold full-face** | 5 columns × 5 rows | A true 5×5 bitmap font that fills the cube’s full side-face width for larger, bolder characters. |

Set the message, font mode, hue, and dedicated scroll speed; then select **Show Banner**. The firmware starts with the message **`CUBE 4 3 2 1 0`**. It accepts up to 60 characters and supports uppercase `A–Z`, digits `0–9`, spaces, period, dash, colon, exclamation mark, and question mark. Unsupported characters are safely converted to spaces.

| API request | Example | Result |
|---|---|---|
| Set message | `/api/control?text=HELLO%20CUBE` | Changes the banner message and resets it to the beginning. |
| Set colour | `/api/control?bannerHue=160` | Changes the FastLED hue, `0–255`. |
| Set banner speed | `/api/control?bannerSpeed=210` | Sets independent scroll speed, `1–255`. |
| Set font mode | `/api/control?bannerFont=5` | Uses true bold 5×5 glyphs; use `bannerFont=3` for compact 3×5 glyphs. |
| Show it | `/api/control?pattern=11` | Selects the banner and enters manual mode. |

## Physical buttons

```text
GPIO4 ────[ NEXT ]─────────── GND
GPIO8 ────[ AUTO / MANUAL ]── GND
```

Both buttons use `INPUT_PULLUP` and 35 ms software debounce. GPIO4 advances patterns only when auto mode is paused. GPIO8 changes auto/manual mode while keeping the current pattern selected.

> **GPIO8 safety note.** GPIO8 is an ESP32-C3 strapping pin. Release its button while resetting or powering the board, and use a normally-open switch. If the SuperMini board does not already keep GPIO8 high at boot, add a 10 kΩ pull-up resistor from GPIO8 to 3V3. ESPressif documents GPIO2, GPIO8, and GPIO9 as strapping pins and explains the boot-mode interaction of GPIO8 and GPIO9.[3] [4]

## Web API

The interface is deliberately simple, which also makes it easy to build physical remote controls or integrations later.

| Endpoint | Example | Result |
|---|---|---|
| `/api/state` | `/api/state` | Current pattern and controller state in JSON. |
| `/api/frame` | `/api/frame` | Current 125-voxel colour framebuffer in JSON. |
| `/api/control` | `/api/control?pattern=7` | Select a pattern and switch to manual mode. |
| `/api/control` | `/api/control?brightness=80&speed=180&cycle=45` | Update live controls. |
| `/api/control` | `/api/control?auto=1` | Start automatic cycling. |
| `/api/control` | `/api/control?next=1` | Enter manual mode and advance one pattern. |
| `/api/control` | `/api/control?reseed=1` | Seed a fresh 3-D Life world. |
| `/api/control` | `/api/control?text=HELLO%20CUBE&bannerFont=5&bannerHue=96&bannerSpeed=150` | Set the banner message, font mode, colour, and scroll speed. |

## Credits

CubeFXWeb is inspired by the excellent [WS2812FX][1] project by kitesurfer1404; its cube renderer and browser firmware are purpose-built for SkeletonCube Patterns.

> **FEED ME , I'M POOR AND I MADE THIS FOR FREE — https://paypal.me/Mystereon**

Created by Dad (MysterEon) & Manus, 2026. Released under the repository MIT License.

## References

[1]: https://github.com/kitesurfer1404/WS2812FX "WS2812FX by kitesurfer1404"
[2]: https://github.com/FastLED/FastLED/blob/master/cookbook/core-concepts/power.md "FastLED power considerations"
[3]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html "Espressif ESP32-C3 GPIO summary"
[4]: https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html "Espressif ESP32-C3 boot-mode selection"
