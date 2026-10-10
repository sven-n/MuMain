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

A document may bind geometry only when the geometry is the state itself -- a gauge's
length, something following the pointer or a projected point, a size the user dragged --
and it says so where it does it: a `<!-- bound-geometry: <why> -->` comment in the
document. A document that binds geometry without one fails the build, so new work cannot
quietly push layout from C++, and the reason sits beside the bindings it explains. A
marker on a document that no longer binds geometry fails too: delete it.

Deliberately allowed everywhere, unlisted: expressions that reference only the root
transform (`root_x`, `root_y`, `root_scale`, and the older `panel_x`/`panel_y` spelling
of the same `m_Pos` placement, and a dialog's `canvas_top`). That set *is* the scaling bridge -- the panel's own
placement -- and is not something a theme should be overriding. Everything else needs the
document's marker.

Never allowed, marked or not: a number scaled by `root_scale` (or a Hud stretch's
`scale_x`/`scale_y`) in any bound style, such as `(160 * root_scale) + 'px'` or
`scale(1 / root_scale)`. That is a counter-scaled layer's length or transform, which the
theme states in RCSS: `calc(160px * var(--root-scale))`, and base.rcss's `.sharp-text` /
`.counter-scaled`.

A custom property bound with a length unit (`data-style---w="width + 'px'"`) is checked
the same way: the theme's calc() makes it a box, so it is the same geometry by another
route. The native text metrics (`--text-px`, `--line-px`, `--line-height`, ...) are exempt,
as `text_px` is for a font size; an index, a count or a fraction carries no unit and is data.

Also deliberately narrow: this checks the four box offsets and the two sizes only. A
bound `color`, `decorator` or `font-size` has the same override problem, but those are
a judgement call per case (a per-frame fade, a native text metric), whereas a bound
static coordinate is nearly always layout that belongs in RCSS. Widen it when a bound
colour actually bites, not speculatively.

Usage: python3 check_rml_bound_geometry.py [--asset-root DIR] [--review]
Exit code 0 = clean, 1 = an unmarked document binds geometry, or a marker is stale (stderr).
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
ROOT_TRANSFORM_FIELDS = {"root_x", "root_y", "root_scale", "panel_x", "panel_y", "canvas_top"}

# A custom property bound with a length unit is geometry too -- the theme's calc() turns it into a
# box -- unless it is a native text metric, which a theme sizes rows from the way it sizes text
# from text_px. An index, a count or a fraction carries no unit and is data.
CUSTOM_PROPERTY_BINDING_RE = re.compile(r'data-style---([\w-]+)\s*=\s*"([^"]*)"')
LENGTH_UNIT_RE = re.compile(r"'(?:px|dp|%)'")
NATIVE_METRIC_PROPERTIES = {"text-px", "line-px", "bold-line-px", "row-px", "line-height", "bold-line-height"}

# Any bound style, and a numeric literal outside its string literals.
STYLE_BINDING_RE = re.compile(r'data-style-([\w-]+)\s*=\s*"([^"]*)"')
NUMBER_RE = re.compile(r"(?<![\w.])\d+(?:\.\d+)?")


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
    for name, expression in CUSTOM_PROPERTY_BINDING_RE.findall(text):
        if name not in NATIVE_METRIC_PROPERTIES and LENGTH_UNIT_RE.search(expression):
            found |= bound_fields(expression) - ROOT_TRANSFORM_FIELDS
    for declarations in STYLE_ATTRIBUTE_RE.findall(text):
        found |= {"style=" + name for name in STYLE_GEOMETRY_RE.findall(declarations)}
    return found


def scaled_literals(text):
    """Bound styles that scale a number by the root scale: lengths the theme should state."""
    found = []
    for name, expression in STYLE_BINDING_RE.findall(text):
        code = STRING_LITERAL_RE.sub(" ", expression)
        if re.search(r"\b(root_scale(_y)?|scale_[xy])\b", code) and NUMBER_RE.search(code):
            found.append("%s=\"%s\"" % (name, expression))
    return found


# The marker a document gives its reason in: <!-- bound-geometry: <why> -->.
MARKER_RE = re.compile(r"<!--\s*bound-geometry:\s*(.*?)\s*-->", re.S)


def marker_reason(text):
    """The document's bound-geometry reason, one line, or None."""
    match = MARKER_RE.search(text)
    return " ".join(match.group(1).split()) if match else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--asset-root", default="src/bin/Data/Interface/RmlUi")
    parser.add_argument(
        "--review",
        action="store_true",
        help="print every marked document with the fields it actually binds, beside its reason, "
        "and exit 0. A reason that does not describe those fields has gone stale: the document "
        "can keep needing its marker while the reason stops being true, which the stale-marker "
        "check cannot catch.",
    )
    args = parser.parse_args()

    asset_root = pathlib.Path(args.asset_root)
    if not asset_root.is_dir():
        sys.stderr.write("asset root not found: %s\n" % asset_root)
        return 1

    documents = sorted(asset_root.rglob("*.rml"))

    unmarked = []
    marked = {}
    stale = []
    scaled = []
    for document in documents:
        relative = document.relative_to(asset_root).as_posix()
        text = document.read_text(encoding="utf-8", errors="replace")
        scaled += [(relative, binding) for binding in scaled_literals(text)]
        fields = offending_fields(text)
        reason = marker_reason(text)
        if not fields:
            if reason is not None:
                stale.append(relative)
            continue
        if reason:
            marked[relative] = (sorted(fields), reason)
        else:
            unmarked.append((relative, sorted(fields)))

    if args.review:
        for relative, (fields, reason) in sorted(marked.items()):
            print("%s\n  binds:  %s\n  reason: %s\n" % (relative, ", ".join(fields), reason))
        for relative, fields in unmarked:
            print("%s\n  binds:  %s\n  reason: -- NOT MARKED --\n" % (relative, ", ".join(fields)))
        print("%d marked, %d unmarked" % (len(marked), len(unmarked)))
        return 0

    failed = False
    if stale:
        sys.stderr.write(
            "RML inline-geometry guard: %d document(s) keep a bound-geometry marker but bind no "
            "geometry any more; delete the marker: %s\n\n" % (len(stale), ", ".join(stale))
        )
        failed = True

    if scaled:
        sys.stderr.write(
            "RML inline-geometry guard: %d binding(s) scale a number by root_scale. A counter-scaled "
            "layer's lengths belong\nin RCSS as calc(Npx * var(--root-scale)), and its transform is "
            "base.rcss's .sharp-text / .counter-scaled.\n\n" % len(scaled)
        )
        for relative, binding in scaled:
            sys.stderr.write("  %s -> %s\n" % (relative, binding))
        failed = True

    if unmarked:
        sys.stderr.write(
            "RML inline-geometry guard: %d document(s) put layout out of a theme's reach.\n\n"
            "A bound left/top/width/height, a style= attribute setting one, and a custom property "
            "bound with a length unit are\ninline geometry, which no theme can override -- see this "
            "script's docstring and docs/rmlui-ui-system/building-new-ui.md's\nOwnership section. "
            "Place static layout in RCSS, by id or class.\n\n"
            "If the geometry is the state itself (a gauge, something following the pointer, a "
            "projected point), say so in the\ndocument: <!-- bound-geometry: <why> -->.\n\n"
            % len(unmarked)
        )
        for relative, fields in unmarked:
            sys.stderr.write("  %s -> %s\n" % (relative, ", ".join(fields)))
        failed = True

    if failed:
        return 1
    print("RML inline-geometry guard: OK (%d .rml checked, %d marked)" % (len(documents), len(marked)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
