#pragma once

#include <Arduino.h>

// Every official FastLED example directory in the pinned pure-source submodule
// receives one separate CubeFX interpretation. These are CubeFX-owned renderings
// inspired by the source names; the original source remains unmodified under
// third_party/FastLED and is never substituted by this registry.
constexpr uint8_t FASTLED_INTERPRETATION_COUNT = 83;
constexpr uint8_t FASTLED_INTERPRETATION_CANONICAL_FIRST = 80;

enum FastLEDInterpretationProfile : uint8_t {
  FLX_PRISM, FLX_MOTION, FLX_FIRE, FLX_FIELD, FLX_WAVE,
  FLX_AUDIO, FLX_PALETTE, FLX_SIGNAL
};

static const char *const FASTLED_INTERPRETATION_SOURCES[FASTLED_INTERPRETATION_COUNT] = {
  "AnalogOutput", "Animartrix", "AnimartrixRing", "Apa102", "Apa102HD", "Asio", "Async", "Audio", "AudioFftParity", "AudioInput", "AudioReactive", "AudioUrl", "AutoResearch", "BeatDetection", "Blink", "BlinkParallel", "Blur", "Blur2d", "BlurBenchmark", "Chromancer", "Codec", "ColorBoost", "ColorPalette", "ColorTemperature", "Corkscrew", "Cylon", "DemoReel100", "Downscale", "EaseInOut", "ElPanelReactive", "Esp8266Uart", "FestivalStick", "Fire2012", "Fire2012WithPalette", "Fire2023", "FireCylinder", "FireMatrix", "FirstLight", "FlowField", "Fx", "HD107", "HSVTest", "Json", "LuminescentGrand", "Luminova", "MoodRing", "Multiple", "Noise", "NoisePlayground", "NoisePlusPalette", "OTA", "Overclock", "Pacifica", "ParallelSPI", "PerfDisc", "PinMode", "Pintest", "Ports", "Pride2015", "RGBCalibrate", "RGBSetDemo", "RGBW", "RGBWColorimetric", "RGBWEmulated", "RGBWW", "RX", "Remote", "SIMD", "Sailboat", "SmartMatrix", "SpecialDrivers", "Spi", "Test", "TwinkleFox", "UITest", "WS2816", "WasmScreenCoords", "Wave", "Wave2d", "XYMatrix", "XYPath", "hydropack", "wasm"
};

inline FastLEDInterpretationProfile fastLEDInterpretationProfile(uint8_t index) {
  const char *name = FASTLED_INTERPRETATION_SOURCES[index];
  if (strstr(name, "Fire")) return FLX_FIRE;
  if (strstr(name, "Audio") || strstr(name, "Beat")) return FLX_AUDIO;
  if (strstr(name, "Wave") || strstr(name, "Pacifica") || strstr(name, "Sail")) return FLX_WAVE;
  if (strstr(name, "Noise") || strstr(name, "Flow")) return FLX_FIELD;
  if (strstr(name, "Color") || strstr(name, "HSV") || strstr(name, "RGB") || strstr(name, "Chrom") || strstr(name, "Mood")) return FLX_PALETTE;
  if (strstr(name, "Blink") || strstr(name, "Blur") || strstr(name, "Cylon") || strstr(name, "Corkscrew") || strstr(name, "Twinkle")) return FLX_MOTION;
  if (strstr(name, "Test") || strstr(name, "Pin") || strstr(name, "Port") || strstr(name, "Driver") || strstr(name, "SPI") || strstr(name, "Apa") || strstr(name, "HD") || strstr(name, "WS")) return FLX_SIGNAL;
  return FLX_PRISM;
}
