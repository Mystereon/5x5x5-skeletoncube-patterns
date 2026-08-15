/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon
  StargateDialUp.ino — standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.
*/
#include <FastLED.h>
#include <math.h>
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t N=5, BRIGHTNESS=100; constexpr uint16_t NUM_LEDS=125;
CRGB leds[NUM_LEDS]; uint16_t idx(uint8_t x,uint8_t y,uint8_t z){return z*25+y*5+x;}
void voxel(int8_t x,int8_t y,int8_t z,CRGB c){if(x>=0&&x<5&&y>=0&&y<5&&z>=0&&z<5)leds[idx(x,y,z)]=c;}
void setup(){FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){
  float t=millis()*.001f; fill_solid(leds,NUM_LEDS,CRGB::Black); uint8_t lock=min<uint8_t>(7,uint8_t(fmodf(t*.8f,9)));
  for(int8_t x=0;x<5;x++)for(int8_t y=0;y<5;y++)voxel(x,y,0,CRGB(25,18,12));
  for(int8_t z=1;z<5;z++)for(int8_t x=0;x<5;x++){float dx=x-2,dz=z-2.3f,r=sqrtf(dx*dx+dz*dz);if(r>1.5f&&r<2.65f)voxel(x,2,z,CRGB(36,42,45));if(lock>=7&&r<1.45f)voxel(x,2,z,CHSV(145,185,160+uint8_t(sinf(t*5+x+z)*45)));}
  const int8_t cx[7]={2,4,4,2,0,0,2},cz[7]={4,3,1,1,1,3,4};for(uint8_t i=0;i<lock;i++)voxel(cx[i],2,cz[i],CRGB::Orange);FastLED.show();
}
