#!/bin/bash
# Turn the assembly that the C includes (INCLUDE_ASM, INCLUDE_RODATA: the
# files under <dir>/nonmatchings) back into Shift JIS. splat writes the game's
# Shift JIS text as UTF-8; the assembly files the Makefile builds go through
# iconv on their way to gas, but gas reads an included file as it is.
#
# usage: tools/sjis_includes.sh <splat asm dir of a binary>

set -eo pipefail

dir="$1/nonmatchings"
[ -d "$dir" ] || exit 0
{ grep -rlP --include='*.s' '[^\x00-\x7F]' "$dir" || true; } | while read -r f; do
    iconv -f UTF-8 -t SHIFT-JIS "$f" > "$f.sjis"
    mv "$f.sjis" "$f"
done
