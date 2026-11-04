#!/usr/bin/env python3
"""Every test case reaches CTest whole.

doctest_discover_tests (tests/third_party/doctest/doctestAddTests.cmake) reads the names a test binary lists as one
CMake list, a name per element, and registers each as a CTest entry that runs `--test-case=<name>`. CMake's list syntax
changes a name that has
- a ';': the list separator cuts the name in two, and neither half matches a case, so both entries pass running none;
- more '[' than ']', or more ']' than '[': a ';' inside brackets separates nothing, so the name and every name listed
  after it in its binary become one entry that runs none;
- a '\\' at its end: it escapes the separator, so the name and the next one become one entry that runs none;
- a '\\' anywhere else: the discovery escapes only commas for doctest's filter, and the filter reads '\\\\' as one '\\'
  and splits at '\\,', so the entry's filter may match no case.
A comma is safe: the discovery escapes it for doctest's filter.

Usage: check_test_case_names.py <tests dir>. Reads the names of the TEST_CASE and TEST_CASE_FIXTURE lines in the .cpp
and .h files under it (third_party and fixtures left out) as the compiler reads them: comments and preprocessor lines
left out, adjacent literals joined. Prints each name CMake would change, and exits 1 when there is one, when a name is
not written as string literals alone (a macro in it, say), or when it finds no test case at all.
"""

import re
import sys
from pathlib import Path

from cpp_text import code_only

CASE = re.compile(r"^[ \t]*(TEST_CASE|TEST_CASE_FIXTURE)\s*\(", re.M)
FIXTURE = re.compile(r"\s*[\w:]+\s*,")
# The name: string literals the compiler joins into one, then the end of the arguments or a decorator.
LITERAL = re.compile(r'\s*"((?:[^"\\\n]|\\.)*)"')
NAME_END = re.compile(r"\s*[)*]")
SKIPPED_DIRS = {"third_party", "fixtures"}


def name_at(text, at):
    """The name whose literals start at `at`, or None if it is not written as string literals alone."""
    parts = []
    while literal := LITERAL.match(text, at):
        parts.append(re.sub(r"\\(.)", r"\1", literal.group(1)))
        at = literal.end()
    if not parts or not NAME_END.match(text, at):
        return None
    return "".join(parts)


def cases(text):
    """The line and name of each test case in a source. The cases are found in the code alone, and their literals read
    with the comments between them left out."""
    code = code_only(text)
    source = code_only(text, keep_literals=True)
    for case in CASE.finditer(code):
        at = case.end()
        if case.group(1) == "TEST_CASE_FIXTURE":
            fixture = FIXTURE.match(source, at)
            at = fixture.end() if fixture else at
        yield text.count("\n", 0, case.start()) + 1, name_at(source, at)


def problem(name):
    if name is None:
        return "is not written as string literals, so this check cannot read it"
    if ";" in name:
        return "has a ';', which cuts it in two; neither half runs a case"
    if name.count("[") != name.count("]"):
        return "has unbalanced brackets, which join it with every name after it; that entry runs no case"
    if name.endswith("\\"):
        return "ends with a '\\', which joins it with the next name; that entry runs no case"
    if "\\" in name:
        return ("has a '\\', which doctest's filter reads as an escape before a '\\' or a ',' "
                "(then the entry runs no case)")
    return None


def main():
    root = Path(sys.argv[1])
    sources = [path for path in sorted(root.rglob("*"))
               if path.suffix in (".cpp", ".h") and not SKIPPED_DIRS & set(path.relative_to(root).parts)]
    checked = 0
    found = []
    for path in sources:
        for line, name in cases(path.read_text(encoding="utf-8", errors="replace")):
            checked += 1
            if reason := problem(name):
                shown = "the name" if name is None else f'"{name}"'
                found.append(f"{path}:{line}: {shown} {reason}")
    for offence in found:
        print(offence)
    if found:
        print("Write each name as string literals, without ';' or '\\' (a comma is fine) and with balanced brackets.")
        return 1
    if checked == 0:
        print(f"No TEST_CASE found under {root}")
        return 1
    print(f"{checked} test case names reach CTest whole")
    return 0


if __name__ == "__main__":
    sys.exit(main())
