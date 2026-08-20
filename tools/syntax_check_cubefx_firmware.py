#!/usr/bin/env python3
"""Run the Arduino-generated ESP32-S3 sketch command in syntax-only mode."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DATABASE = ROOT / ".build-cubefx-contract/compile_commands.json"


def main() -> int:
    entries = json.loads(DATABASE.read_text(encoding="utf-8"))
    sketch = next((entry for entry in entries if entry["file"].endswith("CubeFXWeb.ino.cpp")), None)
    if sketch is None:
        print("CubeFX syntax check: no sketch compiler command found", file=sys.stderr)
        return 1

    source = sketch["file"]
    command = sketch["arguments"]
    filtered: list[str] = []
    skip_next = False
    for index, argument in enumerate(command):
        if skip_next:
            skip_next = False
            continue
        if argument == "-c":
            continue
        if argument == "-o":
            skip_next = True
            continue
        filtered.append(argument)
    filtered.extend(["-fsyntax-only", source])
    # The original source occurs before -o; retain no duplicate source path.
    first_source = filtered.index(source)
    filtered.pop(first_source)
    print("CubeFX syntax check: validating generated ESP32-S3 sketch translation unit")
    return subprocess.run(filtered, cwd=sketch["directory"], check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
