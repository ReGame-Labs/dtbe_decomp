#!/usr/bin/env python3
"""Count the workarounds the matched C needs, by the markers they carry, and
fail on what the source must never have.

    tools/hacks.py                  the counts; fails on forbidden code
    tools/hacks.py --list           every one, with its file, line and function
    tools/hacks.py --check FILE...  also fail if the badge or the table,
                                    wherever they are in these files, differs

Every spot where the C only matches through a form natural C wouldn't take
carries a comment that says so. This finds them in src/ and include/:

- fake matches: a comment that starts with "fake match:", the last resort,
  a form forced only for the codegen;
- unused frame locals: a local the code never uses, kept because the
  original's stack frame has room for it, marked with exactly
  /* unused, but it is in the original stack frame */
- form-dependent matches: a comment that says "match depends on", where the
  match needs one of several equivalent forms (a statement macro's
  do-while, an extra block, braces left out, a copy of a variable, a type,
  one version's own form of a loop);
- functions still in assembly: INCLUDE_ASM lines.

--check FILE... compares the counts with a badge and a table in those
files, once the project has them. The functions still in assembly are counted apart, and
--list also shows what is assembly without being a hack: the rodata still
behind INCLUDE_RODATA and the macros that wrap inline asm for what C can't
say (moving $sp to the scratchpad, GTE instructions).

A function either matches as C or stays behind its INCLUDE_ASM, so these
always fail:

- NON_MATCHING (or NONMATCHING) code, and #if 0 blocks: C that doesn't
  match, or a draft, doesn't belong in src/;
- inline asm written in a function's body, a register variable pinned with
  asm("$reg"), or a top-level asm, data included: assembly in place of C
  (data, padding too, is written as C).
"""

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DIRS = ("src", "include")
# where INCLUDE_ASM and INCLUDE_RODATA are defined
INFRA = ("include/include_asm.h",)

FRAME_MARKER = "unused, but it is in the original stack frame"

# (key, its label in the table)
KINDS = (
    ("fake", "Fake matches"),
    ("frame", "Unused frame locals"),
    ("form", "Form-dependent matches"),
    ("asm", "Functions still in assembly"),
)
HACKS = ("frame", "form")

# what --list shows besides: assembly, but not a hack
OTHER = (
    ("rodata", "Rodata still in assembly (INCLUDE_RODATA)"),
    ("macros", "Macros that wrap inline asm"),
)

# strings and comments, in the order they come, so that neither a comment
# inside a string nor a quote inside a comment is taken for the other
TOKEN = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|/\*.*?\*/|//[^\n]*', re.S)
INCLUDE_ASM = re.compile(r"^\s*INCLUDE_ASM\s*\(\s*[^,()]*,\s*(\w+)\s*\)", re.M)
INCLUDE_RODATA = re.compile(r"^\s*INCLUDE_RODATA\s*\(\s*[^,()]*,\s*(\w+)\s*\)", re.M)
ASM = re.compile(r"\b(?:__asm__|__asm|asm)\b")
# a top-level asm statement
TOP_ASM = re.compile(r"^(?:__asm__|__asm|asm)\s*\(", re.M)
DEFINE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+(\w+)(?:[^\n]*\\\n)*[^\n]*", re.M)
REGISTER_ASM = re.compile(r"\bregister\b[^;{}()]*\b(?:__asm__|__asm|asm)\s*\(")
NON_MATCHING = re.compile(r"\bNON_?MATCHING\b")
IF_0 = re.compile(r"^[ \t]*#[ \t]*if[ \t]+0\b", re.M)
FUNC_NAME = re.compile(r"(\w+)\s*\([^;{}]*\)\s*$")
DIRECTIVE = re.compile(r"^[ \t]*#(?:[^\n]*\\\n)*[^\n]*", re.M)
# the shared files (src/menu_common/*.inc.c) name their functions with
# OVL_NAME(name), directly or through a macro of their own, which each
# overlay that includes them defines to its prefix
OVL_NAME = re.compile(r"\bOVL_NAME\s*\(\s*(\w+)\s*\)")
OVL_ALIAS = re.compile(r"^[ \t]*#[ \t]*define[ \t]+(\w+)[ \t]+OVL_NAME\s*\(\s*(\w+)\s*\)", re.M)
INCLUDE_INC = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"([^"]+\.inc\.c)"', re.M)


def comment_text(comment):
    """A comment's words, without its delimiters and leading asterisks."""
    if comment.startswith("//"):
        body = comment[2:]
    else:
        body = comment[2:-2]
    lines = [re.sub(r"^\s*\*(?!/)", "", line) for line in body.split("\n")]
    return " ".join(" ".join(lines).split())


def blank(match):
    """The match with everything but its newlines turned into spaces."""
    return re.sub(r"[^\n]", " ", match[0])


def blank_comment(match):
    """blank for a comment; a string stays as it is."""
    return blank(match) if match[0][0] == "/" else match[0]


def functions(code, aliases=None, prefix="OVL_NAME_"):
    """{line: function} for each line inside a function's braces, and the
    first line of each function definition, from code without comments or
    strings. A function named with OVL_NAME(name), or with one of the
    aliases ({macro: name}) of it, gets the name with the prefix."""
    aliases = aliases or {}
    owner = {}
    depth = 0
    current = None
    start = 0
    lines = code.split("\n")
    for n, line in enumerate(lines, 1):
        for i, c in enumerate(line):
            if c == "{":
                if depth == 0:
                    # the header is the text since the last top-level ; or }
                    head = " ".join(lines[start:n - 1] + [line[:i]])
                    head = head.split(";")[-1].split("}")[-1]
                    shared = OVL_NAME.sub(r"\1", head)
                    m = FUNC_NAME.search(shared.strip())
                    current = m[1] if m and "=" not in head else None
                    if current in aliases:
                        current = prefix + aliases[current]
                    elif current and shared != head:
                        current = prefix + current
                    if current:
                        owner[n] = current
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    current = None
                    start = n - 1
            elif c == ";" and depth == 0:
                start = n - 1
        if current:
            owner[n] = current
    return owner, lines


def function_at(owner, lines, line):
    """The function a marker at this line is in or, outside of one, the
    function right after it (a comment on top of a function or a macro)."""
    if line in owner:
        return owner[line]
    for n in range(line + 1, len(lines) + 1):
        if n in owner:
            return owner[n]
    return ""


def line_of(text, pos):
    return text.count("\n", 0, pos) + 1


def includers():
    """{shared file: the prefix its functions get}, as {A,B}_ for the
    overlays A and B (src/a, src/b) that include it, or A_ for one."""
    users = {}
    for path in sorted((ROOT / "src").rglob("*.c")):
        for m in INCLUDE_INC.finditer(path.read_text(errors="replace")):
            inc = (path.parent / m[1]).resolve().relative_to(ROOT).as_posix()
            users.setdefault(inc, set()).add(path.parent.name.upper())
    return {inc: (f"{{{','.join(sorted(names))}}}_" if len(names) > 1 else f"{min(names)}_")
            for inc, names in users.items()}


def scan():
    """{kind: [(path, line, function, text)]}, the forbidden code as
    [(path, line, function, why)], and what is assembly without being a
    hack, as {key of OTHER: [(path, line, text)]}."""
    found = {key: [] for key, _ in KINDS}
    forbidden = []
    other = {key: [] for key, _ in OTHER}
    shared = includers()
    for top in DIRS:
        for path in sorted((ROOT / top).rglob("*")):
            rel = path.relative_to(ROOT).as_posix()
            if path.suffix not in (".c", ".cpp", ".h"):
                continue
            text = path.read_text(errors="replace")
            tokens = list(TOKEN.finditer(text))
            # strings and comments blanked out, so that neither a brace nor
            # an INCLUDE_ASM inside one counts
            code = TOKEN.sub(blank, text)
            # and the preprocessor's lines, so that a macro's braces don't
            # look like a function's
            plain = DIRECTIVE.sub(blank, code)
            aliases = {m[1]: m[2] for m in OVL_ALIAS.finditer(code)}
            owner, lines = functions(plain, aliases, shared.get(rel, "OVL_NAME_"))
            for m in tokens:
                if m[0][0] != "/":
                    continue
                words = comment_text(m[0])
                n = line_of(text, m.start())
                if words.startswith("fake match:"):
                    kind = "fake"
                elif words == FRAME_MARKER:
                    kind = "frame"
                elif "match depends on" in words:
                    kind = "form"
                else:
                    continue
                func = function_at(owner, lines, n)
                if kind == "frame":
                    words = text.split("\n")[n - 1].split("/*")[0].strip()
                found[kind].append((rel, n, func, words))
            for m in INCLUDE_ASM.finditer(code):
                found["asm"].append((rel, line_of(code, m.start()), m[1], ""))
            for m in INCLUDE_RODATA.finditer(code):
                other["rodata"].append((rel, line_of(code, m.start()), m[1]))

            def forbid(pos, why):
                n = line_of(code, pos)
                forbidden.append((rel, n, owner.get(n, ""), why))

            for m in NON_MATCHING.finditer(code):
                forbid(m.start(), "NON_MATCHING code: a function that doesn't match stays behind its INCLUDE_ASM")
            for m in IF_0.finditer(code):
                forbid(m.start(), "#if 0: drafts don't belong in src/")
            pinned = set()
            for m in REGISTER_ASM.finditer(code):
                forbid(m.start(), "a register variable pinned with asm()")
                pinned.add(line_of(code, m.end()))
            if rel in INFRA:
                continue
            for m in ASM.finditer(plain):
                n = line_of(plain, m.start())
                if n in owner and n not in pinned:
                    forbid(m.start(), "inline asm in a function's body")
            for m in TOP_ASM.finditer(DIRECTIVE.sub(blank, TOKEN.sub(blank_comment, text))):
                if line_of(text, m.start()) not in owner:
                    forbid(m.start(), "top-level asm: write it, data too, in C")
            for m in DEFINE.finditer(code):
                if ASM.search(m[0]):
                    other["macros"].append((rel, line_of(code, m.start()), m[1]))
    return found, forbidden, other


def doc_counts(text):
    """The badge's two numbers and the table's count for each kind, of those
    the text has."""
    counts = {}
    m = re.search(r"img\.shields\.io/badge/fake%20matches%20%7C%20hacks-(\d+)%20%7C%20(\d+)-", text)
    if m:
        counts["badge fake"] = int(m[1])
        counts["badge hacks"] = int(m[2])
    for key, label in KINDS:
        m = re.search(r"^\|\s*" + re.escape(label) + r"\s*\|\s*([\d,]+)\s*\|", text, re.M)
        if m:
            counts[key] = int(m[1].replace(",", ""))
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--list", action="store_true", help="list every one")
    parser.add_argument("--check", metavar="FILE", nargs="+",
                        help="check the badge and the table, wherever they are in these files")
    args = parser.parse_args()

    found, forbidden, other = scan()
    count = {key: len(found[key]) for key in found}
    hacks = sum(count[key] for key in HACKS)

    if args.list:
        for key, label in KINDS:
            print(f"{label} ({count[key]}):")
            for path, line, func, text in found[key]:
                where = f"{path}:{line}"
                print(f"  {where:48} {func:36} {text if key != 'asm' else ''}".rstrip())
            print()
        for key, label in OTHER:
            print(f"{label} ({len(other[key])}):")
            for path, line, text in other[key]:
                where = f"{path}:{line}" if line else path
                print(f"  {where:48} {text}".rstrip())
            print()

    for key, label in KINDS:
        print(f"{label}: {count[key]}")
    print(f"Badge: fake matches | hacks = {count['fake']} | {hacks}")

    errors = [f"{path}:{line}: {why}" + (f" (in {func})" if func else "") for path, line, func, why in forbidden]

    if args.check:
        # each count from the first file that has it, as if the files were one
        have = {}
        source = {}
        for name in args.check:
            for key, value in doc_counts(Path(name).read_text()).items():
                if key not in have:
                    have[key] = value
                    source[key] = name
        files = ", ".join(args.check)
        want = dict(count, **{"badge fake": count["fake"], "badge hacks": hacks})
        labels = dict(KINDS, **{"badge fake": "the badge's fake matches", "badge hacks": "the badge's hacks"})
        stale = []
        for key in want:
            if key not in have:
                stale.append(f"{files}: no count for {labels[key]}")
            elif have[key] != want[key]:
                stale.append(f"{source[key]}: {labels[key]} says {have[key]}, the source has {want[key]}")
        if stale:
            errors += stale + [f"{files}: out of date: run tools/hacks.py and update them"]
        else:
            # one line, the first file's name first: scripts grep for it
            others = "".join(f", and {name}" for name in args.check[1:])
            print(f"{args.check[0]} is up to date{others}" + (" too" if others else ""))

    for e in errors:
        print(e)
    if errors:
        sys.exit(1)


if __name__ == "__main__":
    main()
