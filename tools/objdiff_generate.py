#!/usr/bin/env python3
"""Write objdiff.json with one unit per C or C++ file under src/, for one version.

Target objects are splat's full disassembly of each C file
(expected/<version>/asm/<binary>/<file>.s.o, the binary being the executable,
main, or an overlay, whose config's src_path holds the file); base objects
are the files built from src/ (build/<version>/src/...), where every function
still behind INCLUDE_ASM carries a .NON_MATCHING label that objdiff drops
from the progress count. The PsyQ libraries stay splat's disassembly, outside
src/, so the progress counts only the game.

Every unit counts in "game", the whole game, and in "executable" or in
"overlays" and its overlay's own category, "overlays.<category>" (OVERLAYS).

usage: objdiff_generate.py [version]   (default: $VERSION, or jp)
"""

import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# the overlays' categories for the progress, by binary
OVERLAYS = {
    "movie": "movie",
    "title": "title",
    "game": "game",
    **{f"st{n:02}": "stages" for n in range(10)},
}
CATEGORY_NAMES = {
    "movie": "Movie player (movie.bin)",
    "title": "Title and menus (title.bin)",
    "game": "Battle (game.bin)",
    "stages": "Battle stages (st00-st09.bin)",
}


def source_dirs(version: str) -> dict:
    """Each binary's C folder (its splat config's src_path), longest first"""
    dirs = {}
    for binary in ["main", *OVERLAYS]:
        config = (ROOT / "config" / version / f"{binary}.yaml").read_text()
        dirs[re.search(r"^\s*src_path:\s*(\S+)", config, re.M).group(1)] = binary
    return dict(sorted(dirs.items(), key=lambda d: -len(d[0])))


def main() -> None:
    version = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("VERSION", "jp")
    dirs = source_dirs(version)
    units = []
    sources = [*(ROOT / "src").rglob("*.c"), *(ROOT / "src").rglob("*.cpp")]
    for src in sorted(sources):
        path = src.relative_to(ROOT).as_posix()
        folder = next(d for d in dirs if path.startswith(d + "/"))
        binary = dirs[folder]
        rest = Path(path).relative_to(folder).with_suffix("").as_posix()
        if binary == "main":
            categories = ["game", "executable"]
        else:
            categories = ["game", "overlays", f"overlays.{OVERLAYS[binary]}"]
        units.append(
            {
                "name": src.relative_to(ROOT / "src").with_suffix("").as_posix(),
                "target_path": f"expected/{version}/asm/{binary}/{rest}.s.o",
                "base_path": f"build/{version}/{path}.o",
                "metadata": {"progress_categories": categories},
            }
        )
    categories = [
        {"id": "game", "name": "Game"},
        {"id": "executable", "name": "Executable"},
        {"id": "overlays", "name": "Overlays"},
        *({"id": f"overlays.{c}", "name": n} for c, n in CATEGORY_NAMES.items()),
    ]

    config = {
        "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
        "custom_make": "make",
        "custom_args": [f"VERSION={version}"],
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.c", "*.cpp", "*.h", "*.s", "*.inc"],
        "units": units,
        "progress_categories": categories,
    }

    with open(ROOT / "objdiff.json", "w") as f:
        json.dump(config, f, indent=2)
        f.write("\n")


if __name__ == "__main__":
    main()
