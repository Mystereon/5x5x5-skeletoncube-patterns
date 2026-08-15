/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon
  Chequerboard3D.ino — standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.
*/
#include <FastLED.h>
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t N=5,BRIGHTNESS=100;constexpr uint16_t NUM_LEDS=125;CRGB leds[NUM_LEDS];
uint16_t idx(uint8_t x,uint8_t y,uint8_t z){return z*25+y*5+x;}void voxel(int8_t x,int8_t y,int8_t z,CRGB c){if(x>=0&&x<5&&y>=0&&y<5&&z>=0&&z<5)leds[idx(x,y,z)]=c;}
void setup(){FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){int8_t s=int8_t(millis()*.0013f);fill_solid(leds,NUM_LEDS,CRGB::Black);for(int8_t z=0;z<5;z++)for(int8_t y=0;y<5;y++)for(int8_t x=0;x<5;x++){bool plane=(x+y+s)%5==z||((x+z+s)%5==y&&(s&2));if(plane&&((x+y+z+s)&1)==0)voxel(x,y,z,CHSV(130+(s&31),130,235));}FastLED.show();}
