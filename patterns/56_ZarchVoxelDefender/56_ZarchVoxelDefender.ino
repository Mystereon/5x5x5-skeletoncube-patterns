/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  ZarchVoxelDefender.ino — a compact 5×5×5 FastLED terrain-defence scene
  Created by Dad (MysterEon) & Manus, 2026.

  GPIO2 short press: reseed the cached terrain.
  GPIO4 short press: fire a forward shot.
  For the ESP32-S3 Zero enclosure-friendly map: DATA=GPIO6, B1=GPIO2, B2=GPIO4.
*/

#include <FastLED.h>
#include <esp_system.h>

constexpr uint8_t N = 5;
constexpr uint16_t NUM_LEDS = N * N * N;
constexpr uint8_t DATA_PIN = 6;
constexpr uint8_t PRIMARY_BUTTON_PIN = 2;
constexpr uint8_t SECONDARY_BUTTON_PIN = 4;
CRGB leds[NUM_LEDS];

uint16_t idx(uint8_t x, uint8_t y, uint8_t z) { return uint16_t(z) * 25 + uint16_t(y) * 5 + x; }
void voxel(int8_t x, int8_t y, int8_t z, const CRGB &c) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[idx(x, y, z)] = c; }
void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &c) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[idx(x, y, z)] += c; }

constexpr uint8_t ENEMIES = 2;
CRGB terrainLut[NUM_LEDS];
int8_t craftX = 2, enemyX[ENEMIES] = {0, 4}, enemyY[ENEMIES] = {4, 3};
bool enemyAlive[ENEMIES] = {true, true}, shotActive = false;
int8_t shotX = 2, shotY = 1, impactX = 2, impactY = 2;
uint8_t impactLife = 0;
uint32_t craftAt, enemyAt, shotAt;
bool lastPrimary = HIGH, lastSecondary = HIGH;

void buildTerrainLut() {
  fill_solid(terrainLut, NUM_LEDS, CRGB::Black);
  for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    uint8_t h = random8() < 48 ? 1 : 0;
    if (random8() < 18) h = 2;
    for (uint8_t z = 0; z <= h; ++z) {
      const uint8_t green = 52 + z * 28 + random8(22);
      terrainLut[idx(x, y, z)] = z == h ? CRGB(18, green + 34, 9) : CRGB(10, green, 4);
    }
  }
}

void resetScene() {
  buildTerrainLut(); craftX = 2;
  enemyX[0] = 0; enemyY[0] = 4; enemyX[1] = 4; enemyY[1] = 3;
  enemyAlive[0] = enemyAlive[1] = true; shotActive = false; impactLife = 0;
  craftAt = enemyAt = shotAt = millis();
}

void fireShot() { if (!shotActive) { shotActive = true; shotX = craftX; shotY = 1; } }

void updateScene() {
  const uint32_t now = millis();
  if (now - craftAt >= 430) { craftAt = now; craftX = (craftX + 1) % N; }
  if (now - enemyAt >= 570) {
    enemyAt = now;
    for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i]) {
      if (enemyX[i] < craftX) ++enemyX[i]; else if (enemyX[i] > craftX) --enemyX[i];
      if (--enemyY[i] < 0) { resetScene(); return; }
    }
  }
  if (shotActive && now - shotAt >= 105) { shotAt = now; if (++shotY >= N) shotActive = false; }
  if (shotActive) for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i] && shotX == enemyX[i] && shotY == enemyY[i]) {
    enemyAlive[i] = false; shotActive = false; impactX = enemyX[i]; impactY = enemyY[i]; impactLife = 8;
  }
  if (impactLife) --impactLife;
  if (!enemyAlive[0] && !enemyAlive[1]) resetScene();
}

void renderScene() {
  ::memcpy(leds, terrainLut, sizeof(leds));
  voxel(craftX, 1, 4, CRGB(110, 255, 40)); voxel(craftX - 1, 0, 3, CRGB(0, 105, 105));
  voxel(craftX + 1, 0, 3, CRGB(0, 105, 105)); voxel(craftX, 0, 3, CRGB(180, 255, 80));
  for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i]) { voxel(enemyX[i], enemyY[i], 3, CRGB(255, 36, 8)); voxel(enemyX[i], enemyY[i], 2, CRGB(100, 9, 2)); }
  if (shotActive) voxel(shotX, shotY, 4, CRGB(210, 255, 255));
  if (impactLife) { const uint8_t v = impactLife * 30; addVoxel(impactX, impactY, 3, CRGB(v, v / 3, 0)); addVoxel(impactX - 1, impactY, 3, CRGB(v / 2, v / 7, 0)); addVoxel(impactX + 1, impactY, 3, CRGB(v / 2, v / 7, 0)); }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); FastLED.setBrightness(100);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500); randomSeed(esp_random());
  pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP); pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP); resetScene();
}

void loop() {
  const bool primary = digitalRead(PRIMARY_BUTTON_PIN), secondary = digitalRead(SECONDARY_BUTTON_PIN);
  if (lastPrimary && !primary) resetScene(); if (lastSecondary && !secondary) fireShot();
  lastPrimary = primary; lastSecondary = secondary;
  updateScene(); renderScene(); FastLED.show(); delay(12);
}
