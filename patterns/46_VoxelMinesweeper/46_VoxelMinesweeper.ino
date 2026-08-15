/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  VoxelMinesweeper.ino — directly uploadable 5×5×5 FastLED demo
  Created by Dad (MysterEon) & Manus, 2026.

  Falling probes strike illuminated base pads and burst into orange 3×3×3 impacts.
*/
#include <FastLED.h>
#include <esp_system.h>

constexpr uint8_t N = 5, BRIGHTNESS = 100;
constexpr uint16_t NUM_LEDS = N * N * N;
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) { return uint16_t(z) * 25 + uint16_t(y) * 5 + x; }
void setVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[indexFromXYZ(x, y, z)] = colour; }
void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[indexFromXYZ(x, y, z)] += colour; }

const uint8_t targets[5][2] = {{0,1},{1,4},{2,2},{3,0},{4,3}};
uint8_t targetIndex = 0;
int8_t probeX = 0, probeY = 1, probeZ = 4, burstX = 0, burstY = 1;
uint32_t lastStepAt = 0, burstAt = 0;
bool bursting = false;
void resetProbe() { probeX = targets[targetIndex][0]; probeY = targets[targetIndex][1]; probeZ = 4; bursting = false; }
void renderMinesweeper() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t i = 0; i < 5; ++i) setVoxel(targets[i][0], targets[i][1], 0, i == targetIndex ? CRGB(72,230,35) : CRGB(12,60,12));
  if (bursting) {
    const uint16_t age = millis() - burstAt;
    if (age >= 540) { targetIndex = (targetIndex + 1) % 5; resetProbe(); }
    else { const uint8_t value = 255 - uint32_t(age) * 255 / 540; for (int8_t z = 0; z <= 2; ++z) for (int8_t y = -1; y <= 1; ++y) for (int8_t x = -1; x <= 1; ++x) addVoxel(burstX+x, burstY+y, z, CHSV(18+(x+y+z)*2,250,value)); return; }
  }
  if (millis() - lastStepAt >= 260) { lastStepAt = millis(); --probeZ; if (probeZ < 0) { burstX = probeX; burstY = probeY; burstAt = millis(); bursting = true; } }
  setVoxel(probeX, probeY, probeZ, CRGB(255,165,18)); addVoxel(probeX, probeY, probeZ+1, CRGB(95,20,0));
}
void setup() { FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); FastLED.setBrightness(BRIGHTNESS); FastLED.setMaxPowerInVoltsAndMilliamps(5,1500); randomSeed(esp_random()); }
void loop() { renderMinesweeper(); FastLED.show(); }
