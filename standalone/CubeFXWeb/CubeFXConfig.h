#pragma once

/*
  CubeFX hardware profile
  -----------------------
  Edit these three values before uploading CubeFXWeb if your wiring differs.
  The LED data pin is a FastLED compile-time setting, so a data-pin change
  always requires recompiling and flashing this sketch. Button pins can also
  be changed from the Android controller after this initial upload.

  ESP32-S3 Zero enclosure profile:
  - GPIO6 is the single WS2812B data chain: 125 skeletal-cube LEDs followed
    by a 12-pixel rear-facing enclosure mood ring.
  - GPIO2 is the primary button and GPIO4 is the secondary button. Both
    normally-open switches connect to GND and use INPUT_PULLUP.
  - GPIO0 remains BOOT, GPIO21 is the onboard RGB LED, GPIO33–37 belong to
    PSRAM, and GPIO43/44 are UART0. Leave those pins alone.
*/

#define CUBEFX_LED_DATA_PIN 6
#define CUBEFX_PRIMARY_BUTTON_PIN 2
#define CUBEFX_SECONDARY_BUTTON_PIN 4

// Physical dimensions. The reported LED total is always the product below.
// CubeFXWeb's current visual effects are authored for a 5×5×5 skeletal cube;
// retain 5, 5, 5 until a generic rectangular-volume renderer is introduced.
#define CUBEFX_COLUMNS 5
#define CUBEFX_ROWS 5
#define CUBEFX_LAYERS 5
#define CUBEFX_MATRIX_LEDS (CUBEFX_COLUMNS * CUBEFX_ROWS * CUBEFX_LAYERS)
#define CUBEFX_MOOD_LED_COUNT 12
// Applied before FastLED's global brightness limit. 160/255 makes the acrylic
// enclosure visibly part of a scene while the global brightness cap still
// protects the full 137-LED installation.
#define CUBEFX_MOOD_RING_BRIGHTNESS 160
#define CUBEFX_TOTAL_LEDS (CUBEFX_MATRIX_LEDS + CUBEFX_MOOD_LED_COUNT)
