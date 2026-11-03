#!/usr/bin/env python3
"""Draws take the game's objects read-only (ADR 0003).

Every function named Render*, Draw* or Describe* in the given source tree takes OBJECT, CHARACTER, PART_t, JOINT,
PARTICLE and ITEM pointers and references as const, and declares none it could write through, except one that points
at a value of those types declared in its own body: a copy the draw owns.

Usage: check_draws_read_only.py <source dir>. Prints each offence and exits 1 when there is one.
"""

import re
import sys
from pathlib import Path

TYPES = r"(?:OBJECT|CHARACTER|PART_t|JOINT|PARTICLE|ITEM)"
FUNCTION = re.compile(r"(?m)^[A-Za-z_][\w:<>\s\*&]*?\b((?:\w+::)*(?:Render|Draw|Describe)\w*)\s*\(")
WRITABLE = re.compile(r"(?<![\w:])(?<!const )" + TYPES + r"\s*[\*&]\s*(\w+)\s*(?:=\s*([^;\n]*)|;|\)|,|:)")
VALUE = re.compile(r"(?<![\w:*&])" + TYPES + r"\s+(\w+)\s*(?:=|;|\{|\[)")


def closing(text, open_at, opener, closer):
    depth = 0
    for at in range(open_at, len(text)):
        if text[at] == opener:
            depth += 1
        elif text[at] == closer:
            depth -= 1
            if depth == 0:
                return at
    return -1


def offences(path):
    text = path.read_text(encoding="utf-8", errors="replace")
    found = []
    for function in FUNCTION.finditer(text):
        params_end = closing(text, function.end() - 1, "(", ")")
        if params_end < 0:
            continue
        line = text.count("\n", 0, function.start()) + 1
        name = function.group(1)
        for match in WRITABLE.finditer(text[function.end() : params_end + 1]):
            found.append(f"{path}:{line}: {name} takes a writable {match.group(0).strip(' ,);:')}")
        body_at = text.find("{", params_end)
        if body_at < 0 or text.find(";", params_end, body_at) >= 0:
            continue
        body = text[body_at : closing(text, body_at, "{", "}") + 1]
        copies = set(VALUE.findall(body))
        for match in WRITABLE.finditer(body):
            target = re.match(r"&\s*(\w+)\b", (match.group(2) or "").strip())
            if target and target.group(1) in copies:
                continue
            at = line + body[: match.start()].count("\n") + text.count("\n", function.start(), body_at)
            found.append(f"{path}:{at}: {name} declares a writable {match.group(0).strip(' ,);:')}")
    return found


def main():
    root = Path(sys.argv[1])
    found = []
    for path in sorted(root.rglob("*")):
        if path.suffix in (".cpp", ".h") and "ThirdParty" not in path.parts:
            found += offences(path)
    for offence in found:
        print(offence)
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
