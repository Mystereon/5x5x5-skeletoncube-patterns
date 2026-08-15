#pragma once

/*
  CubeFX hardware profile
  -----------------------
  Edit these three values before uploading CubeFXWeb if your wiring differs.
  The LED data pin is a FastLED compile-time setting, so a data-pin change
  always requires recompiling and flashing this sketch. Button pins can also
  be changed from the Android controller after this initial upload.

  ESP32-C3 safety notes:
  - GPIO2, GPIO8, and GPIO9 are boot strapping pins; only use them when the
    associated switch is released at boot. GPIO8 is kept as the project
    default because it matches Dad's existing SuperMini wiring.
  - GPIO12–GPIO17 are reserved for SPI flash on typical ESP32-C3 boards.
  - GPIO18–GPIO19 disable USB-JTAG if repurposed.
*/

#define CUBEFX_LED_DATA_PIN 2
#define CUBEFX_PRIMARY_BUTTON_PIN 4
#define CUBEFX_SECONDARY_BUTTON_PIN 8

// Physical dimensions. The reported LED total is always the product below.
// CubeFXWeb's current visual effects are authored for a 5×5×5 skeletal cube;
// retain 5, 5, 5 until a generic rectangular-volume renderer is introduced.
#define CUBEFX_COLUMNS 5
#define CUBEFX_ROWS 5
#define CUBEFX_LAYERS 5
#define CUBEFX_TOTAL_LEDS (CUBEFX_COLUMNS * CUBEFX_ROWS * CUBEFX_LAYERS)
