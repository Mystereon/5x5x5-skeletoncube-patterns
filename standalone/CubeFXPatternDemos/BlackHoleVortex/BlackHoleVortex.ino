/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon
  BlackHoleVortex.ino — standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.
*/
#include <FastLED.h>
#include <math.h>
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t N=5, BRIGHTNESS=100; constexpr uint16_t NUM_LEDS=125;
CRGB leds[NUM_LEDS];
uint16_t idx(uint8_t x,uint8_t y,uint8_t z){return z*25+y*5+x;}
void voxel(int8_t x,int8_t y,int8_t z,CRGB c){if(x>=0&&x<5&&y>=0&&y<5&&z>=0&&z<5)leds[idx(x,y,z)]=c;}
void setup(){FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){
  const float t=millis()*.001f; fill_solid(leds,NUM_LEDS,CRGB::Black);
  for(int8_t z=0;z<N;z++)for(int8_t y=0;y<N;y++)for(int8_t x=0;x<N;x++){
    float dx=x-2,dy=y-2,dz=z-2,r=sqrtf(dx*dx+dy*dy+dz*dz); if(r<1.05f||r>3.1f)continue;
    float ribbon=sinf(atan2f(dy,dx)+t*1.2f+r*2.7f+dz*.8f);
    if(ribbon>.36f)voxel(x,y,z,CHSV(180+uint8_t(r*18),210,uint8_t(70+ribbon*150)));
  } FastLED.show();
}
