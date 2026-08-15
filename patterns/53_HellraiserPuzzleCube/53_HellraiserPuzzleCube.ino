/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon
  HellraiserPuzzleCube.ino — standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.
*/
#include <FastLED.h>
#include <math.h>
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t N=5,BRIGHTNESS=100;constexpr uint16_t NUM_LEDS=125;CRGB leds[NUM_LEDS];
uint16_t idx(uint8_t x,uint8_t y,uint8_t z){return z*25+y*5+x;}void voxel(int8_t x,int8_t y,int8_t z,CRGB c){if(x>=0&&x<5&&y>=0&&y<5&&z>=0&&z<5)leds[idx(x,y,z)]=c;}
void setup(){FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){float t=millis()*.001f;fill_solid(leds,NUM_LEDS,CRGB::Black);uint8_t seam=uint8_t(120+sinf(t*3)*95);for(int8_t z=0;z<5;z++)for(int8_t y=0;y<5;y++)for(int8_t x=0;x<5;x++){if(!(x==0||x==4||y==0||y==4||z==0||z==4))continue;bool line=((x+int(t*.6f))%2==0&&(y==0||y==4))||(z==2&&((x+y)&1));if(line)voxel(x,y,z,CHSV(151,175,seam));else if((x*3+y*5+z*7+int(t))%9==0)voxel(x,y,z,CRGB(175,55,5));else voxel(x,y,z,CRGB(18,12,7));}FastLED.show();}
