# 35_RunningLegs — Running Legs

A miniature voxel runner with alternating legs.

This is a **fully standalone, directly uploadable** Arduino FastLED sketch. It
contains its own 5×5×5 mapper, all supporting code, `setup()`, and `loop()`.
There is no browser-controller, master-sketch, or local-header dependency.

## Upload

Open `35_RunningLegs.ino` in Arduino IDE and upload to the ESP32-C3. Defaults: 125 GRB
WS2812B LEDs on data pin 2, brightness 100, bottom–rear–left origin, and
`index = z * 25 + y * 5 + x`.
