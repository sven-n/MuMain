"""Write the glyph masks as an OpenType (CFF) font: traced into curves, slanted back to the italic,
and spaced like the originals."""
import math
import unicodedata

import numpy as np
import potrace
from fontTools.agl import AGL2UV, UV2AGL
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.t2CharStringPen import T2CharStringPen

from letters import BASELINE, SLANT, X_HEIGHT, blur
from recipes import CYRILLIC_FROM_LATIN

FAMILY = "MU Map Name"
UNITS_PER_EM = 1000
X_HEIGHT_UNITS = 400
UNITS = X_HEIGHT_UNITS / X_HEIGHT       # font units per mask pixel
EDGE_SMOOTHING = 2.0    # mask pixels: rounds off the steps of cut and pen-drawn shapes
EXTRA_ROOM = 4          # mask pixels on each side of every letter, a little more air than the originals
OTHER_ROOM = 17.5       # mask pixels on each side of figures and punctuation
MIN_SPACE = 160         # font units: the originals squeeze word gaps to fit the image
MIN_ADVANCE = 60
# Letters spaced like another: their base letter (é like e), or the Latin letter they are drawn as.
SPACING_ALIASES = {"ł": "l", "Ł": "L", "ø": "o", "Ø": "O", "ı": "i", "ȷ": "j", **CYRILLIC_FROM_LATIN}
EXTRA_CODES = {"dotlessj": 0x0237}
TRACE = dict(turdsize=40, alphamax=1.15, opticurve=True, opttolerance=0.5)


def code_of(name):
    if len(name) == 1:
        return ord(name)
    return EXTRA_CODES.get(name) or AGL2UV[name]


def glyph_name(code):
    return UV2AGL.get(code, f"uni{code:04X}")


def body_edges(mask):
    """Ink columns of the x-height band, or of the whole glyph when it has none (quotes)."""
    cols = np.nonzero(mask[BASELINE - X_HEIGHT:BASELINE].any(axis=0))[0]
    if len(cols) == 0:
        cols = np.nonzero(mask.any(axis=0))[0]
    return cols[0], cols[-1] + 1


def side_room(ch, bearings):
    """Room left and right of the glyph's body, in mask pixels."""
    base = unicodedata.normalize("NFD", ch)[0]
    base = SPACING_ALIASES.get(base, base)
    if not base.isalpha():
        return OTHER_ROOM, OTHER_ROOM
    if base in bearings:
        left, right = bearings[base]
    else:
        same_case = [b for c, b in bearings.items() if c.isupper() == base.isupper()]
        left, right = np.median(same_case, axis=0)
    return left + EXTRA_ROOM, right + EXTRA_ROOM


def outline(mask, room_left, room_right):
    """Contours in font units and the advance: the body starts room_left after the origin and the
    next glyph room_right after the body ends (mask pixels)."""
    smooth = blur(mask.astype(np.float32), EDGE_SMOOTHING) > 0.5
    left, right = body_edges(smooth)
    advance = (right - left + room_left + room_right) * UNITS

    def point(p):
        x, y = (p.x - left + room_left) * UNITS, (BASELINE - p.y) * UNITS
        return round(x + SLANT * y, 1), round(y, 1)

    contours = []
    for curve in potrace.Bitmap(~smooth).trace(**TRACE):
        ops = []
        for seg in curve.segments:
            if seg.is_corner:
                ops += [("line", point(seg.c)), ("line", point(seg.end_point))]
            else:
                ops.append(("curve", point(seg.c1), point(seg.c2), point(seg.end_point)))
        contours.append((point(curve.start_point), ops))
    return contours, int(round(max(advance, MIN_ADVANCE)))


def charstring(contours, advance):
    pen = T2CharStringPen(advance, None)
    for start, ops in contours:
        pen.moveTo(start)
        for op in ops:
            if op[0] == "line":
                pen.lineTo(op[1])
            else:
                pen.curveTo(*op[1:])
        pen.closePath()
    return pen.getCharString()


def notdef():
    pen = T2CharStringPen(500, None)
    for box in (((60, 0), (440, 0), (440, 620), (60, 620)), ((100, 40), (100, 580), (400, 580), (400, 40))):
        pen.moveTo(box[0])
        for p in box[1:]:
            pen.lineTo(p)
        pen.closePath()
    return pen.getCharString()


def write_font(glyph_masks, bearings, half_gap, word_gap, cap_height, out_path):
    """glyph_masks: glyph name (or the character itself) -> mask. bearings: traced letter -> room
    left and right of its body; half_gap: the typical room; word_gap: room between words. All in
    mask pixels."""
    entries = {}
    for name, mask in glyph_masks.items():
        code = code_of(name)
        entries[code] = (glyph_name(code), *outline(mask, *side_room(chr(code), bearings)))
    space = int(round(max(((word_gap or 0) - 2 * half_gap) * UNITS, MIN_SPACE)))

    order = [".notdef", "space", "uni00A0"] + [entries[c][0] for c in sorted(entries)]
    charstrings = {".notdef": notdef()}
    metrics = {".notdef": (500, 60)}
    for name in ("space", "uni00A0"):
        charstrings[name], metrics[name] = T2CharStringPen(space, None).getCharString(), (space, 0)
    ys = [0.0]
    for name, contours, advance in entries.values():
        charstrings[name] = charstring(contours, advance)
        xs = [x for _, ops in contours for op in ops for x, _ in op[1:]]
        ys += [y for _, ops in contours for op in ops for _, y in op[1:]]
        metrics[name] = (advance, int(round(min(xs))) if xs else 0)
    ascent, descent = int(math.ceil(max(ys))), int(math.floor(min(ys)))

    fb = FontBuilder(UNITS_PER_EM, isTTF=False)
    fb.setupGlyphOrder(order)
    fb.setupCharacterMap({0x20: "space", 0xA0: "uni00A0", **{code: e[0] for code, e in entries.items()}})
    fb.setupCFF(FAMILY.replace(" ", ""), {"FullName": f"{FAMILY} Italic"}, charstrings, {})
    fb.setupHorizontalMetrics(metrics)
    fb.setupHorizontalHeader(ascent=ascent, descent=descent, lineGap=0)
    fb.setupNameTable({"familyName": FAMILY, "styleName": "Italic",
                       "description": "The MU client's map-name lettering, traced from its English map-name "
                                      "images and completed from their parts (tools/MapNameFont)."})
    fb.setupOS2(sTypoAscender=ascent, sTypoDescender=descent, sTypoLineGap=0, usWinAscent=ascent,
                usWinDescent=-descent, sxHeight=X_HEIGHT_UNITS, sCapHeight=int(round(cap_height * UNITS)),
                fsSelection=0x01, usWeightClass=400)
    fb.updateHead(macStyle=0x02)
    fb.setupPost(italicAngle=-math.degrees(math.atan(SLANT)))
    fb.save(out_path)
    return entries
