#!/usr/bin/env python3
"""Draws take the game's objects read-only (ADR 0003).

Every function named Render*, Draw* or Describe* in the given source tree:
- takes OBJECT, CHARACTER, PART_t, JOINT, PARTICLE, ITEM and ITEM_t pointers and references to const objects (a const
  pointer to a writable object, `OBJECT* const`, is writable);
- declares no pointer or reference, `auto` included, through which it could write a game object, unless it leads to a
  copy the draw owns: a value of those types declared in a block around it (not `extern`), one of the copy's value
  members, or a pointer or reference to those;
- reaches through a copy only the copy's own fields: a pointer member of a copy (its owner, its bones, its parts) leads
  to another object, which the draw reads through the object it copied.

The check reads the code as the compiler does: comments (continued ones too), string and character literals (with
their prefixes) and preprocessor lines are left out, and a declaration is found across lines and attributes. It follows
the types of the expressions it can read (names, `&`, `*`, `->`, `.`, `[]`, casts and calls that return game objects),
the scope of each declaration and what `auto` deduces. Usage: check_draws_read_only.py <source dir>. Prints each
offence and exits 1 when there is one.
"""

import re
import sys
from pathlib import Path

TYPE_NAMES = ("OBJECT", "CHARACTER", "PART_t", "JOINT", "PARTICLE", "ITEM", "ITEM_t")
TYPES = "(?:" + "|".join(TYPE_NAMES) + ")"
NAME = re.compile(r"\b((?:\w+::)*(?:Render|Draw|Describe)\w*)\s*\(")
NOT_TYPES = {"return", "else", "case", "if", "while", "for", "switch", "new", "delete", "throw", "goto", "co_return",
             "do", "sizeof", "decltype", "typedef", "using", "operator", "default"}
# A game object, pointer or reference declared: TYPE [const] [*|&]... [const] name, then how the declaration goes on.
DECLARATION = re.compile(
    r"(?<![\w:])(" + TYPES + r")\b(?!\s*::)(\s+const\b)?\s*((?:[\*&]\s*(?:const\b\s*)?)*)(\w+)\s*"
    r"(=\s*[^;\n]*|;|\)|,|:|\[[^\]]*\]\s*(?:=[^;\n]*)?;|\{[^;\n]*\}\s*;)"
)
AUTO = re.compile(r"(?<![\w:])auto\b\s*((?:[\*&]\s*(?:const\b\s*)?)*)(\w+)\s*=\s*([^;\n]*)")
# Any other declaration, to know when a name is shadowed by something that is not a game object.
OTHER = re.compile(r"(?:^|[;{}(,])\s*(?:(?:const|static|constexpr|volatile)\s+)*([A-Za-z_][\w:]*(?:<[^;{}()]*>)?)"
                   r"\s*[\*&]*\s*(\w+)\s*(?:=|;|\{|\[|\()", re.M)
PARAMETER = re.compile(r"(const\s+)?(" + TYPES + r")\b(\s+const\b)?\s*((?:[\*&]\s*(?:const\b\s*)?)*)(\w+)")


def blank(text, start, stop):
    return "".join(c if c == "\n" else " " for c in text[start:stop])


def is_digit_separator(text, at):
    """An apostrophe inside a number (1'000, 0xFF'FF), not the start of a character literal (L'}')."""
    start = at
    while start > 0 and (text[start - 1].isalnum() or text[start - 1] in "_.'"):
        start -= 1
    return start < at and text[start].isdigit() and at + 1 < len(text) and text[at + 1].isalnum()


def line_end(text, at):
    """Where a line ends, a line continued by a backslash going on to the next."""
    while at < len(text) and not (text[at] == "\n" and text[at - 1] != "\\"):
        at += 1
    return at


def code_only(text):
    """The text as the compiler reads it, with comments, literals and preprocessor lines turned into spaces; line
    breaks stay, so diagnostics keep their lines."""
    out = []
    at, end = 0, len(text)
    line_start = True
    while at < end:
        c = text[at]
        two = text[at : at + 2]
        stop = None
        if line_start and c == "#":
            stop = line_end(text, at)
        elif two == "//":
            stop = line_end(text, at)
        elif two == "/*":
            stop = text.find("*/", at + 2)
            stop = end if stop < 0 else stop + 2
        elif c == '"':
            stop = at + 1
            if at > 0 and text[at - 1] == "R":
                delimiter = text[at + 1 : text.find("(", at)]
                stop = text.find(")" + delimiter + '"', at)
                stop = end if stop < 0 else stop + len(delimiter) + 2
            else:
                while stop < end and text[stop] != '"' and text[stop] != "\n":
                    stop += 2 if text[stop] == "\\" else 1
                stop = min(stop + 1, end)
        elif c == "'" and not is_digit_separator(text, at):
            stop = at + 1
            while stop < end and text[stop] != "'" and text[stop] != "\n":
                stop += 2 if text[stop] == "\\" else 1
            stop = min(stop + 1, end)
        if stop is None:
            out.append(c)
            if c == "\n":
                line_start = True
            elif not c.isspace():
                line_start = False
            at += 1
            continue
        out.append(blank(text, at, stop))
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


def split_top_level(params):
    parts, depth, current = [], 0, ""
    for c in params:
        if c in "(<[{":
            depth += 1
        elif c in ")>]}":
            depth -= 1
        if c == "," and depth == 0:
            parts.append(current)
            current = ""
        else:
            current += c
    return parts + [current] if current.strip() else parts


class Kinds:
    """What the headers say about the game types: their members, the globals and the functions that return them."""

    def __init__(self, texts):
        # members[type][name] = (game type or None, is pointer, points to or is const, is array)
        self.members = {t: {} for t in TYPE_NAMES}
        self.pointer_members = set()
        self.globals = {}
        self.functions = {}
        for text in texts:
            for type_name in TYPE_NAMES:
                for body in self.bodies(text, type_name):
                    for m in re.finditer(r"(?m)^\s*(const\s+)?([\w:<>]+)\s*(\*+)?\s*(\w+)\s*(\[[^\]]*\])?\s*[;={]", body):
                        member_type = m.group(2) if m.group(2) in TYPE_NAMES else None
                        self.members[type_name].setdefault(
                            m.group(4), (member_type, bool(m.group(3)), bool(m.group(1)), bool(m.group(5))))
                        if m.group(3):
                            self.pointer_members.add(m.group(4))
            for m in re.finditer(r"(?m)^\s*(?:extern\s+)?(const\s+)?(" + TYPES + r")\s*(\*?)\s*(\w+)\s*(\[[^\]\n]*\])?"
                                 r"\s*(?:=[^;,)\n]*)?;", text):
                self.globals.setdefault(m.group(4), (m.group(2), bool(m.group(3)), bool(m.group(1)), bool(m.group(5))))
            for m in re.finditer(r"(?<![\w:])(const\s+)?(" + TYPES + r")\s*\*\s*(?:\w+::)*(\w+)\s*\(", text):
                self.functions.setdefault(m.group(3), (m.group(2), bool(m.group(1))))

    @staticmethod
    def bodies(text, type_name):
        for match in re.finditer(r"(?:struct|class)\s+" + type_name + r"\b[^;{]*\{", text):
            yield text[match.end() - 1 : closing(text, match.end() - 1, "{", "}")]
        for match in re.finditer(r"\}\s*" + type_name + r"\s*[,;]", text):
            depth, at = 0, match.start()
            while at >= 0:
                depth += {"}": 1, "{": -1}.get(text[at], 0)
                if depth == 0:
                    break
                at -= 1
            yield text[at : match.start()]


class Symbol:
    """A name declared in a draw: a game object value (owned unless extern), a pointer or reference to one, or
    something else, which only shadows."""

    def __init__(self, name, at, scope, kind, type_name=None, const=False, owned=False):
        self.name, self.at, self.scope = name, at, scope
        self.kind, self.type, self.const, self.owned = kind, type_name, const, owned


class Value:
    """An expression's type: a game object value or a pointer to one, its constness and whether the draw owns it."""

    def __init__(self, pointer, type_name, const, owned, array=False):
        self.pointer, self.type, self.const, self.owned, self.array = pointer, type_name, const, owned, array


def address_of(value):
    return Value(True, value.type, value.const, value.owned) if value is not None and not value.pointer else None


def enclosing_block(body, at):
    depth, start = 0, at
    while start > 0:
        start -= 1
        depth += {"}": 1, "{": -1}.get(body[start], 0)
        if depth < 0:
            break
    return start, closing(body, start, "{", "}")


class Draw:
    def __init__(self, path, name, params, body, body_line, kinds):
        self.path, self.name, self.body, self.body_line, self.kinds = path, name, body, body_line, kinds
        self.symbols = []
        self.found = []
        for param in split_top_level(params):
            m = PARAMETER.search(param)
            if m:
                kind = "pointer" if m.group(4).strip() else "value"
                const = bool(m.group(1) or m.group(3))
                self.symbols.append(Symbol(m.group(5), -1, (0, len(body)), kind, m.group(2), const, kind == "value"))

    def lookup(self, name, at):
        best = None
        for symbol in self.symbols:
            if symbol.name == name and symbol.at < at and symbol.scope[0] <= at <= symbol.scope[1]:
                if best is None or symbol.at > best.at:
                    best = symbol
        return best

    def evaluate(self, expression, at):
        """The type of an expression the check can read, else None."""
        text = expression.strip()
        while text.startswith("(") and closing(text, 0, "(", ")") == len(text) - 1:
            text = text[1:-1].strip()
        if text.startswith("&"):
            return address_of(self.evaluate(text[1:], at))
        if text.startswith("*"):
            inner = self.evaluate(text[1:], at)
            return Value(False, inner.type, inner.const, inner.owned) if inner and inner.pointer else None
        cast = re.match(r"(?:(?:static|reinterpret|const)_cast\s*<\s*(const\s+)?(\w+)\s*\*\s*>|\(\s*(const\s+)?(\w+)\s*\*\s*\))",
                        text)
        if cast:
            type_name = cast.group(2) or cast.group(4)
            return Value(True, type_name, bool(cast.group(1) or cast.group(3)), False) if type_name in TYPE_NAMES else None
        base = re.match(r"(\w+)\s*", text)
        if not base:
            return None
        rest = text[base.end() :]
        if rest.startswith("("):
            returned = self.kinds.functions.get(base.group(1))
            value = Value(True, returned[0], returned[1], False) if returned else None
            rest = rest[closing(rest, 0, "(", ")") + 1 :]
        else:
            symbol = self.lookup(base.group(1), at)
            if symbol is not None:
                if symbol.kind == "other":
                    return None
                value = Value(symbol.kind == "pointer", symbol.type, symbol.const, symbol.owned)
            elif base.group(1) in self.kinds.globals:
                type_name, pointer, const, array = self.kinds.globals[base.group(1)]
                value = Value(pointer, type_name, const, False, array)
            else:
                return None
        while value is not None and rest.strip():
            step = re.match(r"\s*(->|\.)\s*(\w+)|\s*\[", rest)
            if not step:
                return None
            if step.group(0).strip() == "[":
                rest = rest[closing(rest, rest.index("["), "[", "]") + 1 :]
                value = Value(False, value.type, value.const, value.owned) if value.pointer or value.array else None
                continue
            rest = rest[step.end() :]
            if (step.group(1) == "->") != value.pointer or rest.lstrip().startswith("("):
                return None
            member = self.kinds.members.get(value.type, {}).get(step.group(2))
            if member is None or member[0] is None:
                return None
            member_type, pointer, member_const, array = member
            if pointer:
                # A pointer member leads to another object, whatever the constness of the one holding it.
                value = Value(True, member_type, member_const, False)
            else:
                value = Value(False, member_type, value.const or member_const, value.owned, array)
        return value

    def check(self):
        body = self.body
        declarations = []
        for m in DECLARATION.finditer(body):
            extern = re.search(r"\bextern\s+(?:const\s+)?$", body[: m.start()]) is not None
            declarations.append((m.start(), "extern" if extern else "game", m))
        for m in AUTO.finditer(body):
            declarations.append((m.start(), "auto", m))
        for m in OTHER.finditer(body):
            if m.group(1) in TYPE_NAMES or m.group(1) in NOT_TYPES or m.group(1) in ("auto", "extern"):
                continue
            declarations.append((m.start(2), "other", m))
        for at, kind, m in sorted(declarations, key=lambda d: d[0]):
            scope = enclosing_block(body, at)
            if kind == "other":
                self.symbols.append(Symbol(m.group(2), at, scope, "other"))
            elif kind == "auto":
                self.declare_auto(at, scope, m)
            else:
                self.declare(at, scope, m, extern=kind == "extern")
        self.check_reach()
        return self.found

    def declare(self, at, scope, m, extern=False):
        stars, name, rest = m.group(3), m.group(4), m.group(5)
        const = re.search(r"\bconst\s*$", self.body[: m.start()]) is not None or bool(m.group(2))
        if not stars.strip():
            self.symbols.append(Symbol(name, at, scope, "value", m.group(1), const, owned=not extern))
            return
        init = rest[1:].strip() if rest.startswith("=") else None
        target = self.evaluate(init, at) if init else None
        if stars.strip().startswith("&") and target is not None:
            target = address_of(target) if not target.pointer else None
        owned = target is not None and target.pointer and target.owned
        self.symbols.append(Symbol(name, at, scope, "pointer", m.group(1), const, owned))
        if not const and not owned:
            self.offence(at, "declares a writable " + " ".join(m.group(0).split()).strip(" ,);:["))

    def declare_auto(self, at, scope, m):
        stars, name, init = m.group(1), m.group(2), m.group(3)
        leading = re.search(r"\bconst\s*$", self.body[: m.start()])
        leading_const = leading is not None
        value = self.evaluate(init, at)
        if value is None:
            call = re.search(r"(\w+)\s*\((?:[^()]|\([^()]*\))*\)\s*$", init.strip())
            returned = self.kinds.functions.get(call.group(1)) if call else None
            value = Value(True, returned[0], returned[1], False) if returned else None
        if value is None:
            self.symbols.append(Symbol(name, at, scope, "other"))
            return
        if "&" in stars:
            if value.pointer:
                self.symbols.append(Symbol(name, at, scope, "other"))
                return
            const = leading_const or value.const
            self.symbols.append(Symbol(name, at, scope, "pointer", value.type, const, value.owned))
            writable = not const and not value.owned
        elif value.pointer:
            # `const auto p` and `auto* const p` make the pointer const; the object keeps the initializer's constness,
            # unless the const stands for the object (`const auto* p`).
            const = value.const or (leading_const and "*" in stars)
            self.symbols.append(Symbol(name, at, scope, "pointer", value.type, const, value.owned))
            writable = not const and not value.owned
        else:
            # A game object by value is a copy the draw owns.
            self.symbols.append(Symbol(name, at, scope, "value", value.type, leading_const, True))
            writable = False
        if writable:
            written = self.body[leading.start() if leading else m.start() : m.end()]
            self.offence(at, "declares a writable " + " ".join(written.split()))

    def check_reach(self):
        members = self.kinds.pointer_members
        if not members:
            return
        reach = re.compile(r"(?<![\w.>])(\w+)\s*(?:\.|->)\s*(?:\w+\s*(?:\[[^\]]*\]\s*)?(?:\.|->)\s*)*(" +
                           "|".join(sorted(members)) + r")\b(\s*=(?!=))?")
        for m in reach.finditer(self.body):
            symbol = self.lookup(m.group(1), m.start())
            if symbol is None or symbol.kind == "other" or not symbol.owned or m.group(3):
                continue
            self.offence(m.start(),
                         f"reaches {m.group(2)} through its copy {m.group(1)}; read it through the object it copied")

    def offence(self, at, what):
        self.found.append(f"{self.path}:{self.body_line + self.body[:at].count(chr(10))}: {self.name} {what}")


def is_declaration(text, name_at):
    """Whether the name starts a declaration: what stands before it, back to the previous statement, is a return type
    with specifiers (attributes, access labels and templates left out), across lines."""
    start = max(text.rfind(c, 0, name_at) for c in ";{}") + 1
    prefix = text[start:name_at]
    prefix = re.sub(r"\[\[.*?\]\]", " ", prefix, flags=re.S)
    prefix = re.sub(r"\b(?:__declspec|alignas|__attribute__)\s*\((?:[^()]|\([^()]*\))*\)", " ", prefix)
    prefix = re.sub(r"\b(?:public|private|protected)\s*:", " ", prefix)
    prefix = re.sub(r"\btemplate\s*<[^;{}]*?>", " ", prefix).strip()
    if not prefix or not re.fullmatch(r"(?:[\w:<>,]+[\s\*&]+)+", prefix + " "):
        return False
    words = re.findall(r"\w+", prefix)
    return bool(words) and not NOT_TYPES.intersection(words)


def offences(path, text, kinds):
    found = []
    for function in NAME.finditer(text):
        if not is_declaration(text, function.start()):
            continue
        params_end = closing(text, function.end() - 1, "(", ")")
        if params_end < 0:
            continue
        line = text.count("\n", 0, function.start()) + 1
        name = function.group(1)
        params = text[function.end() : params_end]
        for param in split_top_level(params):
            m = PARAMETER.search(param)
            if m and m.group(4).strip() and not (m.group(1) or m.group(3)):
                found.append(f"{path}:{line}: {name} takes a writable {' '.join(m.group(0).split())}")
        body_at = text.find("{", params_end)
        if body_at < 0 or text.find(";", params_end, body_at) >= 0:
            continue
        body = text[body_at : closing(text, body_at, "{", "}") + 1]
        body_line = line + text.count("\n", function.start(), body_at)
        found += Draw(path, name, params, body, body_line, kinds).check()
    return found


def main():
    root = Path(sys.argv[1])
    paths = [p for p in sorted(root.rglob("*")) if p.suffix in (".cpp", ".h") and "ThirdParty" not in p.parts]
    texts = {p: code_only(p.read_text(encoding="utf-8", errors="replace")) for p in paths}
    game = Path(__file__).resolve().parents[2] / "src" / "source"
    headers = [code_only(p.read_text(encoding="utf-8", errors="replace"))
               for p in sorted(game.rglob("*.h")) if "ThirdParty" not in p.parts]
    kinds = Kinds([text for path, text in texts.items() if path.suffix == ".h"] + headers)
    found = []
    for path, text in texts.items():
        found += offences(path, text, kinds)
    for offence in found:
        print(offence)
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
