# Standalone pattern library

This folder now contains the canonical **48 distinct visual effects**. Every numbered pattern folder is a complete, directly uploadable Arduino project with a matching `.ino` file. No pattern here depends on a relative include, the web controller, or the master pattern sketch.

| # | Pattern | Description |
|---:|---|---|
| 01 | [`Red Vector Cube`](01_RedVectorCube/) | Rotating red 3×3×3 vector wireframe. |
| 02 | [`Solid Cube`](02_SolidCube/) | Rotating colour-shifting solid cube. |
| 03 | [`Sphere`](03_Sphere/) | Rotating colour-shifting solid sphere. |
| 04 | [`Octahedron`](04_Octahedron/) | Rotating colour-shifting octahedron. |
| 05 | [`Voxel Model`](05_VoxelModel/) | Hand-authored spinning voxel model. |
| 06 | [`Bouncing 2×2×2 Block`](06_BouncingBlock/) | Solid 2×2×2 cube bouncing within the volume. |
| 07 | [`Blue Rain`](07_BlueRain/) | Sparse blue rain falling downward through the cube. |
| 08 | [`Column Sweep`](08_ColumnSweep/) | Colour-shifting vertical x-column sweep. |
| 09 | [`Matrix Rain`](09_MatrixRain/) | Dense classic green Matrix-style rain. |
| 10 | [`Corner Cubes`](10_CornerCubes/) | Eight colour-shifting 2×2×2 corner cubes. |
| 11 | [`White Glitter`](11_WhiteGlitter/) | Random white voxel sparkle with trails. |
| 12 | [`Single-player Pong`](12_Pong/) | Player-controlled Pong: GPIO4 moves a full-height rear paddle left; GPIO8 moves it right. |
| 13 | [`3-D Tetris`](13_Tetris/) | Autoplaying falling polycubes in a 5×5×5 well. |
| 14 | [`Blinking Eye`](14_BlinkingEye/) | A tracked 3-D eye that periodically blinks. |
| 15 | [`DNA Helix`](15_DNAHelix/) | Animated dual-strand DNA helix. |
| 16 | [`Meteors`](16_Meteors/) | Wrapped diagonal meteor stream. |
| 17 | [`3-D Snake`](17_Snake/) | Self-guided wrapping 3-D snake. |
| 18 | [`Space Invaders`](18_SpaceInvaders/) | Marching voxel invaders with a laser. |
| 19 | [`Conway 3-D Life`](19_ConwayLife/) | B5/S45 three-dimensional cellular automaton. |
| 20 | [`Cloud Volume`](20_Clouds/) | Slow volumetric cloud field. |
| 21 | [`Neon Plasma`](21_NeonPlasma/) | High-energy 3-D neon plasma. |
| 22 | [`Volume Fire`](22_Fire/) | Rising 3-D fire volume. |
| 23 | [`3-D Hourglass`](23_Hourglass/) | Cyan glass and shifting gold sand. |
| 24 | [`Pulse Cube`](24_PulseCube/) | Expanding and contracting wire cube. |
| 25 | [`Upward Red Rain`](25_UpwardRedRain/) | Dense upward red rain with fast fade. |
| 26 | [`3-D Spirals`](26_Spirals/) | Twin animated 3-D spirals. |
| 27 | [`Single-Point Bouncer`](27_PointBouncer/) | Single bright voxel ricocheting through the volume. |
| 28 | [`Wrapping Comets`](28_WrappingComets/) | Multiple 3-D comets wrapping over all axes. |
| 29 | [`Perimeter Banner`](29_PerimeterBanner/) | 3×5 or 5×5 exterior text banner with CUBE 4 3 2 1 0 default. |
| 30 | [`Bullet Wall`](30_BulletWall/) | Red particles collide with one grey plate and throw impact sparks. |
| 31 | [`Padded Cell`](31_PaddedCell/) | Red pixel ricochets inside a dim-grey 3×3×3 chamber. |
| 32 | [`Block Run`](32_BlockRun/) | A hopping red runner crosses a scrolling grass-and-block level. |
| 33 | [`Parallax Starfield`](33_ParallaxStarfield/) | Three star layers create animated depth. |
| 34 | [`Trench Run`](34_TrenchRun/) | Blue-grey trench tunnel, reticle, and red target. |
| 35 | [`Running Legs`](35_RunningLegs/) | A miniature voxel runner with alternating legs. |
| 36 | [`Fairies in Green Box`](36_FairiesInGreenBox/) | Wing-pulsing fairies drift in a green wireframe enclosure. |
| 37 | [`Orange Fish Tank`](37_OrangeFishTank/) | Blue-framed water volume with orange fish. |
| 38 | [`Three-Layer Pyramid`](38_ThreeLayerPyramid/) | A 3×3 base, 2×2 middle, and pulsing apex. |
| 39 | [`Matrix Drift`](39_MatrixDrift/) | Intentional diagonal deep-green Matrix wash. |
| 40 | [`Intense Fire`](40_IntenseFire/) | High-energy orange-and-gold fire volume. |
| 41 | [`Magical Blue Fire`](41_MagicalBlueFire/) | Magical deep-blue and azure fire volume. |
| 42 | [`Explosions`](42_Explosions/) | Repeated expanding explosion shells with fade-off. |
| 43 | [`Launching Fireworks`](43_LaunchingFireworks/) | Rockets launch upward and burst across the top. |
| 44 | [`Pixel Pasture`](44_PixelPasture/) | Green field, brown pixel cows, clouds on layers 3–4, and a layer-five sun. |
| 45 | [`Red Matrix Rain`](45_RedMatrixRain/) | Straight-down bright red heads with deep crimson fading trails. |
| 46 | [`Voxel Minesweeper`](46_VoxelMinesweeper/) | Falling probes land on base targets and make orange 3×3×3 impact bursts. |
| 47 | [`Big Moon & Stars`](47_BigMoonStars/) | A large shaded moon surrounded by gently twinkling blue stars. |
| 48 | [`Nixie Tube`](48_NixieTube/) | Dull orange inactive tube segments and bright orange 0–9 numerals. |

## Controller examples

The ESP32-C3 button-controller examples are retained outside the visual-pattern sequence under [`controllers/`](../controllers/), because they are hardware control sketches rather than animation patterns.
