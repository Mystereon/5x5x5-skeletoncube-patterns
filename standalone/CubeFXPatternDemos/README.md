# CubeFXPatternDemos — Actual Standalone Arduino Sketches

Each folder below contains a **complete, directly uploadable** Arduino `.ino` sketch. Every demo carries its own FastLED setup, 5×5×5 mapper, supporting functions, and selected pattern implementation. There are no relative includes and no requirement to copy CubeFXWeb or the master sketch alongside it.

All demos default to the established hardware map: 125 GRB WS2812B LEDs, `DATA_PIN 2`, brightness 100, and the bottom–rear–left origin with `index = z * 25 + y * 5 + x`.

| # | Folder | Pattern |
|---:|---|---|
| 01 | [`RedVectorCube`](RedVectorCube/) | Rotating red 3×3×3 vector wireframe. |
| 02 | [`MatrixRain`](MatrixRain/) | Straight-down deep emerald Matrix streams with yellow-green heads. |
| 03 | [`NeonPlasma`](NeonPlasma/) | Saturated trigonometric voxel plasma. |
| 04 | [`VolumeFire`](VolumeFire/) | Rising volumetric noise-driven fire. |
| 05 | [`TwinSpirals`](TwinSpirals/) | Opposing animated cyan and magenta spirals. |
| 06 | [`WrappingComets`](WrappingComets/) | Three wrapped diagonal comets with fading tails. |
| 07 | [`SelfPlayingPong`](SelfPlayingPong/) | A single ball with opposing voxel paddles. |
| 08 | [`Conway3DLife`](Conway3DLife/) | B5/S45 three-dimensional cellular automaton. |
| 09 | [`CloudVolume`](CloudVolume/) | Slow cyan volumetric noise clouds. |
| 10 | [`WhiteGlitter`](WhiteGlitter/) | Fast-fading random voxel sparkle. |
| 11 | [`CornerCubes`](CornerCubes/) | Eight colour-shifting 2×2×2 corner cubes. |
| 12 | [`PerimeterBanner`](PerimeterBanner/) | 3×5 or 5×5 exterior text banner, defaulting to CUBE 4 3 2 1 0. |
| 13 | [`BulletWall`](BulletWall/) | Red particles colliding with a single grey plate. |
| 14 | [`PaddedCell`](PaddedCell/) | A red pixel bouncing inside a dim-grey 3×3×3 padded chamber. |
| 15 | [`BlockRun`](BlockRun/) | A hopping red runner crossing a scrolling grass-and-block course. |
| 16 | [`ParallaxStarfield`](ParallaxStarfield/) | Three depth layers of independently moving stars. |
| 17 | [`TrenchRun`](TrenchRun/) | Blue-grey tunnel, red target, and white pilot reticle. |
| 18 | [`RunningLegs`](RunningLegs/) | Alternating voxel knees and wide-stepping feet. |
| 19 | [`FairiesInGreenBox`](FairiesInGreenBox/) | Drifting wing-pulsing fairy lights in a green wireframe enclosure. |
| 20 | [`OrangeFishTank`](OrangeFishTank/) | Blue-framed water volume with two orange fish. |
| 21 | [`ThreeLayerPyramid`](ThreeLayerPyramid/) | A 3×3 base, 2×2 middle tier, and pulsing apex. |
| 22 | [`MatrixDrift`](MatrixDrift/) | The intentional diagonal deep-green Matrix wash. |
| 23 | [`IntenseFire`](IntenseFire/) | High-energy orange-and-gold fire volume. |
| 24 | [`MagicalBlueFire`](MagicalBlueFire/) | Magical deep-blue and azure fire volume. |
| 25 | [`Explosions`](Explosions/) | Repeated expanding explosion shells with fade-off. |
| 26 | [`LaunchingFireworks`](LaunchingFireworks/) | Rockets launch upward and burst across the top. |
| 27 | [`PixelPasture`](PixelPasture/) | Green field, brown pixel cows, clouds, and a layer-five sun. |
| 28 | [`RedMatrixRain`](RedMatrixRain/) | Straight-down bright red heads with deep crimson fading trails. |

## Upload

Install FastLED, open one `.ino` file in its folder, select the ESP32-C3 board, and upload. The demo already contains the cube mapper and does not auto-cycle into another effect.
