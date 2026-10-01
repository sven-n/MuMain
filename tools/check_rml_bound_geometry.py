#!/usr/bin/env python3
"""Inline-geometry ownership guard.

`data-style-*` writes an *inline* property, and this RmlUi build resolves inline
properties ahead of every stylesheet rule with no `!important` for a theme to reach
for (`ElementStyle::GetLocalProperty()`, and see docs/rmlui-ui-system/engine-findings.md).
So a `left`/`top`/`width`/`height` bound from a C++ model is not merely awkward to
restyle -- it is a property **no theme, mod or user stylesheet can override at all**,
and the failure mode is silence: the theme author's rule simply has no effect, with no
parse error and no log line.

A plain `style="left: 16px"` attribute in the markup is the same inline property by the
same mechanism, and so is just as unreachable. It is easy to miss, because it looks like
ordinary authoring rather than a binding, and a scan for C++ presentation members will
not see it at all: gens_ranking.rml placed its rank strip, description box and scroll
track this way. Both forms are checked here.

That makes it worth a mechanical check, because the tree currently holds two
populations of document that are indistinguishable from the outside:

  - documents whose layout lives in RCSS, addressed by id or class, where an RCSS edit
    works (e.g. mu_helper_config.rml);
  - documents ported by transcribing a native Render() into the model, where the whole
    layout arrives as bound coordinates and an RCSS edit does nothing
    (e.g. guard_window.rml).

This script does NOT try to shrink the second set -- retrofitting it is a re-port, not
a cleanup (architecture-principles.md §26). It freezes it: every document that binds
geometry today is listed, with a reason, in rml_bound_geometry_allowlist.txt, and a
document that starts binding geometry without being listed fails the build. New work
therefore cannot quietly join the second population, and the list is a reviewed
inventory rather than an implicit one.

Deliberately allowed everywhere, unlisted: expressions that reference only the root
transform (`root_x`, `root_y`, `root_scale`, and the older `panel_x`/`panel_y` spelling
of the same `m_Pos` placement). That pair *is* the scaling bridge -- the panel's own
placement and the `.sharp-text` counter-scale -- and is not something a theme should be
overriding. Everything else needs a line in the allowlist.

Also deliberately narrow: this checks the four box offsets and the two sizes only. A
bound `color`, `decorator` or `font-size` has the same override problem, but those are
a judgement call per case (a per-frame fade, a native text metric), whereas a bound
static coordinate is nearly always layout that belongs in RCSS. Widen it when a bound
colour actually bites, not speculatively.

Usage: python3 check_rml_bound_geometry.py [--asset-root DIR] [--allowlist FILE]
Exit code 0 = clean, 1 = an unlisted document binds geometry (printed to stderr).
"""
import argparse
import pathlib
import re
import sys

# The properties this guard covers, and the whole of what it covers.
GEOMETRY_PROPERTIES = "left|top|right|bottom|width|height"
GEOMETRY_BINDING_RE = re.compile(
    r'data-style-(%s)\s*=\s*"([^"]*)"' % GEOMETRY_PROPERTIES
)
# A style attribute, and the geometry declarations inside one. `style` matched on its own
# word boundary so `data-style-left` does not also land here.
STYLE_ATTRIBUTE_RE = re.compile(r'(?<![-\w])style\s*=\s*"([^"]*)"')
STYLE_GEOMETRY_RE = re.compile(r'(?:^|;)\s*(%s)\s*:' % GEOMETRY_PROPERTIES)
# Anything that could be a model field reference. Dotted forms (`b.left`, `t.label_left`)
# and loop variables both land here, which is what we want -- they are all model data.
IDENTIFIER_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_.]*")
# String literals inside an expression are units ('px', 'dp'), never field names.
STRING_LITERAL_RE = re.compile(r"'[^']*'|\"[^\"]*\"")

# The scaling bridge: a document may place and counter-scale itself without being listed.
# panel_x/panel_y are the same m_Pos placement under an older name, used by the windows that
# place themselves without a root scale; one spelling should win, which is a separate tidy-up.
ROOT_TRANSFORM_FIELDS = {"root_x", "root_y", "root_scale", "panel_x", "panel_y"}


def bound_fields(expression):
    """Model fields an expression reads, ignoring units and numeric literals."""
    return {
        name
        for name in IDENTIFIER_RE.findall(STRING_LITERAL_RE.sub(" ", expression))
        if not name.replace(".", "").isdigit()
    }


def offending_fields(text):
    """What this document puts out of a theme's reach: every non-root-transform field bound
    to a geometry property, and every geometry property set by a style attribute."""
    found = set()
    for _property, expression in GEOMETRY_BINDING_RE.findall(text):
        found |= bound_fields(expression) - ROOT_TRANSFORM_FIELDS
    for declarations in STYLE_ATTRIBUTE_RE.findall(text):
        found |= {"style=" + name for name in STYLE_GEOMETRY_RE.findall(declarations)}
    return found


def read_allowlist(path):
    """{relative posix path: reason}. Blank lines and # comments ignored."""
    entries = {}
    if not path.is_file():
        return entries
    for line_number, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if ":" not in line:
            sys.stderr.write(
                "%s:%d: expected '<path>: <reason>', got %r\n" % (path, line_number, raw)
            )
            continue
        name, reason = line.split(":", 1)
        entries[name.strip()] = reason.strip()
    return entries


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--asset-root", default="src/bin/Data/Interface/RmlUi")
    parser.add_argument(
        "--allowlist", default=str(pathlib.Path(__file__).with_name("rml_bound_geometry_allowlist.txt"))
    )
    parser.add_argument(
        "--review",
        action="store_true",
        help="print every listed document with the fields it actually binds, beside its reason, "
        "and exit 0. An entry whose reason does not describe those fields has gone stale: the "
        "document can keep needing its entry while the reason stops being true, which the "
        "unneeded-entry report cannot catch.",
    )
    args = parser.parse_args()

    asset_root = pathlib.Path(args.asset_root)
    if not asset_root.is_dir():
        sys.stderr.write("asset root not found: %s\n" % asset_root)
        return 1

    allowlist = read_allowlist(pathlib.Path(args.allowlist))
    documents = sorted(asset_root.rglob("*.rml"))

    unlisted = []
    listed_and_binding = {}
    for document in documents:
        relative = document.relative_to(asset_root).as_posix()
        fields = offending_fields(document.read_text(encoding="utf-8", errors="replace"))
        if not fields:
            continue
        if relative in allowlist:
            listed_and_binding[relative] = sorted(fields)
        else:
            unlisted.append((relative, sorted(fields)))

    if args.review:
        for relative, fields in sorted(listed_and_binding.items()):
            print("%s\n  binds:  %s\n  reason: %s\n" % (relative, ", ".join(fields), allowlist[relative]))
        for relative, fields in unlisted:
            print("%s\n  binds:  %s\n  reason: -- NOT LISTED --\n" % (relative, ", ".join(fields)))
        return 0

    stale = sorted(set(allowlist) - set(listed_and_binding))
    if stale:
        # Good news, not a failure: these documents stopped binding geometry. Printed every
        # build so the inventory shrinks as work lands instead of quietly over-covering.
        print(
            "RML inline-geometry guard: %d allowlist entr%s no longer needed, delete: %s"
            % (len(stale), "y is" if len(stale) == 1 else "ies are", ", ".join(stale))
        )

    if unlisted:
        sys.stderr.write(
            "RML inline-geometry guard: %d document(s) put layout out of a theme's reach "
            "without being allowlisted.\n\n"
            "A bound left/top/width/height, and a style= attribute setting one, are both "
            "inline properties, which no theme can\n"
            "override -- see this script's docstring and "
            "docs/rmlui-ui-system/building-new-ui.md's Ownership section. Place static\n"
            "layout in RCSS, by id or class.\n\n"
            "If the geometry genuinely varies with data per frame, add a line to %s with the "
            "reason.\n\n" % (len(unlisted), args.allowlist)
        )
        for relative, fields in unlisted:
            sys.stderr.write("  %s -> %s\n" % (relative, ", ".join(fields)))
        return 1

    print(
        "RML inline-geometry guard: OK (%d .rml checked, %d allowlisted)"
        % (len(documents), len(listed_and_binding))
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
