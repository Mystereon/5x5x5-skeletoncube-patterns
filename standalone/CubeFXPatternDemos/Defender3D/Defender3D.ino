/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon
  Defender3D.ino — standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.
*/
#include <FastLED.h>
#include <math.h>
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
constexpr uint8_t N=5,BRIGHTNESS=100; constexpr uint16_t NUM_LEDS=125; CRGB leds[NUM_LEDS];int8_t ship=2;
uint16_t idx(uint8_t x,uint8_t y,uint8_t z){return z*25+y*5+x;}void voxel(int8_t x,int8_t y,int8_t z,CRGB c){if(x>=0&&x<5&&y>=0&&y<5&&z>=0&&z<5)leds[idx(x,y,z)]=c;}
void setup(){pinMode(4,INPUT_PULLUP);pinMode(8,INPUT_PULLUP);FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){
  if(!digitalRead(4)){ship=max<int8_t>(0,ship-1);delay(90);}if(!digitalRead(8)){ship=min<int8_t>(4,ship+1);delay(90);}float t=millis()*.001f;fadeToBlackBy(leds,NUM_LEDS,68);
  voxel(ship,4,1,CRGB(0,150,235));voxel(ship-1,4,0,CRGB(0,60,90));voxel(ship+1,4,0,CRGB(0,60,90));int8_t ey=3-int8_t(fmodf(t*1.5f,4));for(int8_t i=0;i<3;i++){int8_t ex=(i*2+int(t*1.2f))%5;voxel(ex,max<int8_t>(0,ey),3,CRGB(225,35,12));voxel(ex,max<int8_t>(0,ey),2,CRGB(90,8,2));}voxel(ship,4-int8_t(fmodf(t*8,5)),1,CRGB(180,235,255));FastLED.show();
}
