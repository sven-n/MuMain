#!/usr/bin/env python3
"""Draws take the game's objects read-only (ADR 0003).

Every function named Render*, Draw* or Describe* in the given source tree:
- takes OBJECT, CHARACTER, PART_t, JOINT, PARTICLE, ITEM and ITEM_t pointers and references as const (a const pointer to a
  writable object, `OBJECT* const`, is writable);
- declares no writable pointer or reference to one, nor a writable `auto` pointer or reference, except one that points
  at a copy the draw owns (a value of those types declared earlier in a block around it) or at one of the copy's value
  members;
- reaches through a copy only the copy's own fields: a pointer member of a copy (its owner, its bones, its parts)
  leads to another object, which the draw reads through the object it copied.

Comments and string literals are ignored. Usage: check_draws_read_only.py <source dir>. Prints each offence and exits 1
when there is one.
"""

import re
import sys
from pathlib import Path

TYPE_NAMES = ("OBJECT", "CHARACTER", "PART_t", "JOINT", "PARTICLE", "ITEM", "ITEM_t")
TYPES = "(?:" + "|".join(TYPE_NAMES) + ")"
NAME = re.compile(r"\b((?:\w+::)*(?:Render|Draw|Describe)\w*)\s*\(")
# What may stand before a function's name in its declaration: a return type and specifiers, nothing else.
DECLARATION_PREFIX = re.compile(r"^(?:[\w:<>,]+[\s\*&]+)+$")
NOT_TYPES = {"return", "else", "case", "if", "while", "for", "switch", "new", "delete", "throw", "goto", "co_return"}
POINTER = re.compile(r"(?<![\w:])" + TYPES + r"\s*(?:[\*&]\s*)+(?:const\s+)?(\w+)\s*(?:=\s*([^;\n]*)|;|\)|,|:|\[)")
AUTO = re.compile(r"(?<![\w:])auto\s*([\*&]\s*)?(\w+)\s*=\s*([^;\n]*)")
VALUE = re.compile(r"(?<![\w:*&])" + TYPES + r"\s+(\w+)\s*(?:=|;|\{|\[)")


def writable(text, matches):
    """The matches not qualified const: `const` before them, across any spaces and line breaks."""
    return [m for m in matches if not re.search(r"\bconst\s*$", text[: m.start()])]


def enclosing_block(body, at):
    """The span of the innermost braces around `at`."""
    depth, start = 0, at
    while start > 0:
        start -= 1
        depth += {"}": 1, "{": -1}.get(body[start], 0)
        if depth < 0:
            break
    return start, closing(body, start, "{", "}")


def owned(initializer, body, at, copies, members):
    """Whether an initializer is the address of a copy the draw owns, or of one of its value members: the copy is
    declared before `at` in a block around it. A pointer member of a copy leads to another object, so it is not
    owned."""
    match = re.fullmatch(r"&\s*(\w+)((?:\s*\.\s*\w+(?:\s*\[[^\]]*\])*)*)\s*", initializer)
    if not match or any(member in members for member in re.findall(r"\.\s*(\w+)", match.group(2))):
        return False
    for name, declared in copies:
        if name == match.group(1) and declared < at:
            start, end = enclosing_block(body, declared)
            if start < at < end:
                return True
    return False


def blank_comments_and_strings(text):
    """The text with comments and string and character literals turned into spaces; line breaks stay."""
    out = []
    at, end = 0, len(text)
    while at < end:
        two = text[at : at + 2]
        if two == "//":
            stop = text.find("\n", at)
            stop = end if stop < 0 else stop
        elif two == "/*":
            stop = text.find("*/", at + 2)
            stop = end if stop < 0 else stop + 2
        elif text[at] in "\"'" and not (text[at] == "'" and at > 0 and text[at - 1].isalnum()):
            quote, stop = text[at], at + 1
            raw = text[at] == '"' and at > 0 and text[at - 1] == "R"
            if raw:
                delimiter = text[at + 1 : text.find("(", at)]
                stop = text.find(")" + delimiter + '"', at)
                stop = end if stop < 0 else stop + len(delimiter) + 2
            else:
                while stop < end and text[stop] != quote and text[stop] != "\n":
                    stop += 2 if text[stop] == "\\" else 1
                stop = min(stop + 1, end)
        else:
            out.append(text[at])
            at += 1
            continue
        out.append("".join(c if c == "\n" else " " for c in text[at:stop]))
        at = stop
    return "".join(out)


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


def pointer_members(sources):
    """The pointer members of the game object types: a copy's pointer member leads to another object."""
    members = set()
    for text in sources:
        for type_name in TYPE_NAMES:
            bodies = []
            for match in re.finditer(r"(?:struct|class)\s+" + type_name + r"\b[^;{]*\{", text):
                bodies.append(text[match.end() - 1 : closing(text, match.end() - 1, "{", "}")])
            for match in re.finditer(r"\}\s*" + type_name + r"\s*[,;]", text):
                depth, at = 0, match.start()
                while at >= 0:
                    depth += {"}": 1, "{": -1}.get(text[at], 0)
                    if depth == 0:
                        break
                    at -= 1
                bodies.append(text[at : match.start()])
            for body in bodies:
                members |= set(re.findall(r"(?m)^\s*(?:const\s+)?[\w:<>]+\s*\*+\s*(\w+)\s*(?:\[[^\]]*\])?\s*[;={]", body))
    return members


def object_names(sources):
    """The names the headers declare with a game object type (globals, members, defaults: `Hero`, `Effects`,
    `Object`) and the functions that return a pointer to one (`GetCharacter`)."""
    names, functions = set(), set()
    for text in sources:
        # Statements only (`;` or an array): a parameter's default (`CHARACTER* c = NULL`) names no object.
        names |= set(re.findall(r"(?m)^\s*(?:extern\s+)?" + TYPES + r"\s*(?:\*\s*)?(\w+)\s*(?:\[[^\]\n]*\])?\s*(?:=[^;,)\n]*)?;", text))
        functions |= set(re.findall(r"(?<![\w:])" + TYPES + r"\s*\*\s*(?:\w+::)*(\w+)\s*\(", text))
    return names, functions


def leads_to_object(initializer, names, functions):
    """Whether an auto declaration's initializer is a game object: its address, a cast to one, or an expression that
    ends in a name declared as one or a call that returns one."""
    initializer = initializer.strip().rstrip(")").strip() if initializer.count(")") > initializer.count("(") else initializer.strip()
    if initializer.startswith("&"):
        return True
    if initializer.startswith("["):
        return False
    cast = re.match(r"(?:(?:static|reinterpret|const)_cast\s*<\s*|\(\s*)(?:const\s+)?(\w+)\s*\*", initializer)
    if cast:
        return cast.group(1) in TYPE_NAMES
    last = re.search(r"(\w+)\s*(\((?:[^()]|\([^()]*\))*\))?\s*(?:\[[^\]]*\]\s*)*$", initializer)
    if not last:
        return False
    return last.group(1) in (functions if last.group(2) else names)


def is_declaration(text, name_at):
    line_start = text.rfind("\n", 0, name_at) + 1
    prefix = text[line_start:name_at].strip()
    if not prefix or not DECLARATION_PREFIX.match(prefix + " "):
        return False
    words = re.findall(r"\w+", prefix)
    return bool(words) and not NOT_TYPES.intersection(words)


def offences(path, text, members, names, functions):
    found = []
    reach = re.compile(r"\s*(?:\.|->)\s*(?:\w+\s*(?:\.|->)\s*)*(" + "|".join(sorted(members)) + r")\b(\s*=(?!=))?")
    for function in NAME.finditer(text):
        if not is_declaration(text, function.start()):
            continue
        params_end = closing(text, function.end() - 1, "(", ")")
        if params_end < 0:
            continue
        line = text.count("\n", 0, function.start()) + 1
        name = function.group(1)
        params = text[function.end() : params_end + 1]
        for match in writable(params, POINTER.finditer(params)):
            found.append(f"{path}:{line}: {name} takes a writable {match.group(0).strip(' ,);:[')}")
        body_at = text.find("{", params_end)
        if body_at < 0 or text.find(";", params_end, body_at) >= 0:
            continue
        body = text[body_at : closing(text, body_at, "{", "}") + 1]
        body_line = line + text.count("\n", function.start(), body_at)

        def at(offset):
            return body_line + body[:offset].count("\n")

        copies = [(m.group(1), m.start()) for m in VALUE.finditer(body)]
        bound = {copy for copy, _ in copies}
        for match in writable(body, POINTER.finditer(body)):
            if owned((match.group(2) or "").strip(), body, match.start(), copies, members):
                bound.add(match.group(1))
                continue
            declared = " ".join(match.group(0).split()).strip(" ,);:[")
            found.append(f"{path}:{at(match.start())}: {name} declares a writable {declared}")
        for match in writable(body, AUTO.finditer(body)):
            if owned(match.group(3).strip(), body, match.start(), copies, members):
                bound.add(match.group(2))
                continue
            if leads_to_object(match.group(3), names, functions):
                declared = " ".join(match.group(0).split())
                found.append(f"{path}:{at(match.start())}: {name} declares a writable {declared}")
        for copy in sorted(bound):
            for match in re.finditer(r"(?<![\w.>])" + copy + reach.pattern, body):
                if match.group(2):
                    continue
                found.append(
                    f"{path}:{at(match.start())}: {name} reaches {match.group(1)} through its copy {copy}; "
                    "read it through the object it copied"
                )
    return found


def main():
    root = Path(sys.argv[1])
    paths = [p for p in sorted(root.rglob("*")) if p.suffix in (".cpp", ".h") and "ThirdParty" not in p.parts]
    texts = {p: blank_comments_and_strings(p.read_text(encoding="utf-8", errors="replace")) for p in paths}
    here = Path(__file__).resolve().parents[2] / "src" / "source"
    headers = [blank_comments_and_strings(p.read_text(encoding="utf-8", errors="replace"))
               for p in sorted(here.rglob("*.h")) if "ThirdParty" not in p.parts]
    members = pointer_members(list(texts.values()) + headers)
    names, functions = object_names([text for path, text in texts.items() if path.suffix == ".h"] + headers)
    found = []
    for path, text in texts.items():
        found += offences(path, text, members, names, functions)
    for offence in found:
        print(offence)
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
