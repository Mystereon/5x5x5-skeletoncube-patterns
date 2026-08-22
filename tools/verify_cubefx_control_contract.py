#!/usr/bin/env python3
"""Fail fast when a CubeFX controller surface drifts from the ESP32-S3 contract.

This intentionally checks source-level command coverage rather than attempting to
mock BLE or an Arduino WebServer. It is fast enough to run before every firmware
and APK build and catches the historical failure mode: visible controls that do
not reach a recognised firmware command.
"""

from __future__ import annotations

import re
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
FIRMWARE = (ROOT / "standalone/CubeFXWeb/CubeFXWeb.ino").read_text(encoding="utf-8")
WEB = (ROOT / "standalone/CubeFXWeb/CubeFXWebPage.h").read_text(encoding="utf-8")
CATALOG = (ROOT / "apps/CubeFXPhone/app/src/main/java/com/mystereon/cubefxcontroller/CubeFxPatternCatalog.kt").read_text(encoding="utf-8")
PHONE = (ROOT / "apps/CubeFXPhone/app/src/main/java/com/mystereon/cubefxcontroller/MainActivity.kt").read_text(encoding="utf-8")
BLE = (ROOT / "apps/CubeFXPhone/app/src/main/java/com/mystereon/cubefxcontroller/CubeFxBleClient.kt").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def quoted_names(source: str) -> list[str]:
    match = re.search(r"const names=\[(.*?)\];", source, flags=re.DOTALL)
    require(match is not None, "Web pattern-name array was not found")
    return re.findall(r"'(?:\\'|[^'])*'", match.group(1))


def main() -> int:
    # Web routing: one explicit button route and one selection-change route.
    require('if (web.hasArg("pattern"))' in FIRMWARE, "Firmware no longer accepts web pattern commands")
    require('if (web.hasArg("secret"))' in FIRMWARE, "Firmware no longer accepts web secret commands")
    require("function selectPattern()" in WEB, "Web gallery has no explicit Play Selected route")
    require("$('pattern').onchange=()=>selectPattern()" in WEB, "Web gallery selection is not wired to its play route")
    require("PLAY SELECTED PATTERN" in WEB, "Web gallery has no direct replay button")
    require(len(quoted_names(WEB)) == 57, "Web gallery no longer exposes all 57 CubeFXWeb patterns")
    for scene in range(5):
        require(f"triggerSecret({scene}," in WEB, f"Web deck is missing secret scene {scene}")

    # The source is embedded inside a C++ raw string, so validate the exact
    # browser JavaScript payload separately as well as checking its routes.
    script = re.search(r"<script>(.*?)</script>", WEB, flags=re.DOTALL)
    require(script is not None, "Embedded browser JavaScript was not found")
    with tempfile.NamedTemporaryFile(suffix=".js", mode="w", encoding="utf-8") as temporary:
        temporary.write(script.group(1))
        temporary.flush()
        result = subprocess.run(["node", "--check", temporary.name], capture_output=True, text=True, check=False)
    require(result.returncode == 0, f"Embedded browser JavaScript has a syntax error: {result.stderr.strip()}")

    # Android routing: gallery only presents real direct-play controls as buttons;
    # standalone sketches remain labelled references rather than dead controls.
    require("CubeFxPatternCatalog.embeddedModes" in PHONE, "Android gallery does not use the embedded mode subset")
    require("cubeFx.sendPattern(pattern.id)" in PHONE, "Android embedded play button has no BLE pattern route")
    require("CubeFxPatternCatalog.standaloneModes" in PHONE, "Android gallery does not distinguish standalone sketches")
    require("if (pattern.embedded) cubeFx.sendPattern(pattern.id)" not in PHONE, "Android still has silent conditional catalogue rows")
    for scene in range(5):
        require(f"cubeFx.sendSecretScene({scene})" in PHONE, f"Android deck is missing secret scene {scene}")

    # BLE routing: direct pattern, secret, and acknowledgement IDs must agree.
    canonical_cases = {int(value) for value in re.findall(r"case (\d+): currentPattern = PATTERN_", FIRMWARE)}
    expected_cases = {1, 9, 10, 11, 12, 19, 20, 21, 22, 26, 28, *range(29, 75)}
    require(canonical_cases == expected_cases, "Firmware canonical BLE selector drifted from the 57 embedded-mode contract")
    for uuid in (
        "6c75a300-7b1d-4f29-a221-000000000001",
        "6c75a300-7b1d-4f29-a221-000000000002",
        "6c75a300-7b1d-4f29-a221-000000000003",
    ):
        require(uuid in FIRMWARE and uuid in BLE, f"BLE UUID {uuid} is not shared by firmware and Android")
    require("enableStatusNotifications" in BLE and "handleStatus" in BLE, "Android has no firmware acknowledgement path")

    print("CubeFX control contract: 57 web modes, 57 Android BLE modes, 5 web eggs, 5 Android eggs, shared BLE acknowledgements — OK")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"CubeFX control contract: FAILED — {error}", file=sys.stderr)
        raise SystemExit(1)
