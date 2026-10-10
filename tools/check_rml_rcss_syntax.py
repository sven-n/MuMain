#!/usr/bin/env python3
"""RML/RCSS syntax guard.

RmlUi's own parser doesn't error on most syntax mistakes in these two formats --
it silently drops or misparses the offending content and keeps going, so a
broken file (or a broken shared file linked by many windows) can go unnoticed
for a long time, surfacing only as unrelated-looking runtime symptoms in
whichever windows happen to load it. This script catches what the real parser
won't tell you about, ahead of time:

- RML: unterminated/nested `<!-- -->` comments, and balanced element nesting.
- RCSS: balanced `{`/`}`, and unterminated/nested `/* */` comments.

Both comment syntaxes share the same trap: they don't nest. `/* ... "/* x */"
... */` (or the `<!-- -->` equivalent) closes at the FIRST close-marker, not
the intended one, so everything between that premature close and the next
real close-marker is parsed as garbage markup/CSS -- with no error, just
silent misbehavior. A comment written this way in a shared base.rcss once
broke every window that linked it in one shot (see engine-findings.md).

Element nesting is checked because an unbalanced `</div>` fails the same silent
way: RmlUi closes whatever is open and carries on, so a stray closer ends an
ancestor early and every following sibling escapes the container it was written
in. A `data-if` gate one level up stops applying, and absolutely-positioned
children resolve against a different box -- which reads as "the wrong panel is
visible" and "a panel is drawn off the window", not as a markup error. One
leftover closing tag from a removed wrapper did exactly that to guild_info.rml.

Still deliberately not a full RML/RCSS grammar validator (no XML well-formedness
check, no property/tag validation) -- this codebase's RML comments routinely
contain a literal `--` for prose (technically invalid per the XML spec, but
RmlUi's actual parser tolerates it fine), so a strict validator would flag
most files for a non-issue. This only catches the one structural failure
modes that have actually gone unnoticed here before.

Usage: python3 check_rml_rcss_syntax.py [--asset-root DIR]
Exit code 0 = clean, 1 = violation(s) found (printed to stderr).
"""
import argparse
import pathlib
import re
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
    errors.extend(check_rml_nesting(path, text))
    return errors


# RmlUi accepts these without a closing tag; everything else must be closed or self-closed.
VOID_ELEMENTS = frozenset(
    {"br", "img", "input", "meta", "link", "hr", "source", "track", "area", "base", "col",
     "embed", "param", "wbr"}
)

TAG_RE = re.compile(r"""<(/?)([a-zA-Z][\w-]*)((?:[^>"']|"[^"]*"|'[^']*')*?)(/?)>""", re.S)


def _line_indent(text: str, pos: int) -> str | None:
    """The whitespace before `pos` on its own line, or None if something else precedes it."""
    start = text.rfind("\n", 0, pos) + 1
    prefix = text[start:pos]
    return prefix if prefix.strip() == "" else None


def check_rml_nesting(path: pathlib.Path, text: str) -> list[str]:
    # Blank the comments but keep line numbering, so a tag inside a comment is not counted.
    stripped = re.sub(r"<!--.*?-->", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    errors: list[str] = []
    stack: list[tuple[str, int, str | None]] = []
    # Closers whose indentation disagrees with the element they closed. On their own these are
    # only a formatting smell, so they are reported only when the file is also unbalanced --
    # where they are the best available pointer at which closer is the extra one, since a stray
    # </div> among same-named tags pops the innermost match and only shows up at EOF.
    suspects: list[str] = []
    for match in TAG_RE.finditer(stripped):
        closing, tag, _attrs, selfclose = match.groups()
        if tag.lower() in VOID_ELEMENTS or selfclose:
            continue
        line = stripped.count("\n", 0, match.start()) + 1
        indent = _line_indent(stripped, match.start())
        if not closing:
            stack.append((tag, line, indent))
            continue
        if stack and stack[-1][0] == tag:
            open_tag, open_line, open_indent = stack.pop()
            if (
                open_line != line
                and indent is not None
                and open_indent is not None
                and indent != open_indent
            ):
                suspects.append(
                    f"{path}:{line}: '</{tag}>' is indented differently from the "
                    f"'<{open_tag}>' it closes (line {open_line}) -- likely the extra closer"
                )
            continue
        if stack:
            open_tag, open_line, _ = stack[-1]
            errors.append(
                f"{path}:{line}: stray '</{tag}>' -- the innermost open element is "
                f"'<{open_tag}>' from line {open_line}. RmlUi closes that one here instead, "
                "so every following sibling leaves the container it was written in: a "
                "data-if above stops gating them, and absolute positions resolve against "
                "another box"
            )
        else:
            errors.append(
                f"{path}:{line}: stray '</{tag}>' with nothing open -- it closes the "
                "document body early"
            )
    for tag, line, _ in stack:
        errors.append(f"{path}:{line}: '<{tag}>' is never closed")
    if errors:
        errors.extend(suspects)
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
