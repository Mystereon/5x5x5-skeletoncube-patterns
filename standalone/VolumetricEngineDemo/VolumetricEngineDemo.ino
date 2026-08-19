/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  VolumetricEngineDemo.ino
  A reusable millis()-driven volumetric NeoPixel engine and step-pattern tool.
  Created by Dad (MysterEon) & Manus, 2026.

  Hardware profile supplied here:
  - Waveshare ESP32-S3-Zero
  - GPIO6 -> 125 LED 5x5x5 cube, followed by a 12-pixel rear ring
  - Cube origin is bottom-rear-left: x=left->right, y=rear->front, z=bottom->top

  Why this exists
  ---------------
  Existing patterns are deliberately hand-authored. This sketch is a separate
  developer tool for building a sequence out of time-based volumetric steps.
  There are no delay() calls: positions are derived from millis(), so scene
  timing remains stable at any practical FastLED frame rate.

  Start by editing DEMO_STEPS below. Each row is one timed primitive. The
  README in this folder explains every field and how to add your own steps.
*/

#include <FastLED.h>
#include <math.h>

// Arduino generates function prototypes before compiling the sketch. This
// forward declaration keeps those generated signatures valid until the full
// step-editor record is defined below.
struct PatternStep;

// -----------------------------------------------------------------------------
// 1. Hardware and scalable-volume configuration
// -----------------------------------------------------------------------------

#define DATA_PIN 6
#define CHIPSET WS2812B
#define COLOR_ORDER GRB

// Change these three dimensions for another rectangular cube or matrix. The
// physical mapper, primitives, timing and editor automatically use the product.
constexpr uint8_t VOLUME_X = 5;
constexpr uint8_t VOLUME_Y = 5;
constexpr uint8_t VOLUME_Z = 5;
constexpr uint16_t MATRIX_LEDS = uint16_t(VOLUME_X) * VOLUME_Y * VOLUME_Z;

// Set this to zero for a matrix with no separate enclosure ring.
constexpr uint8_t RING_LEDS = 12;
constexpr uint16_t TOTAL_LEDS = MATRIX_LEDS + RING_LEDS;

// These four mapping switches let one sketch suit common cube wiring layouts.
constexpr bool SWAP_XY = false;
constexpr bool FLIP_X = false;
constexpr bool FLIP_Y = false;
constexpr bool FLIP_Z = false;
constexpr bool SERPENTINE_ROWS = false;
constexpr bool SERPENTINE_LAYERS = false;

constexpr uint8_t GLOBAL_BRIGHTNESS = 100;
constexpr uint8_t RING_LOCAL_BRIGHTNESS = 160;  // Applied before global brightness.
constexpr uint8_t TARGET_FPS = 100;
constexpr uint16_t FRAME_INTERVAL_MS = 1000U / TARGET_FPS;

static_assert(VOLUME_X > 0 && VOLUME_Y > 0 && VOLUME_Z > 0,
              "Every volume dimension must be at least one.");
static_assert(MATRIX_LEDS <= 650,
              "Keep this demo below 650 matrix LEDs unless you review its static RAM budget.");

CRGB leds[TOTAL_LEDS];
uint8_t radius16[MATRIX_LEDS];            // Calculated once at boot, not per frame.

constexpr uint8_t ORBIT_SAMPLES = 24;
int8_t orbitX[ORBIT_SAMPLES];
int8_t orbitY[ORBIT_SAMPLES];
int8_t orbitZ[ORBIT_SAMPLES];

// -----------------------------------------------------------------------------
// 2. Matrix mapper and drawing primitives
// -----------------------------------------------------------------------------

uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) {
  uint8_t px = x, py = y, pz = z;
  if (SWAP_XY) { const uint8_t temp = px; px = py; py = temp; }
  if (FLIP_X) px = VOLUME_X - 1 - px;
  if (FLIP_Y) py = VOLUME_Y - 1 - py;
  if (FLIP_Z) pz = VOLUME_Z - 1 - pz;
  if (SERPENTINE_ROWS && (py & 1)) px = VOLUME_X - 1 - px;
  if (SERPENTINE_LAYERS && (pz & 1)) py = VOLUME_Y - 1 - py;
  return uint16_t(pz) * VOLUME_X * VOLUME_Y + uint16_t(py) * VOLUME_X + px;
}

bool insideVolume(int8_t x, int8_t y, int8_t z) {
  return x >= 0 && x < VOLUME_X && y >= 0 && y < VOLUME_Y && z >= 0 && z < VOLUME_Z;
}

void setVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) {
  if (insideVolume(x, y, z)) leds[indexFromXYZ(x, y, z)] = colour;
}

void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) {
  if (insideVolume(x, y, z)) leds[indexFromXYZ(x, y, z)] += colour;
}

void clearMatrix() { fill_solid(leds, MATRIX_LEDS, CRGB::Black); }

uint8_t dimensionForAxis(uint8_t axis) {
  return axis == 0 ? VOLUME_X : (axis == 1 ? VOLUME_Y : VOLUME_Z);
}

int8_t coordinateFromProgress(uint8_t progress, uint8_t size) {
  if (size <= 1) return 0;
  return int8_t((uint16_t(progress) * (size - 1) + 127U) / 255U);
}

void drawWireBox(uint8_t margin, const CRGB &colour) {
  if (VOLUME_X < 2 || VOLUME_Y < 2 || VOLUME_Z < 2) return;
  const int8_t minX = margin, maxX = VOLUME_X - 1 - margin;
  const int8_t minY = margin, maxY = VOLUME_Y - 1 - margin;
  const int8_t minZ = margin, maxZ = VOLUME_Z - 1 - margin;
  if (minX >= maxX || minY >= maxY || minZ >= maxZ) return;
  for (int8_t z = minZ; z <= maxZ; ++z) for (int8_t y = minY; y <= maxY; ++y) for (int8_t x = minX; x <= maxX; ++x) {
    const uint8_t faces = (x == minX || x == maxX) + (y == minY || y == maxY) + (z == minZ || z == maxZ);
    if (faces >= 2) setVoxel(x, y, z, colour);
  }
}

void drawPlaneSweep(uint8_t axis, uint8_t progress, const CRGB &colour) {
  const int8_t position = coordinateFromProgress(progress, dimensionForAxis(axis));
  for (int8_t z = 0; z < VOLUME_Z; ++z) for (int8_t y = 0; y < VOLUME_Y; ++y) for (int8_t x = 0; x < VOLUME_X; ++x) {
    if ((axis == 0 && x == position) || (axis == 1 && y == position) || (axis == 2 && z == position)) {
      setVoxel(x, y, z, colour);
    }
  }
}

void drawOrbit(uint8_t progress, const CRGB &colour) {
  const uint8_t sample = uint8_t((uint16_t(progress) * ORBIT_SAMPLES) >> 8) % ORBIT_SAMPLES;
  setVoxel(orbitX[sample], orbitY[sample], orbitZ[sample], colour);
  setVoxel(orbitX[(sample + ORBIT_SAMPLES - 1) % ORBIT_SAMPLES], orbitY[(sample + ORBIT_SAMPLES - 1) % ORBIT_SAMPLES], orbitZ[(sample + ORBIT_SAMPLES - 1) % ORBIT_SAMPLES], CRGB(colour.r / 4, colour.g / 4, colour.b / 4));
  setVoxel(orbitX[(sample + ORBIT_SAMPLES - 2) % ORBIT_SAMPLES], orbitY[(sample + ORBIT_SAMPLES - 2) % ORBIT_SAMPLES], orbitZ[(sample + ORBIT_SAMPLES - 2) % ORBIT_SAMPLES], CRGB(colour.r / 10, colour.g / 10, colour.b / 10));
}

void drawSphereShell(uint8_t targetRadius16, const CRGB &colour) {
  for (uint8_t z = 0; z < VOLUME_Z; ++z) for (uint8_t y = 0; y < VOLUME_Y; ++y) for (uint8_t x = 0; x < VOLUME_X; ++x) {
    const uint8_t r = radius16[indexFromXYZ(x, y, z)];
    const uint8_t difference = abs(int16_t(r) - targetRadius16);
    if (difference <= 8) setVoxel(x, y, z, colour);
  }
}

void drawBouncer(uint8_t progress, const CRGB &colour) {
  // Three different triangle-wave phases create a full-volume path with no
  // stored state and no blocking update timing.
  const int8_t x = coordinateFromProgress(triwave8(progress), VOLUME_X);
  const int8_t y = coordinateFromProgress(triwave8(progress + 83), VOLUME_Y);
  const int8_t z = coordinateFromProgress(triwave8(progress + 167), VOLUME_Z);
  setVoxel(x, y, z, colour);
  addVoxel(x - 1, y, z, CRGB(colour.r / 10, colour.g / 10, colour.b / 10));
  addVoxel(x + 1, y, z, CRGB(colour.r / 10, colour.g / 10, colour.b / 10));
}

uint8_t hashVoxel(uint8_t x, uint8_t y, uint8_t z, uint8_t beat) {
  return uint8_t(x * 37U + y * 73U + z * 109U + beat * 29U + (x ^ (y << 2) ^ (z << 4)));
}

void drawSparkles(uint8_t progress, uint8_t density, const CRGB &colour) {
  const uint8_t beat = progress >> 4;
  for (uint8_t z = 0; z < VOLUME_Z; ++z) for (uint8_t y = 0; y < VOLUME_Y; ++y) for (uint8_t x = 0; x < VOLUME_X; ++x) {
    const uint8_t h = hashVoxel(x, y, z, beat);
    if (h < density) {
      const uint8_t value = 100 + (h >> 1);
      setVoxel(x, y, z, CRGB(scale8(colour.r, value), scale8(colour.g, value), scale8(colour.b, value)));
    }
  }
}

// -----------------------------------------------------------------------------
// 3. Setup-time lookup tables: the render loop does no sqrt(), sin(), or cos().
// -----------------------------------------------------------------------------

void buildLUTs() {
  const float centreX = (VOLUME_X - 1) * 0.5f;
  const float centreY = (VOLUME_Y - 1) * 0.5f;
  const float centreZ = (VOLUME_Z - 1) * 0.5f;
  for (uint8_t z = 0; z < VOLUME_Z; ++z) for (uint8_t y = 0; y < VOLUME_Y; ++y) for (uint8_t x = 0; x < VOLUME_X; ++x) {
    const float dx = x - centreX, dy = y - centreY, dz = z - centreZ;
    radius16[indexFromXYZ(x, y, z)] = uint8_t(sqrtf(dx * dx + dy * dy + dz * dz) * 16.0f + 0.5f);
  }

  const float radiusX = max(0.5f, (VOLUME_X - 1) * 0.42f);
  const float radiusY = max(0.5f, (VOLUME_Y - 1) * 0.42f);
  const float radiusZ = max(0.5f, (VOLUME_Z - 1) * 0.34f);
  for (uint8_t i = 0; i < ORBIT_SAMPLES; ++i) {
    const float a = 6.2831853f * i / ORBIT_SAMPLES;
    orbitX[i] = int8_t(roundf(centreX + cosf(a) * radiusX));
    orbitY[i] = int8_t(roundf(centreY + sinf(a) * radiusY));
    orbitZ[i] = int8_t(roundf(centreZ + sinf(a * 2.0f) * radiusZ));
  }
}

// -----------------------------------------------------------------------------
// 4. Step-based pattern editor
// -----------------------------------------------------------------------------

enum Primitive : uint8_t {
  PRIM_VOXEL,
  PRIM_WIRE_BOX,
  PRIM_SWEEP,
  PRIM_ORBIT,
  PRIM_SPHERE,
  PRIM_BOUNCE,
  PRIM_SPARKLES
};

enum RingMode : uint8_t {
  RING_OFF,
  RING_STEADY,
  RING_BREATHE,
  RING_CHASE,
  RING_PROJECTOR
};

struct PatternStep {
  uint16_t durationMs; // How long the step runs before the next row starts.
  Primitive primitive;
  uint8_t hue;
  uint8_t saturation;
  uint8_t value;
  RingMode ringMode;
  int8_t a;            // Primitive parameter: see README table.
  int8_t b;
  int8_t c;
};

#define STEP(ms, primitive, hue, sat, val, ring, a, b, c) {ms, primitive, hue, sat, val, ring, a, b, c}

// -----------------------------------------------------------------------------
// EDIT HERE: every row is an independent, non-blocking timeline step.
// The sample is intentionally varied so a first upload proves all primitives.
// -----------------------------------------------------------------------------
const PatternStep DEMO_STEPS[] = {
  STEP(2600, PRIM_WIRE_BOX, 145, 255, 210, RING_BREATHE,   0, 0, 0), // cyan wire cube
  STEP(2200, PRIM_SWEEP,     88, 255, 235, RING_CHASE,     0, 0, 0), // X-axis green plane
  STEP(2200, PRIM_SWEEP,    182, 255, 235, RING_CHASE,     2, 0, 0), // Z-axis violet plane
  STEP(2800, PRIM_ORBIT,    157, 255, 255, RING_PROJECTOR, 0, 0, 0), // precomputed 3-D orbit
  STEP(2600, PRIM_SPHERE,    22, 245, 255, RING_BREATHE,   3, 0, 0), // expanding warm shell
  STEP(3000, PRIM_BOUNCE,   224, 255, 255, RING_CHASE,     0, 0, 0), // tri-wave full-volume bouncer
  STEP(2600, PRIM_SPARKLES, 145, 180, 255, RING_PROJECTOR, 56, 0, 0), // deterministic star field
  STEP(1500, PRIM_VOXEL,     96, 255, 255, RING_STEADY,    2, 2, 2)  // central beacon
};

constexpr uint8_t STEP_COUNT = sizeof(DEMO_STEPS) / sizeof(DEMO_STEPS[0]);
uint8_t currentStepIndex = 0;
uint32_t stepStartedAt = 0;
uint32_t lastFrameAt = 0;

uint8_t stepProgress(const PatternStep &step, uint32_t now) {
  const uint32_t age = now - stepStartedAt;
  return step.durationMs <= 1 ? 255 : uint8_t(min<uint32_t>(255, age * 255UL / step.durationMs));
}

void advanceTimeline(uint32_t now) {
  // A while loop preserves timing if Wi-Fi, serial, or a browser handler ever
  // causes a long frame. The scene advances rather than freezing behind schedule.
  while (now - stepStartedAt >= DEMO_STEPS[currentStepIndex].durationMs) {
    stepStartedAt += DEMO_STEPS[currentStepIndex].durationMs;
    currentStepIndex = (currentStepIndex + 1) % STEP_COUNT;
  }
}

void renderRing(const PatternStep &step, uint8_t progress) {
  if (RING_LEDS == 0) return;
  for (uint8_t i = 0; i < RING_LEDS; ++i) {
    uint8_t value = 0;
    uint8_t hue = step.hue;
    switch (step.ringMode) {
      case RING_STEADY:
        value = RING_LOCAL_BRIGHTNESS;
        break;
      case RING_BREATHE:
        value = scale8(RING_LOCAL_BRIGHTNESS, qadd8(64, sin8(progress + i * 5) / 2));
        break;
      case RING_CHASE:
        value = scale8(RING_LOCAL_BRIGHTNESS, qadd8(28, scale8(sin8(progress + uint8_t(i * 255U / RING_LEDS)), 225)));
        hue += i * 3;
        break;
      case RING_PROJECTOR:
        value = scale8(RING_LOCAL_BRIGHTNESS, qadd8(38, scale8(sin8(progress * 2 + uint8_t(i * 255U / RING_LEDS)), 210)));
        hue = 150 + ((i + progress / 32) & 1) * 8;
        break;
      default:
        break;
    }
    leds[MATRIX_LEDS + i] = CHSV(hue, step.saturation, value);
  }
}

void renderCurrentStep(uint32_t now) {
  const PatternStep &step = DEMO_STEPS[currentStepIndex];
  const uint8_t progress = stepProgress(step, now);
  const CRGB colour = CHSV(step.hue, step.saturation, step.value);
  clearMatrix();

  switch (step.primitive) {
    case PRIM_VOXEL:
      setVoxel(step.a, step.b, step.c, colour);
      break;
    case PRIM_WIRE_BOX:
      drawWireBox(max<int8_t>(0, step.a), colour);
      break;
    case PRIM_SWEEP:
      drawPlaneSweep(constrain(step.a, 0, 2), progress, colour);
      break;
    case PRIM_ORBIT:
      drawOrbit(progress, colour);
      break;
    case PRIM_SPHERE:
      drawSphereShell(uint8_t(10 + scale8(uint8_t(max<int8_t>(1, step.a) * 16), progress)), colour);
      break;
    case PRIM_BOUNCE:
      drawBouncer(progress, colour);
      break;
    case PRIM_SPARKLES:
      drawSparkles(progress, uint8_t(max<int8_t>(1, step.a)), colour);
      break;
  }
  renderRing(step, progress); // Always after matrix primitives: separate output range.
}

// -----------------------------------------------------------------------------
// 5. Arduino lifecycle: all animation state derives from millis().
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  buildLUTs();
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, TOTAL_LEDS);
  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  clearMatrix();
  if (RING_LEDS) fill_solid(leds + MATRIX_LEDS, RING_LEDS, CRGB::Black);
  FastLED.show();
  stepStartedAt = millis();
  Serial.printf("Volumetric Engine Demo: %ux%ux%u, %u matrix LEDs + %u ring LEDs\n",
                VOLUME_X, VOLUME_Y, VOLUME_Z, MATRIX_LEDS, RING_LEDS);
}

void loop() {
  const uint32_t now = millis();
  advanceTimeline(now);            // State/timeline timing is independent of frame rate.
  if (now - lastFrameAt < FRAME_INTERVAL_MS) return;
  lastFrameAt = now;
  renderCurrentStep(now);
  FastLED.show();
}
