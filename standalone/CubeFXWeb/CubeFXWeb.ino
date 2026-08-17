/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  CubeFXWeb.ino — browser-controlled 5 x 5 x 5 WS2812B cube pattern app
  Created by Dad (MysterEon) & Manus, 2026.

  A cube-aware, lightweight ESP32-C3 controller inspired by the immediacy of
  WS2812FX-style web controls. It uses only Arduino-ESP32's built-in WiFi and
  WebServer classes plus FastLED: no AsyncWebServer, filesystem, or cloud.

  FIRST BOOT
    1. Upload this sketch to an ESP32-C3 SuperMini.
    2. Join Wi-Fi AP: CubeFX-5x5x5  /  cubecontrol
    3. Browse to: http://192.168.4.1

  Optional home Wi-Fi
    Put your credentials in WIFI_SSID and WIFI_PASSWORD below. If connection
    fails, CubeFX falls back to the local access point above.

  Cube map
    origin (0,0,0) = bottom, rear, left
    x: left -> right, y: rear -> front, z: bottom -> top
    index = z * 25 + y * 5 + x

  Optional buttons (normally-open switches to GND)
    GPIO4 / Button 1: short = pattern-aware primary action; long = banner mode.
    GPIO8 / Button 2: short = pattern-aware secondary action; long = next pattern.

  GPIO8 is an ESP32-C3 boot-strapping pin: RELEASE it while resetting or
  powering up. Use a 10 kΩ pull-up from GPIO8 to 3V3 if your board does not
  already hold it high. Some SuperMini variants attach a status LED to GPIO8.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <FastLED.h>
#include <Preferences.h>
#include <esp_system.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "CubeFXConfig.h"

// -----------------------------------------------------------------------------
// Wi-Fi setup
// -----------------------------------------------------------------------------
const char *WIFI_SSID = "";       // Fill in to join an existing Wi-Fi network.
const char *WIFI_PASSWORD = "";   // Leave both blank for access-point-only use.
const char *AP_SSID = "CubeFX-5x5x5";
const char *AP_PASSWORD = "cubecontrol";  // 8+ characters required by WPA2.

WebServer web(80);
bool usingAccessPoint = true;
String networkAddress = "192.168.4.1";

// Android CubeFX BLE controller contract. The same UUIDs are defined in
// cubefx_ble_controller/lib/ble-protocol.ts. Commands are compact JSON over a
// write-without-response characteristic; status is available via read/notify.
constexpr char CUBEFX_BLE_SERVICE_UUID[] = "6c75a300-7b1d-4f29-a221-000000000001";
constexpr char CUBEFX_BLE_COMMAND_UUID[] = "6c75a300-7b1d-4f29-a221-000000000002";
constexpr char CUBEFX_BLE_STATUS_UUID[] = "6c75a300-7b1d-4f29-a221-000000000003";
BLECharacteristic *bleStatusCharacteristic = nullptr;
BLEServer *bleServer = nullptr;

// -----------------------------------------------------------------------------
// Cube configuration and physical mapper
// -----------------------------------------------------------------------------
constexpr uint8_t COLUMNS = CUBEFX_COLUMNS;
constexpr uint8_t ROWS = CUBEFX_ROWS;
constexpr uint8_t LAYERS = CUBEFX_LAYERS;
constexpr uint8_t N = COLUMNS;
constexpr uint16_t NUM_LEDS = CUBEFX_TOTAL_LEDS;
static_assert(COLUMNS == 5 && ROWS == 5 && LAYERS == 5,
  "CubeFXWeb v0.5 patterns currently require a 5x5x5 cube. Use the Android setup total as a planning value for other dimensions.");
#define DATA_PIN CUBEFX_LED_DATA_PIN
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t DEFAULT_BRIGHTNESS = 100;
constexpr bool SWAP_XY = false;
constexpr bool FLIP_X = false;
constexpr bool FLIP_Y = false;
constexpr bool FLIP_Z = false;
constexpr bool SERPENTINE_ROWS = false;
constexpr bool SERPENTINE_LAYERS = false;

CRGB leds[NUM_LEDS];
uint8_t brightness = DEFAULT_BRIGHTNESS;

// Geometry is fixed for a 5×5×5 cube. Build these once at boot instead of
// recalculating roots and angles inside 120 FPS render loops.
struct VoxelGeometry {
  uint8_t centreRadius16;
  uint8_t moonRadius16;
  uint8_t moonShade;
  uint8_t blackHoleAngle;
  int8_t z2;
};
VoxelGeometry voxelGeometry[NUM_LEDS];
uint8_t explosionRadius16[2][NUM_LEDS];
uint8_t stargateRadius16[N][N];

void buildGeometryLUT() {
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    const uint16_t i = indexFromXYZ(x, y, z);
    const float dx = x - 2.0f, dy = y - 2.0f, dz = z - 2.0f;
    const float centreRadius = sqrtf(dx * dx + dy * dy + dz * dz);
    const float moonDx = x - 1.30f, moonDy = y - 2.15f, moonDz = z - 3.0f;
    const float moonRadius = sqrtf(moonDx * moonDx + moonDy * moonDy + moonDz * moonDz);
    voxelGeometry[i].centreRadius16 = uint8_t(centreRadius * 16.0f + 0.5f);
    voxelGeometry[i].moonRadius16 = uint8_t(moonRadius * 16.0f + 0.5f);
    voxelGeometry[i].moonShade = moonRadius < 2.15f ? uint8_t(220 - moonRadius * 56 - max(0.0f, moonDx) * 34) : 0;
    voxelGeometry[i].blackHoleAngle = uint8_t((atan2f(dy, dx) + 3.1415926f) * 255.0f / 6.2831853f);
    voxelGeometry[i].z2 = int8_t(z * 2 - 4);
    for (uint8_t centre = 0; centre < 2; ++centre) {
      const float ex = x - (centre ? 3.0f : 2.0f);
      explosionRadius16[centre][i] = uint8_t(sqrtf(ex * ex + dy * dy + dz * dz) * 16.0f + 0.5f);
    }
  }
  for (uint8_t z = 0; z < N; ++z) for (uint8_t x = 0; x < N; ++x) {
    const float dx = x - 2.0f, dz = z - 2.3f;
    stargateRadius16[x][z] = uint8_t(sqrtf(dx * dx + dz * dz) * 16.0f + 0.5f);
  }
}

uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) {
  uint8_t px = x;
  uint8_t py = y;
  uint8_t pz = z;
  if (SWAP_XY) {
    const uint8_t temp = px;
    px = py;
    py = temp;
  }
  if (FLIP_X) px = N - 1 - px;
  if (FLIP_Y) py = N - 1 - py;
  if (FLIP_Z) pz = N - 1 - pz;
  if (SERPENTINE_ROWS && (py & 1)) px = N - 1 - px;
  if (SERPENTINE_LAYERS && (pz & 1)) py = N - 1 - py;
  return uint16_t(pz) * N * N + uint16_t(py) * N + px;
}

void setVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) {
  if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) {
    leds[indexFromXYZ(x, y, z)] = colour;
  }
}

void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) {
  if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) {
    leds[indexFromXYZ(x, y, z)] += colour;
  }
}

// -----------------------------------------------------------------------------
// Pattern engine
// -----------------------------------------------------------------------------
enum Pattern : uint8_t {
  PATTERN_VECTOR_CUBE,
  PATTERN_MATRIX_RAIN,
  PATTERN_PLASMA,
  PATTERN_FIRE,
  PATTERN_SPIRALS,
  PATTERN_COMETS,
  PATTERN_PONG,
  PATTERN_LIFE,
  PATTERN_CLOUDS,
  PATTERN_GLITTER,
  PATTERN_CORNER_CUBES,
  PATTERN_BANNER,
  PATTERN_BULLET_WALL,
  PATTERN_PADDED_CELL,
  PATTERN_BLOCK_RUN,
  PATTERN_PARALLAX,
  PATTERN_TRENCH_RUN,
  PATTERN_RUNNING_LEGS,
  PATTERN_FAIRY_BOX,
  PATTERN_AQUARIUM,
  PATTERN_PYRAMID,
  PATTERN_MATRIX_DRIFT,
  PATTERN_INTENSE_FIRE,
  PATTERN_BLUE_FIRE,
  PATTERN_EXPLOSIONS,
  PATTERN_FIREWORKS,
  PATTERN_PIXEL_PASTURE,
  PATTERN_RED_MATRIX_RAIN,
  PATTERN_MINESWEEPER,
  PATTERN_MOON_STARS,
  PATTERN_NIXIE_TUBE,
  PATTERN_BLACK_HOLE,
  PATTERN_STARGATE,
  PATTERN_DEFENDER,
  PATTERN_CHEQUERBOARD,
  PATTERN_PUZZLE_CUBE,
  PATTERN_RUBIKS_CUBE,
  PATTERN_LISSAJOUS_RIPPLE,
  PATTERN_ZARCH,
  PATTERN_COUNT
};

const char *const patternNames[PATTERN_COUNT] = {
  "Red Vector Cube", "3-D Matrix Rain", "Neon Plasma", "Volume Fire",
  "Twin Spirals", "Wrapping Comets", "Single-player Pong", "Conway 3-D Life",
  "Cloud Volume", "White Glitter", "Corner Cubes", "3x5 Perimeter Banner",
  "Bullet Wall", "Padded Cell", "Block Run", "Parallax Starfield", "Trench Run",
  "Running Legs", "Fairies in Green Box", "Orange Fish Tank", "Three-Layer Pyramid", "Matrix Drift",
  "Intense Fire", "Magical Blue Fire", "Explosions", "Launching Fireworks", "Pixel Pasture", "Red Matrix Rain",
  "Voxel Minesweeper", "Big Moon & Stars", "Nixie Tube", "Black Hole Vortex", "Stargate Dial-Up",
  "3-D Defender", "3-D Chequerboard", "Hellraiser Puzzle Cube", "3-D Rubik's Cube", "Lissajous Layer Ripple", "Zarch: Voxel Defender"
};

Pattern currentPattern = PATTERN_VECTOR_CUBE;
bool autoCycle = true;
uint8_t speedControl = 150;        // 1 slow .. 255 fast motion scaling
uint8_t frameRateLimit = 120;       // 30 .. 120 FPS render cap; default is performance mode
uint32_t cycleDurationMs = 30000;   // 5 .. 120 seconds from the browser
uint32_t patternStartedAt = 0;
uint32_t lastFrameAt = 0;

uint16_t frameIntervalMs() {
  return 1000U / frameRateLimit;
}

float effectTime() {
  // Motion speed is independent of render cadence. This is roughly 2.4× faster
  // at the existing default value and reaches about 2.5× the former top speed.
  return millis() * 0.001f * (0.15f + speedControl / 48.0f);
}

void renderVectorCube(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const float c = cosf(t * 0.8f);
  const float s = sinf(t * 0.8f);
  for (int8_t z = 0; z < N; ++z) {
    for (int8_t y = 0; y < N; ++y) {
      for (int8_t x = 0; x < N; ++x) {
        const float fx = x - 2.0f;
        const float fy = y - 2.0f;
        const float fz = z - 2.0f;
        const float rx = fx * c - fy * s;
        const float ry = fx * s + fy * c;
        const float edge = max(max(fabsf(rx), fabsf(ry)), fabsf(fz));
        if (edge > 0.75f && edge < 1.32f) setVoxel(x, y, z, CRGB::Red);
      }
    }
  }
}

// Each (x,y) stream has a deliberately irregular phase so it falls vertically
// down Z without producing a travelling diagonal plane across the cube.
const uint8_t MATRIX_COLUMN_PHASE[N][N] = {
  {0, 6, 2, 8, 3},
  {5, 1, 7, 4, 0},
  {3, 8, 5, 1, 6},
  {7, 2, 0, 6, 4},
  {1, 5, 8, 3, 7}
};

void renderMatrixRain(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 62);
  const uint8_t frame = uint8_t(t * 7.0f);
  for (uint8_t x = 0; x < N; ++x) {
    for (uint8_t y = 0; y < N; ++y) {
      const int8_t head = (MATRIX_COLUMN_PHASE[y][x] + frame) % (N + 4) - 2;
      // Deep emerald body; the head is yellow-green, never white.
      addVoxel(x, y, head, CHSV(76, 255, 255));
      addVoxel(x, y, head - 1, CRGB(0, 118, 16));
      addVoxel(x, y, head - 2, CRGB(0, 42, 5));
      addVoxel(x, y, head - 3, CRGB(0, 16, 2));
    }
  }
}

void renderRedMatrixRain(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 70);
  const uint8_t frame = uint8_t(t * 8.0f);
  for (uint8_t x = 0; x < N; ++x) {
    for (uint8_t y = 0; y < N; ++y) {
      const int8_t head = (MATRIX_COLUMN_PHASE[y][x] + frame) % (N + 5) - 2;
      // A hot red leading point followed by successively deeper crimson trails.
      addVoxel(x, y, head, CRGB(255, 42, 12));
      addVoxel(x, y, head - 1, CRGB(155, 5, 2));
      addVoxel(x, y, head - 2, CRGB(70, 0, 0));
      addVoxel(x, y, head - 3, CRGB(24, 0, 0));
    }
  }
}

void renderMatrixDrift(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 52);
  const uint8_t frame = uint8_t(t * 6.0f);
  // Preserve the original diagonal wash intentionally as a separate effect.
  for (uint8_t x = 0; x < N; ++x) for (uint8_t y = 0; y < N; ++y) {
    const int8_t head = (frame + x * 2 + y * 3) % (N + 5) - 2;
    addVoxel(x, y, head, CHSV(80, 245, 235));
    addVoxel(x, y, head - 1, CRGB(0, 105, 12));
    addVoxel(x, y, head - 2, CRGB(0, 36, 4));
    addVoxel(x, y, head - 3, CRGB(0, 10, 1));
  }
}

void renderPlasma(float t) {
  const uint8_t phase = uint8_t(t * 37.0f);
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const uint16_t wave = uint16_t(sin8(x * 43 + phase)) + sin8(y * 33 - phase) + sin8(z * 49 + phase / 2);
        const uint8_t folded = wave / 3;
        const uint8_t hue = folded + phase / 2;
        const uint8_t value = 105 + scale8(sin8(folded * 2 + phase), 150);
        setVoxel(x, y, z, CHSV(hue, 255, value));
      }
    }
  }
}

uint8_t lissajousHue = 155;

void renderLissajousRipple(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // The calm trace lives on layer 3 (z=2). Only travelling wave crests lift
  // into layer 4 or dip into layer 2; the pattern never fills the whole cube.
  constexpr uint8_t SAMPLES = 20;
  for (uint8_t sample = 0; sample < SAMPLES; ++sample) {
    const float a = (6.2831853f * sample / SAMPLES) + t * 0.82f;
    const int8_t x = int8_t(roundf(2.0f + 1.78f * sinf(a * 3.0f)));
    const int8_t y = int8_t(roundf(2.0f + 1.78f * sinf(a * 2.0f + 1.05f)));
    const float ripple = sinf(a * 2.0f - t * 3.6f);
    const int8_t z = ripple > 0.42f ? 3 : (ripple < -0.42f ? 1 : 2);
    const uint8_t value = 130 + uint8_t((ripple + 1.0f) * 62.0f);
    addVoxel(x, y, z, CHSV(lissajousHue + sample * 4, 225, value));
    if (z != 2) addVoxel(x, y, 2, CHSV(lissajousHue + sample * 4, 190, 42));
  }
}

// Zarch: Voxel Defender -------------------------------------------------------
// Terrain is a complete 125-voxel colour cache. It is rebuilt only when the
// scene reseeds; every animation frame merely copies the cached scene and adds
// the moving craft, landers, laser, and a small impact spark.
constexpr uint8_t ZARCH_ENEMY_COUNT = 2;
CRGB zarchTerrainLut[NUM_LEDS];
int8_t zarchCraftX = 2;
int8_t zarchEnemyX[ZARCH_ENEMY_COUNT] = {0, 4};
int8_t zarchEnemyY[ZARCH_ENEMY_COUNT] = {4, 3};
bool zarchEnemyAlive[ZARCH_ENEMY_COUNT] = {true, true};
bool zarchShotActive = false;
int8_t zarchShotX = 2;
int8_t zarchShotY = 1;
uint8_t zarchImpactLife = 0;
int8_t zarchImpactX = 2;
int8_t zarchImpactY = 2;
uint32_t zarchLastCraftStepAt = 0;
uint32_t zarchLastEnemyStepAt = 0;
uint32_t zarchLastShotStepAt = 0;

void buildZarchTerrainLUT() {
  fill_solid(zarchTerrainLut, NUM_LEDS, CRGB::Black);
  for (uint8_t y = 0; y < N; ++y) {
    for (uint8_t x = 0; x < N; ++x) {
      // The height field is generated once. The render loop never asks for a
      // root, sine, noise sample, or terrain coordinate conversion.
      uint8_t height = random8() < 48 ? 1 : 0;
      if (random8() < 18) height = 2;
      for (uint8_t z = 0; z <= height; ++z) {
        const uint8_t green = 52 + z * 28 + random8(22);
        zarchTerrainLut[indexFromXYZ(x, y, z)] = z == height
          ? CRGB(18, green + 34, 9)
          : CRGB(10, green, 4);
      }
    }
  }
}

void resetZarchScene() {
  buildZarchTerrainLUT();
  zarchCraftX = 2;
  zarchEnemyX[0] = 0; zarchEnemyY[0] = 4;
  zarchEnemyX[1] = 4; zarchEnemyY[1] = 3;
  zarchEnemyAlive[0] = true; zarchEnemyAlive[1] = true;
  zarchShotActive = false;
  zarchImpactLife = 0;
  zarchLastCraftStepAt = millis();
  zarchLastEnemyStepAt = millis();
  zarchLastShotStepAt = millis();
}

void fireZarchShot() {
  if (zarchShotActive) return;
  zarchShotActive = true;
  zarchShotX = zarchCraftX;
  zarchShotY = 1;
}

void updateZarchScene() {
  const uint32_t now = millis();
  if (now - zarchLastCraftStepAt >= 430U) {
    zarchLastCraftStepAt = now;
    zarchCraftX = (zarchCraftX + 1) % N;
  }
  if (now - zarchLastEnemyStepAt >= 570U) {
    zarchLastEnemyStepAt = now;
    for (uint8_t i = 0; i < ZARCH_ENEMY_COUNT; ++i) {
      if (!zarchEnemyAlive[i]) continue;
      if (zarchEnemyX[i] < zarchCraftX) ++zarchEnemyX[i];
      else if (zarchEnemyX[i] > zarchCraftX) --zarchEnemyX[i];
      --zarchEnemyY[i];
      if (zarchEnemyY[i] < 0) resetZarchScene();
    }
  }
  if (zarchShotActive && now - zarchLastShotStepAt >= 105U) {
    zarchLastShotStepAt = now;
    ++zarchShotY;
    if (zarchShotY >= N) zarchShotActive = false;
  }
  if (zarchShotActive) {
    for (uint8_t i = 0; i < ZARCH_ENEMY_COUNT; ++i) {
      if (zarchEnemyAlive[i] && zarchShotX == zarchEnemyX[i] && zarchShotY == zarchEnemyY[i]) {
        zarchEnemyAlive[i] = false;
        zarchShotActive = false;
        zarchImpactX = zarchEnemyX[i];
        zarchImpactY = zarchEnemyY[i];
        zarchImpactLife = 8;
      }
    }
  }
  if (zarchImpactLife > 0) --zarchImpactLife;
  if (!zarchEnemyAlive[0] && !zarchEnemyAlive[1]) resetZarchScene();
}

void renderZarch(float t) {
  (void)t;
  updateZarchScene();
  ::memcpy(leds, zarchTerrainLut, sizeof(leds));

  // Lime/cyan player craft high above the voxel terrain.
  setVoxel(zarchCraftX, 1, 4, CRGB(110, 255, 40));
  setVoxel(zarchCraftX - 1, 0, 3, CRGB(0, 105, 105));
  setVoxel(zarchCraftX + 1, 0, 3, CRGB(0, 105, 105));
  setVoxel(zarchCraftX, 0, 3, CRGB(180, 255, 80));

  for (uint8_t i = 0; i < ZARCH_ENEMY_COUNT; ++i) {
    if (!zarchEnemyAlive[i]) continue;
    setVoxel(zarchEnemyX[i], zarchEnemyY[i], 3, CRGB(255, 36, 8));
    setVoxel(zarchEnemyX[i], zarchEnemyY[i], 2, CRGB(100, 9, 2));
  }
  if (zarchShotActive) setVoxel(zarchShotX, zarchShotY, 4, CRGB(210, 255, 255));
  if (zarchImpactLife > 0) {
    const uint8_t value = zarchImpactLife * 30;
    addVoxel(zarchImpactX, zarchImpactY, 3, CRGB(value, value / 3, 0));
    addVoxel(zarchImpactX - 1, zarchImpactY, 3, CRGB(value / 2, value / 7, 0));
    addVoxel(zarchImpactX + 1, zarchImpactY, 3, CRGB(value / 2, value / 7, 0));
    addVoxel(zarchImpactX, zarchImpactY - 1, 3, CRGB(value / 2, value / 7, 0));
    addVoxel(zarchImpactX, zarchImpactY + 1, 3, CRGB(value / 2, value / 7, 0));
  }
}

void renderFire(float t) {
  const uint16_t drift = uint16_t(t * 35.0f);
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const uint8_t noise = inoise8(x * 66 + drift, y * 66, z * 66 - drift * 2);
        const uint16_t heat = uint16_t(noise) * (N - z) / N;
        const uint8_t hue = heat > 185 ? 28 : (heat > 100 ? 10 : 0);
        const uint8_t sat = heat > 220 ? 70 : 255;
        setVoxel(x, y, z, CHSV(hue, sat, heat));
      }
    }
  }
}

void renderIntenseFire(float t) {
  const uint16_t drift = uint16_t(t * 72.0f);
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const uint8_t noise = inoise8(x * 92 + drift, y * 92 - drift / 3, z * 78 - drift * 4);
        const uint8_t height = uint8_t(255 - z * 46);
        const uint8_t heat = scale8(noise, height);
        CRGB flame = HeatColor(heat);
        if (heat > 190) flame += CRGB(70, 20, 0);
        if (heat > 235) flame += CRGB(35, 45, 0);
        setVoxel(x, y, z, flame);
      }
    }
  }
}

void renderBlueFire(float t) {
  const uint16_t drift = uint16_t(t * 82.0f);
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const uint8_t noise = inoise8(x * 88 + drift, y * 88 + drift / 4, z * 92 - drift * 5);
        const uint8_t power = scale8(noise, uint8_t(255 - z * 44));
        const uint8_t hue = 148 + (power >> 4);  // azure through blue-violet.
        CRGB flame = CHSV(hue, 245, power);
        if (power > 210) flame += CRGB(0, 45, 75);
        setVoxel(x, y, z, flame);
      }
    }
  }
}

void renderExplosions(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const float phase = fmodf(t * 1000.0f, 3100.0f);
  const uint8_t radius16 = uint8_t((phase < 2200.0f ? phase * 3.6f / 2200.0f : 3.6f) * 16.0f);
  const float fade = phase < 2200.0f ? 1.0f : (3100.0f - phase) / 900.0f;
  const uint8_t centre = uint8_t(t * 0.35f) & 1;
  for (int8_t z = 0; z < N; ++z) for (int8_t y = 0; y < N; ++y) for (int8_t x = 0; x < N; ++x) {
    const uint8_t distance16 = explosionRadius16[centre][indexFromXYZ(x, y, z)];
    const uint8_t shell16 = abs(int16_t(distance16) - radius16);
    if (shell16 < 12) {
      const uint8_t value = uint8_t(255.0f * fade * (12 - shell16) / 12);
      const uint8_t hue = distance16 < radius16 * 45 / 100 ? 18 : 5;
      setVoxel(x, y, z, CHSV(hue, 245, value));
    } else if (distance16 < radius16 && ((x * 13 + y * 7 + z * 3 + uint8_t(phase)) & 3) == 0) {
      setVoxel(x, y, z, CRGB(uint8_t(85 * fade), 0, 0));
    }
  }
}

void renderFireworks(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const float phase = fmodf(t * 1000.0f, 3600.0f);
  const uint8_t launchX = 1 + (uint8_t(t * 0.23f) % 3);
  const uint8_t launchY = 1 + (uint8_t(t * 0.17f) % 3);
  if (phase < 1200.0f) {
    const int8_t z = constrain(int8_t(phase / 240.0f), 0, 4);
    setVoxel(launchX, launchY, z, CRGB::White);
    if (z > 0) setVoxel(launchX, launchY, z - 1, CRGB(120, 35, 0));
    return;
  }

  const float age = (phase - 1200.0f) / 2400.0f;
  const float radius = age * 3.5f;
  const uint8_t value = uint8_t(255.0f * (1.0f - age));
  const int8_t directions[14][3] = {
    {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},
    {1,1,0},{-1,1,0},{1,0,1},{-1,0,1},{0,1,1},{0,-1,1},{1,-1,0},{-1,-1,0}
  };
  const uint8_t hue = uint8_t(18 + uint8_t(t * 11.0f));
  for (uint8_t i = 0; i < 14; ++i) {
    const int8_t x = int8_t(roundf(launchX + directions[i][0] * radius));
    const int8_t y = int8_t(roundf(launchY + directions[i][1] * radius));
    const int8_t z = int8_t(roundf(4 + directions[i][2] * radius));
    setVoxel(x, y, z, CHSV(hue + i * 11, 225, value));
  }
}

void renderPixelPasture(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // Layer 1: an uneven bright-green field.
  for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x)
    setVoxel(x, y, 0, ((x + y) & 1) ? CRGB(0, 125, 15) : CRGB(0, 92, 8));

  // Layer 2: two deliberately blocky brown cows with pale patches.
  const CRGB cow(112, 48, 12);
  const CRGB patch(190, 155, 95);
  setVoxel(0, 1, 1, cow); setVoxel(1, 1, 1, cow); setVoxel(0, 2, 1, cow);
  setVoxel(1, 2, 1, patch); setVoxel(0, 0, 1, CRGB(60, 28, 8));
  setVoxel(3, 2, 1, cow); setVoxel(4, 2, 1, cow); setVoxel(3, 3, 1, patch);
  setVoxel(4, 3, 1, cow); setVoxel(4, 1, 1, CRGB(60, 28, 8));

  // Layers 3–4: slow white clouds drift across the sky.
  const int8_t cloudShift = int8_t(uint8_t(t * 1.4f) % N);
  const CRGB cloud(105, 150, 190);
  for (uint8_t z = 2; z <= 3; ++z) {
    setVoxel((0 + cloudShift) % N, 0, z, cloud);
    setVoxel((1 + cloudShift) % N, 0, z, cloud);
    setVoxel((1 + cloudShift) % N, 1, z, CRGB(145, 185, 220));
    setVoxel((3 + cloudShift) % N, 4, z, cloud);
    setVoxel((4 + cloudShift) % N, 4, z, cloud);
  }

  // Layer 5: a 3×3 golden sun in the upper-front corner.
  const uint8_t pulse = 185 + (sin8(uint8_t(t * 18.0f)) >> 2);
  for (uint8_t y = 2; y < N; ++y) for (uint8_t x = 2; x < N; ++x)
    setVoxel(x, y, 4, CHSV(30, 230, pulse));
}

// Voxel Minesweeper: a probe falls to illuminated base targets. A target hit
// becomes a vivid orange 3×3×3 impact burst before the next drop begins.
const uint8_t MINE_TARGETS[5][2] = {{0, 1}, {1, 4}, {2, 2}, {3, 0}, {4, 3}};
uint8_t mineTargetIndex = 0;
int8_t mineDropX = 0, mineDropY = 1, mineDropZ = 4;
uint32_t mineLastStepAt = 0;
bool mineBurstActive = false;
int8_t mineBurstX = 0, mineBurstY = 1;
uint32_t mineBurstAt = 0;

void resetMineDrop() {
  mineDropX = MINE_TARGETS[mineTargetIndex][0];
  mineDropY = MINE_TARGETS[mineTargetIndex][1];
  mineDropZ = 4;
  mineBurstActive = false;
}

void triggerMineBurst(int8_t x, int8_t y) {
  mineBurstX = x; mineBurstY = y;
  mineBurstAt = millis();
  mineBurstActive = true;
}

void renderMinesweeper(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t target = 0; target < 5; ++target) {
    const bool armed = target == mineTargetIndex;
    setVoxel(MINE_TARGETS[target][0], MINE_TARGETS[target][1], 0, armed ? CRGB(72, 230, 35) : CRGB(12, 60, 12));
  }
  if (mineBurstActive) {
    const uint16_t age = millis() - mineBurstAt;
    if (age >= 540) {
      mineTargetIndex = (mineTargetIndex + 1) % 5;
      resetMineDrop();
    } else {
      const uint8_t value = uint8_t(255 - (uint32_t(age) * 255 / 540));
      for (int8_t z = 0; z <= 2; ++z) for (int8_t y = -1; y <= 1; ++y) for (int8_t x = -1; x <= 1; ++x) {
        addVoxel(mineBurstX + x, mineBurstY + y, z, CHSV(18 + (x + y + z) * 2, 250, value));
      }
      return;
    }
  }
  const uint16_t stepMs = 480 - uint16_t(speedControl) * 330 / 255;
  if (millis() - mineLastStepAt >= stepMs) {
    mineLastStepAt = millis();
    --mineDropZ;
    if (mineDropZ < 0) triggerMineBurst(mineDropX, mineDropY);
  }
  setVoxel(mineDropX, mineDropY, mineDropZ, CRGB(255, 165, 18));
  addVoxel(mineDropX, mineDropY, mineDropZ + 1, CRGB(95, 20, 0));
}

void renderMoonStars(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    const VoxelGeometry &g = voxelGeometry[indexFromXYZ(x, y, z)];
    if (g.moonShade) {
      const uint8_t shade = g.moonShade;
      setVoxel(x, y, z, CRGB(shade, shade, uint8_t(shade * 0.82f)));
    }
  }
  for (uint8_t star = 0; star < 12; ++star) {
    const uint8_t x = (star * 2 + 3) % N, y = (star * 3 + 1) % N, z = (star * 4 + 2) % N;
    if (voxelGeometry[indexFromXYZ(x, y, z)].moonRadius16 > 34) {
      const uint8_t twinkle = 85 + (sin8(uint8_t(t * 24 + star * 31)) >> 1);
      addVoxel(x, y, z, CRGB(twinkle / 3, twinkle / 2, twinkle));
    }
  }
}

const uint8_t NIXIE_SEGMENTS[10] = {
  B00111111, B00000110, B01011011, B01001111, B01100110,
  B01101101, B01111101, B00000111, B01111111, B01101111
};

void drawNixieSegment(uint8_t segment, const CRGB &colour) {
  // Active segment order is a,b,c,d,e,f,g on the cube's y=4 front face.
  const int8_t startX[7] = {1, 4, 4, 1, 0, 0, 1};
  const int8_t startZ[7] = {4, 3, 0, 0, 0, 3, 2};
  const int8_t deltaX[7] = {1, 0, 0, 1, 0, 0, 1};
  const int8_t deltaZ[7] = {0, 1, 1, 0, 1, 1, 0};
  const uint8_t length[7] = {3, 2, 2, 3, 2, 2, 3};
  for (uint8_t pixel = 0; pixel < length[segment]; ++pixel) {
    const int8_t x = startX[segment] + deltaX[segment] * pixel;
    const int8_t z = startZ[segment] + deltaZ[segment] * pixel;
    setVoxel(x, 4, z, colour);
    addVoxel(x, 3, z, CRGB(colour.r / 7, colour.g / 7, 0));
  }
}

void renderNixieTube(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint16_t dwellMs = 1600 - uint16_t(speedControl) * 1000 / 255;
  const uint8_t digit = (millis() / dwellMs) % 10;
  const uint8_t pulse = 205 + (sin8(uint8_t(t * 38)) >> 2);
  for (uint8_t segment = 0; segment < 7; ++segment) {
    const bool active = NIXIE_SEGMENTS[digit] & (1 << segment);
    drawNixieSegment(segment, active ? CRGB(pulse, 70 + pulse / 4, 4) : CRGB(28, 7, 0));
  }
}

void renderSpirals(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t z = 0; z < N; ++z) {
    const float a = t * 2.4f + z * 1.50f;
    const int8_t x1 = int8_t(roundf(2.0f + 1.75f * cosf(a)));
    const int8_t y1 = int8_t(roundf(2.0f + 1.75f * sinf(a)));
    const int8_t x2 = int8_t(roundf(2.0f + 1.75f * cosf(a + 3.14159f)));
    const int8_t y2 = int8_t(roundf(2.0f + 1.75f * sinf(a + 3.14159f)));
    setVoxel(x1, y1, z, CRGB::Aqua);
    setVoxel(x2, y2, z, CRGB::Magenta);
  }
}

int8_t wrapCoordinate(int16_t value) {
  value %= N;
  if (value < 0) value += N;
  return int8_t(value);
}

void renderComets(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 60);
  const uint8_t step = uint8_t(t * 5.0f);
  for (uint8_t comet = 0; comet < 3; ++comet) {
    const uint8_t hue = 20 + comet * 80;
    for (uint8_t tail = 0; tail < 5; ++tail) {
      const int8_t p = int8_t(step + comet * 13 - tail);
      const int8_t x = wrapCoordinate(p + comet * 2);
      const int8_t y = wrapCoordinate(p * 2 + comet);
      const int8_t z = wrapCoordinate(p * 3 + comet * 3);
      addVoxel(x, y, z, CHSV(hue, 255, 255 - tail * 45));
    }
  }
}

int8_t pongX = 2, pongY = 2;
int8_t pongDX = 1, pongDY = 1;
int8_t playerPaddleX = 2;
int8_t cpuPaddleX = 2;
uint8_t pongPlayerScore = 0, pongCpuScore = 0;
uint32_t lastPongAt = 0;
void resetPongRound(int8_t direction) {
  pongX = 2; pongY = 2;
  pongDX = random8(2) ? 1 : -1;
  pongDY = direction;
}

void renderPong(float t) {
  const uint16_t stepMs = 350 - uint16_t(speedControl) * 250 / 255;
  if (millis() - lastPongAt >= stepMs) {
    lastPongAt = millis();
    pongX += pongDX; pongY += pongDY;
    if (pongX <= 0 || pongX >= 4) { pongDX = -pongDX; pongX = constrain(pongX, 0, 4); }
    // The CPU tracks the ball, but only one row per ball tick so it is beatable.
    if (cpuPaddleX < pongX) ++cpuPaddleX;
    else if (cpuPaddleX > pongX) --cpuPaddleX;
    if (pongY <= 0) {
      if (pongX == playerPaddleX) { pongY = 0; pongDY = 1; }
      else { ++pongCpuScore; resetPongRound(1); }
    }
    if (pongY >= 4) {
      if (pongX == cpuPaddleX) { pongY = 4; pongDY = -1; }
      else { ++pongPlayerScore; resetPongRound(-1); }
    }
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // A genuine full-height z-axis paddle slides left/right across the rear wall.
  // Short Button 1 moves it left; short Button 2 moves it right.
  for (uint8_t z = 0; z < N; ++z) setVoxel(playerPaddleX, 0, z, CRGB::Aqua);
  for (uint8_t z = 0; z < N; ++z) setVoxel(cpuPaddleX, 4, z, CRGB::Red);
  setVoxel(pongX, pongY, 2, CRGB::White);
  for (uint8_t score = 0; score < min(pongPlayerScore, uint8_t(3)); ++score) setVoxel(score, 0, 4, CRGB::Aqua);
  for (uint8_t score = 0; score < min(pongCpuScore, uint8_t(3)); ++score) setVoxel(4 - score, 4, 4, CRGB::Red);
}

bool life[N][N][N];
bool nextLife[N][N][N];
uint8_t lifeAge[N][N][N];
uint32_t lastLifeAt = 0;
uint16_t lifeGeneration = 0;

uint8_t countLifeNeighbours(int8_t x, int8_t y, int8_t z) {
  uint8_t total = 0;
  for (int8_t dz = -1; dz <= 1; ++dz) for (int8_t dy = -1; dy <= 1; ++dy) for (int8_t dx = -1; dx <= 1; ++dx) {
    if (dx == 0 && dy == 0 && dz == 0) continue;
    const int8_t nx = x + dx, ny = y + dy, nz = z + dz;
    if (nx >= 0 && nx < N && ny >= 0 && ny < N && nz >= 0 && nz < N && life[nx][ny][nz]) ++total;
  }
  return total;
}

void seedLife() {
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    life[x][y][z] = random8() < 57;
    lifeAge[x][y][z] = life[x][y][z] ? 1 : 0;
  }
  lifeGeneration = 0;
}

void stepLife() {
  uint8_t population = 0;
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    const uint8_t n = countLifeNeighbours(x, y, z);
    nextLife[x][y][z] = life[x][y][z] ? (n == 4 || n == 5) : (n == 5);
  }
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    const bool wasAlive = life[x][y][z];
    life[x][y][z] = nextLife[x][y][z];
    if (life[x][y][z]) {
      const uint8_t candidate = lifeAge[x][y][z] + 1;
      lifeAge[x][y][z] = wasAlive ? (candidate > 12 ? 12 : candidate) : 1;
      ++population;
    } else lifeAge[x][y][z] = 0;
  }
  ++lifeGeneration;
  if (population == 0 || lifeGeneration > 170) seedLife();
}

void renderLife(float t) {
  const uint16_t generationMs = 1080 - speedControl * 3;
  if (millis() - lastLifeAt >= generationMs) {
    lastLifeAt = millis();
    stepLife();
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    if (!life[x][y][z]) continue;
    const uint8_t a = lifeAge[x][y][z];
    if (a <= 2) setVoxel(x, y, z, CRGB::White);
    else setVoxel(x, y, z, CHSV(88 + min(uint8_t(a * 3), uint8_t(38)), 235, 180 + min(uint8_t(a * 7), uint8_t(70))));
  }
}

void renderClouds(float t) {
  const uint16_t drift = uint16_t(t * 18.0f);
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    const uint8_t n = inoise8(x * 72 + drift, y * 72 + drift / 2, z * 72);
    const uint8_t value = n > 115 ? n - 105 : 0;
    setVoxel(x, y, z, CHSV(145, 80, value));
  }
}

void renderGlitter(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 35);
  const uint8_t grains = 1 + speedControl / 55;
  for (uint8_t i = 0; i < grains; ++i) addVoxel(random8(N), random8(N), random8(N), CRGB::White);
}

void renderCornerCubes(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t hue = uint8_t(t * 35.0f);
  for (uint8_t cz = 0; cz < 2; ++cz) for (uint8_t cy = 0; cy < 2; ++cy) for (uint8_t cx = 0; cx < 2; ++cx) {
    const uint8_t offsetHue = hue + cx * 60 + cy * 30 + cz * 90;
    for (uint8_t dz = 0; dz < 2; ++dz) for (uint8_t dy = 0; dy < 2; ++dy) for (uint8_t dx = 0; dx < 2; ++dx) {
      setVoxel(cx ? 3 + dx : dx, cy ? 3 + dy : dy, cz ? 3 + dz : dz, CHSV(offsetHue, 255, 220));
    }
  }
}

// -----------------------------------------------------------------------------
// Micro-world scenes: all use bounded math and at most 125 voxel writes/frame.
// -----------------------------------------------------------------------------
void renderBulletWall(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // A complete grey impact wall at the right side of the cube.
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) {
    const uint8_t shade = 75 + ((y + z) & 1) * 35;
    setVoxel(4, y, z, CRGB(shade, shade, shade));
  }
  // Three red shots fly from left to right on staggered trajectories.
  for (uint8_t shot = 0; shot < 3; ++shot) {
    const int8_t x = int8_t(fmodf(t * (3.2f + shot * 0.6f) + shot * 2.5f, 9.0f)) - 2;
    const int8_t y = 1 + shot;
    const int8_t z = 1 + ((shot * 2 + int(t * 1.8f)) % 3);
    setVoxel(x, y, z, CRGB::Red);
    setVoxel(x - 1, y, z, CRGB(90, 0, 0));
    if (x == 4) {
      setVoxel(4, y, z, CRGB::White);
      addVoxel(4, y - 1, z, CRGB(200, 30, 0));
      addVoxel(4, y + 1, z, CRGB(200, 30, 0));
      addVoxel(4, y, z - 1, CRGB(200, 30, 0));
      addVoxel(4, y, z + 1, CRGB(200, 30, 0));
    }
  }
}

int8_t cellX = 2, cellY = 2, cellZ = 2;
int8_t cellDX = 1, cellDY = -1, cellDZ = 1;
uint32_t lastCellStepAt = 0;

void renderPaddedCell(float t) {
  const uint16_t stepMs = 440 - uint16_t(speedControl) * 360 / 255;
  if (millis() - lastCellStepAt >= stepMs) {
    lastCellStepAt = millis();
    cellX += cellDX; cellY += cellDY; cellZ += cellDZ;
    if (cellX < 1 || cellX > 3) { cellDX = -cellDX; cellX = constrain(cellX, 1, 3); }
    if (cellY < 1 || cellY > 3) { cellDY = -cellDY; cellY = constrain(cellY, 1, 3); }
    if (cellZ < 1 || cellZ > 3) { cellDZ = -cellDZ; cellZ = constrain(cellZ, 1, 3); }
  }
  // All six outside surfaces form a dim grey padded room. The untouched
  // interior coordinates (1..3 on x/y/z) form the red pixel's 3×3×3 chamber.
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    if (x == 0 || x == 4 || y == 0 || y == 4 || z == 0 || z == 4) {
      const uint8_t pad = 28 + ((x + y + z) & 1) * 14;
      setVoxel(x, y, z, CRGB(pad, pad, pad));
    } else setVoxel(x, y, z, CRGB::Black);
  }
  setVoxel(cellX, cellY, cellZ, CRGB::Red);
}

void renderBlockRun(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const int32_t scroll = int32_t(t * 3.5f);
  // The grass floor establishes the platform world.
  for (uint8_t x = 0; x < N; ++x) for (uint8_t y = 0; y < N; ++y) {
    setVoxel(x, y, 0, CHSV(78 + ((x + y + scroll) & 1) * 8, 235, 145));
    const int32_t tile = x + y * 3 + scroll;
    if ((tile % 11 + 11) % 11 < 3) {
      setVoxel(x, y, 1, CRGB(140, 65, 12));
      if (((tile + 1) % 7 + 7) % 7 == 0) setVoxel(x, y, 2, CRGB(210, 145, 25));
    }
  }
  // A tiny red-and-skin runner jumps as the blocks pass beneath.
  const uint8_t hop = uint8_t((sinf(t * 5.0f) + 1.0f) * 1.4f);
  setVoxel(1, 2, 1 + hop, CRGB(230, 35, 20));
  setVoxel(1, 2, 2 + hop, CRGB(255, 185, 110));
  setVoxel(0, 2, 1 + hop, CRGB(230, 35, 20));
}

void renderParallax(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // Three independent depth bands move at different rates, producing parallax.
  for (uint8_t layer = 0; layer < 3; ++layer) {
    const uint8_t cadence = 2 + layer * 2;
    const uint8_t step = uint8_t(t * cadence);
    const uint8_t value = 65 + layer * 85;
    for (uint8_t star = 0; star < 7; ++star) {
      const uint8_t x = (star * 2 + step * (layer + 1)) % N;
      const uint8_t y = (star * 3 + step + layer) % N;
      const uint8_t z = (star + step * 2 + layer * 3) % N;
      addVoxel(x, y, z, CHSV(145 + layer * 24, 120, value));
    }
  }
}

void renderTrenchRun(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t pulse = 55 + uint8_t((sinf(t * 5.0f) + 1.0f) * 35.0f);
  // A blue-grey square tunnel, with darker centre space and a moving red target.
  for (uint8_t z = 0; z < N; ++z) for (uint8_t edge = 0; edge < N; ++edge) {
    const uint8_t shade = pulse + z * 8;
    setVoxel(0, edge, z, CRGB(shade, shade, shade + 28));
    setVoxel(4, edge, z, CRGB(shade, shade, shade + 28));
    setVoxel(edge, 0, z, CRGB(shade, shade, shade + 28));
    setVoxel(edge, 4, z, CRGB(shade, shade, shade + 28));
  }
  const int8_t targetZ = 4 - (int(t * 6.0f) % 7);
  const int8_t targetX = 1 + (int(t * 2.0f) % 3);
  const int8_t targetY = 1 + (int(t * 1.3f) % 3);
  setVoxel(targetX, targetY, targetZ, CRGB::Red);
  setVoxel(targetX, targetY, targetZ + 1, CRGB(90, 0, 0));
  setVoxel(2, 2, 0, CRGB::White); // pilot reticle at the viewer end
}

void drawBoxFrame(const CRGB &colour) {
  for (uint8_t z = 0; z < N; ++z) for (uint8_t y = 0; y < N; ++y) for (uint8_t x = 0; x < N; ++x) {
    const uint8_t sides = (x == 0 || x == 4) + (y == 0 || y == 4) + (z == 0 || z == 4);
    if (sides >= 2) setVoxel(x, y, z, colour);
  }
}

void renderRunningLegs(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const float gait = sinf(t * 4.25f);
  const float swings[2] = {gait, -gait};
  const int8_t legX[2] = {1, 3};
  // Visible hip -> knee -> ankle chains make a proper articulated stride.
  setVoxel(2, 2, 4, CRGB(50, 120, 255));
  setVoxel(2, 2, 3, CRGB(230, 50, 35));
  setVoxel(1, int8_t(roundf(2.0f - 1.25f * gait)), 3, CRGB(255, 175, 95));
  setVoxel(3, int8_t(roundf(2.0f + 1.25f * gait)), 3, CRGB(255, 175, 95));
  for (uint8_t leg = 0; leg < 2; ++leg) {
    const float swing = swings[leg];
    const float lift = max(0.0f, swing);
    const int8_t kneeY = int8_t(roundf(2.0f + 1.15f * swing));
    const int8_t kneeZ = int8_t(roundf(1.65f + 0.95f * lift));
    const int8_t footY = int8_t(roundf(2.0f + 1.75f * swing));
    setVoxel(legX[leg], 2, 3, CRGB(255, 132, 45));
    setVoxel(legX[leg], int8_t(roundf((2.0f + kneeY) * 0.5f)), 2, CRGB(255, 184, 58));
    setVoxel(legX[leg], kneeY, kneeZ, CRGB(255, 232, 95));
    setVoxel(legX[leg], int8_t(roundf((kneeY + footY) * 0.5f)), 1, CRGB(255, 205, 72));
    setVoxel(legX[leg], footY, 0, CRGB::White);
    addVoxel(legX[leg] + (leg == 0 ? -1 : 1), footY, 0, CRGB(90, 90, 90));
  }
}

void renderFairyBox(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  drawBoxFrame(CRGB(0, 110, 24));
  // Three drifting fairy bodies with pulsing, asymmetric wings.
  for (uint8_t fairy = 0; fairy < 3; ++fairy) {
    const int8_t x = wrapCoordinate(int(t * (2.2f + fairy * 0.4f)) + fairy * 2);
    const int8_t y = wrapCoordinate(int(t * (1.7f + fairy * 0.3f)) + fairy * 3);
    const int8_t z = 1 + ((int(t * (1.2f + fairy * 0.2f)) + fairy) % 3);
    const CRGB body = fairy == 0 ? CRGB(255, 210, 50) : (fairy == 1 ? CRGB(255, 80, 180) : CRGB(100, 220, 255));
    setVoxel(x, y, z, body);
    const uint8_t wingValue = 80 + uint8_t((sinf(t * 10.0f + fairy) + 1.0f) * 85.0f);
    addVoxel(x - 1, y, z, CHSV(145 + fairy * 35, 120, wingValue));
    addVoxel(x + 1, y, z, CHSV(145 + fairy * 35, 120, wingValue));
    addVoxel(x, y, z + 1, CRGB(35, 95, 35));
  }
}

void renderAquarium(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  // Dim water volume behind a bright blue tank frame.
  for (uint8_t z = 1; z < 4; ++z) for (uint8_t y = 1; y < 4; ++y) for (uint8_t x = 1; x < 4; ++x) {
    const uint8_t water = 18 + uint8_t((sinf(t * 2.0f + x + y + z) + 1.0f) * 13.0f);
    setVoxel(x, y, z, CHSV(151, 220, water));
  }
  drawBoxFrame(CRGB(0, 70, 255));
  for (uint8_t fish = 0; fish < 2; ++fish) {
    const bool swimsRight = fish == 0;
    const int8_t x = wrapCoordinate(int(t * (2.5f + fish * 0.4f)) + fish * 3);
    const int8_t y = 1 + fish * 2;
    const int8_t z = 1 + ((int(t * 1.4f) + fish * 2) % 3);
    const CRGB orange = fish == 0 ? CRGB(255, 85, 0) : CRGB(255, 145, 15);
    setVoxel(x, y, z, orange);
    setVoxel(x, y, z + 1, orange);
    const int8_t tail = swimsRight ? x - 1 : x + 1;
    addVoxel(tail, y, z, CRGB(170, 35, 0));
    addVoxel(tail, y, z + 1, CRGB(170, 35, 0));
    addVoxel(x, y + (swimsRight ? 1 : -1), z, CRGB(255, 220, 95));
  }
}

void renderPyramid(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t hue = 20 + uint8_t(t * 22.0f);
  // Three deliberately discrete tiers: 3×3 base, 2×2 middle, one apex.
  for (uint8_t x = 1; x <= 3; ++x) for (uint8_t y = 1; y <= 3; ++y) {
    setVoxel(x, y, 0, CHSV(hue, 240, 120));
  }
  for (uint8_t x = 1; x <= 2; ++x) for (uint8_t y = 1; y <= 2; ++y) {
    setVoxel(x, y, 1, CHSV(hue + 24, 245, 190));
  }
  const uint8_t apex = 140 + uint8_t((sinf(t * 8.0f) + 1.0f) * 57.0f);
  setVoxel(2, 2, 2, CHSV(hue + 48, 110, apex));
}

// -----------------------------------------------------------------------------
// 3×5 scrolling text banner around the four vertical outer faces
// -----------------------------------------------------------------------------
// Perimeter positions travel clockwise when viewed from above. Corners appear
// once only, giving a 16-column loop around the side of the 5×5×5 cube.
constexpr uint8_t PERIMETER_COLUMNS = 16;
constexpr uint8_t BANNER_TEXT_MAX = 60;
char bannerText[BANNER_TEXT_MAX + 1] = "CUBE 4 3 2 1 0";
uint8_t bannerHue = 96;          // 0..255 FastLED hue wheel, default electric green
uint8_t bannerScrollSpeed = 150; // 1 slow .. 255 fast
enum BannerFont : uint8_t { BANNER_FONT_3X5 = 3, BANNER_FONT_5X5 = 5 };
BannerFont bannerFont = BANNER_FONT_3X5;
uint16_t bannerOffset = 0;
uint32_t lastBannerStepAt = 0;

const char FONT_CHARACTERS[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-:!?";
const uint8_t FONT_3X5[][5] = {
  {0,0,0,0,0},                         // space
  {2,5,7,5,5}, {6,5,6,5,6}, {3,4,4,4,3}, {6,5,5,5,6}, {7,4,6,4,7}, // A-E
  {7,4,6,4,4}, {3,4,5,5,3}, {5,5,7,5,5}, {7,2,2,2,7}, // F-I
  {1,1,1,5,2}, {5,5,6,5,5}, {4,4,4,4,7}, {5,7,7,5,5}, // J-M
  {5,7,7,7,5}, {2,5,5,5,2}, {6,5,6,4,4}, {2,5,5,2,1}, // N-Q
  {6,5,6,5,5}, {3,4,2,1,6}, {7,2,2,2,2}, {5,5,5,5,7}, // R-U
  {5,5,5,5,2}, {5,5,7,7,5}, {5,5,2,5,5}, {5,5,2,2,2}, // V-Y
  {7,1,2,4,7},                         // Z
  {7,5,5,5,7}, {2,6,2,2,7}, {6,1,7,4,7}, {6,1,3,1,6}, // 0-3
  {5,5,7,1,1}, {7,4,6,1,6}, {3,4,6,5,2}, {7,1,2,4,4}, // 4-7
  {2,5,2,5,2}, {2,5,3,1,6},             // 8-9
  {0,0,7,0,0}, {0,0,0,0,2}, {0,2,0,2,0}, {2,2,2,0,2}, {6,1,2,0,2} // punctuation
};
static_assert(sizeof(FONT_3X5) / sizeof(FONT_3X5[0]) == sizeof(FONT_CHARACTERS) - 1,
              "Each 3x5 banner character needs exactly one glyph");

// A true 5×5 bitmap font for the bold full-face option.
const uint8_t FONT_5X5[][5] = {
  {0,0,0,0,0},
  {14,17,31,17,17}, {30,17,30,17,30}, {14,17,16,17,14}, {30,17,17,17,30}, {31,16,30,16,31},
  {31,16,30,16,16}, {14,16,23,17,14}, {17,17,31,17,17}, {31,4,4,4,31}, {1,1,1,17,14},
  {17,18,28,18,17}, {16,16,16,16,31}, {17,27,21,17,17}, {17,25,21,19,17}, {14,17,17,17,14},
  {30,17,30,16,16}, {14,17,17,19,15}, {30,17,30,18,17}, {15,16,14,1,30}, {31,4,4,4,4},
  {17,17,17,17,14}, {17,17,17,10,4}, {17,17,21,27,17}, {17,10,4,10,17}, {17,10,4,4,4},
  {31,2,4,8,31},
  {14,19,21,25,14}, {4,12,4,4,14}, {14,17,2,4,31}, {30,1,14,1,30}, {18,18,31,2,2},
  {31,16,30,1,30}, {14,16,30,17,14}, {31,1,2,4,4}, {14,17,14,17,14}, {14,17,15,1,14},
  {0,0,0,0,4}, {0,0,31,0,0}, {0,4,0,4,0}, {4,4,4,0,4}, {14,1,6,0,4}
};
static_assert(sizeof(FONT_5X5) / sizeof(FONT_5X5[0]) == sizeof(FONT_CHARACTERS) - 1,
              "Each 5x5 banner character needs exactly one glyph");

uint8_t glyphIndex(char character) {
  character = toupper(static_cast<unsigned char>(character));
  const char *found = strchr(FONT_CHARACTERS, character);
  return found ? uint8_t(found - FONT_CHARACTERS) : 0;
}

bool glyphPixel3x5(char character, uint8_t column, uint8_t row) {
  if (column >= 3 || row >= 5) return false;
  const uint8_t rowBits = FONT_3X5[glyphIndex(character)][row];
  return rowBits & (1 << (2 - column));
}

bool glyphPixel5x5(char character, uint8_t column, uint8_t row) {
  if (column >= 5 || row >= 5) return false;
  const uint8_t rowBits = FONT_5X5[glyphIndex(character)][row];
  return rowBits & (1 << (4 - column));
}

uint8_t bannerGlyphWidth() {
  return bannerFont == BANNER_FONT_5X5 ? 5 : 3;
}

bool bannerGlyphPixel(char character, uint8_t column, uint8_t row) {
  return bannerFont == BANNER_FONT_5X5 ? glyphPixel5x5(character, column, row)
                                       : glyphPixel3x5(character, column, row);
}

void setPerimeterVoxel(uint8_t p, uint8_t z, const CRGB &colour) {
  p %= PERIMETER_COLUMNS;
  if (p < 5) setVoxel(p, 0, z, colour);                 // rear: left -> right
  else if (p < 9) setVoxel(4, p - 4, z, colour);         // right: rear -> front
  else if (p < 13) setVoxel(12 - p, 4, z, colour);       // front: right -> left
  else setVoxel(0, 16 - p, z, colour);                   // left: front -> rear
}

void setBannerMessage(const String &source) {
  uint8_t written = 0;
  for (uint16_t i = 0; i < source.length() && written < BANNER_TEXT_MAX; ++i) {
    char character = toupper(static_cast<unsigned char>(source.charAt(i)));
    // Unsupported characters become spaces, avoiding JSON/UI injection and
    // guaranteeing every byte has a glyph representation.
    bannerText[written++] = strchr(FONT_CHARACTERS, character) ? character : ' ';
  }
  if (written == 0) bannerText[written++] = ' ';
  bannerText[written] = '\0';
  bannerOffset = 0;
}

void renderBanner(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t glyphWidth = bannerGlyphWidth();
  const uint8_t glyphPitch = glyphWidth + 1; // one blank column between glyphs
  const uint16_t textColumns = strlen(bannerText) * glyphPitch;
  if (textColumns == 0) return;

  const uint16_t stepMs = 430 - uint16_t(bannerScrollSpeed) * 360 / 255;
  if (millis() - lastBannerStepAt >= stepMs) {
    lastBannerStepAt = millis();
    // Glyphs already read forward; travel the message in the opposite direction
    // from the earlier build so the whole phrase now progresses forward in view.
    bannerOffset = (bannerOffset + textColumns - 1) % textColumns;
  }

  for (uint8_t p = 0; p < PERIMETER_COLUMNS; ++p) {
    const uint16_t messageColumn = (bannerOffset + p) % textColumns;
    const uint8_t characterIndex = messageColumn / glyphPitch;
    const uint8_t glyphColumn = messageColumn % glyphPitch;
    if (glyphColumn == glyphWidth) continue; // inter-character spacer

    for (uint8_t z = 0; z < N; ++z) {
      // Perimeter p runs opposite to the font's screen-space x-axis. Reverse
      // glyph sampling to preserve forward-facing 3×5 and 5×5 letters; only
      // bannerOffset above controls the visible travel direction.
      const uint8_t sampledColumn = glyphWidth - 1 - glyphColumn;
      if (!bannerGlyphPixel(bannerText[characterIndex], sampledColumn, N - 1 - z)) continue;
      setPerimeterVoxel(p, z, CHSV(bannerHue + z * 4, 255, 255));
    }
  }
}

// -----------------------------------------------------------------------------
// Compact expansion patterns 49–54 and temporary hidden scenes. No filesystem
// assets or frame buffers are used, protecting firmware size headroom.
// -----------------------------------------------------------------------------
bool blackHoleReverse = false;
bool stargateHeldOpen = false;
int8_t defenderShipX = 2;
bool chequerReverse = false;
uint8_t puzzleGlow = 150;
bool rubikSolvedReveal = false;
uint8_t secretScene = 255;
uint32_t secretStartedAt = 0;
uint8_t secretPatternRune = 0;
uint8_t primaryTapStreak = 0;
uint32_t lastPrimaryTapAt = 0;

void startSecretScene(uint8_t scene) {
  secretScene = scene % 5;
  secretStartedAt = millis();
  patternStartedAt = millis();
}

void recordPatternRune(uint8_t candidate) {
  const uint8_t rune[4] = {uint8_t(PATTERN_VECTOR_CUBE), uint8_t(PATTERN_MATRIX_RAIN), uint8_t(PATTERN_VECTOR_CUBE), uint8_t(PATTERN_MATRIX_RAIN)};
  if (candidate == rune[secretPatternRune]) {
    if (++secretPatternRune >= 4) {
      startSecretScene(2);
      secretPatternRune = 0;
    }
  } else {
    secretPatternRune = candidate == rune[0] ? 1 : 0;
  }
}

void renderBlackHole(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t spin = uint8_t(t * (blackHoleReverse ? -49.0f : 49.0f));
  for (int8_t z = 0; z < N; ++z) for (int8_t y = 0; y < N; ++y) for (int8_t x = 0; x < N; ++x) {
    const VoxelGeometry &g = voxelGeometry[indexFromXYZ(x, y, z)];
    if (g.centreRadius16 < 17 || g.centreRadius16 > 50) continue;
    const uint8_t ribbon = sin8(g.blackHoleAngle + spin + g.centreRadius16 * 7 + g.z2 * 10);
    if (ribbon > 174) setVoxel(x, y, z, CHSV(180 + g.centreRadius16, 210, 70 + scale8(ribbon - 174, 190)));
  }
}

void renderStargate(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t chevrons = stargateHeldOpen ? 7 : min<uint8_t>(7, uint8_t(fmodf(t * 0.8f, 9.0f)));
  for (int8_t x = 0; x < N; ++x) for (int8_t y = 0; y < N; ++y) setVoxel(x, y, 0, CRGB(25, 18, 12));
  for (int8_t z = 1; z < N; ++z) for (int8_t x = 0; x < N; ++x) {
    const uint8_t radius16 = stargateRadius16[x][z];
    if (radius16 > 24 && radius16 < 43) setVoxel(x, 2, z, CRGB(36, 42, 45));
    if ((chevrons >= 7 || stargateHeldOpen) && radius16 < 23) setVoxel(x, 2, z, CHSV(145, 185, 160 + scale8(sin8(uint8_t(t * 52) + x * 29 + z * 17), 90)));
  }
  const int8_t cx[7] = {2,4,4,2,0,0,2};
  const int8_t cz[7] = {4,3,1,1,1,3,4};
  for (uint8_t i = 0; i < chevrons; ++i) setVoxel(cx[i], 2, cz[i], CRGB::Orange);
}

void renderDefender(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 68);
  setVoxel(defenderShipX, 4, 1, CRGB(0, 150, 235));
  setVoxel(defenderShipX - 1, 4, 0, CRGB(0, 60, 90));
  setVoxel(defenderShipX + 1, 4, 0, CRGB(0, 60, 90));
  const int8_t enemyY = 3 - int8_t(fmodf(t * 1.5f, 4.0f));
  for (int8_t i = 0; i < 3; ++i) {
    const int8_t enemyX = (i * 2 + int(t * 1.2f)) % 5;
    setVoxel(enemyX, max<int8_t>(0, enemyY), 3, CRGB(225, 35, 12));
    setVoxel(enemyX, max<int8_t>(0, enemyY), 2, CRGB(90, 8, 2));
  }
  setVoxel(defenderShipX, 4 - int8_t(fmodf(t * 8.0f, 5.0f)), 1, CRGB(180, 235, 255));
}

void renderChequerboard(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const int8_t step = int8_t(t * (chequerReverse ? -1.3f : 1.3f));
  for (int8_t z = 0; z < N; ++z) for (int8_t y = 0; y < N; ++y) for (int8_t x = 0; x < N; ++x) {
    const int8_t folded = (x + y + step) % 5;
    const bool onPlane = folded == z || ((x + z + step) % 5 == y && (step & 2));
    if (onPlane && ((x + y + z + step) & 1) == 0) setVoxel(x, y, z, CHSV(130 + (step & 31), 130, 235));
  }
}

void renderPuzzleCube(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t seam = uint8_t(95 + sinf(t * 3.0f) * puzzleGlow);
  for (int8_t z = 0; z < N; ++z) for (int8_t y = 0; y < N; ++y) for (int8_t x = 0; x < N; ++x) {
    const bool surface = x == 0 || x == 4 || y == 0 || y == 4 || z == 0 || z == 4;
    if (!surface) continue;
    const bool seamLine = ((x + int(t * .6f)) % 2 == 0 && (y == 0 || y == 4)) || (z == 2 && ((x + y) & 1));
    if (seamLine) setVoxel(x, y, z, CHSV(151, 175, seam));
    else if (((x * 3 + y * 5 + z * 7 + int(t)) % 9) == 0) setVoxel(x, y, z, CRGB(175, 55, 5));
    else setVoxel(x, y, z, CRGB(18, 12, 7));
  }
}

void renderRubiksCube(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  const uint8_t turn = rubikSolvedReveal ? 0 : uint8_t(t * 1.2f) % 6;
  for (int8_t z = 1; z <= 3; ++z) for (int8_t y = 1; y <= 3; ++y) for (int8_t x = 1; x <= 3; ++x) {
    const bool surface = x == 1 || x == 3 || y == 1 || y == 3 || z == 1 || z == 3;
    if (!surface) continue;
    uint8_t hue = x == 1 ? 0 : x == 3 ? 32 : y == 1 ? 96 : y == 3 ? 160 : z == 1 ? 192 : 42;
    if (!rubikSolvedReveal && ((x + y + z + turn) % 4 == 0)) hue += 70;
    setVoxel(x, y, z, CHSV(hue, 235, 245));
  }
}

void renderSecretScene(float t) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  if (secretScene == 0) {
    const int8_t pupil = 2 + int8_t(sinf(t * 2.5f) * 1.4f);
    for (int8_t x = 0; x < N; ++x) for (int8_t z = 1; z < 4; ++z) if (abs(x - 2) + abs(z - 2) < 3) setVoxel(x, 4, z, CRGB(90, 0, 0));
    setVoxel(pupil, 4, 2, CRGB::Red);
  } else if (secretScene == 1) {
    for (uint16_t i = 0; i < NUM_LEDS; ++i) leds[i] = CRGB(13, 13, 13);
    const uint8_t phantom = uint8_t(t * 19) % NUM_LEDS;
    leds[phantom] = CRGB::White;
    leds[(phantom + 1) % NUM_LEDS] += CRGB(90, 90, 90);
  } else if (secretScene == 2) {
    for (int8_t z = 0; z < N; ++z) { setVoxel(1, 2, z, CRGB(0, 80, 8)); setVoxel(3, 2, z, CRGB(0, 80, 8)); }
    setVoxel(2, 2, uint8_t(t * 4) % 5, CRGB(120, 255, 18));
  } else if (secretScene == 3) {
    const int8_t layer = uint8_t(t * 3) % 5;
    for (int8_t x = 0; x < N; ++x) for (int8_t y = 0; y < N; ++y) setVoxel(x, y, layer, CRGB(8, 100, 220));
  } else {
    const int8_t p = uint8_t(t * 6) % 16;
    setPerimeterVoxel(p, 2, CRGB(255, 78, 4));
    setVoxel(2, 2, 2, CRGB(255, 140, 14));
  }
}

void renderCurrentPattern() {
  const float t = effectTime();
  if (secretScene != 255) {
    if (millis() - secretStartedAt < 5200UL) {
      renderSecretScene(t);
      return;
    }
    secretScene = 255;
  }
  switch (currentPattern) {
    case PATTERN_VECTOR_CUBE: renderVectorCube(t); break;
    case PATTERN_MATRIX_RAIN: renderMatrixRain(t); break;
    case PATTERN_PLASMA: renderPlasma(t); break;
    case PATTERN_FIRE: renderFire(t); break;
    case PATTERN_SPIRALS: renderSpirals(t); break;
    case PATTERN_COMETS: renderComets(t); break;
    case PATTERN_PONG: renderPong(t); break;
    case PATTERN_LIFE: renderLife(t); break;
    case PATTERN_CLOUDS: renderClouds(t); break;
    case PATTERN_GLITTER: renderGlitter(t); break;
    case PATTERN_CORNER_CUBES: renderCornerCubes(t); break;
    case PATTERN_BANNER: renderBanner(t); break;
    case PATTERN_BULLET_WALL: renderBulletWall(t); break;
    case PATTERN_PADDED_CELL: renderPaddedCell(t); break;
    case PATTERN_BLOCK_RUN: renderBlockRun(t); break;
    case PATTERN_PARALLAX: renderParallax(t); break;
    case PATTERN_TRENCH_RUN: renderTrenchRun(t); break;
    case PATTERN_RUNNING_LEGS: renderRunningLegs(t); break;
    case PATTERN_FAIRY_BOX: renderFairyBox(t); break;
    case PATTERN_AQUARIUM: renderAquarium(t); break;
    case PATTERN_PYRAMID: renderPyramid(t); break;
    case PATTERN_MATRIX_DRIFT: renderMatrixDrift(t); break;
    case PATTERN_INTENSE_FIRE: renderIntenseFire(t); break;
    case PATTERN_BLUE_FIRE: renderBlueFire(t); break;
    case PATTERN_EXPLOSIONS: renderExplosions(t); break;
    case PATTERN_FIREWORKS: renderFireworks(t); break;
    case PATTERN_PIXEL_PASTURE: renderPixelPasture(t); break;
    case PATTERN_RED_MATRIX_RAIN: renderRedMatrixRain(t); break;
    case PATTERN_MINESWEEPER: renderMinesweeper(t); break;
    case PATTERN_MOON_STARS: renderMoonStars(t); break;
    case PATTERN_NIXIE_TUBE: renderNixieTube(t); break;
    case PATTERN_BLACK_HOLE: renderBlackHole(t); break;
    case PATTERN_STARGATE: renderStargate(t); break;
    case PATTERN_DEFENDER: renderDefender(t); break;
    case PATTERN_CHEQUERBOARD: renderChequerboard(t); break;
    case PATTERN_PUZZLE_CUBE: renderPuzzleCube(t); break;
    case PATTERN_RUBIKS_CUBE: renderRubiksCube(t); break;
    case PATTERN_LISSAJOUS_RIPPLE: renderLissajousRipple(t); break;
    case PATTERN_ZARCH: renderZarch(t); break;
    default: break;
  }
}

void advancePattern() {
  currentPattern = Pattern((currentPattern + 1) % PATTERN_COUNT);
  recordPatternRune(currentPattern);
  patternStartedAt = millis();
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

// -----------------------------------------------------------------------------
// Optional hardware buttons: GPIO4 pattern-primary/banner; GPIO8
// pattern-secondary/next. Long presses are global; short presses are local.
// -----------------------------------------------------------------------------
uint8_t primaryButtonPin = CUBEFX_PRIMARY_BUTTON_PIN;
uint8_t secondaryButtonPin = CUBEFX_SECONDARY_BUTTON_PIN;
Preferences pinPreferences;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;
constexpr uint16_t BUTTON_LONG_PRESS_MS = 650;
bool primaryRaw = HIGH, primaryStable = HIGH, secondaryRaw = HIGH, secondaryStable = HIGH;
uint32_t primaryChangedAt = 0, secondaryChangedAt = 0;
uint32_t primaryPressedAt = 0, secondaryPressedAt = 0;

void activateBannerMode() {
  autoCycle = false;
  currentPattern = PATTERN_BANNER;
  bannerOffset = 0;
  patternStartedAt = millis();
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void cycleBrightness() {
  brightness = brightness >= 230 ? 64 : brightness + 32;
  FastLED.setBrightness(brightness);
}

void cycleSpeed() {
  speedControl = speedControl >= 225 ? 70 : speedControl + 31;
}

void runShortPatternAction(bool primary) {
  // Pattern-aware operations first. Every other pattern still receives a
  // safe, useful fallback: Button 1 adjusts speed; Button 2 adjusts brightness.
  switch (currentPattern) {
    case PATTERN_BANNER:
      if (primary) bannerHue += 32;
      else {
        bannerFont = bannerFont == BANNER_FONT_3X5 ? BANNER_FONT_5X5 : BANNER_FONT_3X5;
        bannerOffset = 0;
      }
      break;
    case PATTERN_LIFE:
      if (primary) seedLife();
      else stepLife();
      break;
    case PATTERN_PONG:
      if (primary) playerPaddleX = max(int8_t(0), playerPaddleX - 1);
      else playerPaddleX = min(int8_t(4), playerPaddleX + 1);
      break;
    case PATTERN_MINESWEEPER:
      if (primary) {
        mineTargetIndex = (mineTargetIndex + 1) % 5;
        resetMineDrop();
      } else {
        mineDropZ = 4;
      }
      break;
    case PATTERN_MOON_STARS:
      if (primary) bannerHue += 21;
      else cycleBrightness();
      break;
    case PATTERN_NIXIE_TUBE:
      if (primary) speedControl = speedControl >= 210 ? 80 : speedControl + 45;
      else cycleBrightness();
      break;
    case PATTERN_BLACK_HOLE:
      if (primary) blackHoleReverse = !blackHoleReverse;
      else cycleSpeed();
      break;
    case PATTERN_STARGATE:
      if (primary) { stargateHeldOpen = false; patternStartedAt = millis(); }
      else stargateHeldOpen = !stargateHeldOpen;
      break;
    case PATTERN_DEFENDER:
      if (primary) defenderShipX = max<int8_t>(0, defenderShipX - 1);
      else defenderShipX = min<int8_t>(4, defenderShipX + 1);
      break;
    case PATTERN_CHEQUERBOARD:
      if (primary) chequerReverse = !chequerReverse;
      else bannerHue += 27;
      break;
    case PATTERN_PUZZLE_CUBE:
      if (primary) puzzleGlow = puzzleGlow >= 220 ? 70 : puzzleGlow + 45;
      else cycleSpeed();
      break;
    case PATTERN_RUBIKS_CUBE:
      if (primary) { rubikSolvedReveal = false; patternStartedAt = millis(); }
      else rubikSolvedReveal = !rubikSolvedReveal;
      break;
    case PATTERN_LISSAJOUS_RIPPLE:
      if (primary) lissajousHue += 37;
      else cycleSpeed();
      break;
    case PATTERN_ZARCH:
      if (primary) resetZarchScene();
      else fireZarchShot();
      break;
    case PATTERN_MATRIX_RAIN:
    case PATTERN_MATRIX_DRIFT:
    case PATTERN_RED_MATRIX_RAIN:
      if (primary) cycleSpeed();
      else bannerHue += 19; // a quick hue accent for Matrix-family controls
      break;
    case PATTERN_FIREWORKS:
    case PATTERN_EXPLOSIONS:
      if (primary) patternStartedAt = millis() - cycleDurationMs + 1; // launch a fresh scene phase
      else cycleSpeed();
      break;
    default:
      if (primary) cycleSpeed();
      else cycleBrightness();
      break;
  }
}

void setupButtons() {
  pinMode(primaryButtonPin, INPUT_PULLUP);
  pinMode(secondaryButtonPin, INPUT_PULLUP);
  primaryRaw = primaryStable = digitalRead(primaryButtonPin);
  secondaryRaw = secondaryStable = digitalRead(secondaryButtonPin);
}

void updateButtons() {
  const uint32_t now = millis();
  const bool primary = digitalRead(primaryButtonPin);
  if (primary != primaryRaw) { primaryRaw = primary; primaryChangedAt = now; }
  if ((now - primaryChangedAt) >= BUTTON_DEBOUNCE_MS && primaryStable != primaryRaw) {
    primaryStable = primaryRaw;
    if (primaryStable == LOW) primaryPressedAt = now;
    else if (now - primaryPressedAt >= BUTTON_LONG_PRESS_MS) activateBannerMode();
    else {
      primaryTapStreak = now - lastPrimaryTapAt < 1050 ? primaryTapStreak + 1 : 1;
      lastPrimaryTapAt = now;
      if (primaryTapStreak >= 4) {
        startSecretScene(1);
        primaryTapStreak = 0;
      } else runShortPatternAction(true);
    }
  }

  const bool secondary = digitalRead(secondaryButtonPin);
  if (secondary != secondaryRaw) { secondaryRaw = secondary; secondaryChangedAt = now; }
  if ((now - secondaryChangedAt) >= BUTTON_DEBOUNCE_MS && secondaryStable != secondaryRaw) {
    secondaryStable = secondaryRaw;
    if (secondaryStable == LOW) secondaryPressedAt = now;
    else if (now - secondaryPressedAt >= BUTTON_LONG_PRESS_MS) {
      autoCycle = false;
      advancePattern();
    } else runShortPatternAction(false);
  }
}

// -----------------------------------------------------------------------------
// Browser API and UI
// -----------------------------------------------------------------------------
String stateJson() {
  String body;
  body.reserve(180);
  body += "{\"pattern\":" + String(uint8_t(currentPattern));
  body += ",\"name\":\"" + String(patternNames[currentPattern]) + "\"";
  body += ",\"brightness\":" + String(brightness);
  body += ",\"speed\":" + String(speedControl);
  body += ",\"fps\":" + String(frameRateLimit);
  body += ",\"auto\":" + String(autoCycle ? "true" : "false");
  body += ",\"cycle\":" + String(cycleDurationMs / 1000);
  body += ",\"banner\":\"" + String(bannerText) + "\"";
  body += ",\"bannerHue\":" + String(bannerHue);
  body += ",\"bannerSpeed\":" + String(bannerScrollSpeed);
  body += ",\"bannerFont\":" + String(uint8_t(bannerFont));
  body += ",\"ip\":\"" + networkAddress + "\"";
  body += ",\"ap\":" + String(usingAccessPoint ? "true" : "false") + "}";
  return body;
}

String frameJson() {
  String body = "{\"voxels\":[";
  body.reserve(1100);
  for (uint16_t i = 0; i < NUM_LEDS; ++i) {
    if (i) body += ',';
    char colour[10];
    snprintf(colour, sizeof(colour), "#%02X%02X%02X", leds[i].r, leds[i].g, leds[i].b);
    body += '"'; body += colour; body += '"';
  }
  body += "]}";
  return body;
}

#include "CubeFXWebPage.h"


void handleRoot() { web.send_P(200, "text/html", INDEX_HTML); }
void handleState() { web.send(200, "application/json", stateJson()); }
void handleFrame() { web.send(200, "application/json", frameJson()); }

void handleControl() {
  if (web.hasArg("pattern")) {
    const int value = web.arg("pattern").toInt();
    if (value >= 0 && value < PATTERN_COUNT) {
      currentPattern = Pattern(value);
      recordPatternRune(currentPattern);
      autoCycle = false;
      patternStartedAt = millis();
    }
  }
  if (web.hasArg("brightness")) {
    brightness = constrain(web.arg("brightness").toInt(), 1, 255);
    FastLED.setBrightness(brightness);
  }
  if (web.hasArg("speed")) speedControl = constrain(web.arg("speed").toInt(), 1, 255);
  if (web.hasArg("fps")) frameRateLimit = constrain(web.arg("fps").toInt(), 30, 120);
  if (web.hasArg("cycle")) cycleDurationMs = constrain(web.arg("cycle").toInt(), 5, 120) * 1000UL;
  if (web.hasArg("text")) {
    const String requestedText = web.arg("text");
    setBannerMessage(requestedText);
    if (requestedText == "OPEN EYE") startSecretScene(0);
  }
  if (web.hasArg("bannerHue")) bannerHue = constrain(web.arg("bannerHue").toInt(), 0, 255);
  if (web.hasArg("bannerSpeed")) bannerScrollSpeed = constrain(web.arg("bannerSpeed").toInt(), 1, 255);
  if (web.hasArg("bannerFont")) {
    bannerFont = web.arg("bannerFont").toInt() == 5 ? BANNER_FONT_5X5 : BANNER_FONT_3X5;
    bannerOffset = 0;
  }
  if (web.hasArg("auto")) { autoCycle = web.arg("auto").toInt() != 0; patternStartedAt = millis(); }
  if (web.hasArg("next")) { autoCycle = false; advancePattern(); }
  if (web.hasArg("reseed")) seedLife();
  handleState();
}

int readJsonInt(const String &json, const char *key, int fallback) {
  const String marker = String('"') + key + "\":";
  const int start = json.indexOf(marker);
  if (start < 0) return fallback;
  return json.substring(start + marker.length()).toInt();
}

bool readJsonBool(const String &json, const char *key, bool fallback) {
  const String marker = String('"') + key + "\":";
  const int start = json.indexOf(marker);
  if (start < 0) return fallback;
  const String value = json.substring(start + marker.length());
  if (value.startsWith("true")) return true;
  if (value.startsWith("false")) return false;
  return fallback;
}

String readJsonString(const String &json, const char *key, const String &fallback) {
  const String marker = String('"') + key + "\":\"";
  const int start = json.indexOf(marker);
  if (start < 0) return fallback;
  const int valueStart = start + marker.length();
  const int valueEnd = json.indexOf('"', valueStart);
  return valueEnd < 0 ? fallback : json.substring(valueStart, valueEnd);
}

bool isAllowedButtonPin(int pin) {
  // GPIO12–17 are normally flash pins; GPIO18/19 are USB-JTAG pins. Button
  // choices also must not overlap the compile-time FastLED output.
  return ((pin >= 0 && pin <= 11 && pin != DATA_PIN) || pin == 20 || pin == 21);
}

void loadButtonPinPreferences() {
  pinPreferences.begin("cubefx", true);
  const uint8_t savedPrimary = pinPreferences.getUChar("button1", primaryButtonPin);
  const uint8_t savedSecondary = pinPreferences.getUChar("button2", secondaryButtonPin);
  pinPreferences.end();
  if (isAllowedButtonPin(savedPrimary)) primaryButtonPin = savedPrimary;
  if (isAllowedButtonPin(savedSecondary) && savedSecondary != primaryButtonPin) secondaryButtonPin = savedSecondary;
}

bool saveButtonPins(int primary, int secondary) {
  if (!isAllowedButtonPin(primary) || !isAllowedButtonPin(secondary) || primary == secondary) return false;
  pinMode(primaryButtonPin, INPUT);
  pinMode(secondaryButtonPin, INPUT);
  primaryButtonPin = uint8_t(primary);
  secondaryButtonPin = uint8_t(secondary);
  setupButtons();
  pinPreferences.begin("cubefx", false);
  pinPreferences.putUChar("button1", primaryButtonPin);
  pinPreferences.putUChar("button2", secondaryButtonPin);
  pinPreferences.end();
  return true;
}

bool selectCanonicalPattern(int canonicalId) {
  // The mobile app catalog is the 48-pattern standalone catalogue. CubeFXWeb
  // currently embeds the 31 effects mapped here; other canonical demos remain
  // individually uploadable from patterns/ and are intentionally rejected.
  switch (canonicalId) {
    case 1: currentPattern = PATTERN_VECTOR_CUBE; break;
    case 9: currentPattern = PATTERN_MATRIX_RAIN; break;
    case 10: currentPattern = PATTERN_CORNER_CUBES; break;
    case 11: currentPattern = PATTERN_GLITTER; break;
    case 12: currentPattern = PATTERN_PONG; break;
    case 19: currentPattern = PATTERN_LIFE; break;
    case 20: currentPattern = PATTERN_CLOUDS; break;
    case 21: currentPattern = PATTERN_PLASMA; break;
    case 22: currentPattern = PATTERN_FIRE; break;
    case 26: currentPattern = PATTERN_SPIRALS; break;
    case 28: currentPattern = PATTERN_COMETS; break;
    case 29: currentPattern = PATTERN_BANNER; break;
    case 30: currentPattern = PATTERN_BULLET_WALL; break;
    case 31: currentPattern = PATTERN_PADDED_CELL; break;
    case 32: currentPattern = PATTERN_BLOCK_RUN; break;
    case 33: currentPattern = PATTERN_PARALLAX; break;
    case 34: currentPattern = PATTERN_TRENCH_RUN; break;
    case 35: currentPattern = PATTERN_RUNNING_LEGS; break;
    case 36: currentPattern = PATTERN_FAIRY_BOX; break;
    case 37: currentPattern = PATTERN_AQUARIUM; break;
    case 38: currentPattern = PATTERN_PYRAMID; break;
    case 39: currentPattern = PATTERN_MATRIX_DRIFT; break;
    case 40: currentPattern = PATTERN_INTENSE_FIRE; break;
    case 41: currentPattern = PATTERN_BLUE_FIRE; break;
    case 42: currentPattern = PATTERN_EXPLOSIONS; break;
    case 43: currentPattern = PATTERN_FIREWORKS; break;
    case 44: currentPattern = PATTERN_PIXEL_PASTURE; break;
    case 45: currentPattern = PATTERN_RED_MATRIX_RAIN; break;
    case 46: currentPattern = PATTERN_MINESWEEPER; break;
    case 47: currentPattern = PATTERN_MOON_STARS; break;
    case 48: currentPattern = PATTERN_NIXIE_TUBE; break;
    case 49: currentPattern = PATTERN_BLACK_HOLE; break;
    case 50: currentPattern = PATTERN_STARGATE; break;
    case 51: currentPattern = PATTERN_DEFENDER; break;
    case 52: currentPattern = PATTERN_CHEQUERBOARD; break;
    case 53: currentPattern = PATTERN_PUZZLE_CUBE; break;
    case 54: currentPattern = PATTERN_RUBIKS_CUBE; break;
    case 55: currentPattern = PATTERN_LISSAJOUS_RIPPLE; break;
    case 56: currentPattern = PATTERN_ZARCH; break;
    default: return false;
  }
  autoCycle = false;
  recordPatternRune(currentPattern);
  patternStartedAt = millis();
  return true;
}

void publishBleStatus(bool ok, const char *message) {
  if (!bleStatusCharacteristic) return;
  String status = "{\"ok\":" + String(ok ? "true" : "false") + ",\"pattern\":" + String(uint8_t(currentPattern));
  status += ",\"message\":\"" + String(message) + "\"}";
  bleStatusCharacteristic->setValue(status.c_str());
  bleStatusCharacteristic->notify();
}

void handleBleCommand(const String &json) {
  const String op = readJsonString(json, "op", "");
  if (op == "pattern") {
    if (selectCanonicalPattern(readJsonInt(json, "id", -1))) publishBleStatus(true, "pattern selected");
    else publishBleStatus(false, "standalone-only pattern");
    return;
  }
  if (op == "engine") {
    brightness = constrain(readJsonInt(json, "brightness", brightness), 1, 255);
    speedControl = constrain(readJsonInt(json, "speed", speedControl), 1, 255);
    cycleDurationMs = constrain(readJsonInt(json, "cycle", cycleDurationMs / 1000), 5, 120) * 1000UL;
    autoCycle = readJsonBool(json, "auto", autoCycle);
    FastLED.setBrightness(brightness);
    if (brightness == 42 && speedControl == 42) startSecretScene(3);
    publishBleStatus(true, "engine applied");
    return;
  }
  if (op == "banner") {
    const String requestedText = readJsonString(json, "text", String(bannerText));
    setBannerMessage(requestedText);
    if (requestedText == "OPEN EYE") startSecretScene(0);
    bannerFont = readJsonInt(json, "font", uint8_t(bannerFont)) == 5 ? BANNER_FONT_5X5 : BANNER_FONT_3X5;
    bannerHue = constrain(readJsonInt(json, "hue", bannerHue), 0, 255);
    bannerScrollSpeed = constrain(readJsonInt(json, "speed", bannerScrollSpeed), 1, 255);
    bannerOffset = 0;
    publishBleStatus(true, "banner applied");
    return;
  }
  if (op == "next") { autoCycle = false; advancePattern(); publishBleStatus(true, "next pattern"); return; }
  if (op == "reseed") { seedLife(); publishBleStatus(true, "life reseeded"); return; }
  if (op == "action") {
    runShortPatternAction(readJsonBool(json, "primary", true));
    publishBleStatus(true, "pattern action");
    return;
  }
  if (op == "pins") {
    const int requestedData = readJsonInt(json, "dataPin", DATA_PIN);
    const int requestedPrimary = readJsonInt(json, "primaryPin", primaryButtonPin);
    const int requestedSecondary = readJsonInt(json, "secondaryPin", secondaryButtonPin);
    if (!saveButtonPins(requestedPrimary, requestedSecondary)) {
      publishBleStatus(false, "unsafe button pins");
    } else if (requestedData != DATA_PIN) {
      publishBleStatus(true, "buttons saved; reflash data pin");
    } else {
      publishBleStatus(true, "button pins saved");
    }
    return;
  }
  publishBleStatus(false, "unknown command");
}

class CubeFXBleCommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    const String received = characteristic->getValue();
    if (received.length() > 0 && received.length() <= 180) handleBleCommand(received);
  }
};

class CubeFXBleServerCallbacks : public BLEServerCallbacks {
  void onDisconnect(BLEServer *server) override {
    // Android may reconnect after a screen rotation or range change; resume
    // advertising immediately so the CubeFX device remains discoverable.
    server->getAdvertising()->start();
  }
};

void setupBle() {
  BLEDevice::init("CubeFX-5x5x5");
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new CubeFXBleServerCallbacks());
  BLEService *service = bleServer->createService(CUBEFX_BLE_SERVICE_UUID);
  BLECharacteristic *command = service->createCharacteristic(
    CUBEFX_BLE_COMMAND_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  command->setCallbacks(new CubeFXBleCommandCallbacks());
  bleStatusCharacteristic = service->createCharacteristic(
    CUBEFX_BLE_STATUS_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  bleStatusCharacteristic->addDescriptor(new BLE2902());
  publishBleStatus(true, "ready");
  service->start();
  bleServer->getAdvertising()->addServiceUUID(CUBEFX_BLE_SERVICE_UUID);
  bleServer->getAdvertising()->start();
}

void setupWeb() {
  web.on("/", HTTP_GET, handleRoot);
  web.on("/api/state", HTTP_GET, handleState);
  web.on("/api/frame", HTTP_GET, handleFrame);
  web.on("/api/control", HTTP_GET, handleControl);
  web.onNotFound(handleRoot);
  web.begin();
}

void setupNetwork() {
  WiFi.mode(WIFI_STA);
  if (strlen(WIFI_SSID) > 0) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    const uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < 12000) delay(100);
    if (WiFi.status() == WL_CONNECTED) {
      usingAccessPoint = false;
      networkAddress = WiFi.localIP().toString();
      return;
    }
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  usingAccessPoint = true;
  networkAddress = WiFi.softAPIP().toString();
}

void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  buildGeometryLUT();
  randomSeed(esp_random());
  resetZarchScene();
  seedLife();
  loadButtonPinPreferences();
  setupButtons();
  setupNetwork();
  setupWeb();
  setupBle();
  patternStartedAt = millis();
}

void loop() {
  web.handleClient();
  updateButtons();

  const uint32_t now = millis();
  if (autoCycle && now - patternStartedAt >= cycleDurationMs) advancePattern();
  if (!autoCycle && currentPattern == PATTERN_CLOUDS && secretScene == 255 && now - patternStartedAt >= 240000UL) {
    startSecretScene(4);
  }

  // The old fixed 16 ms gate capped all effects at about 60 FPS. At 125 LEDs,
  // WS2812B output occupies only a small fraction of the ESP32-C3's time, so a
  // configurable 30–120 FPS cap keeps motion smooth while handleClient() still
  // runs on every pass through loop().
  if (now - lastFrameAt >= frameIntervalMs()) {
    lastFrameAt = now;
    renderCurrentPattern();
    FastLED.show();
  }
}
