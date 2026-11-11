#!/usr/bin/env python3
"""One-off migration: convert `<value>; /* token-name */`-tagged literals in
themes/modern/*.rcss into real `token(name)` references (RmlTheme.cpp's
InlineTokenizedStylesheet(), resolved against themes/modern/tokens.ini).

Conservative on purpose -- only converts a line when:
  1. The trailing comment's first word exactly matches a key actually defined
     in tokens.ini (guards against a prose comment like "cool steel-gray
     frame border, was warm bronze" or a derived-value note like "accent-steel
     wash" being misread as a token reference).
  2. The value immediately before the comment is a color literal (#hex or
     rgba(...)) -- replaced in place, so a compound value like
     `1px rgba(140, 146, 152, 130)` keeps its "1px" and only the color
     becomes `token(border-metal)`.
  3. OR the value is a bare `<number><dp|px>` (border-radius's shape) tagged
     with a `radius-*` token -- tokens.ini stores radius values unitless, so
     the unit suffix is kept literally after the token() call
     (`token(radius-sm)dp`), preserving each file's own existing dp-vs-px
     choice instead of forcing one.

Anything that doesn't match either shape (multi-word comments, a token name
not in tokens.ini, an already-token()-ized line) is left untouched and
reported, for manual review -- this script does not try to be exhaustive.

Usage: python3 migrate_rcss_tokens.py [--theme-dir DIR] [--tokens-ini PATH] [--apply]
Without --apply, prints what would change without writing anything.
"""
import argparse
import configparser
import pathlib
import re
import sys

COLOR_RE = re.compile(
    r"(?P<value>#[0-9a-fA-F]{3,8}|rgba\([^)]*\))"
    r";\s*/\*\s*(?P<token>[a-zA-Z][a-zA-Z0-9-]*)\b[^*]*\*/"
)
RADIUS_RE = re.compile(
    r"(?P<num>\d+)(?P<unit>dp|px)"
    r";\s*/\*\s*(?P<token>radius-[a-zA-Z0-9-]*)\b[^*]*\*/"
)


def load_token_names(tokens_ini: pathlib.Path) -> set[str]:
    parser = configparser.ConfigParser()
    # ConfigParser lowercases keys by default, which matches this project's
    # all-lowercase token-name convention already.
    with tokens_ini.open(encoding="utf-8") as f:
        parser.read_string(f.read())
    return set(parser["Tokens"].keys())


def migrate_text(text: str, token_names: set[str]) -> tuple[str, list[str]]:
    skipped: list[str] = []

    def replace_color(m: re.Match) -> str:
        if m.group("token") not in token_names:
            skipped.append(f"comment '{m.group('token')}' not in tokens.ini: ...{m.group(0)!r}")
            return m.group(0)
        return f"token({m.group('token')});"

    def replace_radius(m: re.Match) -> str:
        if m.group("token") not in token_names:
            skipped.append(f"comment '{m.group('token')}' not in tokens.ini: ...{m.group(0)!r}")
            return m.group(0)
        return f"token({m.group('token')}){m.group('unit')};"

    text = COLOR_RE.sub(replace_color, text)
    text = RADIUS_RE.sub(replace_radius, text)
    return text, skipped


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "--theme-dir", type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parent.parent
        / "src" / "bin" / "Data" / "Interface" / "RmlUi" / "themes" / "modern",
    )
    parser.add_argument("--tokens-ini", type=pathlib.Path, default=None)
    parser.add_argument("--apply", action="store_true", help="write changes; default is dry-run")
    args = parser.parse_args()

    tokens_ini = args.tokens_ini or (args.theme_dir / "tokens.ini")
    token_names = load_token_names(tokens_ini)
    print(f"Loaded {len(token_names)} token names from {tokens_ini}")

    any_skipped = False
    for rcss_path in sorted(args.theme_dir.glob("*.rcss")):
        original = rcss_path.read_text(encoding="utf-8")
        migrated, skipped = migrate_text(original, token_names)
        if migrated != original:
            print(f"{'APPLY' if args.apply else 'WOULD CHANGE'}: {rcss_path}")
            if args.apply:
                rcss_path.write_text(migrated, encoding="utf-8")
        for s in skipped:
            any_skipped = True
            print(f"  SKIPPED in {rcss_path.name}: {s}")

    if any_skipped:
        print("\nSome lines were left untouched -- review the SKIPPED lines above by hand.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
