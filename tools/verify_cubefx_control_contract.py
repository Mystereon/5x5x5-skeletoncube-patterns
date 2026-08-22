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
USER_PATTERN_TYPES = (ROOT / "standalone/CubeFXWeb/UserPatternTypes.h").read_text(encoding="utf-8")
FASTLED_INTERPRETATION_TYPES = (ROOT / "standalone/CubeFXWeb/FastLEDInterpretationTypes.h").read_text(encoding="utf-8")
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


def quoted_fastled_sources(source: str) -> list[str]:
    match = re.search(r"const fastledSources=\[(.*?)\];", source, flags=re.DOTALL)
    require(match is not None, "Web FastLED interpretation source array was not found")
    return re.findall(r"'(?:\\'|[^'])*'", match.group(1))


def double_quoted_block(source: str, expression: str, label: str) -> list[str]:
    match = re.search(expression, source, flags=re.DOTALL)
    require(match is not None, f"{label} source array was not found")
    return re.findall(r'"([^"]+)"', match.group(1))


def main() -> int:
    # Web routing: one explicit button route and one selection-change route.
    require('if (web.hasArg("pattern"))' in FIRMWARE, "Firmware no longer accepts web pattern commands")
    require('if (web.hasArg("secret"))' in FIRMWARE, "Firmware no longer accepts web secret commands")
    require("function selectPattern()" in WEB, "Web gallery has no explicit Play Selected route")
    require("$('pattern').onchange=()=>selectPattern()" in WEB, "Web gallery selection is not wired to its play route")
    require("PLAY SELECTED PATTERN" in WEB, "Web gallery has no direct replay button")
    require(len(quoted_names(WEB)) == 62, "Web base gallery no longer exposes 57 native modes plus five user slots")
    require(len(quoted_fastled_sources(WEB)) == 83, "Web gallery no longer exposes all 83 FastLED interpretation sources")
    require("names.push(...fastledSources.map" in WEB, "Web gallery does not append the FastLED interpretation collection")
    for scene in range(5):
        require(f"triggerSecret({scene}," in WEB, f"Web deck is missing secret scene {scene}")

    # User authoring: five independently playable slots, each with sixteen
    # masked steps, must remain visible and able to reach the bounded firmware API.
    require("USER_PATTERN_SLOT_COUNT = 5" in USER_PATTERN_TYPES, "Firmware no longer defines five user slots")
    require("USER_PATTERN_STEP_COUNT = 16" in USER_PATTERN_TYPES, "Firmware no longer defines sixteen user steps")
    require("decodeUserPatternMask" in FIRMWARE and "userSave" in FIRMWARE and "userPlay" in FIRMWARE, "Firmware user-pattern upload route is incomplete")
    require("FIVE-SLOT / 16-STEP USER SEQUENCER" in WEB, "Web user sequencer tab is missing")
    require("function sendUserSlot()" in WEB and "function playUserSlot()" in WEB, "Web user sequencer cannot send and play slots")
    require("layerDeck" in WEB and "LAYER z=" in WEB, "Web user sequencer is missing its five layer-panel deck")
    for slot in range(1, 6):
        require(f"User Pattern 0{slot}" in WEB, f"Web gallery is missing User Pattern {slot:02d}")

    # One-to-one interpretation coverage: original FastLED source stays pure in
    # the submodule while each source directory obtains a separate CubeFX mode.
    require("FASTLED_INTERPRETATION_COUNT = 83" in FASTLED_INTERPRETATION_TYPES, "Firmware no longer defines 83 FastLED interpretations")
    require("FASTLED_INTERPRETATION_CANONICAL_FIRST = 80" in FASTLED_INTERPRETATION_TYPES, "FastLED interpretation BLE range no longer starts at 80")
    require("third_party/FastLED" in (ROOT / "FASTLED_COLLECTION.md").read_text(encoding="utf-8"), "Pure FastLED submodule provenance is missing")
    require("CubeFX Interpretation" in FIRMWARE and "renderFastLEDInterpretation" in FIRMWARE, "Firmware interpretation renderer is missing")
    official_sources = sorted(path.name for path in (ROOT / "third_party/FastLED/examples").iterdir() if path.is_dir())
    firmware_sources = double_quoted_block(FASTLED_INTERPRETATION_TYPES, r"FASTLED_INTERPRETATION_SOURCES\[.*?\] = \{(.*?)\};", "Firmware FastLED interpretation")
    web_sources = [value[1:-1] for value in quoted_fastled_sources(WEB)]
    android_sources = double_quoted_block(CATALOG, r"private val fastLedSources = listOf\((.*?)\n    \)", "Android FastLED interpretation")
    require(official_sources == firmware_sources == web_sources == android_sources, "FastLED source names drifted between the pure submodule and a CubeFX control surface")

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
    expected_cases = {1, 9, 10, 11, 12, 19, 20, 21, 22, 26, 28, *range(29, 80)}
    require(canonical_cases == expected_cases, "Firmware fixed canonical BLE selector drifted from the 62 base-mode contract")
    require("canonicalId >= FASTLED_INTERPRETATION_CANONICAL_FIRST" in FIRMWARE, "Firmware does not accept the FastLED interpretation BLE range")
    require("(29..162)" in CATALOG and "fastLedSources" in CATALOG, "Android catalogue does not expose all 83 FastLED interpretations")
    for uuid in (
        "6c75a300-7b1d-4f29-a221-000000000001",
        "6c75a300-7b1d-4f29-a221-000000000002",
        "6c75a300-7b1d-4f29-a221-000000000003",
    ):
        require(uuid in FIRMWARE and uuid in BLE, f"BLE UUID {uuid} is not shared by firmware and Android")
    require("enableStatusNotifications" in BLE and "handleStatus" in BLE, "Android has no firmware acknowledgement path")

    print("CubeFX control contract: 145 live web modes, 145 Android BLE modes, 83 pure FastLED sources, 83 CubeFX interpretations, five user slots, sixteen steps, five web eggs, five Android eggs — OK")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as error:
        print(f"CubeFX control contract: FAILED — {error}", file=sys.stderr)
        raise SystemExit(1)
