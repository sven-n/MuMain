"""Write the glyph masks as an OpenType (CFF) font: traced into curves and slanted back to the italic."""
import math

import numpy as np
import potrace
from fontTools.agl import AGL2UV, UV2AGL
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.t2CharStringPen import T2CharStringPen

from letters import BASELINE, SLANT, X_HEIGHT, blur

FAMILY = "MU Map Name"
UNITS_PER_EM = 1000
X_HEIGHT_UNITS = 400
UNITS = X_HEIGHT_UNITS / X_HEIGHT       # font units per mask pixel
EDGE_SMOOTHING = 2.0    # mask pixels: rounds off the steps of cut and pen-drawn shapes
OTHER_GAP = 70          # font units around figures and punctuation; letters keep the originals' spacing
MIN_SPACE = 160         # the originals squeeze word gaps to fit the image; keep a readable space
MIN_ADVANCE = 60
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


def outline(mask, gap):
    """Contours in font units and the advance: the body starts half a gap after the origin and
    the next glyph half a gap after the body ends."""
    smooth = blur(mask.astype(np.float32), EDGE_SMOOTHING) > 0.5
    left, right = body_edges(smooth)
    advance = (right - left) * UNITS + gap

    def point(p):
        x, y = (p.x - left) * UNITS + gap / 2, (BASELINE - p.y) * UNITS
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


def write_font(glyph_masks, letter_gap, word_gap, cap_height, out_path):
    """glyph_masks: glyph name (or the character itself) -> mask. Gaps in mask pixels."""
    entries = {}
    for name, mask in glyph_masks.items():
        code = code_of(name)
        gap = letter_gap * UNITS + (0 if chr(code).isalpha() else OTHER_GAP)
        entries[code] = (glyph_name(code), *outline(mask, gap))
    space = int(round(max(((word_gap or 0) - letter_gap) * UNITS, MIN_SPACE)))

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
