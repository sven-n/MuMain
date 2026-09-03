#!/usr/bin/env python3
"""Theme-fork drift guard.

`UI::RmlBridge::LoadThemedDocument()` lets any theme fork a window's RML entirely
(`themes/<theme>/<name>.rml`, falling back to the shared `<name>.rml` when no fork
exists -- see docs/rmlui-ui-system/theming-and-modding.md's "Forking a theme's RML"
section). Whichever copy actually loads, the owning C++ still expects the exact
same ids (`GetElementById`), data-model field names (`DataModelConstructor::Bind`),
and event-callback names (`DataModelConstructor::BindEventCallback`) to exist in it.
Nothing enforces that a forked copy still satisfies that contract -- a missing or
renamed name fails completely silently (a dead button, not a build error or even a
log line), the same failure shape documented for `main_frame`'s existing two-file
case in docs/rmlui-ui-system/STATUS.md.

This script:

1. Scans every .cpp under --source-root for a `LoadThemedDocument(context,
   "Data/Interface/RmlUi/<name>.rml")` call -- that's the RML document a file owns.
2. From that same file, collects every `.Bind("...")`, `.BindEventCallback("...")`,
   and `GetElementById("...")` string literal -- that's what C++ needs to exist.
3. For that document, finds every copy that can actually load: the shared
   `<name>.rml` (if present) plus every `themes/*/<name>.rml` fork (if any --
   `main_frame` today has no shared file at all, only two forks).
4. Confirms every needed name appears somewhere in each copy's text.

Deliberately a plain substring/regex presence check, not a real RML/attribute-type
validator -- consistent with check_rml_rcss_syntax.py's own "catches the one
failure mode that's actually gone unnoticed here, not a full grammar validator"
scope. A document with no owning .cpp match (e.g. loading.rml, loaded directly via
Context::LoadDocument with no data model) is simply not checked -- nothing to
verify a static, binding-free document against.

Checked against the UNION of every theme copy's text, not each copy individually --
confirmed necessary by running this against main_frame, the one pre-existing forked
window, before trusting it: C++ deliberately computes multiple, non-overlapping
display-text fields for the same value (e.g. `hp_text` "935 / 935" vs legacy's own
`hp_current_text` "935" -- see NewUIMainFrameWindow.cpp and STATUS.md's worked
example), and each theme's own markup binds only the one it wants. A per-copy
requirement flags that legitimate pattern as drift; a union requirement still
catches the actual failure mode this script exists for (C++ references a name no
theme's RML anywhere still provides -- a renamed/removed id or binding) without
false-positiving on a theme simply not using every alternative C++ offers.

Usage: python3 check_rml_rcss_drift.py [--source-root DIR] [--asset-root DIR]
Exit code 0 = clean, 1 = drift found (printed to stderr).
"""
import argparse
import pathlib
import re
import sys

LOAD_THEMED_RE = re.compile(r'LoadThemedDocument\([^,]*,\s*"Data/Interface/RmlUi/([A-Za-z0-9_]+)\.rml"\)')
BIND_RE = re.compile(r'\.Bind\(\s*"([^"]+)"')
BIND_EVENT_RE = re.compile(r'\.BindEventCallback\(\s*"([^"]+)"')
GET_ELEMENT_RE = re.compile(r'GetElementById\(\s*"([^"]+)"')


def find_document_windows(source_root: pathlib.Path) -> dict[str, set[str]]:
    """Maps each RML document name (e.g. "login") to the union of ids/bound field
    names/event-callback names its owning .cpp file(s) reference."""
    windows: dict[str, set[str]] = {}
    for path in sorted(source_root.rglob("*.cpp")):
        text = path.read_text(encoding="utf-8", errors="ignore")
        doc_names = set(LOAD_THEMED_RE.findall(text))
        if not doc_names:
            continue
        referenced = (
            set(BIND_RE.findall(text))
            | set(BIND_EVENT_RE.findall(text))
            | set(GET_ELEMENT_RE.findall(text))
        )
        for doc_name in doc_names:
            windows.setdefault(doc_name, set()).update(referenced)
    return windows


def theme_copies(asset_root: pathlib.Path, doc_name: str) -> list[pathlib.Path]:
    """The shared file (if it exists) plus every per-theme fork that exists for
    this document. main_frame today has zero shared file, only two forks --
    that's a valid, checkable state, not an error."""
    copies = []
    shared = asset_root / f"{doc_name}.rml"
    if shared.is_file():
        copies.append(shared)
    themes_dir = asset_root / "themes"
    if themes_dir.is_dir():
        for theme_dir in sorted(p for p in themes_dir.iterdir() if p.is_dir()):
            forked = theme_dir / f"{doc_name}.rml"
            if forked.is_file():
                copies.append(forked)
    return copies


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--source-root",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parent.parent / "src" / "source",
        help="Root directory to scan for .cpp files (default: src/source)",
    )
    parser.add_argument(
        "--asset-root",
        type=pathlib.Path,
        default=pathlib.Path(__file__).resolve().parent.parent / "src" / "bin" / "Data" / "Interface" / "RmlUi",
        help="Root directory holding shared .rml files and themes/ (default: src/bin/Data/Interface/RmlUi)",
    )
    args = parser.parse_args()

    if not args.source_root.is_dir():
        print(f"check_rml_rcss_drift: source root not found: {args.source_root}", file=sys.stderr)
        return 1
    if not args.asset_root.is_dir():
        print(f"check_rml_rcss_drift: asset root not found: {args.asset_root}", file=sys.stderr)
        return 1

    windows = find_document_windows(args.source_root)
    errors: list[str] = []
    checked = 0
    for doc_name, needed in sorted(windows.items()):
        copies = theme_copies(args.asset_root, doc_name)
        if not copies:
            errors.append(
                f"{doc_name}: C++ loads this document via LoadThemedDocument but no .rml file "
                f"exists for it anywhere (expected {args.asset_root / (doc_name + '.rml')} or a "
                f"themes/*/{doc_name}.rml fork)"
            )
            continue
        checked += len(copies)
        combined_text = "\n".join(
            copy_path.read_text(encoding="utf-8", errors="ignore") for copy_path in copies
        )
        missing = sorted(name for name in needed if name not in combined_text)
        if missing:
            copy_list = ", ".join(str(p) for p in copies)
            errors.append(
                f"'{doc_name}': {len(missing)} name(s) C++ references appear in none of its "
                f"RML copies ({copy_list}): {', '.join(missing)}"
            )

    if errors:
        print("RML/RCSS theme-fork drift check failed:", file=sys.stderr)
        for e in errors:
            print(f"  {e}", file=sys.stderr)
        return 1

    print(f"RML/RCSS theme-fork drift check: OK ({len(windows)} document(s), {checked} file(s) checked)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
