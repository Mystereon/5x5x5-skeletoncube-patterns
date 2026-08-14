#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PATTERNS="$ROOT/patterns"
rm -rf "$PATTERNS"
mkdir -p "$PATTERNS"

create_example() {
  local number="$1"
  local folder="$2"
  local enum_name="$3"
  local description="$4"
  local path="$PATTERNS/${number}_${folder}"
  mkdir -p "$path"

  cat > "$path/${number}_${folder}.ino" <<EOF
/*
  FEED ME , I'M POOR AND I MADE THIS FOR FREE - https://paypal.me/Mystereon

  SkeletonCube Patterns — ${description}
  Created by Dad (MysterEon) & Manus, 2026.

  This is a standalone reference sketch. It fixes the shared master library to
  one effect, so it can be uploaded, modified, and studied independently.
*/

#define SKELETONCUBE_AUTO_CYCLE 0
#define SKELETONCUBE_FIXED_PATTERN ${enum_name}
#include "../../SkeletonCubePatterns/SkeletonCubePatterns.ino"
EOF

  cat > "$path/README.md" <<EOF
# ${number}: ${folder}

${description}

This example is a lightweight fixed-pattern wrapper around the shared master sketch. Keep its relative folder position intact, then open \
\`${number}_${folder}.ino\` in Arduino IDE. To alter the effect, change the matching \
\`render...()\` function in \
[../../SkeletonCubePatterns/SkeletonCubePatterns.ino](../../SkeletonCubePatterns/SkeletonCubePatterns.ino).
EOF
}

create_example 01 RedVectorCube       PATTERN_WIREFRAME_CUBE  "Rotating red 3×3×3 vector wireframe cube."
create_example 02 SolidCube           PATTERN_SOLID_CUBE      "Tumbling analytic solid cube."
create_example 03 Sphere              PATTERN_SPHERE          "Tumbling analytic voxel sphere."
create_example 04 Octahedron          PATTERN_OCTAHEDRON      "Tumbling analytic octahedron."
create_example 05 VoxelModel          PATTERN_VOXEL_MODEL     "Rotating hand-authored voxel model."
create_example 06 BouncingBlock       PATTERN_BOUNCING_BLOCK  "Independently timed 2×2×2 bouncing block."
create_example 07 BlueRain            PATTERN_RAIN            "Sparse blue rain with fading trails."
create_example 08 ColumnSweep         PATTERN_COLUMN_SWEEP    "Rainbow vertical x-column sweep."
create_example 09 MatrixRain          PATTERN_MATRIX_RAIN     "Dense green 3-D Matrix rain."
create_example 10 CornerCubes         PATTERN_CORNER_CUBES    "Eight colour-shifting 2×2×2 corner cubes."
create_example 11 WhiteGlitter        PATTERN_GLITTER         "Fast-fading random white voxel glitter."
create_example 12 Pong                PATTERN_PONG            "Self-playing 3-D Pong with tracking paddles."
create_example 13 Tetris              PATTERN_TETRIS          "Autoplay falling 3-D polycube Tetris."
create_example 14 BlinkingEye         PATTERN_BLINKING_EYE    "3-D blinking eye with moving pupil."
create_example 15 DNAHelix            PATTERN_DNA_HELIX       "Twin-strand volumetric DNA helix."
create_example 16 Meteors             PATTERN_METEORS         "Wrapped diagonal meteor stream."
create_example 17 Snake               PATTERN_SNAKE           "Autonomous 3-D wrapping snake."
create_example 18 SpaceInvaders       PATTERN_INVADERS        "Marching 3-D Space Invaders scene."
create_example 19 ConwayLife          PATTERN_LIFE            "3-D Conway-style cellular automaton."
create_example 20 Clouds              PATTERN_CLOUDS          "Slow volumetric cloud field."
create_example 21 NeonPlasma          PATTERN_PLASMA          "Animated 3-D neon plasma field."
create_example 22 Fire                PATTERN_FIRE            "Rising volumetric 3-D fire."
create_example 23 Hourglass           PATTERN_HOURGLASS       "Animated 3-D hourglass with falling sand."
create_example 24 PulseCube           PATTERN_PULSE_CUBE      "Reducing and expanding ping-pong wire cube."
create_example 25 UpwardRedRain       PATTERN_UPWARD_RED_RAIN "Dense upward red rain with a fast fade."
create_example 26 Spirals             PATTERN_SPIRALS         "Twin rotating 3-D spirals."
create_example 27 PointBouncer        PATTERN_POINT_BOUNCER   "Single-point 3-D bouncer."
create_example 28 WrappingComets      PATTERN_WRAPPING_COMETS "Multiple wrapping 3-D comets."

cat > "$PATTERNS/README.md" <<'EOF'
# Pattern reference sketches

Every folder here exposes one effect as an individual Arduino sketch. Each is a
small wrapper that disables automatic cycling, selects exactly one `Pattern`,
and includes the shared `SkeletonCubePatterns.ino` source. This avoids 28
copied implementations drifting apart while still providing direct, uploadable
pattern entry points for exploration and reference.

Download or clone the whole repository before opening an example, because the
wrapper uses a relative include path to the shared master sketch.
EOF
