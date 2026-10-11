#!/usr/bin/env python3
"""Write the symbols an overlay takes from splat's symbol files.

    link_imports.py OUT OBJ... --from SYMS... --linked LD...

An overlay links against the executable's symbols (the Makefile's
MAIN_SYMS, from the executable's own link), and takes the rest of what its
objects use from splat's symbol files (undefined_{syms,funcs}_auto_<name>.txt).
OUT gets the names the objects use, don't define, and that no --linked
linker script assigns, with the addresses --from gives them: splat's files
list every name of the configs, and a name the executable defines must come
from the executable's link, not from splat's fixed address.
"""
import argparse
import re

from elftools.elf.elffile import ELFFile

ASSIGNMENT = re.compile(r"^\s*(\w+)\s*=\s*([^;]+);", re.M)


def used_names(paths):
    """The global names the objects use and don't define"""
    used, defined = set(), set()
    for path in paths:
        with open(path, "rb") as f:
            symtab = ELFFile(f).get_section_by_name(".symtab")
            for s in symtab.iter_symbols() if symtab else ():
                if not s.name or s["st_info"]["bind"] == "STB_LOCAL":
                    continue
                (used if s["st_shndx"] == "SHN_UNDEF" else defined).add(s.name)
    return used - defined


def assigned(path):
    """The names a linker script assigns, with what it assigns them"""
    text = re.sub(r"/\*.*?\*/", "", open(path).read(), flags=re.S)
    return {n: v.strip() for n, v in ASSIGNMENT.findall(text)}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("out")
    ap.add_argument("objects", nargs="+")
    ap.add_argument("--from", dest="sources", nargs="+", required=True, help="splat's symbol files")
    ap.add_argument("--linked", nargs="*", default=[], help="the overlay's other linker scripts")
    args = ap.parse_args()
    used = used_names(args.objects)
    for path in args.linked:
        used -= set(assigned(path))
    found = {}
    for path in args.sources:
        for name, value in assigned(path).items():
            if name in used:
                found.setdefault(name, value)
    with open(args.out, "w") as f:
        f.write("".join("%s = %s;\n" % (n, found[n]) for n in sorted(found)))


if __name__ == "__main__":
    main()
