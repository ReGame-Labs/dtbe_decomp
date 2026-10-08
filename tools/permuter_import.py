#!/usr/bin/env python3
"""Set up a decomp-permuter directory for one function.

usage: [UNIT=<unit>] tools/permuter_import.py draft.c func_name

draft.c must compile on its own (the unit's header plus the draft of the
function). UNIT (any part of the path) picks the unit when several have a
function of that name. The result goes to permuter/<func_name>/; run it with

    python3 external/decomp-permuter/permuter.py permuter/<func_name> -j8
"""

import glob
import os
import stat
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, f"{ROOT}/tools")
import version  # noqa: E402

ASM = str(version.ASM_DIR)
BIN = str(version.BIN_DIR)
DEFINES = (
    "-D__GNUC__=2 -D__GNUC_MINOR__=95 -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ "
    "-D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DSKIP_ASM "
    f"-DVERSION_{version.VERSION.upper()}"
)


def main():
    draft, func = sys.argv[1], sys.argv[2]
    asm = glob.glob(f"{ASM}/*/nonmatchings/**/{func}.s", recursive=True)
    if os.environ.get("UNIT"):
        asm = [a for a in asm if os.environ["UNIT"] in a]
    if not asm:
        sys.exit(f"no asm for {func}")
    asm = asm[0]

    cc1 = f"{BIN}/gcc-2.95.2-psx/cc1"

    out = f"{ROOT}/permuter/{func}"
    os.makedirs(out, exist_ok=True)

    inc = f"-I{ROOT}/include -I{ROOT}/external/psyq_headers/psyq_lib47/include"
    base = subprocess.run(
        f"mipsel-linux-gnu-cpp -P -undef {DEFINES} {inc} {draft}",
        shell=True, check=True, capture_output=True, text=True,
    ).stdout
    with open(f"{out}/base.c", "w") as f:
        f.write(base)

    with open(f"{out}/target.s", "w") as f:
        f.write('.include "macro.inc"\n.set noat\n.set noreorder\n.section .text\n')
        f.write(open(asm).read())
    subprocess.run(
        f"mipsel-linux-gnu-as -EL -march=r3000 -mtune=r3000 -no-pad-sections "
        f"-O1 -G0 -I{ROOT}/include -o {out}/target.o {out}/target.s",
        shell=True, check=True,
    )

    compile_sh = f"""#!/bin/bash
# usage: compile.sh input.c -o output.o
set -e
IN="$1"; OUT="$3"; T="$OUT.tmp"
{cc1} -quiet -O2 -G0 -mips1 -mcpu=3000 -mgas -msoft-float \\
    -fsigned-char -w -o "$T.s" "$IN"
python3 {ROOT}/external/maspsx/maspsx.py --aspsx-version=2.86 -G0 \\
    < "$T.s" > "$T.ms.s"
mipsel-linux-gnu-as -EL -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 \\
    -I{ROOT}/include -o "$OUT" "$T.ms.s"
rm -f "$T.s" "$T.ms.s"
"""
    with open(f"{out}/compile.sh", "w") as f:
        f.write(compile_sh)
    os.chmod(f"{out}/compile.sh", os.stat(f"{out}/compile.sh").st_mode | stat.S_IEXEC)

    with open(f"{out}/settings.toml", "w") as f:
        f.write(
            f'func_name = "{func}"\ncompiler_type = "gcc"\n'
            'objdump_command = "mipsel-linux-gnu-objdump -drz -m mips:3000"\n'
        )
    print(out)


if __name__ == "__main__":
    main()
