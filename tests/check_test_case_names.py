#!/usr/bin/env python3
"""Every test case reaches CTest whole.

doctest_discover_tests (tests/third_party/doctest/doctestAddTests.cmake) reads the names a test binary lists as one
CMake list, a name per element, and registers each as a CTest entry that runs `--test-case=<name>`. A name loses its
case when it has
- a ';' or a line break: the list separator, or the line break the discovery turns into one, cuts the name in two,
  and neither half matches a case;
- more '[' than ']', or more ']' than '[': a ';' inside brackets separates nothing, so the name and every name listed
  after it in its binary become one entry that matches none;
- a '\\' at its end: it escapes the separator, so the name and the next one become one entry that matches none;
- a '\\' anywhere else: the discovery escapes only commas for doctest's filter, and the filter reads '\\\\' as one '\\'
  and splits at '\\,', so the entry's filter may match no case;
- nothing at all: CMake drops an empty element, so the case gets no entry and never runs;
- the shape of a line of doctest's list output ("[doctest] " at its start, or the row of '='): the discovery skips
  those lines, so the case gets no entry either;
- a character outside ASCII: on Windows the test binary reads its `--test-case=` argument in the ANSI code page, not as
  the UTF-8 the name is compiled to, so the filter matches no case there.
A comma is safe: the discovery escapes it for doctest's filter. After a build, tests/CMakeLists.txt fails an entry that
runs no case and a binary whose entries are not one per case; this check finds such names before any build.

Usage: check_test_case_names.py <tests dir>. Reads the name of every case a doctest macro registers (TEST_CASE,
TEST_CASE_FIXTURE, TEST_CASE_CLASS, SCENARIO and their template forms, with or without the DOCTEST_ prefix) in the .cpp
and .h files under it (third_party and fixtures left out) as the compiler reads them: comments and preprocessor lines
left out, adjacent literals joined, escapes decoded. Prints each name CMake would change, and exits 1 when there is
one, when a name is not written as string literals alone (a macro in it, say), or when it finds no test case at all.
"""

import re
import sys
import unicodedata
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cpp_text import code_and_source, source_files  # noqa: E402

# The macros that register a case; TEST_CASE_FIXTURE takes the fixture before the name, the others the name first.
CASE = re.compile(r"\b(?:DOCTEST_)?(TEST_CASE(?:_FIXTURE|_CLASS|_TEMPLATE(?:_DEFINE)?)?"
                  r"|SCENARIO(?:_CLASS|_TEMPLATE(?:_DEFINE)?)?)\s*\(")
OPENERS = "(<[{"
CLOSERS = ")>]}"
# The name: string literals the compiler joins into one, then the end of the arguments, the next one or a decorator.
# A literal is raw, read as written, or plain, where a '\' before a line break joins the lines (a line splice).
RAW_LITERAL = re.compile(r'\s*R"([^()\\\s]{0,16})\((.*?)\)\1"', re.DOTALL)
LITERAL = re.compile(r'\s*"((?:[^"\\\n]|\\.|\\\n)*)"')
LINE_SPLICE = "\\\n"
NAME_END = re.compile(r"\s*[),*]")
# A C escape: a hexadecimal or octal code, a universal character name, their delimited forms (\x{41}, \o{101},
# \u{41}, \N{LATIN CAPITAL LETTER A}), or the one character after the '\'.
ESCAPE = re.compile(r"\\(x\{[0-9A-Fa-f]+\}|x[0-9A-Fa-f]+|o\{[0-7]+\}|[0-7]{1,3}|u\{[0-9A-Fa-f]+\}|u[0-9A-Fa-f]{4}"
                    r"|U[0-9A-Fa-f]{8}|N\{[^}\n]+\}|.)")
SIMPLE_ESCAPES = {"n": "\n", "t": "\t", "r": "\r", "a": "\a", "b": "\b", "f": "\f", "v": "\v"}
# doctest's SCENARIO macros put this before the name they list.
SCENARIO_PREFIX = "  Scenario: "
# The lines of doctest's list output around the names, which the discovery skips.
LIST_HEADER = "[doctest] "
LIST_SEPARATOR = "=" * 79
SKIPPED_DIRS = {"third_party", "fixtures"}


def unescaped(escape):
    """The character a C escape stands for. Raises ValueError or KeyError for one that stands for none, such as \\x
    without digits or a code past U+10FFFF."""
    code = escape.group(1)
    delimited = code[1:2] == "{"
    digits = code[2:-1] if delimited else code[1:]
    if code[0] in "xuU":
        return chr(int(digits, 16))
    if code[0] == "o" and delimited:
        return chr(int(digits, 8))
    if code[0] == "N" and delimited:
        return unicodedata.lookup(digits)
    if code[0] in "01234567":
        return chr(int(code, 8))
    return SIMPLE_ESCAPES.get(code, code)


def literal_at(text, at):
    """The text of the string literal at `at` as the compiler reads it and where the literal ends, or None if there
    is none or it holds an escape this check cannot decode."""
    if raw := RAW_LITERAL.match(text, at):
        return raw.group(2), raw.end()
    if plain := LITERAL.match(text, at):
        try:
            return ESCAPE.sub(unescaped, plain.group(1).replace(LINE_SPLICE, "")), plain.end()
        except (ValueError, KeyError):
            return None
    return None


def name_at(text, at):
    """The name whose literals start at `at`, or None if it is not written as string literals alone."""
    parts = []
    while literal := literal_at(text, at):
        part, at = literal
        parts.append(part)
    if not parts or not NAME_END.match(text, at):
        return None
    return "".join(parts)


def after_first_argument(code, at):
    """Where the second argument starts, after the first comma outside brackets: a fixture with template arguments
    (Fixture<int, 2>) is skipped whole. Returns `at` if the arguments end first."""
    depth = 0
    for position in range(at, len(code)):
        character = code[position]
        if character in OPENERS:
            depth += 1
        elif character in CLOSERS:
            if depth == 0:
                return at
            depth -= 1
        elif character == "," and depth == 0:
            return position + 1
    return at


def cases(text):
    """The line and listed name of each test case in a source. The cases are found in the code alone, and their
    literals read with the comments between them left out."""
    code, source = code_and_source(text)
    for case in CASE.finditer(code):
        at = case.end()
        if case.group(1) == "TEST_CASE_FIXTURE":
            at = after_first_argument(code, at)
        name = name_at(source, at)
        if name is not None and case.group(1).startswith("SCENARIO"):
            name = SCENARIO_PREFIX + name
        yield text.count("\n", 0, case.start()) + 1, name


def problem(name):
    if name is None:
        return ("is not written as string literals this check can read (a macro in it, or an escape that stands for "
                "no character)")
    if not name:
        return "is empty, which the discovery drops; the case gets no entry"
    if name.startswith(LIST_HEADER) or name == LIST_SEPARATOR:
        return "reads like a line of doctest's list output, which the discovery skips; the case gets no entry"
    if ";" in name:
        return "has a ';', which cuts it in two; neither half runs a case"
    if "\n" in name:
        return "has a line break, which cuts it in two; neither half runs a case"
    if name.count("[") != name.count("]"):
        return "has unbalanced brackets, which join it with every name after it; that entry runs no case"
    if name.endswith("\\"):
        return "ends with a '\\', which joins it with the next name; that entry runs no case"
    if "\\" in name:
        return ("has a '\\', which doctest's filter reads as an escape before a '\\' or a ',' "
                "(then the entry runs no case)")
    if not name.isascii():
        return "has a character outside ASCII, which the test binary misreads on Windows; that entry runs no case there"
    return None


def main():
    # Piped output on Windows (CTest, the pre-push hook) uses the ANSI code page: escape what it cannot show.
    sys.stdout.reconfigure(errors="backslashreplace")
    root = Path(sys.argv[1])
    checked = 0
    found = []
    for path in source_files(root, SKIPPED_DIRS):
        for line, name in cases(path.read_text(encoding="utf-8-sig", errors="replace")):
            checked += 1
            if reason := problem(name):
                shown = "the name" if name is None else '"' + name.replace("\n", "\\n") + '"'
                found.append(f"{path}:{line}: {shown} {reason}")
    for offence in found:
        print(offence)
    if found:
        print("Write each name in ASCII as string literals, not empty or starting with '[doctest] ', without ';', a "
              "line break or '\\' (a comma is fine) and with balanced brackets.")
        return 1
    if checked == 0:
        print(f"No TEST_CASE found under {root}")
        return 1
    print(f"{checked} test case names reach CTest whole")
    return 0


if __name__ == "__main__":
    sys.exit(main())
