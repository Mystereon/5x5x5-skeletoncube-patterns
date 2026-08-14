# Pattern reference sketches

Every folder here exposes one effect as an individual Arduino sketch. Each is a
small wrapper that disables automatic cycling, selects exactly one `Pattern`,
and includes the shared `SkeletonCubePatterns.ino` source. This avoids 28
copied implementations drifting apart while still providing direct, uploadable
pattern entry points for exploration and reference.

Download or clone the whole repository before opening an example, because the
wrapper uses a relative include path to the shared master sketch.
