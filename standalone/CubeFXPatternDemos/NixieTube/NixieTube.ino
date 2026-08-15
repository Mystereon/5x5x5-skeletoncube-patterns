/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  NixieTube.ino — directly uploadable 5×5×5 FastLED demo
  Created by Dad (MysterEon) & Manus, 2026.

  A dim-orange seven-segment tube cycles 0–9. Lit segments burn bright orange.
*/
#include <FastLED.h>

constexpr uint8_t N = 5, BRIGHTNESS = 100;
constexpr uint16_t NUM_LEDS = N * N * N;
#define DATA_PIN 2
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
uint16_t indexFromXYZ(uint8_t x,uint8_t y,uint8_t z){return uint16_t(z)*25+uint16_t(y)*5+x;}
void setVoxel(int8_t x,int8_t y,int8_t z,const CRGB &c){if(x>=0&&x<N&&y>=0&&y<N&&z>=0&&z<N)leds[indexFromXYZ(x,y,z)]=c;}
void addVoxel(int8_t x,int8_t y,int8_t z,const CRGB &c){if(x>=0&&x<N&&y>=0&&y<N&&z>=0&&z<N)leds[indexFromXYZ(x,y,z)]+=c;}
const uint8_t segments[10]={B00111111,B00000110,B01011011,B01001111,B01100110,B01101101,B01111101,B00000111,B01111111,B01101111};
void drawSegment(uint8_t s,const CRGB &c){const int8_t sx[7]={1,4,4,1,0,0,1},sz[7]={4,3,0,0,0,3,2},dx[7]={1,0,0,1,0,0,1},dz[7]={0,1,1,0,1,1,0};const uint8_t len[7]={3,2,2,3,2,2,3};for(uint8_t p=0;p<len[s];++p){const int8_t x=sx[s]+dx[s]*p,z=sz[s]+dz[s]*p;setVoxel(x,4,z,c);addVoxel(x,3,z,CRGB(c.r/7,c.g/7,0));}}
void renderNixie(){fill_solid(leds,NUM_LEDS,CRGB::Black);const uint8_t digit=(millis()/1000)%10,pulse=205+(sin8(millis()/7)>>2);for(uint8_t s=0;s<7;++s)drawSegment(s,(segments[digit]&(1<<s))?CRGB(pulse,70+pulse/4,4):CRGB(28,7,0));}
void setup(){FastLED.addLeds<CHIPSET,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);FastLED.setBrightness(BRIGHTNESS);FastLED.setMaxPowerInVoltsAndMilliamps(5,1500);}
void loop(){renderNixie();FastLED.show();}
