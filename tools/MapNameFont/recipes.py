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

from letters import X_HEIGHT
from shapes import (Pen, body_u, bottom_v, col, erase, extent_u, flip, largest, mirror, place_left, region,
                    rotate180, row, scale, shift, thicken, top_v)

PEN = None              # set by build(): a nib matched to the traced letters' strokes
PEN_EDGE = 8            # px: the nib's thickness
# A drawn stroke comes out this much wider than the nib's projection (anti-aliasing, round ends).
PEN_SPREAD_STEM, PEN_SPREAD_HAIR = 2, 3
SPIKE_RADIUS = 6        # px: points thinner than this are rounded off where parts are cut apart
FIG = 1.3               # figure height
MARK_GAP = 0.1          # space between a letter and its accent
SHARP_S_TOP = 1.6
# Cyrillic letters drawn like a Latin one; the italic forms of и, п, т, г are u, n, m, r.
CYRILLIC_FROM_LATIN = {
    "А": "A", "В": "B", "Е": "E", "К": "K", "М": "M", "Н": "H", "О": "O", "Р": "P", "С": "C", "Т": "T",
    "Х": "X", "У": "Y", "І": "I", "а": "a", "е": "e", "о": "o", "р": "p", "с": "c", "х": "x", "у": "y",
    "и": "u", "п": "n", "т": "m", "і": "i", "г": "r",
}


def smoothed(mask, radius):
    """Morphological opening with a disc: removes points and spurs thinner than 2 * radius."""
    y, x = np.mgrid[-radius:radius + 1, -radius:radius + 1]
    return ndimage.binary_opening(mask, structure=x * x + y * y <= radius * radius)


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

    g["O"] = enlarge("o", 1.3)
    g["U"] = enlarge("u", 1.25)
    g["W"] = enlarge("w", 1.2)
    g["Y"] = enlarge("y", 1.25, sy_below=1.0)
    g["Z"] = enlarge("z", 1.25)

    # k: its leg runs on under the next letter as a hairline, like K's; in the images that
    # hairline is too thin and dark to cut out.
    k_small = t["k"]
    k_foot = region(k_small, v1=0.2)
    foot_u = extent_u(k_foot)[1] - 0.06
    # It stays under the baseline, so it doesn't count as the letter's body for spacing.
    g["k"] = k_small | PEN.stroke([(foot_u - 0.04, 0.02), (foot_u + 0.06, -0.1), (foot_u + 0.3, -0.16),
                                   (foot_u + 0.5, -0.13), (foot_u + 0.62, -0.07)], start=0.7, mid=0.45, end=0.35)

    # x: a thick fall and a thin rise, as the nib makes them.
    g["x"] = (PEN.stroke([(0.0, 0.86), (0.1, 1.0), (0.2, 0.95), (0.5, 0.12), (0.62, 0.0), (0.74, 0.07)],
                         start=0.6, mid=1.0, end=0.5)
              | PEN.stroke([(0.1, 0.02), (0.38, 0.5), (0.64, 0.98)], start=0.5, mid=0.8, end=0.6))
    g["X"] = scale(g["x"], 1.3, cap, thin=5)

    # j and J: the stem above the baseline, a pen descender curling left below it.
    def with_tail(top, at_v, depth):
        c = centre_at(top, at_v)
        tail = PEN.stroke([(c + 0.01, 0.3), (c - 0.02, -0.1), (c - 0.1, -depth + 0.06), (c - 0.3, -depth),
                           (c - 0.42, -depth + 0.1)], start=1.0, mid=1.0, end=0.45)
        return region(top, v0=0.22) | tail
    g["dotlessi"] = region(t["i"], v1=1.1)
    g["j"] = with_tail(t["i"], 0.5, 0.44)
    g["dotlessj"] = region(g["j"], v1=1.1)
    g["J"] = with_tail(t["I"], 0.6, 0.48)

    # P and R from B: its stem and upper bowl; R adds K's leg.
    b = t["B"]
    b_stem = stem_right(b) + 0.03
    waist = narrowest(b, 0.35 * cap, 0.65 * cap)
    # Opening rounds off the point where B's two bowls met.
    g["P"] = smoothed(erase(b, u0=b_stem, v1=waist - 0.04), SPIKE_RADIUS)

    # F: the only source ("Crywolf Fortress") is squeezed and cut wrongly at its "f F"; the E
    # without its bottom arm is the client's F.
    e = t["E"]
    g["F"] = erase(e, u0=stem_right(e) + 0.03, v1=0.3 * cap)
    k = t["K"]
    k_stem = stem_right(k) + 0.02
    k_joint = narrowest(k, 0.3 * cap, 0.7 * cap)
    leg = region(k, u0=k_stem, v1=k_joint)
    g["R"] = g["P"] | shift(leg, b_stem - k_stem - 0.04, waist - k_joint - 0.02)

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
    # the baseline, as the chancery G does.
    spur_u = extent_u(region(c, v1=0.3))[1] - 0.12      # under the tip of the lower curve
    bar_v = 0.42 * cap
    g["G"] = (c | PEN.stroke([(spur_u - 0.34, bar_v - 0.02), (spur_u - 0.1, bar_v + 0.02), (spur_u + 0.06, bar_v)],
                             start=0.5, mid=0.8, end=0.9)
              | PEN.stroke([(spur_u + 0.02, bar_v), (spur_u, 0.2), (spur_u - 0.03, -0.12), (spur_u - 0.12, -0.3),
                            (spur_u - 0.26, -0.34)], start=1.1, mid=1.1, end=0.45))

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
    f["six"] = scale(t["b"], 0.95, FIG / top_v(t["b"]), thin=0)
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
    b = t["B"]
    b_stem = stem_right(b) + 0.03
    waist = narrowest(b, 0.35 * cap, 0.65 * cap)
    soft = erase(b, u0=b_stem, v0=waist + 0.04)          # stem and lower bowl
    e = t["E"]
    e_stem = stem_right(e) + 0.03
    e_top = region(e, v0=cap - 0.22)
    e_top = shift(e_top, b_stem - e_stem)
    c["Ь"] = soft
    c["Б"] = soft | e_top
    c["Ъ"] = soft | PEN.stroke([(-0.28, cap - 0.04), (b_stem - 0.1, cap - 0.02)], start=0.6, end=0.9)
    c["Ы"] = soft | shift(g["I"], extent_u(soft)[1] + 0.08 - extent_u(g["I"])[0])
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
    k = t["K"]
    k_stem = stem_right(k) + 0.02
    arms = region(k, u0=k_stem - 0.02)
    stem = region(k, u1=k_stem)
    left_arms = mirror(arms)
    left_arms = place_left(left_arms, 0.0)
    stem_part = place_left(stem, extent_u(left_arms)[1] - 0.12)
    right_part = place_left(arms, extent_u(stem_part)[1] - 0.06)
    c["Ж"] = left_arms | stem_part | right_part
    c["Д"] = (PEN.stroke([(0.78, cap), (0.74, 0.0)])
              | PEN.stroke([(0.8, cap), (0.48, cap - 0.08), (0.3, 0.7), (0.12, 0.05)], start=0.8, end=1.0)
              | PEN.stroke([(-0.04, 0.02), (0.98, 0.02)], start=0.8, end=0.8)
              | PEN.stroke([(-0.02, 0.02), (-0.06, -0.26)], start=1.0, end=0.5)
              | PEN.stroke([(0.96, 0.02), (1.0, -0.26)], start=1.0, end=0.5))
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
    oval_small = scale(o, 1.15, 1.0, thin=2)
    om = body_centre(oval_small)
    c["ф"] = oval_small | PEN.stroke([(om + 0.03, 1.62), (om - 0.03, -0.44)])
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


def build(t):
    """All glyph masks, from the traced letters t: (Latin, figures, punctuation, accented Latin,
    Cyrillic, marks), each a dict of glyph name or character to mask."""
    global PEN
    PEN = matched_pen(t)
    g = latin(t)
    f = figures(t, g)
    m = marks(t)
    p = punctuation(t, g)
    a = accented(t, g, m)
    cyr = cyrillic(t, g, f, m)
    return g, f, p, a, cyr, m
