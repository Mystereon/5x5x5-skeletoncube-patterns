/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  SinglePlayerPong.ino — directly uploadable 5×5×5 FastLED demo
  Created by Dad (MysterEon) & Manus, 2026.

  Short GPIO4 press: move the cyan full-height paddle left.
  Short GPIO8 press: move the cyan full-height paddle right.
  GPIO8 is an ESP32-C3 strapping pin: keep it released while booting.
*/
#include <FastLED.h>
#include <esp_system.h>

constexpr uint8_t N = 5, BRIGHTNESS = 100;
constexpr uint16_t NUM_LEDS = N * N * N;
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t LEFT_BUTTON_PIN = 4, RIGHT_BUTTON_PIN = 8;
CRGB leds[NUM_LEDS];

uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) { return uint16_t(z) * 25 + uint16_t(y) * 5 + x; }
void setVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[indexFromXYZ(x, y, z)] = colour; }

int8_t ballX = 2, ballY = 2, ballDX = 1, ballDY = 1, playerX = 2, cpuX = 2;
uint8_t playerScore = 0, cpuScore = 0;
uint32_t lastBallAt = 0, leftChangedAt = 0, rightChangedAt = 0;
bool leftLast = HIGH, rightLast = HIGH;

void resetRound(int8_t direction) { ballX = 2; ballY = 2; ballDX = random8(2) ? 1 : -1; ballDY = direction; }
void movePaddle(int8_t delta) { playerX = constrain(playerX + delta, 0, 4); }
void updateButtons() {
  const uint32_t now = millis(); const bool left = digitalRead(LEFT_BUTTON_PIN), right = digitalRead(RIGHT_BUTTON_PIN);
  if (left != leftLast && now - leftChangedAt > 35) { leftChangedAt = now; leftLast = left; if (!left) movePaddle(-1); }
  if (right != rightLast && now - rightChangedAt > 35) { rightChangedAt = now; rightLast = right; if (!right) movePaddle(1); }
}
void stepPong() {
  if (millis() - lastBallAt < 210) return; lastBallAt = millis();
  ballX += ballDX; ballY += ballDY;
  if (ballX <= 0 || ballX >= 4) { ballDX = -ballDX; ballX = constrain(ballX, 0, 4); }
  if (cpuX < ballX) ++cpuX; else if (cpuX > ballX) --cpuX;
  if (ballY <= 0) { if (ballX == playerX) { ballY = 0; ballDY = 1; } else { ++cpuScore; resetRound(1); } }
  if (ballY >= 4) { if (ballX == cpuX) { ballY = 4; ballDY = -1; } else { ++playerScore; resetRound(-1); } }
}
void renderPong() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (uint8_t z = 0; z < N; ++z) setVoxel(playerX, 0, z, CRGB::Aqua);
  for (uint8_t z = 0; z < N; ++z) setVoxel(cpuX, 4, z, CRGB::Red);
  setVoxel(ballX, ballY, 2, CRGB::White);
  for (uint8_t score = 0; score < min(playerScore, uint8_t(3)); ++score) setVoxel(score, 0, 4, CRGB::Aqua);
  for (uint8_t score = 0; score < min(cpuScore, uint8_t(3)); ++score) setVoxel(4 - score, 4, 4, CRGB::Red);
}
void setup() { FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); FastLED.setBrightness(BRIGHTNESS); FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500); randomSeed(esp_random()); pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP); pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP); }
void loop() { updateButtons(); stepPong(); renderPong(); FastLED.show(); }
