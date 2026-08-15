from pathlib import Path
import re

ROOT = Path('/home/ubuntu/skeletoncube-patterns')
SOURCE = ROOT / 'standalone' / 'CubeFXWeb' / 'CubeFXWeb.ino'
DESTINATION = ROOT / 'standalone' / 'CubeFXPatternDemos'

DEMOS = [
    ('01_RedVectorCube', 'RedVectorCube', 'PATTERN_VECTOR_CUBE', 'Rotating red 3×3×3 vector wireframe.'),
    ('02_MatrixRain', 'MatrixRain', 'PATTERN_MATRIX_RAIN', 'Straight-down deep emerald Matrix streams with yellow-green heads.'),
    ('03_NeonPlasma', 'NeonPlasma', 'PATTERN_PLASMA', 'Saturated trigonometric voxel plasma.'),
    ('04_VolumeFire', 'VolumeFire', 'PATTERN_FIRE', 'Rising volumetric noise-driven fire.'),
    ('05_TwinSpirals', 'TwinSpirals', 'PATTERN_SPIRALS', 'Opposing animated cyan and magenta spirals.'),
    ('06_WrappingComets', 'WrappingComets', 'PATTERN_COMETS', 'Three wrapped diagonal comets with fading tails.'),
    ('07_SelfPlayingPong', 'SelfPlayingPong', 'PATTERN_PONG', 'A single ball with opposing voxel paddles.'),
    ('08_Conway3DLife', 'Conway3DLife', 'PATTERN_LIFE', 'B5/S45 three-dimensional cellular automaton.'),
    ('09_CloudVolume', 'CloudVolume', 'PATTERN_CLOUDS', 'Slow cyan volumetric noise clouds.'),
    ('10_WhiteGlitter', 'WhiteGlitter', 'PATTERN_GLITTER', 'Fast-fading random voxel sparkle.'),
    ('11_CornerCubes', 'CornerCubes', 'PATTERN_CORNER_CUBES', 'Eight colour-shifting 2×2×2 corner cubes.'),
    ('12_PerimeterBanner', 'PerimeterBanner', 'PATTERN_BANNER', '3×5 or 5×5 exterior text banner, defaulting to CUBE 4 3 2 1 0.'),
    ('13_BulletWall', 'BulletWall', 'PATTERN_BULLET_WALL', 'Red particles colliding with a single grey plate.'),
    ('14_PaddedCell', 'PaddedCell', 'PATTERN_PADDED_CELL', 'A red pixel bouncing inside a dim-grey 3×3×3 padded chamber.'),
    ('15_BlockRun', 'BlockRun', 'PATTERN_BLOCK_RUN', 'A hopping red runner crossing a scrolling grass-and-block course.'),
    ('16_ParallaxStarfield', 'ParallaxStarfield', 'PATTERN_PARALLAX', 'Three depth layers of independently moving stars.'),
    ('17_TrenchRun', 'TrenchRun', 'PATTERN_TRENCH_RUN', 'Blue-grey tunnel, red target, and white pilot reticle.'),
    ('18_RunningLegs', 'RunningLegs', 'PATTERN_RUNNING_LEGS', 'Alternating voxel knees and wide-stepping feet.'),
    ('19_FairiesInGreenBox', 'FairiesInGreenBox', 'PATTERN_FAIRY_BOX', 'Drifting wing-pulsing fairy lights in a green wireframe enclosure.'),
    ('20_OrangeFishTank', 'OrangeFishTank', 'PATTERN_AQUARIUM', 'Blue-framed water volume with two orange fish.'),
    ('21_ThreeLayerPyramid', 'ThreeLayerPyramid', 'PATTERN_PYRAMID', 'A 3×3 base, 2×2 middle tier, and pulsing apex.'),
    ('22_MatrixDrift', 'MatrixDrift', 'PATTERN_MATRIX_DRIFT', 'The intentional diagonal deep-green Matrix wash.'),
    ('23_IntenseFire', 'IntenseFire', 'PATTERN_INTENSE_FIRE', 'High-energy orange-and-gold fire volume.'),
    ('24_MagicalBlueFire', 'MagicalBlueFire', 'PATTERN_BLUE_FIRE', 'Magical deep-blue and azure fire volume.'),
    ('25_Explosions', 'Explosions', 'PATTERN_EXPLOSIONS', 'Repeated expanding explosion shells with fade-off.'),
    ('26_LaunchingFireworks', 'LaunchingFireworks', 'PATTERN_FIREWORKS', 'Rockets launch upward and burst across the top.'),
    ('27_PixelPasture', 'PixelPasture', 'PATTERN_PIXEL_PASTURE', 'Green field, brown pixel cows, clouds, and a layer-five sun.'),
    ('28_RedMatrixRain', 'RedMatrixRain', 'PATTERN_RED_MATRIX_RAIN', 'Straight-down bright red heads with deep crimson fading trails.'),
]

source = SOURCE.read_text(encoding='utf-8')
fastled_start = source.index('#include <FastLED.h>')
browser_start = source.index('// -----------------------------------------------------------------------------\n// Browser API and UI')
body = source[fastled_start:browser_start]

wifi_start = body.index('// -----------------------------------------------------------------------------\n// Wi-Fi setup')
cube_start = body.index('// -----------------------------------------------------------------------------\n// Cube configuration')
body = body[:wifi_start] + body[cube_start:]
body = body.replace('Pattern currentPattern = PATTERN_VECTOR_CUBE;', 'Pattern currentPattern = PATTERN_VECTOR_CUBE;')
body = body.replace('bool autoCycle = true;', 'bool autoCycle = false; // Standalone demos never auto-cycle.')

for folder, sketch_name, pattern, description in DEMOS:
    # Arduino IDE expects a sketch folder and its primary .ino file to share a name.
    sketch_dir = DESTINATION / sketch_name
    sketch_dir.mkdir(parents=True, exist_ok=True)
    demo_body = body.replace('Pattern currentPattern = PATTERN_VECTOR_CUBE;', f'Pattern currentPattern = {pattern};')
    header = f'''/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  {sketch_name}.ino — fully standalone SkeletonCube / CubeFX pattern demo
  Created by Dad (MysterEon) & Manus, 2026.

  Pattern: {description}

  This folder is directly uploadable. It has no local includes and no dependency
  on CubeFXWeb, the browser controller, or the master sketch.

  Cube map: origin (0,0,0) is bottom-rear-left.
  x: left -> right; y: rear -> front; z: bottom -> top.
  Default physical map: index = z * 25 + y * 5 + x.

  Hardware defaults: ESP32-C3, 125 GRB WS2812B LEDs, DATA_PIN 2, brightness 100.
  Change the configuration constants below only if your physical wiring differs.
*/

'''
    runtime = '''
void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  randomSeed(esp_random());
  seedLife(); // Harmless for non-Life effects; necessary for the Life demo.
  patternStartedAt = millis();
}

void loop() {
  const uint32_t now = millis();
  if (now - lastFrameAt >= frameIntervalMs()) {
    lastFrameAt = now;
    renderCurrentPattern();
    FastLED.show();
  }
}
'''
    (sketch_dir / f'{sketch_name}.ino').write_text(header + demo_body + runtime, encoding='utf-8')

lines = [
    '# CubeFXPatternDemos — Actual Standalone Arduino Sketches',
    '',
    'Each folder below contains a **complete, directly uploadable** Arduino `.ino` sketch. Every demo carries its own FastLED setup, 5×5×5 mapper, supporting functions, and selected pattern implementation. There are no relative includes and no requirement to copy CubeFXWeb or the master sketch alongside it.',
    '',
    'All demos default to the established hardware map: 125 GRB WS2812B LEDs, `DATA_PIN 2`, brightness 100, and the bottom–rear–left origin with `index = z * 25 + y * 5 + x`.',
    '',
    '| # | Folder | Pattern |',
    '|---:|---|---|',
]
for number, (_, sketch_name, _, description) in enumerate(DEMOS, start=1):
    lines.append(f'| {number:02d} | [`{sketch_name}`]({sketch_name}/) | {description} |')
lines.extend([
    '',
    '## Upload',
    '',
    'Install FastLED, open one `.ino` file in its folder, select the ESP32-C3 board, and upload. The demo already contains the cube mapper and does not auto-cycle into another effect.',
])
(DESTINATION / 'README.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
print(f'Generated {len(DEMOS)} fully standalone CubeFX demos in {DESTINATION}')
