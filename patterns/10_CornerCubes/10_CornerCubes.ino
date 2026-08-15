/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  10_CornerCubes.ino — fully standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.

  Pattern: Eight colour-shifting 2×2×2 corner cubes.

  This is a complete direct-upload Arduino sketch: no local includes, no master
  sketch dependency, and no web-controller dependency.

  Cube map: origin (0,0,0) = bottom-rear-left.
  x: left -> right; y: rear -> front; z: bottom -> top.
  Default map: index = z * 25 + y * 5 + x.
*/

#include <FastLED.h>
#include <esp_system.h>
#include "DemoTypes.h"
#include <math.h>

// ---------- Hardware ----------
#define DATA_PIN    2
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB

constexpr uint8_t N = 5;
constexpr uint16_t LAYER_LEDS = N * N;
constexpr uint16_t NUM_LEDS = N * N * N;
constexpr uint8_t BRIGHTNESS = 100;

CRGB leds[NUM_LEDS];

// ---------- Physical wiring map ----------
// Confirmed default: every row runs from left to right.
const bool SWAP_XY           = false;
const bool FLIP_X            = false;
const bool FLIP_Y            = false;
const bool FLIP_Z            = false;
const bool SERPENTINE_ROWS   = false;
const bool SERPENTINE_LAYERS = false;

// Set true only while validating the physical map.
const bool SHOW_MAPPING_MARKERS = false;

// ---------- Coordinate mapper ----------
uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) {
  uint8_t px = x;
  uint8_t py = y;
  uint8_t pz = z;

  if (SWAP_XY) {
    uint8_t temp = px;
    px = py;
    py = temp;
  }
  if (FLIP_X) px = N - 1 - px;
  if (FLIP_Y) py = N - 1 - py;
  if (FLIP_Z) pz = N - 1 - pz;

  uint8_t xInLayer = px;
  if (SERPENTINE_ROWS && (py & 1)) xInLayer = N - 1 - px;

  uint16_t inLayer = py * N + xInLayer;
  if (SERPENTINE_LAYERS && (pz & 1)) inLayer = LAYER_LEDS - 1 - inLayer;

  return pz * LAYER_LEDS + inLayer;
}

void setVoxel(int x, int y, int z, const CRGB &colour) {
  if (x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N) return;
  leds[indexFromXYZ((uint8_t)x, (uint8_t)y, (uint8_t)z)] = colour;
}

void addVoxel(int x, int y, int z, const CRGB &colour) {
  if (x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N) return;
  leds[indexFromXYZ((uint8_t)x, (uint8_t)y, (uint8_t)z)] += colour;
}

void renderMappingMarkers() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  setVoxel(0, 0, 0, CRGB::Red);    // Origin: bottom-rear-left.
  setVoxel(4, 0, 0, CRGB::Green);  // +x: right.
  setVoxel(0, 4, 0, CRGB::Blue);   // +y: front.
  setVoxel(0, 0, 4, CRGB::White);  // +z: top.
  FastLED.show();
}

// ---------- Small 3-D math layer ----------
struct Voxel {
  int8_t x;
  int8_t y;
  int8_t z;
};

Vec3 rotX(Vec3 p, float a) {
  const float c = cosf(a), s = sinf(a);
  return {p.x, p.y * c - p.z * s, p.y * s + p.z * c};
}

Vec3 rotY(Vec3 p, float a) {
  const float c = cosf(a), s = sinf(a);
  return {p.x * c - p.z * s, p.y, p.x * s + p.z * c};
}

Vec3 rotZ(Vec3 p, float a) {
  const float c = cosf(a), s = sinf(a);
  return {p.x * c - p.y * s, p.x * s + p.y * c, p.z};
}

Vec3 rotateForward(Vec3 p, float ax, float ay, float az) {
  p = rotY(p, ay);
  p = rotX(p, ax);
  return rotZ(p, az);
}

Vec3 rotateInverse(Vec3 p, float ax, float ay, float az) {
  p = rotZ(p, -az);
  p = rotX(p, -ax);
  return rotY(p, -ay);
}

float max3(float a, float b, float c) {
  return max(a, max(b, c));
}

bool insideShape(Vec3 p, uint8_t shape) {
  const float ax = fabsf(p.x);
  const float ay = fabsf(p.y);
  const float az = fabsf(p.z);

  switch (shape) {
    case 1:  // Solid cube.
      return max3(ax, ay, az) <= 1.55f;
    case 2:  // Solid sphere.
      return (p.x * p.x + p.y * p.y + p.z * p.z) <= 3.15f;
    case 3:  // Solid octahedron / diamond.
      return (ax + ay + az) <= 2.45f;
  }
  return false;
}

bool insideVectorCube(Vec3 p) {
  // A 3x3x3 wire cube: points are near its edges when two object-space
  // coordinates are close to +/- 1.  Red is deliberately fixed.
  const float ax = fabsf(p.x);
  const float ay = fabsf(p.y);
  const float az = fabsf(p.z);
  const float edge = 1.0f;
  const float thickness = 0.34f;

  if (max3(ax, ay, az) > edge + thickness) return false;
  uint8_t nearEdge = 0;
  if (ax >= edge - thickness) ++nearEdge;
  if (ay >= edge - thickness) ++nearEdge;
  if (az >= edge - thickness) ++nearEdge;
  return nearEdge >= 2;
}

CRGB objectColour(Vec3 objectPoint) {
  int hue = 128 + (int)(objectPoint.x * 29.0f)
                + (int)(objectPoint.y * 17.0f)
                + (int)(objectPoint.z * 11.0f);
  return CHSV((uint8_t)hue, 255, 255);
}

void renderRedVectorCube(float ax, float ay, float az) {
  for (int z = 0; z < N; ++z) {
    for (int y = 0; y < N; ++y) {
      for (int x = 0; x < N; ++x) {
        Vec3 cubePoint = {(float)x - 2.0f, (float)y - 2.0f, (float)z - 2.0f};
        Vec3 p = rotateInverse(cubePoint, ax, ay, az);
        if (insideVectorCube(p)) setVoxel(x, y, z, CRGB::Red);
      }
    }
  }
}

void renderImplicitShape(float ax, float ay, float az, uint8_t shape) {
  for (int z = 0; z < N; ++z) {
    for (int y = 0; y < N; ++y) {
      for (int x = 0; x < N; ++x) {
        Vec3 cubePoint = {(float)x - 2.0f, (float)y - 2.0f, (float)z - 2.0f};
        Vec3 p = rotateInverse(cubePoint, ax, ay, az);
        if (insideShape(p, shape)) setVoxel(x, y, z, objectColour(p));
      }
    }
  }
}

// ---------- Hand-authored spinning voxel model ----------
const Voxel MODEL[] = {
  { 0, 0,  2},
  { 0, 0,  1}, { 1, 0,  1}, {-1, 0,  1},
  { 0, 0,  0}, { 1, 0,  0}, {-1, 0,  0},
  { 0, 1,  0}, { 0,-1,  0},
  { 0, 0, -1}, { 1, 0, -1}, {-1, 0, -1},
  { 0, 0, -2}
};
constexpr uint8_t MODEL_COUNT = sizeof(MODEL) / sizeof(MODEL[0]);

void renderVoxelModel(float ax, float ay, float az) {
  for (uint8_t i = 0; i < MODEL_COUNT; ++i) {
    Vec3 p = {(float)MODEL[i].x, (float)MODEL[i].y, (float)MODEL[i].z};
    Vec3 q = rotateForward(p, ax, ay, az);
    setVoxel((int)lroundf(q.x + 2.0f),
             (int)lroundf(q.y + 2.0f),
             (int)lroundf(q.z + 2.0f), objectColour(p));
  }
}

// ---------- Bouncing 2 x 2 x 2 cube ----------
constexpr uint8_t BLOCK_SIZE = 2;
constexpr int8_t BLOCK_MAX = N - BLOCK_SIZE;

int8_t blockX = 0, blockY = 1, blockZ = 2;
int8_t velX = 1, velY = 1, velZ = -1;
uint32_t lastXStep = 0, lastYStep = 0, lastZStep = 0;
constexpr uint16_t X_STEP_MS = 115;
constexpr uint16_t Y_STEP_MS = 167;
constexpr uint16_t Z_STEP_MS = 223;

void stepBlockAxis(int8_t &position, int8_t &velocity) {
  if (position + velocity < 0 || position + velocity > BLOCK_MAX) velocity = -velocity;
  position += velocity;
}

void updateBouncingBlock() {
  const uint32_t now = millis();
  if (now - lastXStep >= X_STEP_MS) { lastXStep = now; stepBlockAxis(blockX, velX); }
  if (now - lastYStep >= Y_STEP_MS) { lastYStep = now; stepBlockAxis(blockY, velY); }
  if (now - lastZStep >= Z_STEP_MS) { lastZStep = now; stepBlockAxis(blockZ, velZ); }
}

void renderBouncingBlock() {
  updateBouncingBlock();
  const CRGB colour = CHSV(20 + 27 * blockX + 19 * blockY + 13 * blockZ, 255, 255);
  for (int dz = 0; dz < BLOCK_SIZE; ++dz)
    for (int dy = 0; dy < BLOCK_SIZE; ++dy)
      for (int dx = 0; dx < BLOCK_SIZE; ++dx)
        setVoxel(blockX + dx, blockY + dy, blockZ + dz, colour);
}

// ---------- Sparse blue rain ----------
constexpr uint8_t MAX_DROPS = 15;
constexpr uint16_t RAIN_STEP_MS = 85;

struct Drop {
  int8_t x, y, z;
  bool active;
  CRGB colour;
};

Drop drops[MAX_DROPS];
uint32_t lastRainStep = 0;

void spawnDrop() {
  for (uint8_t i = 0; i < MAX_DROPS; ++i) {
    if (!drops[i].active) {
      const CRGB rainColours[] = {CRGB::Blue, CRGB::DeepSkyBlue, CRGB::Cyan, CRGB::LightBlue};
      drops[i].x = random8(N);
      drops[i].y = random8(N);
      drops[i].z = N - 1;
      drops[i].colour = rainColours[random8(4)];
      drops[i].active = true;
      return;
    }
  }
}

void renderRain() {
  const uint32_t now = millis();
  if (now - lastRainStep < RAIN_STEP_MS) return;
  lastRainStep = now;

  fadeToBlackBy(leds, NUM_LEDS, 60);
  if (random8() < 80) spawnDrop();

  for (uint8_t i = 0; i < MAX_DROPS; ++i) {
    if (!drops[i].active) continue;
    addVoxel(drops[i].x, drops[i].y, drops[i].z, drops[i].colour);
    if (--drops[i].z < 0) drops[i].active = false;
  }
}

// ---------- Vertical x-column sweep ----------
constexpr uint16_t COLUMN_STEP_MS = 500;
uint8_t sweepX = 0;
uint32_t lastColumnStep = 0;

void renderColumnSweep() {
  const uint32_t now = millis();
  if (now - lastColumnStep >= COLUMN_STEP_MS) {
    lastColumnStep = now;
    sweepX = (sweepX + 1) % N;
  }
  const CRGB colour = CHSV(sweepX * 50, 255, 255);
  for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
      setVoxel(sweepX, y, z, colour);
}

// ---------- Dense 3-D green Matrix rain ----------
struct MatrixColumn {
  int8_t headZ;
  uint8_t phase;
};

MatrixColumn matrixColumns[N * N];
bool matrixReady = false;
uint32_t lastMatrixStep = 0;
constexpr uint16_t MATRIX_STEP_MS = 55;

void resetMatrixRain() {
  for (uint8_t i = 0; i < N * N; ++i) {
    matrixColumns[i].headZ = random8(N);
    matrixColumns[i].phase = random8(4);
  }
  matrixReady = true;
}

void renderMatrixRain() {
  if (!matrixReady) resetMatrixRain();
  const uint32_t now = millis();
  if (now - lastMatrixStep < MATRIX_STEP_MS) return;
  lastMatrixStep = now;

  fadeToBlackBy(leds, NUM_LEDS, 72);

  for (uint8_t y = 0; y < N; ++y) {
    for (uint8_t x = 0; x < N; ++x) {
      MatrixColumn &column = matrixColumns[y * N + x];
      if (++column.phase >= 4) {
        column.phase = 0;
        if (--column.headZ < 0) column.headZ = N - 1;
      }

      // Bright head with three green trail levels above it.
      addVoxel(x, y, column.headZ, CRGB(140, 255, 160));
      addVoxel(x, y, column.headZ + 1, CRGB(0, 150, 16));
      addVoxel(x, y, column.headZ + 2, CRGB(0, 72, 5));
      addVoxel(x, y, column.headZ + 3, CRGB(0, 26, 1));
    }
  }
}

// ---------- Eight colour-shifting corner cubes ----------
void renderCornerCubes() {
  const uint8_t t = (uint8_t)(millis() >> 4);
  for (uint8_t corner = 0; corner < 8; ++corner) {
    const int8_t baseX = (corner & 1) ? BLOCK_MAX : 0;
    const int8_t baseY = (corner & 2) ? BLOCK_MAX : 0;
    const int8_t baseZ = (corner & 4) ? BLOCK_MAX : 0;
    const CRGB colour = CHSV(t + corner * 31, 255, 255);

    for (int dz = 0; dz < BLOCK_SIZE; ++dz)
      for (int dy = 0; dy < BLOCK_SIZE; ++dy)
        for (int dx = 0; dx < BLOCK_SIZE; ++dx)
          setVoxel(baseX + dx, baseY + dy, baseZ + dz, colour);
  }
}

// ---------- White random glitter ----------
uint32_t lastGlitterStep = 0;
constexpr uint16_t GLITTER_STEP_MS = 28;

void renderGlitter() {
  const uint32_t now = millis();
  if (now - lastGlitterStep < GLITTER_STEP_MS) return;
  lastGlitterStep = now;

  fadeToBlackBy(leds, NUM_LEDS, 36);
  for (uint8_t i = 0; i < 3; ++i) {
    if (random8() < 145) leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

// ---------- Self-playing 3-D Pong ----------
int8_t pongX = 2, pongY = 2, pongZ = 2;
int8_t pongVX = 1, pongVY = 1, pongVZ = -1;
int8_t leftPaddleY = 1, leftPaddleZ = 1;
int8_t rightPaddleY = 2, rightPaddleZ = 2;
uint32_t lastPongStep = 0;
constexpr uint16_t PONG_STEP_MS = 145;

int8_t clampPaddle(int8_t value) {
  if (value < 0) return 0;
  if (value > N - 2) return N - 2;
  return value;
}

void moveTowards(int8_t &value, int8_t target) {
  if (value < target) ++value;
  else if (value > target) --value;
}

bool ballHitsPaddle(int8_t paddleY, int8_t paddleZ) {
  return pongY >= paddleY && pongY <= paddleY + 1 && pongZ >= paddleZ && pongZ <= paddleZ + 1;
}

void resetPongBall() {
  pongX = 2;
  pongY = random8(N);
  pongZ = random8(N);
  pongVX = random8() & 1 ? 1 : -1;
  pongVY = random8() & 1 ? 1 : -1;
  pongVZ = random8() & 1 ? 1 : -1;
}

void stepPong() {
  moveTowards(leftPaddleY, clampPaddle(pongY - 1));
  moveTowards(leftPaddleZ, clampPaddle(pongZ - 1));
  moveTowards(rightPaddleY, clampPaddle(pongY - 1));
  moveTowards(rightPaddleZ, clampPaddle(pongZ - 1));

  pongY += pongVY;
  pongZ += pongVZ;
  if (pongY < 0 || pongY >= N) { pongVY = -pongVY; pongY += 2 * pongVY; }
  if (pongZ < 0 || pongZ >= N) { pongVZ = -pongVZ; pongZ += 2 * pongVZ; }

  pongX += pongVX;
  if (pongX <= 0) {
    if (ballHitsPaddle(leftPaddleY, leftPaddleZ)) {
      pongX = 0;
      pongVX = 1;
    } else {
      resetPongBall();
    }
  } else if (pongX >= N - 1) {
    if (ballHitsPaddle(rightPaddleY, rightPaddleZ)) {
      pongX = N - 1;
      pongVX = -1;
    } else {
      resetPongBall();
    }
  }
}

void renderPaddle(int x, int y, int z, const CRGB &colour) {
  for (int dz = 0; dz < 2; ++dz)
    for (int dy = 0; dy < 2; ++dy)
      setVoxel(x, y + dy, z + dz, colour);
}

void renderPong() {
  const uint32_t now = millis();
  if (now - lastPongStep >= PONG_STEP_MS) {
    lastPongStep = now;
    stepPong();
  }
  renderPaddle(0, leftPaddleY, leftPaddleZ, CRGB::Blue);
  renderPaddle(N - 1, rightPaddleY, rightPaddleZ, CRGB::Red);
  setVoxel(pongX, pongY, pongZ, CRGB::White);
}

// ---------- Autoplay 3-D Tetris ----------
// Four-cell polycubes fall down the z axis and settle into a 5x5x5 well.
const int8_t TETRIS_SHAPES[4][4][3] = {
  {{0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}},
  {{0,0,0}, {1,0,0}, {0,1,0}, {0,0,1}},
  {{0,0,0}, {1,0,0}, {0,1,0}, {1,1,0}},
  {{0,0,0}, {1,0,0}, {2,0,0}, {1,1,0}}
};

CRGB tetrisWell[N][N][N];
uint8_t tetrisType = 0, tetrisRotation = 0;
int8_t tetrisX = 0, tetrisY = 0, tetrisZ = 0;
CRGB tetrisColour;
bool tetrisReady = false;
uint32_t lastTetrisStep = 0;
constexpr uint16_t TETRIS_STEP_MS = 430;

bool isLit(const CRGB &c) {
  return c.r || c.g || c.b;
}

void getTetrisCell(uint8_t cell, int8_t &x, int8_t &y, int8_t &z) {
  const int8_t bx = TETRIS_SHAPES[tetrisType][cell][0];
  const int8_t by = TETRIS_SHAPES[tetrisType][cell][1];
  z = TETRIS_SHAPES[tetrisType][cell][2];

  switch (tetrisRotation & 3) {
    case 0: x = bx;  y = by;  break;
    case 1: x = -by; y = bx;  break;
    case 2: x = -bx; y = -by; break;
    default:x = by;  y = -bx; break;
  }
}

bool canPlaceTetris(int8_t baseX, int8_t baseY, int8_t baseZ) {
  for (uint8_t cell = 0; cell < 4; ++cell) {
    int8_t cx, cy, cz;
    getTetrisCell(cell, cx, cy, cz);
    const int8_t x = baseX + cx, y = baseY + cy, z = baseZ + cz;
    if (x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N) return false;
    if (isLit(tetrisWell[x][y][z])) return false;
  }
  return true;
}

void clearTetrisWell() {
  for (uint8_t x = 0; x < N; ++x)
    for (uint8_t y = 0; y < N; ++y)
      for (uint8_t z = 0; z < N; ++z)
        tetrisWell[x][y][z] = CRGB::Black;
}

void spawnTetrisPiece() {
  const CRGB palette[] = {CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue, CRGB::Purple};
  tetrisType = random8(4);
  tetrisRotation = random8(4);
  tetrisColour = palette[random8(6)];

  int8_t minX = 9, minY = 9, minZ = 9, maxX = -9, maxY = -9, maxZ = -9;
  for (uint8_t cell = 0; cell < 4; ++cell) {
    int8_t x, y, z;
    getTetrisCell(cell, x, y, z);
    minX = min(minX, x); maxX = max(maxX, x);
    minY = min(minY, y); maxY = max(maxY, y);
    minZ = min(minZ, z); maxZ = max(maxZ, z);
  }

  const uint8_t widthX = maxX - minX + 1;
  const uint8_t widthY = maxY - minY + 1;
  tetrisX = random8(N - widthX + 1) - minX;
  tetrisY = random8(N - widthY + 1) - minY;
  tetrisZ = (N - 1) - maxZ;

  if (!canPlaceTetris(tetrisX, tetrisY, tetrisZ)) {
    clearTetrisWell();
    // The selected piece is now guaranteed to fit into an empty well.
  }
}

void lockTetrisPiece() {
  for (uint8_t cell = 0; cell < 4; ++cell) {
    int8_t cx, cy, cz;
    getTetrisCell(cell, cx, cy, cz);
    tetrisWell[tetrisX + cx][tetrisY + cy][tetrisZ + cz] = tetrisColour;
  }
}

void clearTetrisLayers() {
  for (int z = 0; z < N; ++z) {
    bool full = true;
    for (int x = 0; x < N && full; ++x)
      for (int y = 0; y < N; ++y)
        if (!isLit(tetrisWell[x][y][z])) { full = false; break; }

    if (!full) continue;

    for (int zz = z; zz < N - 1; ++zz)
      for (int x = 0; x < N; ++x)
        for (int y = 0; y < N; ++y)
          tetrisWell[x][y][zz] = tetrisWell[x][y][zz + 1];

    for (int x = 0; x < N; ++x)
      for (int y = 0; y < N; ++y)
        tetrisWell[x][y][N - 1] = CRGB::Black;

    --z;  // Re-check this same level after it collapses.
  }
}

void resetTetris() {
  clearTetrisWell();
  spawnTetrisPiece();
  tetrisReady = true;
}

void updateTetris() {
  if (!tetrisReady) resetTetris();
  const uint32_t now = millis();
  if (now - lastTetrisStep < TETRIS_STEP_MS) return;
  lastTetrisStep = now;

  if (canPlaceTetris(tetrisX, tetrisY, tetrisZ - 1)) {
    --tetrisZ;
  } else {
    lockTetrisPiece();
    clearTetrisLayers();
    spawnTetrisPiece();
  }
}

void renderTetris() {
  updateTetris();
  for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
      for (int z = 0; z < N; ++z)
        if (isLit(tetrisWell[x][y][z])) setVoxel(x, y, z, tetrisWell[x][y][z]);

  for (uint8_t cell = 0; cell < 4; ++cell) {
    int8_t cx, cy, cz;
    getTetrisCell(cell, cx, cy, cz);
    setVoxel(tetrisX + cx, tetrisY + cy, tetrisZ + cz, tetrisColour);
  }
}

// ---------- Blinking 3-D eye ----------
void renderBlinkingEye() {
  const uint32_t cycle = millis() % 4600;
  const bool closed = cycle > 4100 && cycle < 4380;

  if (closed) {
    // A warm eyelid line sweeps across the front of the cube.
    for (int x = 0; x < N; ++x)
      for (int y = 1; y < N; ++y)
        setVoxel(x, y, 2, CRGB(255, 80, 0));
    return;
  }

  // White eyeball volume.
  for (int z = 0; z < N; ++z) {
    for (int y = 1; y < N; ++y) {
      for (int x = 0; x < N; ++x) {
        const int dx = x - 2, dy = y - 2, dz = z - 2;
        if (dx * dx + dy * dy + dz * dz <= 5) setVoxel(x, y, z, CRGB(90, 90, 90));
      }
    }
  }

  // The pupil tracks slowly around the front face.
  const int pupilX = 2 + ((sin8(millis() >> 4) > 170) ? 1 : (sin8(millis() >> 4) < 85 ? -1 : 0));
  const int pupilZ = 2 + ((sin8((millis() >> 4) + 64) > 170) ? 1 : (sin8((millis() >> 4) + 64) < 85 ? -1 : 0));
  setVoxel(pupilX, 4, pupilZ, CRGB::Blue);
  setVoxel(pupilX, 3, pupilZ, CRGB(0, 35, 120));
  setVoxel(pupilX, 4, pupilZ, CRGB::Black);
}

// ---------- DNA helix ----------
void renderDNAHelix() {
  const uint8_t phase = millis() >> 4;
  for (int z = 0; z < N; ++z) {
    const uint8_t a = phase + z * 50;
    const int x1 = 2 + ((int)sin8(a) - 128) / 75;
    const int y1 = 2 + ((int)sin8(a + 64) - 128) / 75;
    const int x2 = 4 - x1;
    const int y2 = 4 - y1;
    setVoxel(x1, y1, z, CRGB::Aqua);
    setVoxel(x2, y2, z, CRGB::Magenta);
    if ((z & 1) == 0) {
      setVoxel(2, 2, z, CRGB(80, 30, 80));
    }
  }
}

// ---------- Wrapped diagonal meteor stream ----------
void renderMeteors() {
  const uint16_t tick = millis() / 75;
  for (uint8_t tail = 0; tail < 5; ++tail) {
    const int p = (int)tick - tail;
    const int x = ((p % N) + N) % N;
    const int y = (((p * 2) % N) + N) % N;
    const int z = (((p * 3) % N) + N) % N;
    const uint8_t value = 255 - tail * 48;
    setVoxel(x, y, z, CHSV(10 + tail * 7, 255, value));
  }
}

// ---------- 3-D wrapping snake ----------
constexpr uint8_t SNAKE_LENGTH = 9;
int8_t snakeX[SNAKE_LENGTH], snakeY[SNAKE_LENGTH], snakeZ[SNAKE_LENGTH];
int8_t snakeDX = 1, snakeDY = 0, snakeDZ = 0;
bool snakeReady = false;
uint32_t lastSnakeStep = 0;
constexpr uint16_t SNAKE_STEP_MS = 175;

int8_t wrap5(int value) {
  value %= N;
  if (value < 0) value += N;
  return value;
}

void resetSnake() {
  for (uint8_t i = 0; i < SNAKE_LENGTH; ++i) {
    snakeX[i] = wrap5(2 - i);
    snakeY[i] = 2;
    snakeZ[i] = 2;
  }
  snakeDX = 1; snakeDY = 0; snakeDZ = 0;
  snakeReady = true;
}

bool snakeOccupies(int8_t x, int8_t y, int8_t z) {
  for (uint8_t i = 0; i < SNAKE_LENGTH - 1; ++i)
    if (snakeX[i] == x && snakeY[i] == y && snakeZ[i] == z) return true;
  return false;
}

void chooseSnakeDirection() {
  const int8_t directions[6][3] = {
    { 1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
    { 0,-1, 0}, { 0, 0, 1}, {0, 0,-1}
  };

  for (uint8_t attempt = 0; attempt < 6; ++attempt) {
    const uint8_t choice = random8(6);
    const int8_t dx = directions[choice][0];
    const int8_t dy = directions[choice][1];
    const int8_t dz = directions[choice][2];
    if (dx == -snakeDX && dy == -snakeDY && dz == -snakeDZ) continue;

    const int8_t nx = wrap5(snakeX[0] + dx);
    const int8_t ny = wrap5(snakeY[0] + dy);
    const int8_t nz = wrap5(snakeZ[0] + dz);
    if (!snakeOccupies(nx, ny, nz)) {
      snakeDX = dx; snakeDY = dy; snakeDZ = dz;
      return;
    }
  }
}

void updateSnake() {
  if (!snakeReady) resetSnake();
  const uint32_t now = millis();
  if (now - lastSnakeStep < SNAKE_STEP_MS) return;
  lastSnakeStep = now;

  if (random8() < 82) chooseSnakeDirection();
  int8_t nx = wrap5(snakeX[0] + snakeDX);
  int8_t ny = wrap5(snakeY[0] + snakeDY);
  int8_t nz = wrap5(snakeZ[0] + snakeDZ);
  if (snakeOccupies(nx, ny, nz)) {
    chooseSnakeDirection();
    nx = wrap5(snakeX[0] + snakeDX);
    ny = wrap5(snakeY[0] + snakeDY);
    nz = wrap5(snakeZ[0] + snakeDZ);
    if (snakeOccupies(nx, ny, nz)) { resetSnake(); return; }
  }

  for (int i = SNAKE_LENGTH - 1; i > 0; --i) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
    snakeZ[i] = snakeZ[i - 1];
  }
  snakeX[0] = nx; snakeY[0] = ny; snakeZ[0] = nz;
}

void renderSnake() {
  updateSnake();
  for (uint8_t i = 0; i < SNAKE_LENGTH; ++i)
    setVoxel(snakeX[i], snakeY[i], snakeZ[i], CHSV(96 - i * 8, 255, 255 - i * 18));
}

// ---------- Marching 3-D Space Invaders ----------
int8_t invaderShift = 0;
int8_t invaderDirection = 1;
int8_t invaderDepth = 0;
uint32_t lastInvaderStep = 0;
constexpr uint16_t INVADER_STEP_MS = 360;

void drawInvader(int cx, int cy, int cz, const CRGB &colour) {
  const int8_t shape[6][2] = {{0,0}, {2,0}, {1,1}, {0,2}, {1,2}, {2,2}};
  for (uint8_t i = 0; i < 6; ++i) {
    for (int dy = 0; dy < 2; ++dy)
      setVoxel(cx + shape[i][0], cy + dy, cz + shape[i][1], colour);
  }
}

void renderInvaders() {
  const uint32_t now = millis();
  if (now - lastInvaderStep >= INVADER_STEP_MS) {
    lastInvaderStep = now;
    invaderShift += invaderDirection;
    if (invaderShift <= 0 || invaderShift >= 2) {
      invaderDirection = -invaderDirection;
      invaderDepth = (invaderDepth + 1) % 3;
    }
  }

  drawInvader(invaderShift, invaderDepth, 0, CRGB::Green);
  drawInvader(2 - invaderShift, invaderDepth, 2, CRGB(0, 180, 28));
  const int laserZ = (millis() / 95) % N;
  setVoxel(2, 4, laserZ, CRGB::Red);
}

// ---------- 3-D Conway-style Game of Life ----------
// A 26-neighbour 3-D rule: birth on 5 neighbours, survive on 4 or 5.
bool lifeCells[N][N][N];
bool lifeNext[N][N][N];
bool lifeReady = false;
uint32_t lastLifeStep = 0;
constexpr uint16_t LIFE_STEP_MS = 560;
uint16_t lifeGeneration = 0;

void resetLife() {
  for (uint8_t x = 0; x < N; ++x)
    for (uint8_t y = 0; y < N; ++y)
      for (uint8_t z = 0; z < N; ++z)
        lifeCells[x][y][z] = random8() < 66;
  lifeGeneration = 0;
  lifeReady = true;
}

uint8_t lifeNeighbours(int x, int y, int z) {
  uint8_t neighbours = 0;
  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const int nx = x + dx, ny = y + dy, nz = z + dz;
        if (nx >= 0 && nx < N && ny >= 0 && ny < N && nz >= 0 && nz < N && lifeCells[nx][ny][nz])
          ++neighbours;
      }
  return neighbours;
}

void updateLife() {
  if (!lifeReady) resetLife();
  const uint32_t now = millis();
  if (now - lastLifeStep < LIFE_STEP_MS) return;
  lastLifeStep = now;

  uint8_t living = 0;
  for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
      for (int z = 0; z < N; ++z) {
        const uint8_t neighbours = lifeNeighbours(x, y, z);
        lifeNext[x][y][z] = lifeCells[x][y][z] ? (neighbours == 4 || neighbours == 5) : (neighbours == 5);
        if (lifeNext[x][y][z]) ++living;
      }

  for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
      for (int z = 0; z < N; ++z)
        lifeCells[x][y][z] = lifeNext[x][y][z];

  ++lifeGeneration;
  if (living < 4 || living > 90 || lifeGeneration > 75) resetLife();
}

void renderLife() {
  updateLife();
  for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
      for (int z = 0; z < N; ++z)
        if (lifeCells[x][y][z]) setVoxel(x, y, z, CHSV(lifeGeneration * 7 + x * 17 + z * 12, 230, 255));
}

// ---------- Slow volumetric clouds ----------
void renderClouds() {
  const uint16_t drift = millis() / 14;
  for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
      for (int x = 0; x < N; ++x) {
        const uint8_t n = inoise8(x * 62 + drift, y * 62 + drift / 3, z * 62 + drift / 5);
        if (n > 148) setVoxel(x, y, z, CHSV(145, 70, qsub8(n, 110)));
      }
}

// ---------- 3-D neon plasma ----------
void renderPlasma() {
  const uint8_t phase = millis() >> 3;
  for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
      for (int x = 0; x < N; ++x) {
        const uint8_t a = sin8(phase + x * 46);
        const uint8_t b = sin8(phase / 2 + y * 55);
        const uint8_t c = sin8(phase / 3 + z * 62);
        const uint8_t hue = ((uint16_t)a + b + c) / 3;
        const uint8_t value = 80 + (((uint16_t)a + b + c) / 5);
        setVoxel(x, y, z, CHSV(hue, 255, value));
      }
}

// ---------- Rising 3-D fire ----------
void renderFire() {
  const uint16_t time = millis() / 5;
  for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
      for (int x = 0; x < N; ++x) {
        const uint8_t noise = inoise8(x * 60, y * 60, z * 72 - time);
        const uint8_t heat = qsub8(noise, z * 39);
        if (heat > 22) setVoxel(x, y, z, HeatColor(heat));
      }
}

// ---------- 3-D hourglass ----------
void renderHourglass() {
  const uint8_t raw = (millis() / 310) % 48;
  const uint8_t fill = raw <= 24 ? raw : 48 - raw;

  // Cyan glass corners taper into the neck at (2,2,2).
  for (int z = 0; z < N; ++z) {
    const int r = abs(z - 2);
    setVoxel(2 - r, 2 - r, z, CRGB(0, 80, 120));
    setVoxel(2 + r, 2 - r, z, CRGB(0, 80, 120));
    setVoxel(2 - r, 2 + r, z, CRGB(0, 80, 120));
    setVoxel(2 + r, 2 + r, z, CRGB(0, 80, 120));
  }

  for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
      for (int x = 0; x < N; ++x) {
        const uint8_t hash = (x * 11 + y * 17 + z * 23) % 25;
        const int distance = abs(x - 2) + abs(y - 2);
        const bool upperShape = z >= 3 && distance <= (z == 4 ? 3 : 2);
        const bool lowerShape = z <= 1 && distance <= (z == 0 ? 3 : 2);
        if (upperShape && hash < 24 - fill) setVoxel(x, y, z, CRGB::Gold);
        if (lowerShape && hash < fill) setVoxel(x, y, z, CRGB::Gold);
      }

  const int fallingZ = 4 - ((millis() / 110) % 5);
  setVoxel(2, 2, fallingZ, CRGB::White);
}

// ---------- Reducing and expanding "ping-pong" cube ----------
void renderPulseCube() {
  const uint8_t wave = sin8(millis() >> 3);
  const int radius = wave < 85 ? 0 : (wave < 170 ? 1 : 2);
  const int lo = 2 - radius;
  const int hi = 2 + radius;
  const CRGB colour = CHSV(millis() >> 4, 230, 255);

  for (int z = lo; z <= hi; ++z)
    for (int y = lo; y <= hi; ++y)
      for (int x = lo; x <= hi; ++x) {
        uint8_t faces = 0;
        if (x == lo || x == hi) ++faces;
        if (y == lo || y == hi) ++faces;
        if (z == lo || z == hi) ++faces;
        if (faces >= 2) setVoxel(x, y, z, colour);
      }
}

// ---------- Dense upward red rain with fast fade ----------
int8_t redRainHeads[N * N];
uint8_t redRainPhase[N * N];
bool redRainReady = false;
uint32_t lastRedRainStep = 0;
constexpr uint16_t RED_RAIN_STEP_MS = 34;

void resetUpwardRedRain() {
  for (uint8_t i = 0; i < N * N; ++i) {
    redRainHeads[i] = random8(N);
    redRainPhase[i] = random8(3);
  }
  redRainReady = true;
}

void renderUpwardRedRain() {
  if (!redRainReady) resetUpwardRedRain();
  const uint32_t now = millis();
  if (now - lastRedRainStep < RED_RAIN_STEP_MS) return;
  lastRedRainStep = now;

  fadeToBlackBy(leds, NUM_LEDS, 145);
  for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
      const uint8_t i = y * N + x;
      if (++redRainPhase[i] >= 3) {
        redRainPhase[i] = 0;
        if (++redRainHeads[i] >= N) redRainHeads[i] = 0;
      }
      addVoxel(x, y, redRainHeads[i], CRGB(255, 24, 0));
      addVoxel(x, y, redRainHeads[i] - 1, CRGB(105, 0, 0));
      addVoxel(x, y, redRainHeads[i] - 2, CRGB(35, 0, 0));
    }
}

// ---------- Twin 3-D spirals ----------
void renderSpirals() {
  const uint8_t phase = millis() >> 3;
  for (int z = 0; z < N; ++z) {
    const uint8_t a = phase + z * 51;
    for (uint8_t arm = 0; arm < 2; ++arm) {
      const uint8_t angle = a + arm * 128;
      const int x = 2 + ((int)sin8(angle) - 128) / 70;
      const int y = 2 + ((int)sin8(angle + 64) - 128) / 70;
      setVoxel(x, y, z, arm ? CRGB::Purple : CRGB::Orange);
    }
  }
}

// ---------- Single-point 3-D bouncer ----------
int8_t pointX = 1, pointY = 2, pointZ = 3;
int8_t pointVX = 1, pointVY = -1, pointVZ = 1;
uint32_t lastPointStep = 0;
constexpr uint16_t POINT_STEP_MS = 90;

void stepPointAxis(int8_t &position, int8_t &velocity) {
  if (position + velocity < 0 || position + velocity >= N) velocity = -velocity;
  position += velocity;
}

void renderPointBouncer() {
  const uint32_t now = millis();
  if (now - lastPointStep >= POINT_STEP_MS) {
    lastPointStep = now;
    stepPointAxis(pointX, pointVX);
    stepPointAxis(pointY, pointVY);
    stepPointAxis(pointZ, pointVZ);
  }
  setVoxel(pointX, pointY, pointZ, CRGB::White);
}

// ---------- Multiple 3-D wrapping comets ----------
void renderWrappingComets() {
  const int tick = millis() / 58;
  for (uint8_t comet = 0; comet < 3; ++comet) {
    for (uint8_t tail = 0; tail < 5; ++tail) {
      const int p = tick - tail;
      const int x = wrap5(p * (comet + 1) + comet * 2);
      const int y = wrap5(p * (comet + 2) + comet);
      const int z = wrap5(p * (comet + 3) + comet * 3);
      const uint8_t value = 255 - tail * 46;
      setVoxel(x, y, z, CHSV(150 + comet * 32, 240, value));
    }
  }
}


void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  randomSeed(esp_random());
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  const float t = millis() * 0.001f;
  const float ax = 0.83f * t;
  const float ay = 1.17f * t;
  const float az = 0.41f * t;
  if (true) fill_solid(leds, NUM_LEDS, CRGB::Black);
  renderCornerCubes();
  FastLED.show();
}
