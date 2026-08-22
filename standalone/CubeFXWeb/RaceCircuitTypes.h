#pragma once

#include <Arduino.h>

// These declarations must be visible before Arduino generates function
// prototypes for CubeFXWeb.ino; the race renderer intentionally uses them in
// return values and parameters.
struct RacePathPoint { int16_t x; int16_t y; };
struct RaceTracksideProp { int16_t x; int16_t y; };
struct RaceRoadHit { uint16_t distanceSq; uint8_t segment; };
