#!/usr/bin/env python3
"""Active transform guard.

Windows no longer run in a layout of their own: the theme places their documents, RmlUi
hit-tests them in screen pixels, and native text is measured in the window manager's one
measuring space. What remains of the ambient `UI::Scaling` transform is infrastructure:
the manager's measuring scope, the text renderer reading it, the inventory's screen scope
and the tooltip's metric scope. Window code that scopes or reads the transform again would
bring back layout decided in C++.

This script keeps it that way: it finds the calls listed in PATTERNS (comments ignored) in
every file under src/source, and any use outside the exempt files fails the build. The
exempt files are the transform's own implementation (EXEMPT_PREFIXES), the world-space users
of the stretched screen and the infrastructure above (EXEMPT_FILES, each with its reason).

Usage: python3 check_layout_transform_users.py [--source-root DIR] [--summary]
Exit code 0 = clean, 1 = a file outside the exempt ones uses the transform.
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
    # The world-space users of the stretched screen.
    "App/Platform/Windows/Winmain.cpp",
    "Core/Input/SyntheticInput.cpp",
    "Engine/Object/ZzzInterface.cpp",
    # The infrastructure: the window manager's one measuring scope, the native text renderer
    # reading it, the inventory's screen scope and the tooltip's metric scope.
    "UI/Core/WindowManager.cpp",
    "Render/Text/CUIRenderTextSDLTtf.cpp",
    "Engine/Object/ZzzInventory.cpp",
    "UI/RmlBridge/RmlTooltip.cpp",
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


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--source-root", default="src/source")
    parser.add_argument("--summary", action="store_true", help="print the uses found, by file")
    args = parser.parse_args()

    source_root = pathlib.Path(args.source_root)
    if not source_root.is_dir():
        sys.stderr.write("source root not found: %s\n" % source_root)
        return 1
    counts = count_uses(source_root)

    if args.summary:
        print("Layout transform guard: %d uses in %d files" % (sum(counts.values()), len(counts)))
        for name, count in sorted(counts.items(), key=lambda item: (-item[1], item[0])):
            print("  %4d  %s" % (count, name))
        return 0

    if counts:
        sys.stderr.write(
            "Layout transform guard: %d file(s) use the active UI transform.\n\n"
            "Place documents in the theme, test the pointer with RmlUi hover or physical pixels, and size\n"
            "text from the typography scale instead -- see this script's docstring.\n\n" % len(counts)
        )
        for name, count in sorted(counts.items()):
            sys.stderr.write("  %s: %d\n" % (name, count))
        return 1

    print("Layout transform guard: OK (no use outside the infrastructure)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
