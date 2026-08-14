/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  Conway3DLife.ino — standalone 5 x 5 x 5 FastLED voxel cellular automaton
  Created by Dad (MysterEon) & Manus, 2026.

  This file is completely independent: copy this folder into your Arduino
  sketchbook, install FastLED, and upload. It does not include the SkeletonCube
  master pattern library.

  Cube map used here:
    (0,0,0) = bottom, rear, left
    x: left to right; y: rear to front; z: bottom to top
    index = z * 25 + y * 5 + x

  Hardware defaults:
    125 WS2812B LEDs, GRB, data on GPIO2, brightness 100.

  Optional ESP32-C3 SuperMini controls:
    GPIO4 ---- normally-open button ---- GND : seed a fresh Life world
    GPIO8 ---- normally-open button ---- GND : pause / resume

  GPIO8 is a C3 strapping pin. Do not hold its button during reset or power-up.
  Add a 10 kΩ pull-up from GPIO8 to 3V3 if your board does not already keep it
  high during boot. GPIO8 may also be attached to a status LED on some boards.
*/

#include <FastLED.h>
#ifdef ESP32
  #include <esp_system.h>
#endif

// ---------- LED cube configuration ----------
constexpr uint8_t N = 5;
constexpr uint16_t NUM_LEDS = N * N * N;
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t BRIGHTNESS = 100;

// This matches Dad's confirmed map: non-serpentine left-to-right rows.
constexpr bool SWAP_XY = false;
constexpr bool FLIP_X = false;
constexpr bool FLIP_Y = false;
constexpr bool FLIP_Z = false;
constexpr bool SERPENTINE_ROWS = false;
constexpr bool SERPENTINE_LAYERS = false;
constexpr bool SHOW_MAPPING_MARKERS = false;

CRGB leds[NUM_LEDS];

uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) {
  uint8_t px = x;
  uint8_t py = y;
  uint8_t pz = z;

  if (SWAP_XY) {
    const uint8_t t = px;
    px = py;
    py = t;
  }
  if (FLIP_X) px = N - 1 - px;
  if (FLIP_Y) py = N - 1 - py;
  if (FLIP_Z) pz = N - 1 - pz;
  if (SERPENTINE_ROWS && (py & 1)) px = N - 1 - px;
  if (SERPENTINE_LAYERS && (pz & 1)) py = N - 1 - py;

  return uint16_t(pz) * N * N + uint16_t(py) * N + px;
}

void setVoxel(uint8_t x, uint8_t y, uint8_t z, const CRGB &colour) {
  if (x < N && y < N && z < N) leds[indexFromXYZ(x, y, z)] = colour;
}

void renderMappingMarkers() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  setVoxel(0, 0, 0, CRGB::Red);       // bottom-rear-left
  setVoxel(4, 0, 0, CRGB::Green);     // bottom-rear-right
  setVoxel(0, 4, 0, CRGB::Blue);      // bottom-front-left
  setVoxel(0, 0, 4, CRGB::White);     // top-rear-left
  FastLED.show();
}

// ---------- Conway-style 3-D Life ----------
// A 3-D cell has up to 26 neighbours. B5/S45 is a compact, lively rule for
// this tiny volume: a dead cell is born with exactly five neighbours; a live
// cell survives with four or five.
constexpr uint8_t BIRTH_NEIGHBOURS = 5;
constexpr uint8_t SURVIVE_MIN = 4;
constexpr uint8_t SURVIVE_MAX = 5;
constexpr uint16_t GENERATION_MS = 280;
constexpr uint8_t INITIAL_DENSITY = 54; // ~21% probability (out of 255)

bool life[N][N][N];
bool nextLife[N][N][N];
uint8_t age[N][N][N];
uint32_t lastGenerationAt = 0;
uint16_t generation = 0;
bool paused = false;

uint8_t neighbourCount(int8_t x, int8_t y, int8_t z) {
  uint8_t count = 0;
  for (int8_t dz = -1; dz <= 1; ++dz) {
    for (int8_t dy = -1; dy <= 1; ++dy) {
      for (int8_t dx = -1; dx <= 1; ++dx) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const int8_t nx = x + dx;
        const int8_t ny = y + dy;
        const int8_t nz = z + dz;
        if (nx >= 0 && nx < N && ny >= 0 && ny < N && nz >= 0 && nz < N && life[nx][ny][nz]) ++count;
      }
    }
  }
  return count;
}

void seedLife() {
  uint8_t population = 0;
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        // Slightly denser central region helps the small cube start interesting.
        const int8_t manhattan = abs(int8_t(x) - 2) + abs(int8_t(y) - 2) + abs(int8_t(z) - 2);
        const uint8_t density = (manhattan <= 3) ? INITIAL_DENSITY + 20 : INITIAL_DENSITY;
        life[x][y][z] = random8() < density;
        age[x][y][z] = life[x][y][z] ? 1 : 0;
        if (life[x][y][z]) ++population;
      }
    }
  }

  // Do not start with an empty or nearly empty world.
  if (population < 9) {
    for (uint8_t i = 0; i < 18; ++i) {
      const uint8_t x = random8(N);
      const uint8_t y = random8(N);
      const uint8_t z = random8(N);
      life[x][y][z] = true;
      age[x][y][z] = 1;
    }
  }
  generation = 0;
  lastGenerationAt = millis();
}

void stepLife() {
  uint8_t population = 0;

  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const uint8_t neighbours = neighbourCount(x, y, z);
        const bool alive = life[x][y][z];
        const bool survives = alive && neighbours >= SURVIVE_MIN && neighbours <= SURVIVE_MAX;
        const bool born = !alive && neighbours == BIRTH_NEIGHBOURS;
        nextLife[x][y][z] = survives || born;
      }
    }
  }

  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const bool wasAlive = life[x][y][z];
        life[x][y][z] = nextLife[x][y][z];
        if (life[x][y][z]) {
          const uint8_t nextAge = age[x][y][z] + 1;
          age[x][y][z] = wasAlive ? (nextAge > 12 ? 12 : nextAge) : 1;
          ++population;
        } else {
          age[x][y][z] = 0;
        }
      }
    }
  }

  ++generation;
  // A small finite cube often settles or dies. Restart rather than going dark.
  if (population == 0 || generation >= 160) seedLife();
}

void renderLife() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        if (!life[x][y][z]) continue;

        const uint8_t cellAge = age[x][y][z];
        // New births are white, then mature through bright emerald/cyan greens.
        if (cellAge <= 2) {
          setVoxel(x, y, z, CRGB::White);
        } else {
          const uint8_t hueOffset = cellAge * 3 > 42 ? 42 : cellAge * 3;
          const uint8_t valueOffset = cellAge * 9 > 110 ? 110 : cellAge * 9;
          const uint8_t hue = 86 + hueOffset;
          const uint8_t value = 140 + valueOffset;
          setVoxel(x, y, z, CHSV(hue, 230, value));
        }
      }
    }
  }

  // Small centre marker says "paused" without hiding the current world.
  if (paused) leds[indexFromXYZ(2, 2, 2)] += CRGB(48, 0, 0);
}

// ---------- Optional GPIO4 / GPIO8 controls ----------
constexpr bool ENABLE_BUTTONS = true;
constexpr uint8_t SEED_BUTTON_PIN = 4;
constexpr uint8_t PAUSE_BUTTON_PIN = 8;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;
bool seedRaw = HIGH, seedStable = HIGH;
bool pauseRaw = HIGH, pauseStable = HIGH;
uint32_t seedChangedAt = 0, pauseChangedAt = 0;

void setupButtons() {
  if (!ENABLE_BUTTONS) return;
  pinMode(SEED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
  seedRaw = seedStable = digitalRead(SEED_BUTTON_PIN);
  pauseRaw = pauseStable = digitalRead(PAUSE_BUTTON_PIN);
}

bool seedButtonPressed() {
  if (!ENABLE_BUTTONS) return false;
  const uint32_t now = millis();
  const bool reading = digitalRead(SEED_BUTTON_PIN);
  if (reading != seedRaw) {
    seedRaw = reading;
    seedChangedAt = now;
  }
  if ((now - seedChangedAt) >= BUTTON_DEBOUNCE_MS && seedStable != seedRaw) {
    seedStable = seedRaw;
    return seedStable == LOW;
  }
  return false;
}

bool pauseButtonPressed() {
  if (!ENABLE_BUTTONS) return false;
  const uint32_t now = millis();
  const bool reading = digitalRead(PAUSE_BUTTON_PIN);
  if (reading != pauseRaw) {
    pauseRaw = reading;
    pauseChangedAt = now;
  }
  if ((now - pauseChangedAt) >= BUTTON_DEBOUNCE_MS && pauseStable != pauseRaw) {
    pauseStable = pauseRaw;
    return pauseStable == LOW;
  }
  return false;
}

void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
#ifdef ESP32
  randomSeed(esp_random());
#else
  randomSeed(analogRead(A0));
#endif
  setupButtons();

  if (SHOW_MAPPING_MARKERS) {
    renderMappingMarkers();
    return;
  }
  seedLife();
  renderLife();
  FastLED.show();
}

void loop() {
  if (SHOW_MAPPING_MARKERS) return;

  if (seedButtonPressed()) seedLife();
  if (pauseButtonPressed()) paused = !paused;

  const uint32_t now = millis();
  if (!paused && now - lastGenerationAt >= GENERATION_MS) {
    lastGenerationAt = now;
    stepLife();
  }

  renderLife();
  FastLED.show();
}
