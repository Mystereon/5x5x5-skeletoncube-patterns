# 56_ZarchVoxelDefender — Zarch: Voxel Defender

A long-form self-playing voxel terrain-defence battle with patrols, staged lander waves, recovery pauses, forward shots, and cached terrain.

This is a **fully standalone, directly uploadable** Arduino FastLED sketch. It
contains its own 5×5×5 mapper, all supporting code, `setup()`, and `loop()`.
There is no browser-controller, master-sketch, or local-header dependency.

## Upload

Open `56_ZarchVoxelDefender.ino` in Arduino IDE and upload it. Defaults: ESP32-S3 Zero with 125 GRB WS2812B LEDs on GPIO6, primary button GPIO2, secondary button GPIO4, brightness 100, bottom–rear–left origin, and `index = z * 25 + y * 5 + x`.
