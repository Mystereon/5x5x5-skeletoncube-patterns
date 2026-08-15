/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  OrangeFishTank.ino — fully standalone SkeletonCube / CubeFX pattern demo
  Created by Dad (MysterEon) & Manus, 2026.

  Pattern: Blue-framed water volume with two orange fish.

  This folder is directly uploadable. It has no local includes and no dependency
  on CubeFXWeb, the browser controller, or the master sketch.

  Cube map: origin (0,0,0) is bottom-rear-left.
  x: left -> right; y: rear -> front; z: bottom -> top.
  Default physical map: index = z * 25 + y * 5 + x.

  Hardware defaults: ESP32-C3, 125 GRB WS2812B LEDs, DATA_PIN 2, brightness 100.
  Change the configuration constants below only if your physical wiring differs.
*/

#include <FastLED.h>
#include <esp_system.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Cube configuration and physical mapper
// -----------------------------------------------------------------------------
constexpr uint8_t N = 5;
constexpr uint16_t NUM_LEDS = N * N * N;
#define DATA_PIN 2
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
  PATTERN_COUNT
};

const char *const patternNames[PATTERN_COUNT] = {
  "Red Vector Cube", "3-D Matrix Rain", "Neon Plasma", "Volume Fire",
  "Twin Spirals", "Wrapping Comets", "Self-playing Pong", "Conway 3-D Life",
  "Cloud Volume", "White Glitter", "Corner Cubes", "3x5 Perimeter Banner",
  "Bullet Wall", "Padded Cell", "Block Run", "Parallax Starfield", "Trench Run",
  "Running Legs", "Fairies in Green Box", "Orange Fish Tank", "Three-Layer Pyramid", "Matrix Drift",
  "Intense Fire", "Magical Blue Fire", "Explosions", "Launching Fireworks", "Pixel Pasture", "Red Matrix Rain"
};

Pattern currentPattern = PATTERN_AQUARIUM;
bool autoCycle = false; // Standalone demos never auto-cycle.
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
  for (uint8_t z = 0; z < N; ++z) {
    for (uint8_t y = 0; y < N; ++y) {
      for (uint8_t x = 0; x < N; ++x) {
        const float wave = sinf(x * 1.7f + t) + sinf(y * 1.3f - t * 1.2f) + sinf(z * 1.9f + t * 0.7f);
        const uint8_t hue = uint8_t((wave + 3.0f) * 42.0f + t * 18.0f);
        const uint8_t value = uint8_t(105 + (sinf(wave * 2.0f + t) + 1.0f) * 75.0f);
        setVoxel(x, y, z, CHSV(hue, 255, value));
      }
    }
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
  const float radius = phase < 2200.0f ? phase * 3.6f / 2200.0f : 3.6f;
  const float fade = phase < 2200.0f ? 1.0f : (3100.0f - phase) / 900.0f;
  const int8_t cx = 2 + (uint8_t(t * 0.35f) & 1);
  const int8_t cy = 2;
  const int8_t cz = 2;
  for (int8_t z = 0; z < N; ++z) for (int8_t y = 0; y < N; ++y) for (int8_t x = 0; x < N; ++x) {
    const float dx = x - cx, dy = y - cy, dz = z - cz;
    const float distance = sqrtf(dx * dx + dy * dy + dz * dz);
    const float shell = fabsf(distance - radius);
    if (shell < 0.72f) {
      const uint8_t value = uint8_t(255.0f * fade * (1.0f - shell / 0.72f));
      const uint8_t hue = distance < radius * 0.45f ? 18 : 5;
      setVoxel(x, y, z, CHSV(hue, 245, value));
    } else if (distance < radius && ((x * 13 + y * 7 + z * 3 + uint8_t(phase)) & 3) == 0) {
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

int8_t pongX = 2, pongY = 2, pongZ = 2;
int8_t pongDX = 1, pongDY = 1, pongDZ = 1;
uint32_t lastPongAt = 0;
void renderPong(float t) {
  const uint16_t stepMs = 1300 - speedControl * 4;
  if (millis() - lastPongAt >= stepMs) {
    lastPongAt = millis();
    pongX += pongDX; pongY += pongDY; pongZ += pongDZ;
    if (pongX <= 0 || pongX >= 4) pongDX = -pongDX;
    if (pongY <= 0 || pongY >= 4) pongDY = -pongDY;
    if (pongZ <= 0 || pongZ >= 4) pongDZ = -pongDZ;
    pongX = constrain(pongX, 0, 4); pongY = constrain(pongY, 0, 4); pongZ = constrain(pongZ, 0, 4);
  }
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t y = 1; y <= 3; ++y) for (uint8_t z = 1; z <= 3; ++z) {
    setVoxel(0, y, z, CRGB::Blue);
    setVoxel(4, y, z, CRGB::Red);
  }
  setVoxel(pongX, pongY, pongZ, CRGB::White);
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
  const bool leftForward = sinf(t * 5.4f) > 0.0f;
  const int8_t leftKnee = leftForward ? 1 : 3;
  const int8_t rightKnee = leftForward ? 3 : 1;
  const int8_t leftFoot = leftForward ? 0 : 4;
  const int8_t rightFoot = leftForward ? 4 : 0;
  // Compact stick runner: hip, two alternating knees, and wide-stepping feet.
  setVoxel(2, 2, 4, CRGB(50, 120, 255));
  setVoxel(2, 2, 3, CRGB(230, 50, 35));
  setVoxel(1, 2, 3, CRGB(255, 175, 95));
  setVoxel(3, 2, 3, CRGB(255, 175, 95));
  setVoxel(1, leftKnee, 2, CRGB(245, 210, 80));
  setVoxel(1, leftFoot, 0, CRGB(255, 255, 255));
  setVoxel(3, rightKnee, 2, CRGB(245, 210, 80));
  setVoxel(3, rightFoot, 0, CRGB(255, 255, 255));
  setVoxel(0, leftFoot, 0, CRGB(120, 120, 120));
  setVoxel(4, rightFoot, 0, CRGB(120, 120, 120));
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

void renderCurrentPattern() {
  const float t = effectTime();
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
    default: break;
  }
}

void advancePattern() {
  currentPattern = Pattern((currentPattern + 1) % PATTERN_COUNT);
  patternStartedAt = millis();
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

// -----------------------------------------------------------------------------
// Optional hardware buttons: GPIO4 next; GPIO8 auto/manual
// -----------------------------------------------------------------------------
constexpr uint8_t NEXT_BUTTON_PIN = 4;
constexpr uint8_t AUTO_BUTTON_PIN = 8;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 35;
bool nextRaw = HIGH, nextStable = HIGH, autoRaw = HIGH, autoStable = HIGH;
uint32_t nextChangedAt = 0, autoChangedAt = 0;

void setupButtons() {
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(AUTO_BUTTON_PIN, INPUT_PULLUP);
  nextRaw = nextStable = digitalRead(NEXT_BUTTON_PIN);
  autoRaw = autoStable = digitalRead(AUTO_BUTTON_PIN);
}

void updateButtons() {
  const uint32_t now = millis();
  const bool n = digitalRead(NEXT_BUTTON_PIN);
  if (n != nextRaw) { nextRaw = n; nextChangedAt = now; }
  if ((now - nextChangedAt) >= BUTTON_DEBOUNCE_MS && nextStable != nextRaw) {
    nextStable = nextRaw;
    if (nextStable == LOW && !autoCycle) advancePattern();
  }

  const bool a = digitalRead(AUTO_BUTTON_PIN);
  if (a != autoRaw) { autoRaw = a; autoChangedAt = now; }
  if ((now - autoChangedAt) >= BUTTON_DEBOUNCE_MS && autoStable != autoRaw) {
    autoStable = autoRaw;
    if (autoStable == LOW) { autoCycle = !autoCycle; patternStartedAt = now; }
  }
}


void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  randomSeed(esp_random());
  seedLife(); // Harmless for non-Life effects; necessary for the Life demo.
  patternStartedAt = millis();
}

void loop() {
  const uint32_t now = millis();
  if (now - lastFrameAt >= frameIntervalMs()) {
    lastFrameAt = now;
    renderCurrentPattern();
    FastLED.show();
  }
}
