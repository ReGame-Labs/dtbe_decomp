#!/usr/bin/env python3
"""Doxygen's input filter (docs/Doxyfile): the C's plain comments that document
something become doc comments, for doxygen only; the files never change.

    tools/doxygen_filter.py FILE    prints FILE filtered, line for line

The comment right above a function, type, global or macro becomes /** */,
one after a member, enumerator or global on its line /**< */, and the
file's first one, before a blank line or an #include, the file's. Those
inside functions and initializers, the offsets (/* 0x1C */) and the rest
stay plain.
"""
import re
import sys

OFFSET = re.compile(r"\s*0x[0-9A-Fa-f]+\s*")
# doxygen's commands and HTML, outside `code` spans
SPECIAL = re.compile(r"([\\@<>&#$%])")
DOC_DIRECTIVES = ("define", "if", "ifdef", "ifndef")
# docs/Doxyfile's PREDEFINED: the only version
VERSIONS = {"VERSION_JP": True}


def escape(body):
    parts = re.split(r"(`[^`\n]*`)", body)
    parts[::2] = [SPECIAL.sub(r"\\\1", p) for p in parts[::2]]
    return "".join(parts)


def version_condition(directive):
    """Whether a #if or #elif on the versions only holds, or None"""
    condition = re.sub(r"/\*.*?\*/", "", directive.split(None, 1)[-1], flags=re.S)
    words = re.findall(r"\w+", condition)
    if not words or any(w not in VERSIONS for w in words) or re.search(r"[^\w\s|&!()]", condition):
        return None
    for name, value in VERSIONS.items():
        condition = condition.replace(name, str(value))
    return eval(condition.replace("||", " or ").replace("&&", " and ").replace("!", " not "))


def skip_comment(text, i):
    end = text.find("*/", i + 2)
    return len(text) if end < 0 else end + 2


def skip_quoted(text, i):
    quote = text[i]
    i += 1
    while i < len(text) and text[i] != quote and text[i] != "\n":
        i += 2 if text[i] == "\\" else 1
    return i + 1


def directive_end(text, i):
    while i < len(text) and text[i] != "\n":
        if text.startswith("/*", i):
            i = skip_comment(text, i)
        elif text.startswith("\\\n", i):
            i += 2
        else:
            i += 1
    return i


def filter_c(text):
    edits = []  # (start, end, replacement)
    scopes = []  # "aggregate" or "opaque", one per open brace
    branches = []  # per #if: [whether this branch is read, whether one was]
    parens = 0
    stmt = ""
    seen_code = False
    file_doc = False
    line_start = 0
    i = 0

    def documentable():
        # not in a function, an initializer or a parameter list
        return (not scopes or scopes[-1] == "aggregate") and parens == 0 \
            and "=" not in stmt and all(read for read, _ in branches)

    while i < len(text):
        c = text[i]
        if c == "\n":
            line_start = i + 1
            i += 1
        elif c in "\"'":
            if documentable():
                stmt += c
            i = skip_quoted(text, i)
        elif text.startswith("//", i):
            i = text.find("\n", i)
            i = len(text) if i < 0 else i
        elif c == "#" and not text[line_start:i].strip():
            end = directive_end(text, i)
            directive = text[i:end]
            name = re.match(r"#\s*(\w*)", directive).group(1)
            # the branches doxygen reads; of one on anything else, the first
            if name.startswith("if"):
                read = version_condition(directive) if name == "if" else None
                branches.append([read is not False, read is not False])
            elif name in ("elif", "else") and branches:
                read = version_condition(directive) if name == "elif" else True
                taken = branches[-1][1]
                branches[-1] = [not taken and read is not False, taken or read is not False]
            elif name == "endif" and branches:
                branches.pop()
            elif name == "define" and documentable() and "\\\n" not in directive:
                m = re.search(r"\S\s*(/\*(?!\*)(.*)\*/)\s*$", directive, re.S)
                if m and "*/" not in m.group(2):
                    edits.append((i + m.start(1), i + m.end(1), "/**<" + escape(m.group(2)) + "*/"))
            i = end
        elif text.startswith("/*", i):
            end = skip_comment(text, i)
            body = text[i + 2:end - 2]
            if documentable() and body.strip() and not body.startswith(("*", "!")) \
                    and not OFFSET.fullmatch(body):
                edit = classify(text, i, end, line_start, body, seen_code, file_doc)
                if edit:
                    edits.append(edit)
                    file_doc = file_doc or edit[2].endswith("\\file */")
            i = end
        else:
            if not all(read for read, _ in branches):
                i += 1
                continue
            if scopes and scopes[-1] == "opaque":
                if c == "{":
                    scopes.append("opaque")
                elif c == "}":
                    scopes.pop()
                    stmt = ""
                i += 1
                continue
            if not c.isspace():
                seen_code = True
            if c == "(":
                parens += 1
            elif c == ")":
                parens = max(parens - 1, 0)
            if c == "{":
                head = stmt.strip()
                if "=" not in head and not head.endswith(")") \
                        and re.search(r"\b(struct|union|enum|class)\b", head):
                    scopes.append("aggregate")
                else:
                    scopes.append("opaque")
                stmt = ""
            elif c == "}":
                if scopes:
                    scopes.pop()
                stmt = ""
            elif c in ";," and parens == 0:
                stmt = ""
            else:
                stmt += c
            i += 1

    out = []
    pos = 0
    for start, end, replacement in edits:
        out.append(text[pos:start])
        out.append(replacement)
        pos = end
    out.append(text[pos:])
    return "".join(out)


def classify(text, start, end, line_start, body, seen_code, file_doc):
    """The doc comment for the comment at text[start:end], or None"""
    before = re.sub(r"/\*.*?\*/", "", text[line_start:start], flags=re.S).strip()
    if before:
        if before.endswith((";", ",")):
            return start, end, "/**<" + escape(body) + "*/"
        return None
    rest = text[end:]
    if rest.split("\n", 1)[0].strip():
        return None  # code after it on its line
    following = rest.lstrip()
    blank = rest[:len(rest) - len(following)].count("\n") > 1
    directive = re.match(r"#\s*(\w+)", following)
    if not seen_code and not file_doc and (blank or (directive and directive.group(1) == "include")):
        return start, end, "/**" + escape(body).rstrip(" ") + " \\file */"
    if blank or not following or following.startswith(("/*", "//", "}")):
        return None
    if directive and directive.group(1) not in DOC_DIRECTIVES:
        return None
    return start, end, "/**" + escape(body) + "*/"


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__.split("\n\n")[1])
    with open(sys.argv[1], encoding="utf-8", errors="surrogateescape") as f:
        text = f.read()
    filtered = filter_c(text)
    assert filtered.count("\n") == text.count("\n"), sys.argv[1]
    sys.stdout.buffer.write(filtered.encode("utf-8", errors="surrogateescape"))


if __name__ == "__main__":
    main()
