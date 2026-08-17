/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  ZarchVoxelDefender.ino — self-playing 5×5×5 terrain-defence scene
  Created by Dad (MysterEon) & Manus, 2026.

  A two-minute miniature story: patrol, contact, crossfire, finale and recovery.
  GPIO2 short press restarts the full scene; GPIO4 adds a support shot.
  ESP32-S3 Zero enclosure map: DATA=GPIO6, B1=GPIO2, B2=GPIO4.
*/

#include <FastLED.h>
#include <esp_system.h>

constexpr uint8_t N = 5, DATA_PIN = 6, PRIMARY_BUTTON_PIN = 2, SECONDARY_BUTTON_PIN = 4;
constexpr uint16_t NUM_LEDS = N * N * N;
constexpr uint8_t ENEMIES = 2;
CRGB leds[NUM_LEDS], terrainLut[NUM_LEDS];
uint16_t idx(uint8_t x, uint8_t y, uint8_t z) { return uint16_t(z) * 25 + uint16_t(y) * 5 + x; }
void voxel(int8_t x, int8_t y, int8_t z, const CRGB &c) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[idx(x, y, z)] = c; }
void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &c) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[idx(x, y, z)] += c; }

constexpr uint8_t PATROL = 0, CONTACT = 1, CROSSFIRE = 2, FINALE = 3, RECOVERY = 4;
int8_t craftX = 2, enemyX[ENEMIES] = {0, 4}, enemyY[ENEMIES] = {4, 3};
int8_t shotX = 2, shotY = 1, impactX = 2, impactY = 2;
bool enemyAlive[ENEMIES] = {false, false}, shotActive = false, lastPrimary = HIGH, lastSecondary = HIGH;
uint8_t impactLife = 0;
uint8_t beat = PATROL;
uint32_t beatAt, craftAt, enemyAt, shotAt, autoShotAt;

void buildTerrainLut() {
  fill_solid(terrainLut, NUM_LEDS, CRGB::Black);
  for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    uint8_t height = random8() < 48 ? 1 : 0; if (random8() < 18) height = 2;
    for (uint8_t z = 0; z <= height; ++z) {
      const uint8_t green = 52 + z * 28 + random8(22);
      terrainLut[idx(x, y, z)] = z == height ? CRGB(18, green + 34, 9) : CRGB(10, green, 4);
    }
  }
}
void setEnemy(uint8_t slot, int8_t x, int8_t y, bool alive) { enemyX[slot] = x; enemyY[slot] = y; enemyAlive[slot] = alive; }
void enterBeat(uint8_t next) {
  beat = next; beatAt = millis(); shotActive = false;
  if (next == PATROL || next == RECOVERY) { setEnemy(0, 0, 4, false); setEnemy(1, 4, 4, false); }
  else if (next == CONTACT) { setEnemy(0, 0, 4, true); setEnemy(1, 4, 4, false); }
  else if (next == CROSSFIRE) { setEnemy(0, 0, 4, true); setEnemy(1, 4, 4, true); }
  else { setEnemy(0, 2, 4, true); setEnemy(1, 4, 3, true); }
}
void resetScene() { buildTerrainLut(); craftX = 2; impactLife = 0; craftAt = enemyAt = shotAt = autoShotAt = millis(); enterBeat(PATROL); }
void fireShot() { if (!shotActive) { shotActive = true; shotX = craftX; shotY = 1; } }
int8_t leadEnemy() { for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i]) return i; return -1; }
void nextBeat() { if (beat == PATROL) enterBeat(CONTACT); else if (beat == CONTACT) enterBeat(CROSSFIRE); else if (beat == CROSSFIRE) enterBeat(FINALE); else if (beat == FINALE) enterBeat(RECOVERY); else resetScene(); }

void updateScene() {
  const uint32_t now = millis();
  const uint32_t duration = beat == PATROL ? 15000UL : beat == CONTACT ? 23000UL : beat == CROSSFIRE ? 30000UL : beat == FINALE ? 22000UL : 8000UL;
  if (now - beatAt >= duration) nextBeat();
  const int8_t lead = leadEnemy(); const int8_t desired = lead >= 0 ? enemyX[lead] : int8_t((now / 2100UL) % N);
  if (now - craftAt >= 430U) { craftAt = now; if (craftX < desired) ++craftX; else if (craftX > desired) --craftX; }
  if (now - enemyAt >= (beat == CROSSFIRE ? 510U : 820U)) {
    enemyAt = now;
    for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i]) {
      if (enemyX[i] < craftX) ++enemyX[i]; else if (enemyX[i] > craftX) --enemyX[i];
      if (--enemyY[i] < 0) { impactX = enemyX[i]; impactY = 0; impactLife = 10; enterBeat(RECOVERY); }
    }
  }
  const uint16_t pause = beat == PATROL ? 2100U : beat == CROSSFIRE ? 850U : 1150U;
  if (!shotActive && now - autoShotAt >= pause && (lead < 0 || craftX == enemyX[lead])) { autoShotAt = now; fireShot(); }
  if (shotActive && now - shotAt >= 105U) { shotAt = now; if (++shotY >= N) shotActive = false; }
  if (shotActive) for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i] && shotX == enemyX[i] && shotY == enemyY[i]) { enemyAlive[i] = false; shotActive = false; impactX = enemyX[i]; impactY = enemyY[i]; impactLife = 8; }
  if (impactLife) --impactLife;
  if (!enemyAlive[0] && !enemyAlive[1] && beat != PATROL && beat != RECOVERY) nextBeat();
}

void renderScene() {
  ::memcpy(leds, terrainLut, sizeof(leds));
  const uint8_t pulse = sin8(millis() / 9); addVoxel((millis() / 680U) % N, 4, 4, CRGB(0, pulse / 7, pulse / 5));
  if (beat == RECOVERY) addVoxel(2, 4, 4, CRGB(0, 40 + pulse / 4, 35 + pulse / 3));
  voxel(craftX, 1, 4, CRGB(110, 255, 40)); voxel(craftX - 1, 0, 3, CRGB(0, 105, 105)); voxel(craftX + 1, 0, 3, CRGB(0, 105, 105)); voxel(craftX, 0, 3, CRGB(180, 255, 80));
  for (uint8_t i = 0; i < ENEMIES; ++i) if (enemyAlive[i]) { voxel(enemyX[i], enemyY[i], 3, CRGB(255, 36, 8)); voxel(enemyX[i], enemyY[i], 2, CRGB(100, 9, 2)); }
  if (shotActive) voxel(shotX, shotY, 4, CRGB(210, 255, 255));
  if (impactLife) { const uint8_t value = impactLife * 30; addVoxel(impactX, impactY, 3, CRGB(value, value / 3, 0)); addVoxel(impactX - 1, impactY, 3, CRGB(value / 2, value / 7, 0)); addVoxel(impactX + 1, impactY, 3, CRGB(value / 2, value / 7, 0)); addVoxel(impactX, impactY - 1, 3, CRGB(value / 2, value / 7, 0)); addVoxel(impactX, impactY + 1, 3, CRGB(value / 2, value / 7, 0)); }
}

void setup() { FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); FastLED.setBrightness(100); FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500); randomSeed(esp_random()); pinMode(PRIMARY_BUTTON_PIN, INPUT_PULLUP); pinMode(SECONDARY_BUTTON_PIN, INPUT_PULLUP); resetScene(); }
void loop() { const bool primary = digitalRead(PRIMARY_BUTTON_PIN), secondary = digitalRead(SECONDARY_BUTTON_PIN); if (lastPrimary && !primary) resetScene(); if (lastSecondary && !secondary) fireShot(); lastPrimary = primary; lastSecondary = secondary; updateScene(); renderScene(); FastLED.show(); delay(12); }
