/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  BigMoonStars.ino — directly uploadable 5×5×5 FastLED demo
  Created by Dad (MysterEon) & Manus, 2026.

  A large shaded moon floats through a sky of slow, cool-blue twinkling stars.
*/
#include <FastLED.h>
#include <math.h>

constexpr uint8_t N = 5, BRIGHTNESS = 100;
constexpr uint16_t NUM_LEDS = N * N * N;
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
uint16_t indexFromXYZ(uint8_t x, uint8_t y, uint8_t z) { return uint16_t(z) * 25 + uint16_t(y) * 5 + x; }
void setVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[indexFromXYZ(x, y, z)] = colour; }
void addVoxel(int8_t x, int8_t y, int8_t z, const CRGB &colour) { if (x >= 0 && x < N && y >= 0 && y < N && z >= 0 && z < N) leds[indexFromXYZ(x, y, z)] += colour; }
void renderMoonStars() {
  fill_solid(leds, NUM_LEDS, CRGB::Black); const float cx=1.30f,cy=2.15f,cz=3.00f;
  for (uint8_t z=0;z<N;++z) for (uint8_t y=0;y<N;++y) for (uint8_t x=0;x<N;++x) { const float dx=x-cx,dy=y-cy,dz=z-cz,d=sqrtf(dx*dx+dy*dy+dz*dz); if (d<2.15f) { const uint8_t shade=uint8_t(220-d*56-max(0.0f,dx)*34); setVoxel(x,y,z,CRGB(shade,shade,uint8_t(shade*.82f))); } }
  for (uint8_t star=0;star<12;++star) { const uint8_t x=(star*2+3)%N,y=(star*3+1)%N,z=(star*4+2)%N; const float dx=x-cx,dy=y-cy,dz=z-cz; if (sqrtf(dx*dx+dy*dy+dz*dz)>2.15f) { const uint8_t v=85+(sin8(uint8_t(millis()/16+star*31))>>1); addVoxel(x,y,z,CRGB(v/3,v/2,v)); } }
}
void setup() { FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); FastLED.setBrightness(BRIGHTNESS); FastLED.setMaxPowerInVoltsAndMilliamps(5,1500); }
void loop() { renderMoonStars(); FastLED.show(); }
