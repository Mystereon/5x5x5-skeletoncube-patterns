# 18_SpaceInvaders — Space Invaders

Marching voxel invaders with a laser.

This is a **fully standalone, directly uploadable** Arduino FastLED sketch. It
contains its own 5×5×5 mapper, all pattern code, `setup()`, and `loop()`. The
only companion file is `DemoTypes.h` in the same folder, used solely so Arduino
sees the shared `Vec3` declaration before generating prototypes; there is no
master-library or outside-project dependency.

## Upload

Open `18_SpaceInvaders.ino` in Arduino IDE and upload to the ESP32-C3. Defaults: 125 GRB
WS2812B LEDs on data pin 2, brightness 100, bottom–rear–left origin, and
`index = z * 25 + y * 5 + x`.
