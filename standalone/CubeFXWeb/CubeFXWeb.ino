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
    GPIO4: NEXT pattern in manual mode
    GPIO8: AUTO / MANUAL toggle

  GPIO8 is an ESP32-C3 boot-strapping pin: RELEASE it while resetting or
  powering up. Use a 10 kΩ pull-up from GPIO8 to 3V3 if your board does not
  already hold it high. Some SuperMini variants attach a status LED to GPIO8.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <esp_system.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

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
  PATTERN_COUNT
};

const char *const patternNames[PATTERN_COUNT] = {
  "Red Vector Cube", "3-D Matrix Rain", "Neon Plasma", "Volume Fire",
  "Twin Spirals", "Wrapping Comets", "Self-playing Pong", "Conway 3-D Life",
  "Cloud Volume", "White Glitter", "Corner Cubes", "3x5 Perimeter Banner"
};

Pattern currentPattern = PATTERN_VECTOR_CUBE;
bool autoCycle = true;
uint8_t speedControl = 150;        // 1 slow .. 255 fast
uint32_t cycleDurationMs = 30000;  // 5 .. 120 seconds from the browser
uint32_t patternStartedAt = 0;
uint32_t lastFrameAt = 0;

float effectTime() {
  return millis() * 0.001f * (0.20f + speedControl / 130.0f);
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

void renderMatrixRain(float t) {
  fadeToBlackBy(leds, NUM_LEDS, 56);
  const uint8_t frame = uint8_t(t * 7.0f);
  for (uint8_t x = 0; x < N; ++x) {
    for (uint8_t y = 0; y < N; ++y) {
      const uint8_t phase = uint8_t(x * 37 + y * 53);
      const int8_t head = (phase + frame) % (N + 4) - 2;
      addVoxel(x, y, head, CRGB(160, 255, 175));
      addVoxel(x, y, head - 1, CRGB(0, 130, 25));
      addVoxel(x, y, head - 2, CRGB(0, 50, 8));
    }
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
  {2,5,7,5,5}, {6,5,6,5,6}, {3,4,4,4,3}, {6,5,5,5,6}, // A-E
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
    bannerOffset = (bannerOffset + 1) % textColumns;
  }

  for (uint8_t p = 0; p < PERIMETER_COLUMNS; ++p) {
    const uint16_t messageColumn = (bannerOffset + p) % textColumns;
    const uint8_t characterIndex = messageColumn / glyphPitch;
    const uint8_t glyphColumn = messageColumn % glyphPitch;
    if (glyphColumn == glyphWidth) continue; // inter-character spacer

    for (uint8_t z = 0; z < N; ++z) {
      if (!bannerGlyphPixel(bannerText[characterIndex], glyphColumn, N - 1 - z)) continue;
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

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>CUBE.FX / 5×5×5</title><style>
:root{--ink:#050706;--panel:#0c110e;--line:#c8ff20;--acid:#8cff00;--muted:#789077;--red:#ff4e40}
*{box-sizing:border-box}body{margin:0;background:var(--ink);color:#eaffd2;font-family:ui-monospace,Menlo,monospace;min-height:100vh}
header{border-bottom:3px solid var(--line);padding:16px 20px;display:flex;justify-content:space-between;gap:12px;background:#0b100d}h1{margin:0;font-size:clamp(22px,4vw,40px);letter-spacing:-2px}small{color:var(--muted)}
#shell{display:grid;grid-template-columns:150px 1fr;min-height:calc(100vh - 78px)}nav{border-right:3px solid var(--line);padding:12px;background:#081009}nav button{width:100%;text-align:left;margin:0 0 8px;padding:10px;border:2px solid #426840;background:#101b12;color:#d9ffd1;font:inherit;cursor:pointer}nav button:hover,nav button.active{background:var(--acid);color:#061000;border-color:var(--acid)}main{padding:20px;max-width:1000px}.panel{display:none}.panel.active{display:block}.grid{display:grid;grid-template-columns:minmax(270px,1fr) minmax(270px,1fr);gap:16px}.box{border:2px solid #426840;background:var(--panel);padding:16px}.box h2{margin:0 0 12px;color:var(--line);font-size:16px}.status{font-size:20px;color:var(--acid);padding:8px 0}
select,input,button{font:inherit}select,input[type=text]{width:100%;padding:12px;background:#030603;color:var(--acid);border:2px solid var(--line)}input[type=range]{width:100%;accent-color:var(--acid)}.row{display:flex;justify-content:space-between;gap:12px;align-items:center;padding:8px 0;border-bottom:1px solid #29412a}.row:last-child{border:0}.control{display:block;margin:14px 0}.control label{display:flex;justify-content:space-between;color:#c4ddbd;margin-bottom:6px}.action{padding:11px 13px;border:2px solid var(--line);background:var(--acid);color:#061000;font-weight:800;cursor:pointer}.action.dark{background:#0d170e;color:var(--line)}.action.red{border-color:var(--red);color:#ffd8d0;background:#28100d}.actions{display:flex;gap:8px;flex-wrap:wrap}canvas{width:100%;background:#020403;border:2px solid #426840;image-rendering:pixelated}.note{color:var(--muted);font-size:12px;line-height:1.5}.credit{border-left:3px solid var(--red);padding-left:10px;color:#ffd5cb;font-size:12px;line-height:1.4}
@media(max-width:680px){#shell{grid-template-columns:1fr}nav{border-right:0;border-bottom:3px solid var(--line);display:flex;overflow:auto;padding:8px}nav button{width:auto;white-space:nowrap;margin:0 6px 0 0}.grid{grid-template-columns:1fr}main{padding:12px}}
</style></head><body><header><div><h1>CUBE.FX</h1><small>5×5×5 / 125 VOXELS / ESP32-C3</small></div><div id="net">CONNECTING…</div></header>
<div id="shell"><nav><button class="active" data-tab="live">01 LIVE</button><button data-tab="patterns">02 PATTERNS</button><button data-tab="control">03 CONTROL</button><button data-tab="about">04 ABOUT</button></nav><main>
<section class="panel active" id="live"><div class="grid"><div class="box"><h2>NOW PLAYING</h2><div class="status" id="name">—</div><div class="row"><span>MODE</span><b id="mode">—</b></div><div class="row"><span>BRIGHTNESS</span><b id="brightRead">—</b></div><div class="row"><span>SPEED</span><b id="speedRead">—</b></div><p class="note">The preview is a logical voxel view: red is the bottom–rear–left origin. It updates from the live cube framebuffer.</p><div class="actions"><button class="action" onclick="next()">NEXT</button><button class="action dark" onclick="toggleAuto()" id="autoBtn">AUTO</button></div></div><div class="box"><h2>LIVE VOXEL PREVIEW</h2><canvas id="cube" width="500" height="420"></canvas></div></div></section>
<section class="panel" id="patterns"><div class="box"><h2>PATTERN GALLERY</h2><select id="pattern" size="12"></select><p class="note">Select an effect to enter manual mode immediately. The physical GPIO4 button also advances patterns in manual mode.</p></div></section>
<section class="panel" id="control"><div class="box"><h2>ENGINE CONTROL</h2><div class="control"><label>BRIGHTNESS <b id="bv">100</b></label><input id="brightness" type="range" min="1" max="255" value="100"></div><div class="control"><label>SPEED <b id="sv">150</b></label><input id="speed" type="range" min="1" max="255" value="150"></div><div class="control"><label>AUTO DWELL, SECONDS <b id="cv">30</b></label><input id="cycle" type="range" min="5" max="120" value="30"></div><div class="actions"><button class="action" onclick="applyControls()">APPLY</button><button class="action red" onclick="api('reseed=1')">RESEED LIFE</button></div></div><div class="box" style="margin-top:16px"><h2>3×5 PERIMETER BANNER</h2><div class="control"><label>MESSAGE</label><input id="bannerText" type="text" maxlength="60" value="CUBE 4 3 2 1 0"></div><div class="control"><label>FONT MODE</label><select id="bannerFont"><option value="3">3×5 / COMPACT</option><option value="5">5×5 / BOLD FULL-FACE</option></select></div><div class="control"><label>COLOUR <b id="bhv">96</b></label><input id="bannerHue" type="range" min="0" max="255" value="96"></div><div class="control"><label>SCROLL SPEED <b id="bsv">150</b></label><input id="bannerSpeed" type="range" min="1" max="255" value="150"></div><div class="actions"><button class="action" onclick="applyBanner()">SEND TO CUBE</button><button class="action dark" onclick="api('pattern=11')">SHOW BANNER</button></div><p class="note">Choose compact 3×5 or bold 5×5 full-face letters. The message scrolls clockwise around the rear, right, front, and left exterior faces. Supported: A–Z, 0–9, space, period, dash, colon, exclamation, and question mark.</p></div></section>
<section class="panel" id="about"><div class="box"><h2>WHAT IS CUBE.FX?</h2><p>A cube-first browser controller inspired by the convenience of <a href="https://github.com/kitesurfer1404/WS2812FX" style="color:#c8ff20">WS2812FX</a>, but built for 3-D voxel patterns rather than flat LED strips.</p><p class="credit">FEED ME , I'M POOR AND I MADE THIS FOR FREE — https://paypal.me/Mystereon</p><p class="note">Created by Dad (MysterEon) &amp; Manus. GPIO4 is next-pattern in manual mode. GPIO8 toggles auto/manual; release GPIO8 while resetting because it is a C3 strapping pin.</p></div></section>
</main></div><script>
const names=['Red Vector Cube','3-D Matrix Rain','Neon Plasma','Volume Fire','Twin Spirals','Wrapping Comets','Self-playing Pong','Conway 3-D Life','Cloud Volume','White Glitter','Corner Cubes','3x5 Perimeter Banner'];
let state={}, frame=[];const $=id=>document.getElementById(id);const ctx=$('cube').getContext('2d');
names.forEach((n,i)=>{let o=document.createElement('option');o.value=i;o.textContent=String(i+1).padStart(2,'0')+' / '+n;$('pattern').append(o)});
document.querySelectorAll('nav button').forEach(b=>b.onclick=()=>{document.querySelectorAll('nav button,.panel').forEach(x=>x.classList.remove('active'));b.classList.add('active');$(b.dataset.tab).classList.add('active')});
$('pattern').onchange=()=>api('pattern='+$('pattern').value);['brightness','speed','cycle'].forEach(k=>$(k).oninput=()=>{$(k[0]+'v').textContent=$(k).value});$('bannerHue').oninput=()=>$('bhv').textContent=$('bannerHue').value;$('bannerSpeed').oninput=()=>$('bsv').textContent=$('bannerSpeed').value;
function api(q){fetch('/api/control?'+q).then(refresh)}function next(){api('next=1')}function toggleAuto(){api('auto='+(state.auto?0:1))}function applyControls(){api('brightness='+$('brightness').value+'&speed='+$('speed').value+'&cycle='+$('cycle').value)}function applyBanner(){api('text='+encodeURIComponent($('bannerText').value)+'&bannerFont='+$('bannerFont').value+'&bannerHue='+$('bannerHue').value+'&bannerSpeed='+$('bannerSpeed').value)}
function draw(){const w=500,h=420;ctx.fillStyle='#020403';ctx.fillRect(0,0,w,h);const p=(x,y,z)=>[250+(x-y)*34,335-(x+y)*17-z*47];for(let z=0;z<5;z++)for(let y=0;y<5;y++)for(let x=0;x<5;x++){let [px,py]=p(x,y,z),col=frame[z*25+y*5+x]||'#000000';ctx.beginPath();ctx.fillStyle=col;ctx.arc(px,py,9,0,Math.PI*2);ctx.fill();ctx.strokeStyle='#1c301d';ctx.stroke()}}
function render(){if(!state.name)return;$('name').textContent=state.name;$('mode').textContent=state.auto?'AUTO':'MANUAL';$('brightRead').textContent=state.brightness;$('speedRead').textContent=state.speed;$('net').textContent=(state.ap?'AP @ ':'WIFI @ ')+state.ip;$('autoBtn').textContent=state.auto?'PAUSE AUTO':'RESUME AUTO';$('pattern').value=state.pattern;$('brightness').value=state.brightness;$('speed').value=state.speed;$('cycle').value=state.cycle;$('bannerHue').value=state.bannerHue;$('bannerSpeed').value=state.bannerSpeed;$('bannerFont').value=state.bannerFont;if(document.activeElement!==$('bannerText'))$('bannerText').value=state.banner;['brightness','speed','cycle'].forEach(k=>$(k[0]+'v').textContent=$(k).value);$('bhv').textContent=state.bannerHue;$('bsv').textContent=state.bannerSpeed;draw()}
function refresh(){Promise.all([fetch('/api/state').then(r=>r.json()),fetch('/api/frame').then(r=>r.json())]).then(a=>{state=a[0];frame=a[1].voxels;render()}).catch(()=>{$('net').textContent='RECONNECTING…'})}setInterval(refresh,650);refresh();
</script></body></html>
)HTML";

void handleRoot() { web.send_P(200, "text/html", INDEX_HTML); }
void handleState() { web.send(200, "application/json", stateJson()); }
void handleFrame() { web.send(200, "application/json", frameJson()); }

void handleControl() {
  if (web.hasArg("pattern")) {
    const int value = web.arg("pattern").toInt();
    if (value >= 0 && value < PATTERN_COUNT) {
      currentPattern = Pattern(value);
      autoCycle = false;
      patternStartedAt = millis();
    }
  }
  if (web.hasArg("brightness")) {
    brightness = constrain(web.arg("brightness").toInt(), 1, 255);
    FastLED.setBrightness(brightness);
  }
  if (web.hasArg("speed")) speedControl = constrain(web.arg("speed").toInt(), 1, 255);
  if (web.hasArg("cycle")) cycleDurationMs = constrain(web.arg("cycle").toInt(), 5, 120) * 1000UL;
  if (web.hasArg("text")) setBannerMessage(web.arg("text"));
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
  randomSeed(esp_random());
  seedLife();
  setupButtons();
  setupNetwork();
  setupWeb();
  patternStartedAt = millis();
}

void loop() {
  web.handleClient();
  updateButtons();

  if (autoCycle && millis() - patternStartedAt >= cycleDurationMs) advancePattern();
  if (millis() - lastFrameAt >= 16) {
    lastFrameAt = millis();
    renderCurrentPattern();
    FastLED.show();
  }
}
