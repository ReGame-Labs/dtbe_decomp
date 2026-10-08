"""The version of the game the tools work on, and where its files are.

VERSION comes from the environment, as the Makefile exports it (make
VERSION=jp), and defaults to jp like the Makefile. Each version has its
splat config, symbols and checksum in config/<version>/; splat writes its
disassembly to asm/<version>/, the build goes to build/<version>/ and
objdiff's target objects to expected/<version>/.
"""

import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

VERSIONS = ("jp",)
VERSION = os.environ.get("VERSION") or "jp"
if VERSION not in VERSIONS:
    sys.exit(f"unsupported VERSION {VERSION}; supported: {' '.join(VERSIONS)}")

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
