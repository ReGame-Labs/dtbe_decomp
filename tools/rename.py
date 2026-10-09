#!/usr/bin/env python3
"""Rename symbols, again and again without harm.

    tools/rename.py OLD NEW [-n]
    tools/rename.py -f LIST... [-n]

A list (tools/renames/<module>.txt) has a rename a line, OLD NEW, in the
order they were made; # starts a comment. Each rename changes OLD to NEW as
a whole word in

- the symbol files, config/<version>/*.txt;
- the C, C++, headers and assembly under src/ and include/;
- the docs, *.md.

A name of splat's (func_8002A1B4, D_80064200) that the symbol file doesn't
have yet goes in it at its address, in the section of the module whose
header declares it (// src/engine/<module>/), in address order: a function
as `// type:func`, data without a size, which splat finds itself.

A rename whose OLD is gone and whose NEW is there is already done and
changes nothing, so a branch that still has the old names (or gets new code
with them in a rebase) runs the lists again to catch up. A NEW that is
already a name of something else is refused, and so is a list that gives
two symbols the same name.

-n only lists what would change. Afterwards, splat has to write the
assembly again with the new names: make regenerate.
"""

import argparse
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIG = ROOT / "config"
IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
SPLAT_NAME = re.compile(r"^(func|D)_(8[0-9A-F]{7})$")
SOURCE_SUFFIXES = {".c", ".cpp", ".h", ".s", ".inc"}
COMMENT_OR_STRING = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"', re.S)
SECTION = re.compile(r"^// src/engine/([\w/]+)/$")


def word(name):
    return re.compile(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"(?![A-Za-z0-9_])")


def files():
    """Every file a symbol's name can be in."""
    out = sorted(CONFIG.glob("*/*.txt"))
    for top in ("src", "include"):
        out += sorted(p for p in (ROOT / top).rglob("*") if p.is_file() and p.suffix in SOURCE_SUFFIXES)
    out += sorted(p for p in ROOT.glob("*.md")) + sorted((ROOT / "src").rglob("*.md"))
    return out


def code_only(text):
    """text without its comments and string literals"""
    return COMMENT_OR_STRING.sub(" ", text)


def module_order():
    """The modules in src/README.md's order."""
    text = (ROOT / "src/README.md").read_text()
    return re.findall(r"^\| `([\w/]+)/` \|", text, re.M)


def module_of(name, texts):
    """The module whose header declares name, else whose source has it."""
    pattern = word(name)
    for top in ("include/engine", "src/engine"):
        for path, text in texts.items():
            rel = path.relative_to(ROOT).as_posix()
            if rel.startswith(top + "/") and pattern.search(text):
                parts = Path(rel).relative_to(top).parts[:-1]
                # gfx/prim's names go in gfx/prim, the rest in the top folder
                if parts[:2] == ("gfx", "prim"):
                    return "gfx/prim"
                return parts[0]
    return None


def entry_address(line):
    m = re.match(r"\s*\w+\s*=\s*0x([0-9A-Fa-f]+)\s*;", line)
    return int(m.group(1), 16) if m else None


def add_symbol(text, module, line, address):
    """text with line in module's section of a symbol file, by address."""
    lines = text.split("\n")
    order = module_order()
    rank = order.index(module) if module in order else len(order)
    start = None
    for i, l in enumerate(lines):
        m = SECTION.match(l)
        if m and m.group(1) == module:
            start = i
            break
    if start is None:
        # a new section, ahead of the next module's
        at = next((i for i, l in enumerate(lines) if SECTION.match(l)
                   and (order.index(SECTION.match(l).group(1)) if SECTION.match(l).group(1) in order else len(order)) > rank), None)
        if at is None:
            # after the last module's section
            at = max(i for i, l in enumerate(lines) if SECTION.match(l))
            while lines[at].strip():
                at += 1
            at += 1
        lines[at:at] = [f"// src/engine/{module}/", line, ""]
        return "\n".join(lines)
    i = start + 1
    while i < len(lines) and lines[i].startswith("//"):
        i += 1
    while i < len(lines) and lines[i].strip():
        a = entry_address(lines[i])
        if a is not None and a > address:
            break
        i += 1
    lines.insert(i, line)
    return "\n".join(lines)


def read_list(path):
    out = []
    for n, raw in enumerate(path.read_text().splitlines(), 1):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) != 2:
            sys.exit(f"{path}:{n}: expected OLD NEW")
        out.append(tuple(parts))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("names", nargs="*", help="OLD NEW")
    ap.add_argument("-f", "--file", action="append", type=Path, default=[], help="a list of renames")
    ap.add_argument("-n", "--dry-run", action="store_true", help="only list what would change")
    args = ap.parse_args()
    renames = []
    for path in args.file:
        renames += read_list(path)
    if args.names:
        if len(args.names) != 2:
            ap.error("give OLD NEW, or lists with -f")
        renames.append(tuple(args.names))
    if not renames:
        ap.error("nothing to rename")
    for old, new in renames:
        for name in (old, new):
            if not IDENT.match(name):
                sys.exit(f"{name} is not a C identifier")
    twice = [n for n, c in Counter(new for _, new in renames).items() if c > 1]
    if twice:
        sys.exit("named twice: " + ", ".join(twice))

    texts = {p: p.read_text(errors="surrogateescape") for p in files()}
    symbols = CONFIG / "jp/symbols.txt"
    changed = Counter()
    done = 0
    for old, new in renames:
        old_re, new_re = word(old), word(new)
        hits = [p for p, t in texts.items() if old_re.search(t)]
        # the docs follow the code: a word left only in them is prose
        if not any(p.suffix != ".md" for p in hits):
            hits = []
        if not hits:
            if any(new_re.search(t) for p, t in texts.items() if p.suffix != ".md"):
                done += 1
                continue
            sys.exit(f"{old} is in no symbol file, nor in src/, include/ or the docs")
        # a word of a comment, a string or the docs is no clash
        clash = [p for p, t in texts.items() if p.suffix != ".md" and new_re.search(code_only(t))]
        if clash:
            sys.exit(f"{new} already exists: " + ", ".join(str(p.relative_to(ROOT)) for p in clash[:5]))
        m = SPLAT_NAME.match(old)
        # a C name for another symbol (__asm__("_._4Task")) has no entry of its own
        label = re.compile(re.escape(old) + r'\s*\([^;{]*\)\s*__asm__\("(?!' + re.escape(old) + r'")')
        entry = re.compile(r"^" + re.escape(old) + r"\s*=", re.M)
        if m and not entry.search(texts[symbols]) and not any(label.search(t) for t in texts.values()):
            module = module_of(old, texts)
            if module is None:
                sys.exit(f"{old}: no header or source under engine/ has it")
            kind = " // type:func" if m.group(1) == "func" else ""
            texts[symbols] = add_symbol(texts[symbols], module, f"{old} = 0x{m.group(2)};{kind}", int(m.group(2), 16))
            changed[symbols] += 0
        for p in hits + [symbols]:
            text, n = old_re.subn(new, texts[p])
            texts[p] = text
            changed[p] += n

    for p, n in sorted(changed.items()):
        if not args.dry_run:
            p.write_text(texts[p], errors="surrogateescape")
        print(f"{p.relative_to(ROOT)}: {n}")
    print(f"{len(renames) - done} renamed, {done} already done"
          + ("" if args.dry_run else "; now make regenerate"))


if __name__ == "__main__":
    main()
