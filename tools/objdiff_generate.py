#!/usr/bin/env python3
"""Write objdiff.json with one unit per C file under src/, for one version.

Target objects are splat's full disassembly of each C segment
(expected/<version>/asm/<segment>/<file>.s.o); base objects are the files
built from src/ (build/<version>/src/...), where every function still behind
INCLUDE_ASM carries a .NON_MATCHING label that objdiff drops from the
progress count. The PsyQ libraries stay splat's disassembly, outside src/,
so the progress counts only the game.

usage: objdiff_generate.py [version]   (default: $VERSION, or jp)
"""

import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

CATEGORIES = [
    {"id": "game", "name": "Game"},
]


def main() -> None:
    version = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("VERSION", "jp")
    units = []
    for src in sorted((ROOT / "src").rglob("*.c")):
        name = src.relative_to(ROOT / "src").with_suffix("").as_posix()
        units.append(
            {
                "name": name,
                "target_path": f"expected/{version}/asm/{name}.s.o",
                "base_path": f"build/{version}/src/{name}.c.o",
                "metadata": {"progress_categories": ["game"]},
            }
        )

    config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "make",
        "custom_args": [f"VERSION={version}"],
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.c", "*.h", "*.s", "*.inc"],
        "units": units,
        "progress_categories": CATEGORIES,
    }

    with open(ROOT / "objdiff.json", "w") as f:
        json.dump(config, f, indent=2)
        f.write("\n")


if __name__ == "__main__":
    main()
