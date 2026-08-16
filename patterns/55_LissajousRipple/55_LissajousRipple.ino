/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  LissajousRipple.ino — layer-three Lissajous wave for a 5×5×5 SkeletonCube
  Created by Dad (MysterEon) & Manus, 2026.

  The trace rests on layer 3 (z=2) and its moving crests rise only to layer 4
  or dip only to layer 2. Origin is bottom-rear-left; index = z*25 + y*5 + x.
*/

#include <FastLED.h>
#include <math.h>

constexpr uint8_t N = 5;
constexpr uint16_t NUM_LEDS = 125;
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) { return uint16_t(z) * 25 + uint16_t(y) * 5 + x; }
void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) {
  if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[indexFromXYZ(x, y, z)] += colour;
}

void renderLissajousRipple(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t sample = 0; sample < 20; ++sample) {
    const float a = (6.2831853f * sample / 20.0f) + t * 0.82f;
    const int8_t x = int8_t(roundf(2.0f + 1.78f * sinf(a * 3.0f)));
    const int8_t y = int8_t(roundf(2.0f + 1.78f * sinf(a * 2.0f + 1.05f)));
    const float ripple = sinf(a * 2.0f - t * 3.6f);
    const int8_t z = ripple > 0.42f ? 3 : (ripple < -0.42f ? 1 : 2);
    addVoxel(x, y, z, CHSV(155 + sample * 4, 225, 130 + uint8_t((ripple + 1.0f) * 62.0f)));
    if (z != 2) addVoxel(x, y, 2, CHSV(155 + sample * 4, 190, 42));
  }
}

void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
}

void loop() {
  renderLissajousRipple(millis() * 0.0036f);
  FastLED.show();
}
