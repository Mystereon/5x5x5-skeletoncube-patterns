/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon
  RubiksCube3D.ino — standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.
*/
#include <FastLED.h>
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t N=5,BRIGHTNESS=100;constexpr uint16_t NUM_LEDS=125;CRGB leds[NUM_LEDS];
uint16_t idx(uint8_t x,uint8_t y,uint8_t z){return z*25+y*5+x;}void voxel(int8_t x,int8_t y,int8_t z,CRGB c){if(x>=0&&x<5&&y>=0&&y<5&&z>=0&&z<5)leds[idx(x,y,z)]=c;}
void setup(){FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){fill_solid(leds,NUM_LEDS,CRGB::Black);uint8_t turn=uint8_t(millis()*.0012f)%6;for(int8_t z=1;z<=3;z++)for(int8_t y=1;y<=3;y++)for(int8_t x=1;x<=3;x++){if(!(x==1||x==3||y==1||y==3||z==1||z==3))continue;uint8_t h=x==1?0:x==3?32:y==1?96:y==3?160:z==1?192:42;if((x+y+z+turn)%4==0)h+=70;voxel(x,y,z,CHSV(h,235,245));}FastLED.show();}
