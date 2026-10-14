#!/usr/bin/env python3
"""Rename symbols, again and again without harm.

    tools/rename.py OLD NEW [-n] [--overlay NAME]
    tools/rename.py -f LIST... [-n] [--overlay NAME]

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

The overlays load at the same addresses, so one splat name can be a
different symbol in each of them: an overlay's list runs with --overlay
NAME (config/<version>/NAME.yaml), and then renames only in that overlay's
symbol files (config/<version>/symbols_NAME.txt) and in its source and
header folders (src/<path> and include/<path> of its src_path, without the
folders of another overlay inside them), and in those of the overlays that
link to it (whose yaml reads symbols_NAME.txt: the stages use game.bin's
names). A splat name the overlay's symbol
file doesn't have yet goes at the end of it, in address order, for each
version whose assembly of the overlay (asm/<version>/NAME/) has the name
and whose overlay holds that address, with the type another version's
entry gives it (a label); that is also how a name only the assembly has
gets its entry. Two splat names may take one NEW when no
version has both: a function or datum each version has in its own #if
branch.

A rename whose OLD is gone and whose NEW is there (or the name a later line
of the lists gives NEW) is already done and changes nothing, so a branch
that still has the old names (or gets new code with them in a rebase) runs
the lists again to catch up; a splat name left in new code becomes the name
whose symbol entry has its address. A NEW that is already a name of
something else is refused (but in a sibling overlay, which reads the same
symbol files and never links with this one: each stage can have its own
BonusThrower_init), and so is a list that gives two symbols the same
name (but the twins above).

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
OVERLAY_SECTION = "// the names tools/rename.py gave (tools/renames/), in address order"


def word(name):
    return re.compile(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"(?![A-Za-z0-9_])")


def files():
    """Every file a symbol's name can be in."""
    out = sorted(CONFIG.glob("*/*.txt"))
    for top in ("src", "include"):
        out += sorted(p for p in (ROOT / top).rglob("*") if p.is_file() and p.suffix in SOURCE_SUFFIXES)
    out += sorted(p for p in ROOT.glob("*.md")) + sorted((ROOT / "src").rglob("*.md"))
    return out


def config_of(version, name):
    import yaml

    return yaml.safe_load((CONFIG / version / f"{name}.yaml").read_text())


def overlay_range(version, name):
    """The vram an overlay takes in a version, start and end."""
    segments = config_of(version, name)["segments"]
    return segments[0]["vram"], segments[0]["vram"] + segments[-1][0] - segments[0]["start"]


def overlay_files(name):
    """The files an overlay's names can be in: its symbol files, and the
    source and header folders, without another binary's inside them, of the
    overlay and of the overlays that link to it (their yaml reads its symbol
    file: the stages call game.bin by its names)."""
    out = sorted(CONFIG.glob(f"*/symbols_{name}.txt"))
    configs = {c.stem: config_of("jp", c.stem) for c in (CONFIG / "jp").glob("*.yaml")}
    paths = {o: c["options"]["src_path"] for o, c in configs.items()}
    users = [o for o, c in configs.items()
             if o == name or f"config/jp/symbols_{name}.txt" in c["options"].get("symbol_addrs_path", [])]
    for user in sorted(users):
        own = paths[user]
        nested = [p[len(own):] for o, p in paths.items() if o != user and p.startswith(own + "/")]
        for top in (own, "include" + own[len("src"):]):
            for p in sorted((ROOT / top).rglob("*")):
                rel = p.relative_to(ROOT).as_posix()
                if any(rel.startswith(top + n + "/") for n in nested):
                    continue
                if p.is_file() and (p.suffix in SOURCE_SUFFIXES or p.suffix == ".md"):
                    out.append(p)
    return out


def sibling_files(name):
    """The files of the overlays that never link with name: those that load
    on the same parent overlay as name and read the same symbol files (the
    stages, all on game.bin), whose names can't meet name's."""
    configs = {c.stem: config_of("jp", c.stem) for c in (CONFIG / "jp").glob("*.yaml")}

    def parents(o):
        return set(configs[o]["options"].get("symbol_addrs_path", [])) - {f"config/jp/symbols_{o}.txt"}

    out = set()
    for other in configs:
        on_overlay = any(Path(s).stem.startswith("symbols_") for s in parents(name))
        if other != name and on_overlay and parents(other) == parents(name):
            own = configs[other]["options"]["src_path"]
            out.update(CONFIG.glob(f"*/symbols_{other}.txt"))
            for top in (own, "include" + own[len("src"):]):
                out.update(p for p in (ROOT / top).rglob("*") if p.is_file())
    return out


def add_overlay_symbol(text, line, address):
    """text with line in the section of the names rename.py gave, by address."""
    lines = text.rstrip("\n").split("\n")
    if OVERLAY_SECTION not in lines:
        lines += ["", OVERLAY_SECTION]
    i = lines.index(OVERLAY_SECTION) + 1
    while i < len(lines) and lines[i].strip():
        a = entry_address(lines[i])
        if a is not None and a > address:
            break
        i += 1
    lines.insert(i, line)
    return "\n".join(lines) + "\n"


def overlay_entries(name, old, texts, asm_texts):
    """{symbol file: entry line} for each version that needs old's entry."""
    m = SPLAT_NAME.match(old)
    address = int(m.group(2), 16)
    out = {}
    for version_dir in sorted(CONFIG.iterdir()):
        path = version_dir / f"symbols_{name}.txt"
        if not path.exists():
            continue
        start, end = overlay_range(version_dir.name, name)
        if not start <= address < end:
            continue
        if version_dir.name not in asm_texts:
            asm = ROOT / "asm" / version_dir.name / name
            asm_texts[version_dir.name] = "\n".join(p.read_text(errors="surrogateescape") for p in asm.rglob("*.s"))
        if not word(old).search(asm_texts[version_dir.name]):
            continue
        has = re.compile(r"^(" + re.escape(old) + r"\s*=|\S+\s*=\s*0x0*" + f"{address:X}" + r"\s*;)", re.M | re.I)
        if has.search(texts[path]):
            continue
        kind = " // type:func" if m.group(1) == "func" else ""
        # another version's entry knows better (a label inside a function)
        for other in CONFIG.glob(f"*/symbols_{name}.txt"):
            known = re.search(r"^" + re.escape(old) + r"\s*=[^;]*;\s*//\s*(type:\w+)", texts[other], re.M)
            if known:
                kind = " // " + known.group(1)
        out[path] = f"{old} = 0x{m.group(2)};{kind}"
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


def address_of(name, earlier):
    """The address of a splat name, or of the name earlier renames made of one."""
    for old, new in reversed(earlier):
        if new == name:
            name = old
    m = SPLAT_NAME.match(name)
    return m.group(2) if m else None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("names", nargs="*", help="OLD NEW")
    ap.add_argument("-f", "--file", action="append", type=Path, default=[], help="a list of renames")
    ap.add_argument("-n", "--dry-run", action="store_true", help="only list what would change")
    ap.add_argument("--overlay", help="rename in this overlay only (config/<version>/OVERLAY.yaml)")
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
    twins = {n for n, c in Counter(new for _, new in renames).items() if c > 1}
    twice = [n for n in twins if not args.overlay or not all(SPLAT_NAME.match(old) for old, new in renames if new == n)]
    if twice:
        sys.exit("named twice: " + ", ".join(twice))
    if args.overlay and not (CONFIG / "jp" / f"{args.overlay}.yaml").exists():
        sys.exit(f"no overlay {args.overlay} in config/jp/")

    texts = {p: p.read_text(errors="surrogateescape") for p in files()}
    scope = set(overlay_files(args.overlay)) if args.overlay else set(texts)
    siblings = sibling_files(args.overlay) if args.overlay else set()
    for p in scope - set(texts):
        texts[p] = p.read_text(errors="surrogateescape")
    asm_texts = {}
    twin_files = {}
    symbols = CONFIG / "jp/symbols.txt"
    changed = Counter()
    done = 0
    for i, (old, new) in enumerate(renames):
        old_re, new_re = word(old), word(new)
        hits = [p for p, t in texts.items() if p in scope and old_re.search(t)]
        # the docs follow the code: a word left only in them is prose
        if not any(p.suffix != ".md" for p in hits):
            hits = []
        m = SPLAT_NAME.match(old)
        address = address_of(old, renames[:i])
        # a NEW that a later line renames again is there under its last name
        last = new
        for later_old, later_new in renames[i + 1:]:
            if later_old == last:
                last = later_new
        there = any(word(name).search(t) for name in {new, last}
                    for p, t in texts.items() if p in scope and p.suffix != ".md")
        # the assembly keeps OLD until make regenerate: once NEW is there, only
        # a twin looks at it again
        entries = {}
        if args.overlay and m and (hits or not there or new in twin_files):
            entries = overlay_entries(args.overlay, old, texts, asm_texts)
        if not hits and not entries:
            if there:
                done += 1
                if new in twins:
                    twin_files.setdefault(new, set())
                continue
            sys.exit(f"{old} is in no symbol file, nor in src/, include/ or the docs"
                     + (f", nor in the {args.overlay} assembly" if args.overlay else ""))
        # a word of a comment, a string or the docs is no clash; a twin's
        # NEW clashes with nothing but its other twin
        clash = [p for p, t in texts.items() if p.suffix != ".md" and new_re.search(code_only(t))
                 and p not in siblings]
        # new code that still calls a named symbol by an old name: NEW's entry
        # has OLD's address, so NEW is that symbol and no clash
        same = address and re.search(r"^" + re.escape(new) + r"\s*=\s*0x" + address + ";", texts[symbols], re.M | re.I)
        if same:
            clash = []
        if new in twins:
            if new in twin_files:
                clash = [p for p in clash if p not in scope]
            if set(entries) & twin_files.get(new, set()):
                sys.exit(f"{new}: one version has two of its twins")
            twin_files.setdefault(new, set()).update(entries)
        if clash:
            sys.exit(f"{new} already exists: " + ", ".join(str(p.relative_to(ROOT)) for p in clash[:5]))
        if args.overlay:
            for p, line in entries.items():
                texts[p] = add_overlay_symbol(texts[p], line, int(m.group(2), 16))
                changed[p] += 0
            for p in dict.fromkeys(hits + list(entries)):
                text, n = old_re.subn(new, texts[p])
                texts[p] = text
                changed[p] += n
            continue
        # a C name for another symbol (__asm__("_._4Task")) has no entry of its own
        label = re.compile(re.escape(old) + r'\s*\([^;{]*\)\s*__asm__\("(?!' + re.escape(old) + r'")')
        entry = re.compile(r"^" + re.escape(old) + r"\s*=", re.M)
        if m and not same and not entry.search(texts[symbols]) and not any(label.search(t) for t in texts.values()):
            module = module_of(old, texts)
            if module is None:
                sys.exit(f"{old}: no header or source under engine/ has it (an overlay's name needs --overlay)")
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
