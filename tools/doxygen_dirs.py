#!/usr/bin/env python3
"""Writes the folders' descriptions for doxygen (make docs), from the table
of modules in src/README.md.

    tools/doxygen_dirs.py OUT.dox

Each module's folder, in src/engine/ and include/engine/, gets what the
table says it is; src/engine/ and include/engine/ themselves, the
executable's code and its declarations.
"""
import os
import re
import sys

from doxygen_filter import escape

ROW = re.compile(r"^\| `([\w/]+)/` \|(.*)\|\s*$")
EXE = "SLPS_033.57, the executable"


def rows(path):
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = ROW.match(line)
            if m:
                yield m.group(1), m.group(2).strip()


def entry(folder, brief):
    return f"/** \\dir {folder}\n    \\brief {escape(brief)} */\n"


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__.split("\n\n")[1])
    out = [entry("src/engine", f"{EXE}: its code, one folder per module"),
           entry("include/engine", f"{EXE}: the declarations of its code, one header per source file")]
    for folder, what in rows("src/README.md"):
        for root in ("src/engine", "include/engine"):
            if os.path.isdir(f"{root}/{folder}"):
                out.append(entry(f"{root}/{folder}", what))
    with open(sys.argv[1], "w", encoding="utf-8") as f:
        f.writelines(out)


if __name__ == "__main__":
    main()
