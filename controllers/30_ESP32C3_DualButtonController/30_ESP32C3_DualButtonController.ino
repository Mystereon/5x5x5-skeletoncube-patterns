/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  SkeletonCube Patterns — ESP32-C3 SuperMini dual-button controller
  Created by Dad (MysterEon) & Manus, 2026.

  Wiring:
    GPIO4 ---- momentary push button ---- GND   (next pattern, manual mode)
    GPIO8 ---- momentary push button ---- GND   (toggle auto/manual)

  The gallery starts in automatic mode. Press GPIO8 to pause the cycle and
  enter manual mode. Press GPIO4 to step patterns. Press GPIO8 again to resume
  automatic cycling from the currently displayed pattern.

  GPIO8 is an ESP32-C3 boot strapping pin: do not hold its button during reset
  or power-up. A 10 kΩ pull-up to 3V3 is recommended if the board lacks one.
*/

#define SKELETONCUBE_AUTO_CYCLE 1
#define SKELETONCUBE_PATTERN_BUTTON 1
#define SKELETONCUBE_MODE_BUTTON 1
#define SKELETONCUBE_PATTERN_BUTTON_PIN 4
#define SKELETONCUBE_MODE_BUTTON_PIN 8
#include "../../SkeletonCubePatterns/SkeletonCubePatterns.ino"
