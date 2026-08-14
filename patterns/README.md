# Pattern reference sketches

Every folder here exposes one effect as an individual Arduino sketch. The first
28 are small wrappers that disable automatic cycling, select exactly one
`Pattern`, and include the shared `SkeletonCubePatterns.ino` source. Folder 29
is an ESP32-C3 SuperMini single-button controller, and folder 30 is a dual-button
GPIO3/GPIO4 auto/manual controller. This avoids copied implementations drifting
apart while still providing direct, uploadable pattern entry points for
exploration and reference.

Download or clone the whole repository before opening an example, because the
wrapper uses a relative include path to the shared master sketch.
