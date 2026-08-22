#pragma once

#include <Arduino.h>

constexpr uint8_t USER_PATTERN_SLOT_COUNT = 5;
constexpr uint8_t USER_PATTERN_STEP_COUNT = 16;
constexpr uint8_t USER_PATTERN_MASK_BYTES = 16;

struct UserPatternStep {
  uint8_t mask[USER_PATTERN_MASK_BYTES];
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t holdTicks;
};

struct UserPatternSlot {
  UserPatternStep steps[USER_PATTERN_STEP_COUNT];
};

static_assert(sizeof(UserPatternStep) == 20, "User pattern steps must remain compact");
static_assert(sizeof(UserPatternSlot) == 320, "Each user pattern slot must stay at 320 bytes");
