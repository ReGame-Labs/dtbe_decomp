"""The version of the game the tools work on, and where its files are.

VERSION comes from the environment, as the Makefile exports it (make
VERSION=us), and defaults to jp like the Makefile. Each version has its
settings in mk/version/<version>.mk (the executable's name, where the
overlays load), its splat configs, symbols and checksums in
config/<version>/; the disc's files are in disks/<version>/, splat writes
its disassembly to asm/<version>/, the build goes to build/<version>/ and
objdiff's target objects to expected/<version>/.
"""

import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

VERSIONS = ("jp", "us")
VERSION = os.environ.get("VERSION") or "jp"
if VERSION not in VERSIONS:
    sys.exit(f"unsupported VERSION {VERSION}; supported: {' '.join(VERSIONS)}")

# the NAME := VALUE settings of the version's .mk
_SETTINGS = dict(re.findall(r"^(\w+)\s*:=\s*(.*?)\s*$",
                            (ROOT / "mk" / "version" / f"{VERSION}.mk").read_text(), re.M))

# the executable's name on the disc (SLPS_033.57)
EXE_NAME = _SETTINGS["EXE_NAME"]
# where movie.bin, title.bin and game.bin load, and the stages
OVERLAY_VRAM = int(_SETTINGS["OVERLAY_VRAM"], 16)
STAGE_VRAM = int(_SETTINGS["STAGE_VRAM"], 16)

# the disc's files: the executable and A.VFS
DISK_DIR = ROOT / "disks" / VERSION
# splat config (main.yaml), symbols and checksum
CONFIG_DIR = ROOT / "config" / VERSION
# splat's output (main/...)
ASM_DIR = ROOT / "asm" / VERSION
# objects, the ELF, the executable and the report
BUILD_DIR = ROOT / "build" / VERSION
# objdiff's target objects
EXPECTED_DIR = ROOT / "expected" / VERSION
# the prebuilt compiler and tools (tools/dl_deps.sh), as the Makefile's BIN_DIR
BIN_DIR = Path(os.environ.get("BIN_DIR") or ROOT / "bin")
