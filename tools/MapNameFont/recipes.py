"""Glyphs the original images don't contain, built from the traced letters and a broad-nib pen.

The images hold the capitals A-F, I, K, L, N, S, T, V and the small letters except j and x. The rest
are put together from parts of those (P from B's stem and bowl, R adds K's leg, O and U enlarge o
and u) or drawn with a pen whose nib makes the traced letters' stem and hairline widths.

Design units are x-heights: u to the right of the letters' left edge, v up from the baseline.
Capitals are as high as the traced B; figures FIG.
"""
import math

import numpy as np
from scipy import ndimage

from letters import X_HEIGHT, blur
from shapes import (Pen, body_u, bottom_v, col, empty, erase, extent_u, flip, largest, mirror, pieces,
                    place_left, region, rotate180, row, scale, shift, thicken, top_v)

PEN = None              # set by build(): a nib matched to the traced letters' strokes
PEN_EDGE = 8            # px: the nib's thickness
# A drawn stroke comes out this much wider than the nib's projection (anti-aliasing, round ends).
PEN_SPREAD_STEM, PEN_SPREAD_HAIR = 2, 3
SPIKE_RADIUS = 6        # px: points thinner than this are rounded off where parts are cut apart
JOIN_RADIUS = 8         # px: notches narrower than this are filled where a drawn stroke joins a traced one
FIG = 1.3               # figure height
MARK_GAP = 0.1          # space between a letter and its accent
SHARP_S_TOP = 1.6
# Cyrillic letters drawn like a Latin one; the italic forms of и, п, т, г are u, n, m, r.
CYRILLIC_FROM_LATIN = {
    "А": "A", "В": "B", "Е": "E", "К": "K", "М": "M", "Н": "H", "О": "O", "Р": "P", "С": "C", "Т": "T",
    "Х": "X", "У": "Y", "І": "I", "а": "a", "е": "e", "о": "o", "р": "p", "с": "c", "х": "x", "у": "y",
    "и": "u", "п": "n", "т": "m", "і": "i", "г": "r",
}


def elliptic_outline(mask):
    """The shape's outer outline replaced by its own ellipse (same area, tilt and spread, from
    its moments); its counter (hole) stays."""
    whole = ndimage.binary_fill_holes(mask)
    counter = whole & ~mask
    ys, xs = np.nonzero(whole)
    inverse = np.linalg.inv(np.cov(np.vstack([xs, ys])))
    grid_y, grid_x = np.mgrid[0:mask.shape[0], 0:mask.shape[1]]
    dx, dy = grid_x - xs.mean(), grid_y - ys.mean()
    distance = inverse[0, 0] * dx * dx + 2 * inverse[0, 1] * dx * dy + inverse[1, 1] * dy * dy
    # A filled ellipse with these moments has its edge at distance 4.
    return (distance <= 4.0) & ~counter


def pieces_of(mask):
    return pieces(mask)


def union_of(masks):
    out = empty()
    for m in masks:
        out |= m
    return out


def without_specks(mask, share=0.15):
    """Drop pieces smaller than `share` of the largest: leftovers of a cut."""
    parts = pieces(mask)
    biggest = max((p.sum() for p in parts), default=0)
    return union_of(p for p in parts if p.sum() >= share * biggest)


def disc(radius):
    y, x = np.mgrid[-radius:radius + 1, -radius:radius + 1]
    return x * x + y * y <= radius * radius


def smoothed(mask, radius):
    """Morphological opening with a disc: removes points and spurs thinner than 2 * radius."""
    return ndimage.binary_opening(mask, structure=disc(radius))


def filled(mask, radius):
    """Morphological closing with a disc: fills notches narrower than 2 * radius, such as the
    step where a drawn stroke meets a traced one."""
    padded = np.pad(mask, radius)
    closed = ndimage.binary_closing(padded, structure=disc(radius))
    return closed[radius:-radius, radius:-radius]


def evened(mask, sigma, radius=0):
    """Wavy edges evened out: spurs thinner than 2 * radius are cut off, the outline is blurred by
    sigma px, and notches left are filled. Strokes much wider than sigma keep their width."""
    if radius:
        mask = smoothed(mask, radius)
    return filled(blur(mask.astype(np.float32), sigma) > 0.5, max(radius, JOIN_RADIUS))


def pressure_for(width, start, end):
    """Nib length factor that draws a stroke `width` x-heights wide going from start to end."""
    direction = math.atan2(end[1] - start[1], end[0] - start[0])
    across = abs(math.sin(direction - PEN.angle))
    return (width * X_HEIGHT - PEN.edge - PEN_SPREAD_STEM) / (PEN.length * across)


def centre_line(mask, heights):
    """Points along the rightmost stroke: its middle at each height."""
    return [(sum(runs_at(mask, v)[-1]) / 2, v) for v in heights]


def stroke_width(mask, heights):
    """Mean width of the rightmost stroke at these heights."""
    return float(np.mean([right - left for left, right in (runs_at(mask, v)[-1] for v in heights)]))


def matched_pen(t):
    """A nib that draws the traced letters' stem (small letters) and hairline (o's top and bottom)."""
    def lengths(line):
        d = np.diff(np.concatenate([[0], line.astype(np.int8), [0]]))
        return np.nonzero(d == -1)[0] - np.nonzero(d == 1)[0]
    stem_widths = [w for ch in "ilnu" for v in np.arange(0.3, 0.7, 0.04) for w in lengths(t[ch][int(row(v))])]
    o = t["o"]
    middle = int(round(col(sum(extent_u(o)) / 2)))
    hair_widths = [w for x in range(middle - 10, middle + 11, 5) for w in lengths(o[:, x])]
    across = float(np.median(stem_widths)) - PEN_EDGE - PEN_SPREAD_STEM
    up = float(np.median(hair_widths)) - PEN_EDGE - PEN_SPREAD_HAIR
    return Pen(math.hypot(across, up), math.degrees(math.atan2(up, across)), PEN_EDGE)


# ---------------------------------------------------------------- measuring helpers

def runs_at(mask, v):
    r = int(round(row(v)))
    line = np.concatenate([[False], mask[r], [False]]).astype(np.int8)
    d = np.diff(line)
    starts, ends = np.nonzero(d == 1)[0], np.nonzero(d == -1)[0]
    return [((s - col(0)) / X_HEIGHT, (e - col(0)) / X_HEIGHT) for s, e in zip(starts, ends)]


def stems(mask, v_lo, v_hi, share=0.8, min_width=0.12):
    """Vertical strokes: groups of columns inked on at least `share` of the rows between v_lo
    and v_hi, as (left, right) pairs in x-heights. Thin swashes don't count."""
    r0, r1 = int(round(row(v_hi))), int(round(row(v_lo)))
    filled = mask[r0:r1].mean(axis=0) >= share
    d = np.diff(np.concatenate([[0], filled.astype(np.int8), [0]]))
    starts, ends = np.nonzero(d == 1)[0], np.nonzero(d == -1)[0]
    found = [((s - col(0)) / X_HEIGHT, (e - col(0)) / X_HEIGHT) for s, e in zip(starts, ends)]
    return [(a, b) for a, b in found if b - a >= min_width]


def stem_right(mask, v=None, v_lo=None, v_hi=None):
    """Right edge of the leftmost vertical stroke (of a capital, by default)."""
    cap = top_v(mask)
    left_stems = stems(mask, v_lo if v_lo is not None else 0.2 * cap, v_hi if v_hi is not None else 0.8 * cap)
    if not left_stems:
        left_stems = stems(mask, 0.3 * cap, 0.7 * cap, share=0.6)
    return left_stems[0][1]


def narrowest(mask, v_lo, v_hi):
    """Height between v_lo and v_hi where the shape reaches least far right (a B's waist)."""
    best_v, best_right = v_lo, 9.0
    for v in np.arange(v_lo, v_hi, 0.01):
        found = runs_at(mask, v)
        if found and found[-1][1] < best_right:
            best_v, best_right = v, found[-1][1]
    return best_v


def centre_at(mask, v):
    found = runs_at(mask, v)
    return (found[0][0] + found[0][1]) / 2


def body_centre(mask):
    left, right = body_u(mask)
    return (left + right) / 2


def ink_centre(mask):
    left, right = extent_u(mask)
    return (left + right) / 2


# ---------------------------------------------------------------- marks

def with_mark(base, mark, gap=MARK_GAP, dx=0.0, centre=None, below=False):
    """Put the mark centred over (or under) the base."""
    centre = ink_centre(base) if centre is None else centre
    mark_left, mark_right = extent_u(mark)
    du = centre + dx - (mark_left + mark_right) / 2
    if below:
        dv = bottom_v(base) - gap - top_v(mark)
    else:
        dv = top_v(base) + gap - bottom_v(mark)
    return base | shift(mark, du, dv)


def marks(t):
    dot = largest(region(t["i"], v0=1.12))
    def scaled_dot(size):
        return scale(dot, size, size, anchor_u=ink_centre(dot))
    small_dot = scaled_dot(0.85)
    width = extent_u(small_dot)[1] - extent_u(small_dot)[0]
    m = {
        "acute": PEN.stroke([(0.0, 0.0), (0.2, 0.22)], start=0.45, end=1.0),
        "grave": PEN.stroke([(0.0, 0.22), (0.2, 0.0)], start=1.0, end=0.45),
        "circumflex": PEN.stroke([(0.0, 0.0), (0.13, 0.2)], start=0.4, end=0.9)
                      | PEN.stroke([(0.13, 0.2), (0.27, 0.0)], start=0.9, end=0.4),
        "tilde": PEN.stroke([(0.0, 0.02), (0.08, 0.13), (0.17, 0.07), (0.25, 0.01), (0.33, 0.11)],
                            start=0.6, mid=0.8, end=0.6),
        "dieresis": small_dot | shift(small_dot, width + 0.1),
        "dot": dot,
        "ring": scale(t["o"], 0.42, 0.42, thin=0),
        "breve": PEN.stroke([(0.0, 0.17), (0.05, 0.04), (0.14, 0.0), (0.23, 0.04), (0.28, 0.17)],
                            start=0.5, mid=0.7, end=0.5),
        "macron": PEN.stroke([(0.0, 0.0), (0.3, 0.0)], start=0.7, end=0.7),
        "cedilla": PEN.stroke([(0.0, 0.05), (0.02, -0.08), (0.12, -0.12), (0.15, -0.22), (0.05, -0.3),
                               (-0.06, -0.27)], start=0.6, mid=0.8, end=0.4),
        "ogonek": PEN.stroke([(0.08, 0.04), (-0.04, -0.12), (0.0, -0.25), (0.13, -0.28)],
                             start=0.9, mid=0.8, end=0.35),
    }
    m["caron"] = flip(m["circumflex"], centre_v=0.1)
    return m


# ---------------------------------------------------------------- Latin

def latin(t):
    g = dict(t)
    cap = top_v(t["B"])

    def enlarge(ch, sx, sy_below=None):
        return scale(t[ch], sx, cap, sy_below=sy_below, thin=5)

    g["O"] = smoothed(enlarge("o", 1.3), SPIKE_RADIUS)
    g["U"] = enlarge("u", 1.25)
    g["W"] = enlarge("w", 1.2)
    g["Y"] = enlarge("y", 1.25, sy_below=1.0)
    g["Z"] = enlarge("z", 1.25)

    # k: its leg runs on under the next letter as a hairline, like K's; in the images that
    # hairline is too thin and dark to cut out. Below the arm, leg and swing are one pen stroke:
    # the leg straight from the traced leg's top to its foot and as wide, then thinning into the
    # swing. Its start lies on the traced leg, so the two meet without a step. The swing stays
    # under the baseline, so it doesn't count as the body for spacing.
    k_small = t["k"]
    foot_u = extent_u(region(k_small, v1=0.2))[1] - 0.06
    leg_heights = (0.44, 0.32, 0.2, 0.08)
    top, *_, foot = centre_line(k_small, leg_heights)
    leg_pressure = pressure_for(stroke_width(k_small, leg_heights), top, foot)
    k_stem = stems(k_small, 0.1, 0.4)[0][1]
    arm = erase(k_small, u0=k_stem + 0.04, v1=leg_heights[0] - 0.04)
    middle = ((top[0] + foot[0]) / 2, (top[1] + foot[1]) / 2)
    # The points are about evenly spaced: closer ones make the curve overshoot into a bulge.
    leg = PEN.stroke([top, middle, foot, (foot_u + 0.08, -0.08), (foot_u + 0.3, -0.13), (foot_u + 0.48, -0.1),
                      (foot_u + 0.58, -0.05)],
                     pressures=[leg_pressure, leg_pressure, leg_pressure, 0.56, 0.45, 0.39, 0.35])
    g["k"] = filled(arm | leg, JOIN_RADIUS)

    # x: a thick fall and a thin rise, as the nib makes them.
    g["x"] = (PEN.stroke([(0.0, 0.86), (0.1, 1.0), (0.2, 0.95), (0.5, 0.12), (0.62, 0.0), (0.74, 0.07)],
                         start=0.6, mid=1.0, end=0.5)
              | PEN.stroke([(0.1, 0.02), (0.38, 0.5), (0.64, 0.98)], start=0.5, mid=0.8, end=0.6))
    g["X"] = scale(g["x"], 1.3, cap, thin=5)

    # j and J: the stem's upper part, carried on along its own line into a descender curling left.
    def with_tail(top, v_hi, v_lo, depth):
        c_hi, c_lo = centre_at(top, v_hi), centre_at(top, v_lo)
        lean = (c_hi - c_lo) / (v_hi - v_lo)

        def on_stem(v):
            return c_lo + lean * (v - v_lo)
        tail = PEN.stroke([(on_stem(v_lo), v_lo), (on_stem(0.0), 0.0), (on_stem(-0.6 * depth) - 0.04, -0.6 * depth),
                           (on_stem(-depth) - 0.2, -depth), (on_stem(-depth) - 0.36, -depth + 0.1)],
                          start=1.0, mid=1.0, end=0.45)
        upper = region(top, v0=v_lo + 0.02)
        joined = union_of(p for p in pieces_of(upper) if p.sum() >= 0.05 * upper.sum()) | tail
        return filled(joined, JOIN_RADIUS)
    g["dotlessi"] = region(t["i"], v1=1.1)
    # The stem's line is measured between its serifs.
    g["j"] = with_tail(t["i"], 0.6, 0.3, 0.44)
    g["dotlessj"] = region(g["j"], v1=1.1)
    g["J"] = with_tail(t["I"], 1.0, 0.45, 0.48)

    # P and R from B: its stem and upper bowl; R adds K's leg. At B's waist the two bowls meet in
    # a point, so the bowl's right side is drawn on with the pen from where it is straight, along
    # its centre line and as wide, and curves back to the stem in a hairline. The traced and drawn
    # parts meet on the straight, without a step.
    b = t["B"]
    b_stem = stem_right(b) + 0.03
    waist = narrowest(b, 0.35 * cap, 0.65 * cap)
    side_heights = (waist + 0.34, waist + 0.26)
    side = centre_line(b, side_heights)
    side_pressure = pressure_for(stroke_width(b, side_heights), side[0], side[1])
    upper = erase(b, u0=b_stem, v1=side_heights[1] + 0.02)
    right = side[1][0]
    closing = PEN.stroke(side + [(right - 0.01, waist + 0.16), (right - 0.07, waist + 0.07),
                                 (right - 0.18, waist + 0.02), (b_stem - 0.02, waist - 0.01)],
                         start=side_pressure, mid=0.8, end=0.6)
    g["P"] = smoothed(filled(upper | closing, JOIN_RADIUS), SPIKE_RADIUS)

    # F: the only source ("Crywolf Fortress") is squeezed and cut wrongly at its "f F"; the E
    # without its bottom arm is the client's F.
    e = t["E"]
    g["F"] = erase(e, u0=stem_right(e) + 0.03, v1=0.3 * cap)
    k = t["K"]
    k_stem = stem_right(k) + 0.02
    k_joint = narrowest(k, 0.3 * cap, 0.7 * cap)
    # The traced leg's wavy edges are evened out; it is thick down to its curled end.
    leg = evened(region(k, u0=k_stem, v1=k_joint), 7)
    g["R"] = filled(g["P"] | shift(leg, b_stem - k_stem - 0.04, waist - k_joint - 0.02), JOIN_RADIUS)

    # H: K's stem twice and a hairline bar.
    left_stem = region(k, u1=k_stem)
    width = 0.78
    right_stem = shift(left_stem, width)
    bar_v = 0.5 * cap
    g["H"] = left_stem | right_stem | PEN.stroke([(k_stem - 0.05, bar_v), (k_stem + width - 0.15, bar_v + 0.02)],
                                                 start=0.6, end=0.6)

    # G: C with a spur at its lower end.
    c = t["C"]
    # Keep the C's opening: a short bar and a spur that starts below the middle and drops past
    # the baseline, as the chancery G does. The C's lower curve ends in the spur instead of
    # running past it.
    tip_u = extent_u(region(c, v1=0.3))[1]
    spur_u = tip_u - 0.1
    bar_v = 0.42 * cap
    g["G"] = (erase(c, u0=spur_u + 0.04, v1=0.35)
              | PEN.stroke([(spur_u - 0.26, bar_v), (spur_u + 0.05, bar_v)], start=0.5, end=0.9)
              | PEN.stroke([(spur_u + 0.02, bar_v), (spur_u + 0.01, 0.25), (spur_u - 0.02, 0.0),
                            (spur_u - 0.07, -0.16), (spur_u - 0.18, -0.24)], start=1.0, mid=1.0, end=0.4))

    # M: the only source ("Loren Market") is squeezed and its thin left leg was cut away with the
    # n before it; draw it as the original shows it: thin leg, thick fall, thin rise, thick stem.
    g["M"] = (PEN.stroke([(-0.06, 0.0), (0.05, 0.03), (0.1, 0.7), (0.17, cap - 0.02)], start=0.5, mid=0.42, end=0.5)
              | PEN.stroke([(0.1, cap - 0.06), (0.2, cap), (0.4, 0.62), (0.58, 0.0)], start=0.7, mid=1.0, end=0.8)
              | PEN.stroke([(0.58, 0.0), (0.8, 0.8), (0.96, cap - 0.02)], start=0.45, mid=0.45, end=0.5)
              | PEN.stroke([(0.9, cap - 0.04), (1.0, cap), (0.99, 0.6), (0.97, 0.05), (1.1, -0.01)],
                           start=0.7, mid=1.0, end=0.6))

    # Q: O with a swash tail.
    o_mid = ink_centre(g["O"])
    g["Q"] = g["O"] | PEN.stroke([(o_mid - 0.05, 0.06), (o_mid + 0.15, -0.12), (o_mid + 0.5, -0.12),
                                  (o_mid + 0.78, -0.02)], start=0.9, mid=0.8, end=0.35)
    return g


def figures(t, g):
    f = {}
    f["zero"] = scale(t["o"], 1.05, FIG, thin=3)
    f["one"] = PEN.stroke([(0.02, 1.02), (0.16, 1.16), (0.3, FIG), (0.26, 0.6), (0.22, 0.0)],
                          start=0.4, mid=1.0, end=1.0)
    f["two"] = PEN.stroke([(0.08, 1.0), (0.26, 1.26), (0.5, FIG), (0.68, 1.12), (0.62, 0.8), (0.34, 0.38),
                           (0.06, 0.03)], start=0.6, end=1.0) \
        | PEN.stroke([(0.06, 0.03), (0.32, 0.07), (0.58, 0.03), (0.78, 0.1)], start=1.0, end=0.6)
    f["three"] = PEN.stroke([(0.1, 1.1), (0.34, FIG), (0.6, 1.22), (0.62, 0.96), (0.38, 0.72)], start=0.5, end=0.9) \
        | PEN.stroke([(0.38, 0.72), (0.62, 0.62), (0.7, 0.34), (0.52, 0.06), (0.26, 0.0), (0.04, 0.14)],
                     start=0.9, end=0.5)
    f["four"] = PEN.stroke([(0.6, FIG), (0.04, 0.36)], start=0.8, end=1.0) \
        | PEN.stroke([(0.04, 0.36), (0.8, 0.38)], start=1.0, end=0.7) \
        | PEN.stroke([(0.58, 0.9), (0.54, 0.0)], start=1.0, end=1.0)
    f["five"] = PEN.stroke([(0.26, FIG - 0.02), (0.72, FIG)], start=0.8, end=0.6) \
        | PEN.stroke([(0.26, FIG - 0.02), (0.18, 0.74)], start=1.0, end=1.0) \
        | PEN.stroke([(0.18, 0.74), (0.46, 0.82), (0.7, 0.55), (0.62, 0.15), (0.34, 0.0), (0.04, 0.12)],
                     start=0.9, end=0.5)
    # 6: one stroke from the top's hook down the left side into the bowl.
    f["six"] = PEN.stroke([(0.66, 1.16), (0.5, FIG), (0.3, 1.22), (0.14, 0.92), (0.08, 0.5), (0.14, 0.14),
                           (0.34, 0.0), (0.56, 0.08), (0.66, 0.32), (0.6, 0.56), (0.4, 0.66), (0.2, 0.58),
                           (0.1, 0.42)], start=0.5, mid=1.0, end=0.8)
    f["seven"] = PEN.stroke([(0.06, FIG - 0.02), (0.74, FIG)], start=0.7, end=0.9) \
        | PEN.stroke([(0.74, FIG), (0.44, 0.62), (0.3, 0.0)], start=0.9, end=1.0)
    lower = scale(t["o"], 0.95, 0.72, thin=0)
    upper = scale(t["o"], 0.8, 0.62, thin=0)
    upper = shift(upper, 0.06, top_v(lower) - 0.05 - bottom_v(upper))
    f["eight"] = upper | lower
    f["nine"] = rotate180(f["six"], centre_v=FIG / 2)
    return f


def punctuation(t, g):
    p = {}
    dot = largest(region(t["i"], v0=1.12))
    period = shift(dot, -extent_u(dot)[0] + 0.05, -bottom_v(dot))
    p["period"] = period
    c = ink_centre(period)
    tail = PEN.stroke([(c + 0.04, 0.06), (c + 0.02, -0.12), (c - 0.1, -0.24)], start=0.9, end=0.3)
    p["comma"] = period | tail
    cap = top_v(t["B"])
    p["quoteright"] = shift(p["comma"], 0.08, cap - top_v(p["comma"]))
    p["quotesingle"] = p["quoteright"]
    p["quoteleft"] = rotate180(p["quoteright"], centre_v=cap - 0.15)
    p["quotedblright"] = p["quoteright"] | shift(p["quoteright"], 0.2)
    p["quotedblleft"] = p["quoteleft"] | shift(p["quoteleft"], 0.2)
    p["quotedbl"] = p["quotedblright"]
    p["quotesinglbase"] = p["comma"]
    p["quotedblbase"] = p["comma"] | shift(p["comma"], 0.2)
    p["hyphen"] = PEN.stroke([(0.06, 0.42), (0.42, 0.47)], start=0.8, end=0.8)
    p["endash"] = PEN.stroke([(0.04, 0.42), (0.62, 0.46)], start=0.8, end=0.8)
    p["emdash"] = PEN.stroke([(0.04, 0.42), (1.02, 0.46)], start=0.8, end=0.8)
    p["colon"] = period | shift(period, 0.03, 0.78)
    p["semicolon"] = p["comma"] | shift(period, 0.03, 0.78)
    p["exclam"] = PEN.stroke([(0.3, cap), (0.22, 0.38)], start=1.0, end=0.45) | shift(period, 0.12)
    p["question"] = PEN.stroke([(0.08, 1.18), (0.28, cap), (0.54, cap - 0.06), (0.58, 1.08), (0.36, 0.78),
                                (0.3, 0.38)], start=0.5, mid=1.0, end=0.55) | shift(period, 0.18)
    p["exclamdown"] = rotate180(p["exclam"], centre_v=0.55)
    p["questiondown"] = rotate180(p["question"], centre_v=0.55)
    p["parenleft"] = PEN.stroke([(0.36, 1.62), (0.14, 1.0), (0.1, 0.3), (0.26, -0.36)], start=0.5, mid=1.0, end=0.5)
    p["parenright"] = mirror(p["parenleft"])
    p["slash"] = PEN.stroke([(0.0, -0.2), (0.62, 1.6)], start=0.8, end=0.8)
    p["ellipsis"] = period | shift(period, 0.26) | shift(period, 0.52)
    return p


def accented(t, g, m):
    """Latin letters with marks: Latin-1, Polish and the common carons."""
    out = {}
    spec = {
        "grave": "AEIOUaeiou", "acute": "AEIOUYaeiouyCNSZcnsz", "circumflex": "AEIOUaeiou",
        "tilde": "ANOano", "dieresis": "AEIOUaeiouy", "caron": "CSZcsz", "dot": "Zz",
    }
    for mark, letters in spec.items():
        for ch in letters:
            base = g[ch]
            if ch == "i":
                base = g["dotlessi"]
            name = f"{ch}{mark}" if mark != "dot" else f"{ch}dotaccent"
            centre = body_centre(base) if ch.islower() else None
            out[name] = with_mark(base, m[mark], centre=centre)
    out["Aring"] = with_mark(g["A"], m["ring"])
    out["aring"] = with_mark(g["a"], m["ring"], centre=body_centre(g["a"]))
    out["Ccedilla"] = with_mark(g["C"], m["cedilla"], gap=-0.06, below=True)
    out["ccedilla"] = with_mark(g["c"], m["cedilla"], gap=-0.06, below=True, centre=body_centre(g["c"]))
    for ch in "AEae":
        base = g[ch]
        right = extent_u(base)[1] if ch.isupper() else body_u(base)[1]
        out[f"{ch}ogonek"] = with_mark(base, m["ogonek"], gap=-0.05, below=True, centre=right - 0.12)
    stroke = PEN.stroke([(0.0, 0.0), (0.32, 0.26)], start=0.7, end=0.7)
    l_mid = centre_at(g["l"], 0.9)
    out["lslash"] = g["l"] | shift(stroke, l_mid - 0.16, 0.7)
    big_l = centre_at(g["L"], 0.7)
    out["Lslash"] = g["L"] | shift(stroke, big_l - 0.16, 0.55)
    out["Oslash"] = g["O"] | PEN.stroke([(ink_centre(g["O"]) - 0.5, -0.06), (ink_centre(g["O"]) + 0.5, top_v(g["O"]) + 0.06)],
                                        start=0.5, end=0.5)
    out["oslash"] = g["o"] | PEN.stroke([(body_centre(g["o"]) - 0.35, -0.06), (body_centre(g["o"]) + 0.35, 1.06)],
                                        start=0.5, end=0.5)
    out["germandbls"] = PEN.stroke([(0.1, -0.42), (0.12, 0.9), (0.24, 1.52), (0.48, SHARP_S_TOP), (0.62, 1.38),
                                    (0.42, 1.06), (0.66, 0.8), (0.7, 0.34), (0.46, 0.0), (0.28, 0.06)],
                                   start=0.6, mid=1.0, end=0.5)
    out["ydieresis"] = with_mark(g["y"], m["dieresis"], centre=body_centre(g["y"]))
    return out



# ---------------------------------------------------------------- Cyrillic

def cyrillic(t, g, f, m):
    c = {}
    cap = top_v(t["B"])
    for cyr, latin_name in CYRILLIC_FROM_LATIN.items():
        c[cyr] = g[latin_name]
    # Ь, Ъ, Ы, Б: B's stem and lower bowl. Cut at B's waist, the bowl's top ended in the point
    # where the two bowls met; it is drawn on with the pen instead, a hairline from the stem that
    # runs into the bowl's right side along its centre line and as wide.
    b = t["B"]
    b_stem_left = stems(b, 0.2 * cap, 0.8 * cap)[0][0]
    b_stem = stem_right(b) + 0.03
    waist = narrowest(b, 0.35 * cap, 0.65 * cap)
    side_heights = (0.7 * waist, 0.6 * waist)
    side = centre_line(b, side_heights)
    side_pressure = pressure_for(stroke_width(b, side_heights), side[0], side[1])
    bowl_top = waist - 0.035
    bowl_arc = PEN.stroke([(b_stem - 0.04, bowl_top - 0.01), (b_stem + 0.12, bowl_top), (b_stem + 0.3, bowl_top - 0.03),
                           (side[0][0] - 0.04, side[0][1] + 0.1)] + side,
                          start=0.8, mid=1.0, end=side_pressure)
    with_curl = filled(erase(b, u0=b_stem, v0=side_heights[0] - 0.02) | bowl_arc, JOIN_RADIUS)
    # B's left curl belongs to its upper bowl; without that bowl it hangs loose, so Ь, Ъ and Ы
    # leave it off (the foot stays), and the stem gets a plain serif at its top. The bowl's
    # right side bends in a corner in the trace; it is evened out.
    soft = evened(without_specks(erase(with_curl, u1=b_stem_left - 0.03, v0=0.2)), 9)
    stem_top = top_v(region(soft, u0=b_stem_left, u1=b_stem))
    serif = PEN.stroke([(b_stem_left - 0.12, stem_top - 0.07), (b_stem_left - 0.02, stem_top - 0.02),
                        (b_stem_left + 0.08, stem_top - 0.01)], start=0.4, end=0.8)
    e = t["E"]
    e_stem = stem_right(e) + 0.03
    c["Ь"] = soft | serif
    # Б keeps B's curl, and E's top arm goes on the stem. The notch between the curl's top and
    # the stem's serif is filled, so the curl runs into the stem.
    c["Б"] = filled(evened(with_curl | shift(region(e, v0=cap - 0.22), b_stem - e_stem), 5), 16)
    # Ъ: E's top arm mirrored is its flag, a bar to the left of the stem ending in a drop.
    flag = mirror(region(e, u0=e_stem - 0.05, v0=cap - 0.3))
    flag = shift(flag, b_stem_left + 0.08 - extent_u(flag)[1], stem_top - top_v(flag))
    c["Ъ"] = filled(soft | flag, JOIN_RADIUS)
    c["Ы"] = c["Ь"] | shift(g["I"], extent_u(soft)[1] + 0.08 - extent_u(g["I"])[0])
    c["Г"] = erase(e, u0=e_stem, v1=cap - 0.22)
    c["Ґ"] = c["Г"] | PEN.stroke([(extent_u(c["Г"])[1] - 0.06, cap - 0.06), (extent_u(c["Г"])[1] - 0.02, cap + 0.22)],
                                 start=0.9, end=0.6)
    c["Е"] = g["E"]
    c["Ё"] = with_mark(g["E"], m["dieresis"])
    c["Є"] = g["C"] | PEN.stroke([(extent_u(g["C"])[0] + 0.12, 0.5 * cap), (extent_u(g["C"])[1] - 0.25, 0.5 * cap)],
                                 start=0.6, end=0.6)
    c["Э"] = mirror(g["C"]) | PEN.stroke([(0.3, 0.5 * cap), (extent_u(g["C"])[1] - 0.1, 0.5 * cap)],
                                         start=0.6, end=0.6)
    c["Ї"] = with_mark(g["I"], m["dieresis"])
    # The italic И is a U shape, like its small letter (и = u); a mirrored chancery N reads as V.
    c["И"] = g["U"]
    c["Й"] = with_mark(c["И"], m["breve"])
    c["Я"] = mirror(g["R"])
    c["З"] = scale(f["three"], 1.0, cap / FIG, thin=1)
    # Ж: K's arms, mirrored on the left, around K's bare stem with plain serifs (K's own top-left
    # serif would hook into the left arms).
    k = t["K"]
    k_stem_left = stems(k, 0.2 * cap, 0.8 * cap)[0][0]
    k_stem = stem_right(k) + 0.02
    arms = region(k, u0=k_stem + 0.01)
    bare_stem = region(k, u0=k_stem_left - 0.01, u1=k_stem, v0=0.06, v1=cap - 0.06)
    left_arms = place_left(mirror(arms), 0.0)
    stem_at = extent_u(left_arms)[1] - 0.06
    stem_part = place_left(bare_stem, stem_at)
    stem_width = extent_u(stem_part)[1] - stem_at
    serifs = (PEN.stroke([(stem_at - 0.1, cap - 0.01), (stem_at + stem_width + 0.1, cap)], start=0.6, end=0.6)
              | PEN.stroke([(stem_at - 0.12, 0.01), (stem_at + stem_width + 0.12, 0.0)], start=0.6, end=0.6))
    right_part = place_left(arms, extent_u(stem_part)[1] - 0.06)
    c["Ж"] = without_specks(left_arms | stem_part | serifs | right_part)
    c["Д"] = (PEN.stroke([(0.78, cap), (0.74, 0.0)])
              | PEN.stroke([(0.8, cap), (0.48, cap - 0.08), (0.3, 0.7), (0.12, 0.05)], start=0.8, end=1.0)
              | PEN.stroke([(-0.04, 0.02), (0.98, 0.02)], start=0.8, end=0.8)
              | PEN.stroke([(-0.02, 0.02), (-0.06, -0.26)], start=1.0, end=0.5)
              | PEN.stroke([(0.96, 0.02), (1.0, -0.26)], start=1.0, end=0.5))
    # Л: a leg rising from a small hook, a bar, and a stem.
    c["Л"] = (PEN.stroke([(0.02, 0.06), (0.18, 0.2), (0.36, 1.1), (0.52, cap)], start=0.5, mid=1.0, end=0.9)
              | PEN.stroke([(0.5, cap), (0.84, cap)], start=0.8, end=0.8)
              | PEN.stroke([(0.82, cap), (0.78, 0.0)]))
    h_left = region(k, u1=k_stem)
    c["П"] = h_left | shift(h_left, 0.78) | PEN.stroke([(k_stem - 0.12, cap - 0.03), (k_stem + 0.72, cap - 0.02)],
                                                         start=0.7, end=0.7)
    oval = scale(t["o"], 1.25, 1.1, thin=3)
    oval = shift(oval, 0, 0.18)
    mid = ink_centre(oval)
    c["Ф"] = oval | PEN.stroke([(mid + 0.02, cap + 0.02), (mid - 0.02, -0.02)])
    u_big = g["U"]
    u_right = extent_u(u_big)[1]
    tail = PEN.stroke([(u_right - 0.08, 0.1), (u_right - 0.02, -0.08), (u_right + 0.02, -0.28)], start=1.0, end=0.5)
    c["Ц"] = u_big | tail
    u_left_stem = stem_right(u_big, v_lo=0.4, v_hi=1.2)
    c["Ч"] = erase(u_big, u1=u_left_stem + 0.02, v1=0.62) | PEN.stroke(
        [(u_left_stem - 0.12, 0.66), (u_left_stem + 0.05, 0.5), (u_right - 0.3, 0.6)], start=0.9, end=0.6)
    wide_u = scale(t["u"], 1.7, cap, thin=11)        # the stretch widens stems 1.7 times
    w_right = runs_at(wide_u, 0.8)[-1]
    middle = shift(region(wide_u, u0=w_right[0] - 0.02, v0=0.25), -(w_right[0] - runs_at(wide_u, 0.8)[0][0]) / 2)
    c["Ш"] = wide_u | middle
    shch_right = extent_u(c["Ш"])[1]
    c["Щ"] = c["Ш"] | PEN.stroke([(shch_right - 0.08, 0.1), (shch_right - 0.02, -0.08), (shch_right + 0.02, -0.28)],
                                 start=1.0, end=0.5)
    i_stem = g["I"]
    small_o = scale(t["o"], 1.1, cap * 0.92, thin=4)
    joined = place_left(small_o, extent_u(i_stem)[1] + 0.12)
    c["Ю"] = i_stem | joined | PEN.stroke([(extent_u(i_stem)[1] - 0.05, 0.5 * cap), (extent_u(i_stem)[1] + 0.18, 0.5 * cap)],
                                          start=0.6, end=0.6)

    # Lowercase italic forms.
    o = t["o"]
    o_right = extent_u(o)[1]
    c["б"] = o | PEN.stroke([(o_right - 0.12, 0.72), (o_right - 0.06, 1.25), (o_right + 0.12, 1.58),
                             (o_right + 0.42, 1.62)], start=0.9, mid=1.0, end=0.4)
    c["в"] = thicken(scale(t["B"], 0.85, 1.0 / cap * 1.0, thin=0), 3)
    c["д"] = o | PEN.stroke([(o_right - 0.1, 0.55), (o_right - 0.04, 1.15), (o_right - 0.26, 1.58), (o_right - 0.55, 1.52)],
                            start=1.0, mid=1.0, end=0.4)
    c["ё"] = with_mark(t["e"], m["dieresis"], centre=body_centre(t["e"]))
    k_small = region(t["k"], v1=1.04)
    ks_stem = stem_right(k_small, v_lo=0.2, v_hi=0.8) + 0.02
    ks_arms = region(k_small, u0=ks_stem - 0.02)
    ks_stem_part = region(k_small, u1=ks_stem)
    c["к"] = thicken(scale(t["K"], 0.82, 1.0 / cap, thin=0), 3)
    left = place_left(mirror(ks_arms), 0.0)
    stem_small = place_left(ks_stem_part, extent_u(left)[1] - 0.1)
    c["ж"] = left | stem_small | place_left(ks_arms, extent_u(stem_small)[1] - 0.06)
    c["з"] = thicken(scale(f["three"], 0.9, 1.0 / FIG * 1.02, thin=0), 3)
    c["й"] = with_mark(t["u"], m["breve"], centre=body_centre(t["u"]))
    c["л"] = (PEN.stroke([(0.0, 0.06), (0.14, 0.14), (0.28, 0.62), (0.4, 1.0)], start=0.5, mid=1.0, end=0.9)
              | PEN.stroke([(0.38, 1.0), (0.62, 1.0)], start=0.8, end=0.8)
              | PEN.stroke([(0.62, 1.0), (0.6, 0.1), (0.72, 0.0)], start=1.0, end=0.6))
    c["м"] = thicken(scale(g["M"], 0.8, 1.0 / cap, thin=0), 4)
    c["н"] = thicken(scale(g["H"], 0.8, 1.0 / cap, thin=0), 4)
    # ф: a wide o on a straight stem from ascender to descender, with a small serif on top.
    oval_small = scale(o, 1.25, 1.0, thin=3)
    om = body_centre(oval_small)
    c["ф"] = (oval_small | PEN.stroke([(om, 1.6), (om, 0.5), (om, -0.44)])
              | PEN.stroke([(om - 0.12, 1.54), (om, 1.62)], start=0.5, end=0.9))
    u_small = t["u"]
    us_right = extent_u(u_small)[1]
    small_tail = PEN.stroke([(us_right - 0.06, 0.08), (us_right - 0.02, -0.08), (us_right + 0.02, -0.26)],
                            start=1.0, end=0.5)
    c["ц"] = u_small | small_tail
    us_left = stem_right(u_small, v_lo=0.3, v_hi=0.9)
    c["ч"] = erase(u_small, u1=us_left + 0.02, v1=0.5) | PEN.stroke(
        [(us_left - 0.1, 0.54), (us_left + 0.06, 0.4), (us_right - 0.26, 0.48)], start=0.9, end=0.6)
    c["ш"] = rotate180(t["m"], centre_v=0.5)
    sh_right = extent_u(c["ш"])[1]
    c["щ"] = c["ш"] | PEN.stroke([(sh_right - 0.06, 0.08), (sh_right - 0.02, -0.08), (sh_right + 0.02, -0.26)],
                                 start=1.0, end=0.5)
    c["ь"] = PEN.stroke([(0.16, 1.0), (0.12, 0.04)]) | PEN.stroke(
        [(0.13, 0.5), (0.4, 0.6), (0.62, 0.36), (0.48, 0.04), (0.14, 0.02)], start=0.8, mid=1.0, end=0.8)
    c["ъ"] = c["ь"] | PEN.stroke([(-0.18, 0.98), (0.12, 1.0)], start=0.6, end=0.9)
    c["ы"] = c["ь"] | place_left(g["dotlessi"], extent_u(c["ь"])[1] + 0.06)
    c["э"] = mirror(t["c"]) | PEN.stroke([(0.2, 0.5), (extent_u(t["c"])[1] - 0.12, 0.5)], start=0.6, end=0.6)
    c["є"] = t["c"] | PEN.stroke([(extent_u(t["c"])[0] + 0.1, 0.5), (extent_u(t["c"])[1] - 0.2, 0.5)],
                                 start=0.6, end=0.6)
    stem_i = g["dotlessi"]
    c["ю"] = stem_i | place_left(o, extent_u(stem_i)[1] + 0.1) | PEN.stroke(
        [(extent_u(stem_i)[1] - 0.05, 0.5), (extent_u(stem_i)[1] + 0.14, 0.5)], start=0.6, end=0.6)
    c["я"] = thicken(scale(mirror(g["R"]), 0.85, 1.0 / cap, thin=0), 4)
    c["ї"] = with_mark(g["dotlessi"], m["dieresis"], centre=body_centre(g["dotlessi"]))
    c["ґ"] = t["r"] | PEN.stroke([(extent_u(t["r"])[1] - 0.06, 0.94), (extent_u(t["r"])[1] - 0.02, 1.22)],
                                 start=0.9, end=0.6)
    return c


def repaired(t):
    """The traced letters with their sources' faults mended. o, g and s carry bits of their
    neighbours and pixel noise, smoothed off. y and q come only from squeezed names whose
    neighbours run into them: y's wavy edges are evened out, q is rebuilt from the clean a."""
    r = dict(t)
    for ch, radius in (("o", 8), ("g", 9), ("s", 7)):
        r[ch] = largest(smoothed(t[ch], radius))
    # o: some samples have a small flick at the upper right; averaged, it leaves a lumpy, flat
    # shoulder there (on O, Q, 0 and the Cyrillic о too). The o gets its own ellipse as outline.
    r["o"] = elliptic_outline(r["o"])
    # g: the l or n before it in "Doppelganger" leaves an ear on the bowl's left, low down where
    # the bowl meets the loop. The bowl is widest at its middle.
    g = r["g"]
    bowl_left = min(runs_at(g, v)[0][0] for v in (0.5, 0.6, 0.7, 0.8))
    r["g"] = largest(smoothed(erase(g, u1=bowl_left - 0.01, v0=0.15, v1=0.85), SPIKE_RADIUS))

    # y: its edges are wavier than other letters'; the opening also rounds off the knob at the
    # top of its right arm.
    r["y"] = evened(t["y"], 9, radius=10)
    # q: a, its exit flick taken off and the right stem carried on into a descender on a foot.
    a = t["a"]
    stem_left, stem_right_ = stems(a, 0.25, 0.75)[-1]
    middle = (stem_left + stem_right_) / 2
    r["q"] = (erase(a, u0=stem_right_ + 0.01, v1=0.3)
              | PEN.stroke([(middle, 0.5), (middle - 0.02, 0.0), (middle - 0.04, -0.44)])
              | PEN.stroke([(middle - 0.17, -0.45), (middle + 0.12, -0.44)], start=0.6, end=0.6))
    return r


def build(t):
    """All glyph masks, from the traced letters t: (Latin, figures, punctuation, accented Latin,
    Cyrillic, marks), each a dict of glyph name or character to mask."""
    global PEN
    PEN = matched_pen(t)
    t = repaired(t)
    g = latin(t)
    f = figures(t, g)
    m = marks(t)
    p = punctuation(t, g)
    a = accented(t, g, m)
    cyr = cyrillic(t, g, f, m)
    return g, f, p, a, cyr, m
