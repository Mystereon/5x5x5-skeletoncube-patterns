package com.mystereon.cubefxcontroller

data class CubeFxPattern(val id: Int, val title: String, val embedded: Boolean)

object CubeFxPatternCatalog {
    private val embeddedIds = setOf(1, 9, 10, 11, 12, 19, 20, 21, 22, 26, 28) + (29..73)

    val all = listOf(
        "Red Vector Cube", "Solid Cube", "Sphere", "Octahedron", "Voxel Model", "Bouncing 2×2×2 Block", "Blue Rain", "Column Sweep",
        "3-D Matrix Rain", "Corner Cubes", "White Glitter", "Single-player Pong", "3-D Tetris", "Blinking Eye", "DNA Helix", "Meteors",
        "3-D Snake", "3-D Space Invaders", "Conway 3-D Life", "Cloud Volume", "Neon Plasma", "Volume Fire", "3-D Hourglass", "Expanding Ping-Pong Cube",
        "Upward Dense Red Rain", "Twin Spirals", "3-D Single-Point Bouncer", "Wrapping Comets", "Perimeter Banner", "Bullet Wall", "Padded Cell", "Block Run",
        "Parallax Starfield", "Trench Run", "Running Legs", "Fairies in Green Box", "Orange Fish Tank", "Three-Layer Pyramid", "Matrix Drift", "Intense Fire",
        "Magical Blue Fire", "Explosions", "Launching Fireworks", "Pixel Pasture", "Red Matrix Rain", "Voxel Minesweeper", "Big Moon & Stars", "Nixie Tube",
        "Black Hole Vortex", "Stargate Dial-Up", "3-D Defender", "3-D Chequerboard", "Hellraiser Puzzle Cube", "3-D Rubik's Cube",
        "Lissajous Layer Ripple", "Zarch: Voxel Defender", "Ring Bouncer — GPIO2 ring / GPIO4 voxel colour",
        "Help Me Obi-Wan Hologram — blue projector scene",
        "Voxel World Explorer — virtual block-world fly-through",
        "Phone VU Meter — live microphone link",
        "Phone Spectrum 3-D — live microphone link",
        "Cloud-Top Rain — GPIO2 rain / GPIO4 rear-ring colour",
        "Rotating Gold O — GPIO2 gold / GPIO4 turn speed",
        "Reactor Core — cyan containment / amber core",
        "Reactor Core Meltdown — alert state",
        "Targeting System — live reticle",
        "Phosphor Green Radar — sweeping scope",
        "Ghost Detector — ectoplasmic scan",
        "Alert — red warning field",
        "Intersecting Planes — moving geometry",
        "Oscillating Wave Field — surface motion",
        "Rainbow Spiral — volumetric ribbon",
        "Plasma Entity Containment — energised cage"
    ).mapIndexed { index, title -> CubeFxPattern(index + 1, title, index + 1 in embeddedIds) }

    /** The 56 direct-play modes currently compiled into CubeFXWeb. */
    val embeddedModes: List<CubeFxPattern> = all.filter { it.embedded }

    /** Historical standalone references: useful .ino sketches, not silent app buttons. */
    val standaloneModes: List<CubeFxPattern> = all.filterNot { it.embedded }
}
