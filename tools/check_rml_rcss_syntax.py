#!/usr/bin/env python3
"""RML/RCSS syntax guard.

RmlUi's own parser doesn't error on most syntax mistakes in these two formats --
it silently drops or misparses the offending content and keeps going, so a
broken file (or a broken shared file linked by many windows) can go unnoticed
for a long time, surfacing only as unrelated-looking runtime symptoms in
whichever windows happen to load it. This script catches what the real parser
won't tell you about, ahead of time:

- RML: unterminated/nested `<!-- -->` comments.
- RCSS: balanced `{`/`}`, and unterminated/nested `/* */` comments.

Both comment syntaxes share the same trap: they don't nest. `/* ... "/* x */"
... */` (or the `<!-- -->` equivalent) closes at the FIRST close-marker, not
the intended one, so everything between that premature close and the next
real close-marker is parsed as garbage markup/CSS -- with no error, just
silent misbehavior. A comment written this way in a shared base.rcss once
broke every window that linked it in one shot (see STATUS.md's Findings
section).

Deliberately not a full RML/RCSS grammar validator (no XML well-formedness
check, no property/tag validation) -- this codebase's RML comments routinely
contain a literal `--` for prose (technically invalid per the XML spec, but
RmlUi's actual parser tolerates it fine), so a strict validator would flag
most files for a non-issue. This only catches the one structural failure
mode that's actually gone unnoticed here before.

Usage: python3 check_rml_rcss_syntax.py [--asset-root DIR]
Exit code 0 = clean, 1 = violation(s) found (printed to stderr).
"""
import argparse
import pathlib
import sys


def check_rml(path: pathlib.Path) -> list[str]:
    text = path.read_text(encoding="utf-8")
    errors: list[str] = []
    in_comment = False
    comment_start_line = 0
    line = 1
    i = 0
    n = len(text)
    while i < n:
        if text[i] == "\n":
            line += 1
        if in_comment:
            if text.startswith("<!--", i):
                errors.append(
                    f"{path}:{line}: '<!--' nested inside the comment opened at line "
                    f"{comment_start_line} -- XML comments don't nest; this one does "
                    "nothing, but the outer comment still ends at the next '-->', "
                    "silently turning everything between into live markup"
                )
                i += 4
                continue
            if text.startswith("-->", i):
                in_comment = False
                i += 3
                continue
            i += 1
            continue
        if text.startswith("<!--", i):
            in_comment = True
            comment_start_line = line
            i += 4
            continue
        i += 1
    if in_comment:
        errors.append(f"{path}: unterminated comment opened at line {comment_start_line}")
    return errors


def check_rcss(path: pathlib.Path) -> list[str]:
    text = path.read_text(encoding="utf-8")
    errors: list[str] = []
    in_comment = False
    comment_start_line = 0
    brace_depth = 0
    line = 1
    i = 0
    n = len(text)
    while i < n:
        if text[i] == "\n":
            line += 1
        if in_comment:
            if text.startswith("/*", i):
                errors.append(
                    f"{path}:{line}: '/*' nested inside the comment opened at line "
                    f"{comment_start_line} -- CSS comments don't nest; this one does "
                    "nothing, but the outer comment still ends at the next '*/', "
                    "silently turning everything between into live CSS"
                )
                i += 2
                continue
            if text.startswith("*/", i):
                in_comment = False
                i += 2
                continue
            i += 1
            continue
        if text.startswith("/*", i):
            in_comment = True
            comment_start_line = line
            i += 2
            continue
        if text[i] == "{":
            brace_depth += 1
        elif text[i] == "}":
            brace_depth -= 1
            if brace_depth < 0:
                errors.append(f"{path}:{line}: unmatched '}}'")
                brace_depth = 0
        i += 1
    if in_comment:
        errors.append(f"{path}: unterminated comment opened at line {comment_start_line}")
    if brace_depth > 0:
        errors.append(f"{path}: {brace_depth} unclosed '{{' by end of file")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--asset-root",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parent.parent / "src" / "bin" / "Data" / "Interface" / "RmlUi",
        help="Root directory to scan for .rml/.rcss files (default: src/bin/Data/Interface/RmlUi)",
    )
    args = parser.parse_args()

    if not args.asset_root.is_dir():
        print(f"check_rml_rcss_syntax: asset root not found: {args.asset_root}", file=sys.stderr)
        return 1

    errors: list[str] = []
    for path in sorted(args.asset_root.rglob("*.rml")):
        errors.extend(check_rml(path))
    for path in sorted(args.asset_root.rglob("*.rcss")):
        errors.extend(check_rcss(path))

    if errors:
        print("RML/RCSS syntax check failed:", file=sys.stderr)
        for e in errors:
            print(f"  {e}", file=sys.stderr)
        return 1

    rml_count = len(list(args.asset_root.rglob("*.rml")))
    rcss_count = len(list(args.asset_root.rglob("*.rcss")))
    print(f"RML/RCSS syntax check: OK ({rml_count} .rml, {rcss_count} .rcss)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
