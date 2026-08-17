from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent.parent
MASTER = ROOT / 'SkeletonCubePatterns' / 'SkeletonCubePatterns.ino'
PATTERNS = ROOT / 'patterns'
CUBEFX_SOURCE = ROOT / 'standalone' / 'CubeFXPatternDemos'
CONTROLLERS = ROOT / 'controllers'

LEGACY = [
    ('01_RedVectorCube', 'Red Vector Cube', 'renderRedVectorCube(ax, ay, az);', False, 'Rotating red 3×3×3 vector wireframe.'),
    ('02_SolidCube', 'Solid Cube', 'renderImplicitShape(ax, ay, az, 1);', False, 'Rotating colour-shifting solid cube.'),
    ('03_Sphere', 'Sphere', 'renderImplicitShape(ax, ay, az, 2);', False, 'Rotating colour-shifting solid sphere.'),
    ('04_Octahedron', 'Octahedron', 'renderImplicitShape(ax, ay, az, 3);', False, 'Rotating colour-shifting octahedron.'),
    ('05_VoxelModel', 'Voxel Model', 'renderVoxelModel(ax, ay, az);', False, 'Hand-authored spinning voxel model.'),
    ('06_BouncingBlock', 'Bouncing 2×2×2 Block', 'renderBouncingBlock();', False, 'Solid 2×2×2 cube bouncing within the volume.'),
    ('07_BlueRain', 'Blue Rain', 'renderRain();', True, 'Sparse blue rain falling downward through the cube.'),
    ('08_ColumnSweep', 'Column Sweep', 'renderColumnSweep();', False, 'Colour-shifting vertical x-column sweep.'),
    ('09_MatrixRain', 'Matrix Rain', 'renderMatrixRain();', True, 'Dense classic green Matrix-style rain.'),
    ('10_CornerCubes', 'Corner Cubes', 'renderCornerCubes();', False, 'Eight colour-shifting 2×2×2 corner cubes.'),
    ('11_WhiteGlitter', 'White Glitter', 'renderGlitter();', True, 'Random white voxel sparkle with trails.'),
    ('12_Pong', 'Single-player Pong', 'renderPong();', False, 'Player-controlled Pong: GPIO4 moves a full-height rear paddle left; GPIO8 moves it right.'),
    ('13_Tetris', '3-D Tetris', 'renderTetris();', False, 'Autoplaying falling polycubes in a 5×5×5 well.'),
    ('14_BlinkingEye', 'Blinking Eye', 'renderBlinkingEye();', False, 'A tracked 3-D eye that periodically blinks.'),
    ('15_DNAHelix', 'DNA Helix', 'renderDNAHelix();', False, 'Animated dual-strand DNA helix.'),
    ('16_Meteors', 'Meteors', 'renderMeteors();', False, 'Wrapped diagonal meteor stream.'),
    ('17_Snake', '3-D Snake', 'renderSnake();', False, 'Self-guided wrapping 3-D snake.'),
    ('18_SpaceInvaders', 'Space Invaders', 'renderInvaders();', False, 'Marching voxel invaders with a laser.'),
    ('19_ConwayLife', 'Conway 3-D Life', 'renderLife();', False, 'B5/S45 three-dimensional cellular automaton.'),
    ('20_Clouds', 'Cloud Volume', 'renderClouds();', False, 'Slow volumetric cloud field.'),
    ('21_NeonPlasma', 'Neon Plasma', 'renderPlasma();', False, 'High-energy 3-D neon plasma.'),
    ('22_Fire', 'Volume Fire', 'renderFire();', False, 'Rising 3-D fire volume.'),
    ('23_Hourglass', '3-D Hourglass', 'renderHourglass();', False, 'Cyan glass and shifting gold sand.'),
    ('24_PulseCube', 'Pulse Cube', 'renderPulseCube();', False, 'Expanding and contracting wire cube.'),
    ('25_UpwardRedRain', 'Upward Red Rain', 'renderUpwardRedRain();', True, 'Dense upward red rain with fast fade.'),
    ('26_Spirals', '3-D Spirals', 'renderSpirals();', False, 'Twin animated 3-D spirals.'),
    ('27_PointBouncer', 'Single-Point Bouncer', 'renderPointBouncer();', False, 'Single bright voxel ricocheting through the volume.'),
    ('28_WrappingComets', 'Wrapping Comets', 'renderWrappingComets();', False, 'Multiple 3-D comets wrapping over all axes.'),
]

CUBEFX_UNIQUE = [
    ('29_PerimeterBanner', 'Perimeter Banner', 'PerimeterBanner', '3×5 or 5×5 exterior text banner with CUBE 4 3 2 1 0 default.'),
    ('30_BulletWall', 'Bullet Wall', 'BulletWall', 'Red particles collide with one grey plate and throw impact sparks.'),
    ('31_PaddedCell', 'Padded Cell', 'PaddedCell', 'Red pixel ricochets inside a dim-grey 3×3×3 chamber.'),
    ('32_BlockRun', 'Block Run', 'BlockRun', 'A hopping red runner crosses a scrolling grass-and-block level.'),
    ('33_ParallaxStarfield', 'Parallax Starfield', 'ParallaxStarfield', 'Three star layers create animated depth.'),
    ('34_TrenchRun', 'Trench Run', 'TrenchRun', 'Blue-grey trench tunnel, reticle, and red target.'),
    ('35_RunningLegs', 'Running Legs', 'RunningLegs', 'A miniature voxel runner with alternating legs.'),
    ('36_FairiesInGreenBox', 'Fairies in Green Box', 'FairiesInGreenBox', 'Wing-pulsing fairies drift in a green wireframe enclosure.'),
    ('37_OrangeFishTank', 'Orange Fish Tank', 'OrangeFishTank', 'Blue-framed water volume with orange fish.'),
    ('38_ThreeLayerPyramid', 'Three-Layer Pyramid', 'ThreeLayerPyramid', 'A 3×3 base, 2×2 middle, and pulsing apex.'),
    ('39_MatrixDrift', 'Matrix Drift', 'MatrixDrift', 'Intentional diagonal deep-green Matrix wash.'),
    ('40_IntenseFire', 'Intense Fire', 'IntenseFire', 'High-energy orange-and-gold fire volume.'),
    ('41_MagicalBlueFire', 'Magical Blue Fire', 'MagicalBlueFire', 'Magical deep-blue and azure fire volume.'),
    ('42_Explosions', 'Explosions', 'Explosions', 'Repeated expanding explosion shells with fade-off.'),
    ('43_LaunchingFireworks', 'Launching Fireworks', 'LaunchingFireworks', 'Rockets launch upward and burst across the top.'),
    ('44_PixelPasture', 'Pixel Pasture', 'PixelPasture', 'Green field, brown pixel cows, clouds on layers 3–4, and a layer-five sun.'),
    ('45_RedMatrixRain', 'Red Matrix Rain', 'RedMatrixRain', 'Straight-down bright red heads with deep crimson fading trails.'),
    ('46_VoxelMinesweeper', 'Voxel Minesweeper', 'VoxelMinesweeper', 'Falling probes land on base targets and make orange 3×3×3 impact bursts.'),
    ('47_BigMoonStars', 'Big Moon & Stars', 'BigMoonStars', 'A large shaded moon surrounded by gently twinkling blue stars.'),
    ('48_NixieTube', 'Nixie Tube', 'NixieTube', 'Dull orange inactive tube segments and bright orange 0–9 numerals.'),
    ('49_BlackHoleVortex', 'Black Hole Vortex', 'BlackHoleVortex', 'Negative-space event horizon with violet, indigo, and cyan accretion ribbons.'),
    ('50_StargateDialUp', 'Stargate Dial-Up', 'StargateDialUp', 'Stone base, seven locking chevrons, and a blue portal opening sequence.'),
    ('51_Defender3D', '3-D Defender', 'Defender3D', 'Cyan player ship, advancing orange foes, and a bright laser exchange.'),
    ('52_Chequerboard3D', '3-D Chequerboard', 'Chequerboard3D', 'Negative-space chequer planes fold and transform through the cube.'),
    ('53_HellraiserPuzzleCube', 'Hellraiser Puzzle Cube', 'HellraiserPuzzleCube', 'Dark brass labyrinth cube with blue seams, amber glyphs, and mechanical phases.'),
    ('54_RubiksCube3D', '3-D Rubik\'s Cube', 'RubiksCube3D', 'Inset coloured cube with twisting scrambled layers and a solved reveal.'),
    ('55_LissajousRipple', 'Lissajous Layer Ripple', 'LissajousRipple', 'A 3-D Lissajous trace resting on layer 3, with crests rippling only into layers 2 and 4.'),
    ('56_ZarchVoxelDefender', 'Zarch: Voxel Defender', 'ZarchVoxelDefender', 'A compact voxel terrain-defence scene with a flying craft, landers, forward shots, and cached terrain.'),
]

# Preserve the two old controller examples in their own non-pattern location.
CONTROLLER_FOLDERS = ['29_ESP32C3_NextPatternButton', '30_ESP32C3_DualButtonController']
CONTROLLERS.mkdir(exist_ok=True)
for name in CONTROLLER_FOLDERS:
    source = PATTERNS / name
    destination = CONTROLLERS / name
    if source.exists() and not destination.exists():
        shutil.copytree(source, destination)

# Remove the old controller placement and the previous duplicated CubeFX entries.
for name in CONTROLLER_FOLDERS:
    shutil.rmtree(PATTERNS / name, ignore_errors=True)
for number in range(31, 61):
    for existing in PATTERNS.glob(f'{number:02d}_*'):
        shutil.rmtree(existing, ignore_errors=True)

master = MASTER.read_text(encoding='utf-8')
body = master[master.index('#include <FastLED.h>'):master.index('void setup() {')]
start_playback = body.index('// ---------- Pattern playback ----------')
start_mapper = body.index('// ---------- Coordinate mapper ----------')
body = body[:start_playback] + body[start_mapper:]
body = body.replace('#include "CubeTypes.h"\n', '')
body = body.replace('#include <FastLED.h>', '#include <FastLED.h>\n#include <esp_system.h>\n#include "DemoTypes.h"')

DEMO_TYPES = '''#pragma once
// Kept in this sketch folder so Arduino sees Vec3 before auto-generated prototypes.
struct Vec3 {
  float x;
  float y;
  float z;
};
'''

for folder, label, renderer, keeps_framebuffer, description in LEGACY:
    destination = PATTERNS / folder
    destination.mkdir(parents=True, exist_ok=True)
    sketch = destination / f'{folder}.ino'
    if folder == '12_Pong':
        source = CUBEFX_SOURCE / 'SinglePlayerPong' / 'SinglePlayerPong.ino'
        text = source.read_text(encoding='utf-8').replace('SinglePlayerPong.ino', f'{folder}.ino', 1)
        sketch.write_text(text, encoding='utf-8')
        readme = f'''# {folder} — {label}

{description}

This is a **fully standalone, directly uploadable** Arduino FastLED sketch. Short
GPIO4 presses move the full-height player paddle left; short GPIO8 presses move it
right. The fixed computer paddle is on the opposite face.
'''
        (destination / 'README.md').write_text(readme, encoding='utf-8')
        continue
    header = f'''/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  {folder}.ino — fully standalone SkeletonCube pattern demo
  Created by Dad (MysterEon) & Manus, 2026.

  Pattern: {description}

  This is a complete direct-upload Arduino sketch: no local includes, no master
  sketch dependency, and no web-controller dependency.

  Cube map: origin (0,0,0) = bottom-rear-left.
  x: left -> right; y: rear -> front; z: bottom -> top.
  Default map: index = z * 25 + y * 5 + x.
*/

'''
    runtime = f'''
void setup() {{
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  randomSeed(esp_random());
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}}

void loop() {{
  const float t = millis() * 0.001f;
  const float ax = 0.83f * t;
  const float ay = 1.17f * t;
  const float az = 0.41f * t;
  if ({str(not keeps_framebuffer).lower()}) fill_solid(leds, NUM_LEDS, CRGB::Black);
  {renderer}
  FastLED.show();
}}
'''
    sketch.write_text(header + body + runtime, encoding='utf-8')
    (destination / 'DemoTypes.h').write_text(DEMO_TYPES, encoding='utf-8')
    readme = f'''# {folder} — {label}

{description}

This is a **fully standalone, directly uploadable** Arduino FastLED sketch. It
contains its own 5×5×5 mapper, all pattern code, `setup()`, and `loop()`. The
only companion file is `DemoTypes.h` in the same folder, used solely so Arduino
sees the shared `Vec3` declaration before generating prototypes; there is no
master-library or outside-project dependency.

## Upload

Open `{folder}.ino` in Arduino IDE and upload to the ESP32-C3. Defaults: 125 GRB
WS2812B LEDs on data pin 2, brightness 100, bottom–rear–left origin, and
`index = z * 25 + y * 5 + x`.
'''
    (destination / 'README.md').write_text(readme, encoding='utf-8')

for folder, label, source_name, description in CUBEFX_UNIQUE:
    destination = PATTERNS / folder
    destination.mkdir(parents=True, exist_ok=True)
    source = CUBEFX_SOURCE / source_name / f'{source_name}.ino'
    text = source.read_text(encoding='utf-8')
    text = text.replace(f'{source_name}.ino — fully standalone', f'{folder}.ino — fully standalone', 1)
    (destination / f'{folder}.ino').write_text(text, encoding='utf-8')
    readme = f'''# {folder} — {label}

{description}

This is a **fully standalone, directly uploadable** Arduino FastLED sketch. It
contains its own 5×5×5 mapper, all supporting code, `setup()`, and `loop()`.
There is no browser-controller, master-sketch, or local-header dependency.

## Upload

Open `{folder}.ino` in Arduino IDE and upload to the ESP32-C3. Defaults: 125 GRB
WS2812B LEDs on data pin 2, brightness 100, bottom–rear–left origin, and
`index = z * 25 + y * 5 + x`.
'''
    (destination / 'README.md').write_text(readme, encoding='utf-8')

catalog = [(int(item[0][:2]), item[0], item[1], item[4]) for item in LEGACY]
catalog += [(int(item[0][:2]), item[0], item[1], item[3]) for item in CUBEFX_UNIQUE]
catalog.sort()

lines = [
    '# Standalone pattern library',
    '',
    'This folder now contains the canonical **56 distinct visual effects**. Every numbered pattern folder is a complete, directly uploadable Arduino project with a matching `.ino` file. No pattern here depends on a relative include, the web controller, or the master pattern sketch.',
    '',
    '| # | Pattern | Description |',
    '|---:|---|---|',
]
for number, folder, label, description in catalog:
    lines.append(f'| {number:02d} | [`{label}`]({folder}/) | {description} |')
lines.extend([
    '',
    '## Controller examples',
    '',
    'The ESP32-C3 button-controller examples are retained outside the visual-pattern sequence under [`controllers/`](../controllers/), because they are hardware control sketches rather than animation patterns.',
])
(PATTERNS / 'README.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')

controller_lines = [
    '# ESP32-C3 controller examples',
    '',
    'These sketches configure button control for the pattern library. They are kept outside `patterns/` so the numbered collection remains a pure visual-effect catalog.',
    '',
    '| Folder | Purpose |',
    '|---|---|',
    '| [`29_ESP32C3_NextPatternButton`](29_ESP32C3_NextPatternButton/) | GPIO3 next-pattern button controller. |',
    '| [`30_ESP32C3_DualButtonController`](30_ESP32C3_DualButtonController/) | GPIO4 next-pattern plus GPIO8 auto/manual controller. |',
]
(CONTROLLERS / 'README.md').write_text('\n'.join(controller_lines) + '\n', encoding='utf-8')

print('Built canonical standalone pattern library: 56 effects in patterns/; controllers preserved in controllers.')
