#!/usr/bin/env python3
"""Active transform guard.

Windows no longer run in a layout of their own: the theme places their documents, RmlUi
hit-tests them in screen pixels, and native text is measured in the window manager's one
measuring space. What remains of the ambient `UI::Scaling` transform is infrastructure:
the manager's measuring scope, the text renderer reading it, the inventory's screen scope
and the tooltip's metric scope. Window code that scopes or reads the transform again would
bring back layout decided in C++.

This script keeps it that way: it counts, per file under src/source, the calls listed in
PATTERNS (comments ignored), and compares them with layout_transform_allowlist.txt. A file
whose count grows, or a new file that starts using one, fails the build. A file whose
count dropped is printed so its entry can be lowered; the allowlist may only shrink.

Not counted: the transform's own implementation (EXEMPT_PREFIXES) and the world-space
users of the stretched screen (EXEMPT_FILES).

Usage: python3 check_layout_transform_users.py [--source-root DIR] [--allowlist FILE]
                                               [--summary] [--write]
Exit code 0 = clean, 1 = a count grew or an unlisted file uses the transform.
"""
import argparse
import pathlib
import re
import sys

PATTERNS = [
    r"\bGetActiveTransform\b",
    r"\bScopedActiveTransform\b",
    r"\bContains\(\s*MouseX\b",
]
PATTERN_RE = re.compile("|".join(PATTERNS))

EXEMPT_PREFIXES = ("UI/Scaling/",)
EXEMPT_FILES = {
    "App/Platform/Windows/Winmain.cpp",
    "Core/Input/SyntheticInput.cpp",
    "Engine/Object/ZzzInterface.cpp",
}

COMMENT_RE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"', re.S)


def strip_comments(text):
    """Comments removed; string literals kept, so `"//"` inside one does not cut a line."""
    return COMMENT_RE.sub(lambda m: m.group(0) if m.group(0).startswith('"') else " ", text)


def count_uses(source_root):
    counts = {}
    for path in sorted(source_root.rglob("*")):
        if path.suffix not in (".cpp", ".h") or not path.is_file():
            continue
        relative = path.relative_to(source_root).as_posix()
        if relative.startswith(EXEMPT_PREFIXES) or relative in EXEMPT_FILES:
            continue
        text = strip_comments(path.read_text(encoding="latin-1"))
        count = len(PATTERN_RE.findall(text))
        if count:
            counts[relative] = count
    return counts


def read_allowlist(path):
    entries = {}
    if not path.is_file():
        return entries
    for line_number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        name, _, count = line.rpartition(":")
        if not name or not count.strip().isdigit():
            sys.stderr.write("%s:%d: expected '<path>: <count>', got %r\n" % (path, line_number, raw))
            continue
        entries[name.strip()] = int(count)
    return entries


HEADER = """\
# Files still using the per-window layout transform, with how many uses each may have.
#
# Read tools/check_layout_transform_users.py's docstring first. A file may only go down: lower its
# count (or delete its line) when work removes uses; the build fails when a count grows or a new
# file appears. Regenerate with --write after removing uses, and check the diff only shrinks.

"""


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--source-root", default="src/source")
    parser.add_argument(
        "--allowlist", default=str(pathlib.Path(__file__).with_name("layout_transform_allowlist.txt"))
    )
    parser.add_argument("--summary", action="store_true", help="print the total and the largest files")
    parser.add_argument("--write", action="store_true", help="rewrite the allowlist with today's counts")
    args = parser.parse_args()

    source_root = pathlib.Path(args.source_root)
    if not source_root.is_dir():
        sys.stderr.write("source root not found: %s\n" % source_root)
        return 1
    counts = count_uses(source_root)
    total = sum(counts.values())

    if args.write:
        lines = ["%s: %d" % (name, count) for name, count in sorted(counts.items())]
        pathlib.Path(args.allowlist).write_text(HEADER + "\n".join(lines) + "\n", encoding="utf-8", newline="\n")
        print("Layout transform guard: wrote %d files, %d uses" % (len(counts), total))
        return 0

    if args.summary:
        print("Layout transform guard: %d uses in %d files" % (total, len(counts)))
        for name, count in sorted(counts.items(), key=lambda item: (-item[1], item[0]))[:25]:
            print("  %4d  %s" % (count, name))
        return 0

    allowlist = read_allowlist(pathlib.Path(args.allowlist))
    grew = [(name, count, allowlist.get(name)) for name, count in sorted(counts.items())
            if count > allowlist.get(name, 0)]
    shrank = [(name, counts.get(name, 0), allowed) for name, allowed in sorted(allowlist.items())
              if counts.get(name, 0) < allowed]

    if shrank:
        print(
            "Layout transform guard: %d allowlist entr%s can be lowered (run with --write): %s"
            % (len(shrank), "y" if len(shrank) == 1 else "ies",
               ", ".join("%s %d->%d" % (name, allowed, count) for name, count, allowed in shrank))
        )

    if grew:
        sys.stderr.write(
            "Layout transform guard: %d file(s) use the per-window layout transform more than allowed.\n\n"
            "Place documents in the theme, test the pointer with RmlUi hover or physical pixels, and size\n"
            "text from the typography scale instead -- see this script's docstring.\n\n" % len(grew)
        )
        for name, count, allowed in grew:
            sys.stderr.write("  %s: %d (allowed %s)\n" % (name, count, allowed if allowed is not None else "none"))
        return 1

    print("Layout transform guard: OK (%d uses in %d files)" % (total, len(counts)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
