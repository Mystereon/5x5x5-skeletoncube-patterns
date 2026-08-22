# FastLED Source Collection and CubeFX Interpretations

## Provenance and rule of separation

`third_party/FastLED` is a **pure Git submodule** pointing to the official [FastLED repository](https://github.com/FastLED/FastLED) at commit `c26c006d7b500dae3fd284a2c11ad56bbad08a8c`. Its 83 example directories are retained without CubeFX edits. They remain FastLED's source, copyright, licence, hardware assumptions, and credit trail.

CubeFX interpretations will be placed outside that submodule and titled `CubeFX Interpretation — <Official FastLED Example>`. An interpretation may be three-dimensional, hardware-profiled, or visually reimagined for the 5×5×5 cube, but it will never be described as, or substituted for, FastLED's original source.

> **Catalogue floor:** 57 embedded CubeFXWeb modes + 5 editable user slots + 83 pure FastLED entries + 83 separately labelled CubeFX interpretations = **228 entries** before any later additions.

## Pure official FastLED entries

| # | Official example directory | Pure source location | Planned CubeFX interpretation label |
|---:|---|---|---|
| 1 | AnalogOutput | `third_party/FastLED/examples/AnalogOutput` | CubeFX Interpretation — AnalogOutput |
| 2 | Animartrix | `third_party/FastLED/examples/Animartrix` | CubeFX Interpretation — Animartrix |
| 3 | AnimartrixRing | `third_party/FastLED/examples/AnimartrixRing` | CubeFX Interpretation — AnimartrixRing |
| 4 | Apa102 | `third_party/FastLED/examples/Apa102` | CubeFX Interpretation — Apa102 |
| 5 | Apa102HD | `third_party/FastLED/examples/Apa102HD` | CubeFX Interpretation — Apa102HD |
| 6 | Asio | `third_party/FastLED/examples/Asio` | CubeFX Interpretation — Asio |
| 7 | Async | `third_party/FastLED/examples/Async` | CubeFX Interpretation — Async |
| 8 | Audio | `third_party/FastLED/examples/Audio` | CubeFX Interpretation — Audio |
| 9 | AudioFftParity | `third_party/FastLED/examples/AudioFftParity` | CubeFX Interpretation — AudioFftParity |
| 10 | AudioInput | `third_party/FastLED/examples/AudioInput` | CubeFX Interpretation — AudioInput |
| 11 | AudioReactive | `third_party/FastLED/examples/AudioReactive` | CubeFX Interpretation — AudioReactive |
| 12 | AudioUrl | `third_party/FastLED/examples/AudioUrl` | CubeFX Interpretation — AudioUrl |
| 13 | AutoResearch | `third_party/FastLED/examples/AutoResearch` | CubeFX Interpretation — AutoResearch |
| 14 | BeatDetection | `third_party/FastLED/examples/BeatDetection` | CubeFX Interpretation — BeatDetection |
| 15 | Blink | `third_party/FastLED/examples/Blink` | CubeFX Interpretation — Blink |
| 16 | BlinkParallel | `third_party/FastLED/examples/BlinkParallel` | CubeFX Interpretation — BlinkParallel |
| 17 | Blur | `third_party/FastLED/examples/Blur` | CubeFX Interpretation — Blur |
| 18 | Blur2d | `third_party/FastLED/examples/Blur2d` | CubeFX Interpretation — Blur2d |
| 19 | BlurBenchmark | `third_party/FastLED/examples/BlurBenchmark` | CubeFX Interpretation — BlurBenchmark |
| 20 | Chromancer | `third_party/FastLED/examples/Chromancer` | CubeFX Interpretation — Chromancer |
| 21 | Codec | `third_party/FastLED/examples/Codec` | CubeFX Interpretation — Codec |
| 22 | ColorBoost | `third_party/FastLED/examples/ColorBoost` | CubeFX Interpretation — ColorBoost |
| 23 | ColorPalette | `third_party/FastLED/examples/ColorPalette` | CubeFX Interpretation — ColorPalette |
| 24 | ColorTemperature | `third_party/FastLED/examples/ColorTemperature` | CubeFX Interpretation — ColorTemperature |
| 25 | Corkscrew | `third_party/FastLED/examples/Corkscrew` | CubeFX Interpretation — Corkscrew |
| 26 | Cylon | `third_party/FastLED/examples/Cylon` | CubeFX Interpretation — Cylon |
| 27 | DemoReel100 | `third_party/FastLED/examples/DemoReel100` | CubeFX Interpretation — DemoReel100 |
| 28 | Downscale | `third_party/FastLED/examples/Downscale` | CubeFX Interpretation — Downscale |
| 29 | EaseInOut | `third_party/FastLED/examples/EaseInOut` | CubeFX Interpretation — EaseInOut |
| 30 | ElPanelReactive | `third_party/FastLED/examples/ElPanelReactive` | CubeFX Interpretation — ElPanelReactive |
| 31 | Esp8266Uart | `third_party/FastLED/examples/Esp8266Uart` | CubeFX Interpretation — Esp8266Uart |
| 32 | FestivalStick | `third_party/FastLED/examples/FestivalStick` | CubeFX Interpretation — FestivalStick |
| 33 | Fire2012 | `third_party/FastLED/examples/Fire2012` | CubeFX Interpretation — Fire2012 |
| 34 | Fire2012WithPalette | `third_party/FastLED/examples/Fire2012WithPalette` | CubeFX Interpretation — Fire2012WithPalette |
| 35 | Fire2023 | `third_party/FastLED/examples/Fire2023` | CubeFX Interpretation — Fire2023 |
| 36 | FireCylinder | `third_party/FastLED/examples/FireCylinder` | CubeFX Interpretation — FireCylinder |
| 37 | FireMatrix | `third_party/FastLED/examples/FireMatrix` | CubeFX Interpretation — FireMatrix |
| 38 | FirstLight | `third_party/FastLED/examples/FirstLight` | CubeFX Interpretation — FirstLight |
| 39 | FlowField | `third_party/FastLED/examples/FlowField` | CubeFX Interpretation — FlowField |
| 40 | Fx | `third_party/FastLED/examples/Fx` | CubeFX Interpretation — Fx |
| 41 | HD107 | `third_party/FastLED/examples/HD107` | CubeFX Interpretation — HD107 |
| 42 | HSVTest | `third_party/FastLED/examples/HSVTest` | CubeFX Interpretation — HSVTest |
| 43 | Json | `third_party/FastLED/examples/Json` | CubeFX Interpretation — Json |
| 44 | LuminescentGrand | `third_party/FastLED/examples/LuminescentGrand` | CubeFX Interpretation — LuminescentGrand |
| 45 | Luminova | `third_party/FastLED/examples/Luminova` | CubeFX Interpretation — Luminova |
| 46 | MoodRing | `third_party/FastLED/examples/MoodRing` | CubeFX Interpretation — MoodRing |
| 47 | Multiple | `third_party/FastLED/examples/Multiple` | CubeFX Interpretation — Multiple |
| 48 | Noise | `third_party/FastLED/examples/Noise` | CubeFX Interpretation — Noise |
| 49 | NoisePlayground | `third_party/FastLED/examples/NoisePlayground` | CubeFX Interpretation — NoisePlayground |
| 50 | NoisePlusPalette | `third_party/FastLED/examples/NoisePlusPalette` | CubeFX Interpretation — NoisePlusPalette |
| 51 | OTA | `third_party/FastLED/examples/OTA` | CubeFX Interpretation — OTA |
| 52 | Overclock | `third_party/FastLED/examples/Overclock` | CubeFX Interpretation — Overclock |
| 53 | Pacifica | `third_party/FastLED/examples/Pacifica` | CubeFX Interpretation — Pacifica |
| 54 | ParallelSPI | `third_party/FastLED/examples/ParallelSPI` | CubeFX Interpretation — ParallelSPI |
| 55 | PerfDisc | `third_party/FastLED/examples/PerfDisc` | CubeFX Interpretation — PerfDisc |
| 56 | PinMode | `third_party/FastLED/examples/PinMode` | CubeFX Interpretation — PinMode |
| 57 | Pintest | `third_party/FastLED/examples/Pintest` | CubeFX Interpretation — Pintest |
| 58 | Ports | `third_party/FastLED/examples/Ports` | CubeFX Interpretation — Ports |
| 59 | Pride2015 | `third_party/FastLED/examples/Pride2015` | CubeFX Interpretation — Pride2015 |
| 60 | RGBCalibrate | `third_party/FastLED/examples/RGBCalibrate` | CubeFX Interpretation — RGBCalibrate |
| 61 | RGBSetDemo | `third_party/FastLED/examples/RGBSetDemo` | CubeFX Interpretation — RGBSetDemo |
| 62 | RGBW | `third_party/FastLED/examples/RGBW` | CubeFX Interpretation — RGBW |
| 63 | RGBWColorimetric | `third_party/FastLED/examples/RGBWColorimetric` | CubeFX Interpretation — RGBWColorimetric |
| 64 | RGBWEmulated | `third_party/FastLED/examples/RGBWEmulated` | CubeFX Interpretation — RGBWEmulated |
| 65 | RGBWW | `third_party/FastLED/examples/RGBWW` | CubeFX Interpretation — RGBWW |
| 66 | RX | `third_party/FastLED/examples/RX` | CubeFX Interpretation — RX |
| 67 | Remote | `third_party/FastLED/examples/Remote` | CubeFX Interpretation — Remote |
| 68 | SIMD | `third_party/FastLED/examples/SIMD` | CubeFX Interpretation — SIMD |
| 69 | Sailboat | `third_party/FastLED/examples/Sailboat` | CubeFX Interpretation — Sailboat |
| 70 | SmartMatrix | `third_party/FastLED/examples/SmartMatrix` | CubeFX Interpretation — SmartMatrix |
| 71 | SpecialDrivers | `third_party/FastLED/examples/SpecialDrivers` | CubeFX Interpretation — SpecialDrivers |
| 72 | Spi | `third_party/FastLED/examples/Spi` | CubeFX Interpretation — Spi |
| 73 | Test | `third_party/FastLED/examples/Test` | CubeFX Interpretation — Test |
| 74 | TwinkleFox | `third_party/FastLED/examples/TwinkleFox` | CubeFX Interpretation — TwinkleFox |
| 75 | UITest | `third_party/FastLED/examples/UITest` | CubeFX Interpretation — UITest |
| 76 | WS2816 | `third_party/FastLED/examples/WS2816` | CubeFX Interpretation — WS2816 |
| 77 | WasmScreenCoords | `third_party/FastLED/examples/WasmScreenCoords` | CubeFX Interpretation — WasmScreenCoords |
| 78 | Wave | `third_party/FastLED/examples/Wave` | CubeFX Interpretation — Wave |
| 79 | Wave2d | `third_party/FastLED/examples/Wave2d` | CubeFX Interpretation — Wave2d |
| 80 | XYMatrix | `third_party/FastLED/examples/XYMatrix` | CubeFX Interpretation — XYMatrix |
| 81 | XYPath | `third_party/FastLED/examples/XYPath` | CubeFX Interpretation — XYPath |
| 82 | hydropack | `third_party/FastLED/examples/hydropack` | CubeFX Interpretation — hydropack |
| 83 | wasm | `third_party/FastLED/examples/wasm` | CubeFX Interpretation — wasm |

## Working with the source collection

Clone this project with `git clone --recurse-submodules` or run `git submodule update --init --recursive` after cloning. The source files under `third_party/FastLED` may use their original board, LED-chipset, pin, display, transport, or audio hardware assumptions. They are intentionally not reworked in place.

The future web authoring editor and CubeFX interpretation collection live in CubeFX-owned source outside `third_party/FastLED`. Their output target is always the existing 125-voxel cube plus optional 12-pixel rear ring, using the ESP32-S3 Zero hardware profile.
