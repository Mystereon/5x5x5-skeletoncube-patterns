#!/usr/bin/env python3
"""Verify the structural and visual constraints of CubeFX Race Circuit."""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FIRMWARE = (ROOT / "standalone/CubeFXWeb/CubeFXWeb.ino").read_text(encoding="utf-8")
WEB = (ROOT / "standalone/CubeFXWeb/CubeFXWebPage.h").read_text(encoding="utf-8")
CATALOG = (ROOT / "apps/CubeFXPhone/app/src/main/java/com/mystereon/cubefxcontroller/CubeFxPatternCatalog.kt").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    require("constexpr uint16_t RACE_WORLD_SIZE = 300;" in FIRMWARE, "Race Circuit is no longer a 300×300 world")
    require("constexpr uint16_t RACE_TOTAL_UNITS = RACE_LAP_UNITS * 3;" in FIRMWARE, "Race Circuit is no longer a three-lap race")
    require("PATTERN_RACE_CIRCUIT" in FIRMWARE, "Race Circuit renderer is not registered")
    require("case 74: currentPattern = PATTERN_RACE_CIRCUIT;" in FIRMWARE, "Race Circuit BLE ID 74 is missing")
    require("if (z == 4)" in FIRMWARE and "raceCloudValue" in FIRMWARE, "Cloud ceiling is not isolated to the top layer")
    require("return height;" in FIRMWARE and "height = max<uint8_t>(height, 2);" in FIRMWARE, "Terrain is no longer capped at three physical layers")
    require("if (tree && z <= 2)" in FIRMWARE, "Trees are no longer capped at three physical layers")
    require("RACE_SANDBAGS" in FIRMWARE and "RACE_SPECTATORS" in FIRMWARE, "Trackside sandbags or two-pixel spectators are missing")
    require("renderRaceWinnerBanner" in FIRMWARE and "RACE_WIN_MESSAGES" in FIRMWARE, "Winning-colour perimeter banner is missing")
    require("if (primary) resetRaceCircuit();" in FIRMWARE and "raceViewVariant = (raceViewVariant + 1) % 3;" in FIRMWARE, "Race Circuit button controls are missing")
    require("'Race Circuit'" in WEB and 'size="57"' in WEB, "Browser gallery does not expose Race Circuit")
    require("Race Circuit — leader-following three-lap viewport" in CATALOG and "(29..74)" in CATALOG, "Android catalogue does not expose Race Circuit as BLE 74")

    # The authored path must stay inside the declared virtual map bounds.
    path = re.search(r"const RacePathPoint RACE_PATH\[RACE_PATH_POINTS\] = \{(.*?)\n\};", FIRMWARE, re.DOTALL)
    require(path is not None, "Race Circuit path points were not found")
    points = [(int(x), int(y)) for x, y in re.findall(r"\{(\d+),(\d+)\}", path.group(1))]
    require(len(points) == 24, "Race Circuit path must retain 24 authored corners")
    require(all(0 <= x < 300 and 0 <= y < 300 for x, y in points), "Race Circuit path leaves the 300×300 map")

    print("Race Circuit scene: 300×300 streamed map, 24-point original path, 3 laps, low scenery, cloud roof, winner banner, browser mode 57 / BLE 74 — OK")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"Race Circuit scene: FAILED — {error}", file=sys.stderr)
        raise SystemExit(1)
