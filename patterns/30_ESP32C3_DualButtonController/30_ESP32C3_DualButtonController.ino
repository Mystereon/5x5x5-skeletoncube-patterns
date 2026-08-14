/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  SkeletonCube Patterns — ESP32-C3 SuperMini dual-button controller
  Created by Dad (MysterEon) & Manus, 2026.

  Wiring:
    GPIO3 ---- momentary push button ---- GND   (next pattern, manual mode)
    GPIO4 ---- momentary push button ---- GND   (toggle auto/manual)

  The gallery starts in automatic mode. Press GPIO4 to pause the cycle and
  enter manual mode. Press GPIO3 to step patterns. Press GPIO4 again to resume
  automatic cycling from the currently displayed pattern.
*/

#define SKELETONCUBE_AUTO_CYCLE 1
#define SKELETONCUBE_PATTERN_BUTTON 1
#define SKELETONCUBE_MODE_BUTTON 1
#include "../../SkeletonCubePatterns/SkeletonCubePatterns.ino"
