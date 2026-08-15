/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  SkeletonCube Patterns — ESP32-C3 SuperMini next-pattern button controller
  Created by Dad (MysterEon) & Manus, 2026.

  Wiring:
    GPIO3 ---- momentary push button ---- GND

  GPIO3 uses the ESP32-C3 internal pull-up. A stable LOW press advances the
  gallery by one pattern. There is no automatic cycling in this version.
*/

#define SKELETONCUBE_AUTO_CYCLE 0
#define SKELETONCUBE_PATTERN_BUTTON 1
#define SKELETONCUBE_PATTERN_BUTTON_PIN 3
#include "../../SkeletonCubePatterns/SkeletonCubePatterns.ino"
