"""Letters cut out of the client's English map-name images, as high-resolution masks.

The original map names are images (Data/Local/Eng/ImgsMapName): the name in silver italic
lettering with a dark edge, above an ornament. Every image's name is known, so its letters can be
cut out:

1. The image is straightened by the lettering's slant, and the silver fill is separated from the
   dark edge by brightness.
2. The fill is cut into one piece per letter of the name. Script letters touch, so the cuts are
   placed together (dynamic programming) at thin columns and near each letter's expected width;
   the widths are learned from a first pass.
3. Every sample of a letter is scaled to one x-height (long names were squeezed to fit the image),
   enlarged smoothly, lined up, and the samples that agree are averaged.

The masks share one upright frame: the baseline is row BASELINE, the x-height X_HEIGHT rows, and
each letter's leftmost ink starts at column ANCHOR_X.
"""
from collections import defaultdict, deque
from pathlib import Path

import numpy as np
from scipy import ndimage

from ozt import read_ozt

# file stem, name, squeezed to fit the 166 px image
SOURCES = [
    ("aida", "Aida", False), ("atlans", "Atlans", False), ("devias", "Devias", False),
    ("Elbeland", "Elbeland", False), ("EmpireGuardian", "Varka", False),
    ("Icarus", "Icarus", False), ("Kalima", "Kalima", False), ("lostkalima", "Kalima", False),
    ("kantru", "Kanturu", False), ("lorencia", "Lorencia", False), ("losttower", "Lost Tower", False),
    ("noria", "Noria", False), ("santatown", "Santa Town", False), ("stadium", "Stadium", False),
    ("bloodcastle", "Blood Castle", True), ("chaoscastle", "Chaos Castle", True),
    ("crywolffortress", "Crywolf Fortress", True), ("devilsquare", "Devil Square", True),
    ("Doppelganger", "Doppelganger", True), ("IllusionTemple", "Illusion Temple", True),
    ("loren", "Valley of Loren", True), ("MapName_MarketRolen", "Loren Market", True),
    ("ordeal", "Land of Trial", True), ("MapNameAddStrife", "Battle zone", True),
]
# Dungeon and Tarkan use a bevelled variant of the lettering; the other images a different face.

NAME_ROWS = 60          # rows read: the name, its descenders and swashes, and the ornament's top
ORNAMENT_TOP = 44       # pieces that start this low are the ornament, not part of a letter
# The fill runs from white at the top of a letter to grey at its bottom, so its edge is not one
# brightness: a pixel's coverage is its brightness between the dark edge and the brightest fill
# around it, and the letter is where that is over half.
EDGE_DARK = 20          # brightness of the dark edge
FILL_NEAR = 5           # px: where to look for the fill's brightness
MIN_FILL = 90           # darker surroundings hold no fill
INK_LEVEL = 128         # half coverage (0-255)
SLANT = 0.18            # horizontal shift per row of the italic lettering
STRAIGHTEN_PAD = 24     # room for the straightened image
X_HEIGHT_CHARS = set("acemnorsuvwxz")
BASELINE_CHARS = set("acemnorsuvwxzABCDEFIKLMNSTVbdhiklt")
# Typical letter widths in x-heights: a starting guess, refined from the cut letters.
WIDTH_GUESS = {**{c: 0.95 for c in "abcdeghknopqsuvxyz"}, "i": 0.4, "l": 0.45, "j": 0.45, "t": 0.55,
               "r": 0.65, "f": 0.7, "s": 0.7, "c": 0.75, "e": 0.8, "m": 1.5, "w": 1.4,
               **{c: 1.35 for c in "ABCDEFGHKNOPQRSUVXYZ"}, "I": 0.6, "J": 0.7, "L": 1.1, "T": 1.2,
               "M": 1.8, "W": 1.9}
WIDTH_WEIGHT = 60.0     # cost of a letter one expected width off, in ink pixels crossed by a cut
WIDTH_PASSES = 3

TARGET_XH = 20.0        # normalized x-height in source pixels
UPSCALE = 10            # mask pixels per normalized source pixel
SMOOTHING = 0.45        # blur before thresholding, in source pixels
BASELINE = 520          # frame: rows above the baseline
BELOW = 260             # frame: rows under it
WIDTH = 700
ANCHOR_X = 40
X_HEIGHT = int(TARGET_XH * UPSCALE)
# Detached pieces that belong to the letter: the i and j dots, the client's two-part M.
KEEP_PIECES = set("ijM")
AGREEMENT = 0.55        # overlap with the most typical sample needed to join the average
MAX_AVERAGED = 6
BEARING_PULL = 0.5      # how strongly a letter's side bearing is held near the typical half gap


# ---------------------------------------------------------------- cutting the names

def fill_coverage(rgba):
    """0-255: how much of each pixel is silver fill rather than dark edge or background."""
    rgb = rgba[..., :3].astype(np.float32)
    value = rgb.max(axis=2) * (rgba[..., 3].astype(np.float32) / 255.0)
    fill = ndimage.maximum_filter(value, size=FILL_NEAR)
    coverage = np.clip((value - EDGE_DARK) / np.maximum(fill - EDGE_DARK, 1.0), 0.0, 1.0)
    coverage[fill < MIN_FILL] = 0.0
    return coverage * 255.0


def without_ornament(coverage):
    """Clear the pieces that start below the name: the ornament. Descenders and swashes hang
    from their letters, so they start higher and stay."""
    out = coverage.copy()
    labels, count = ndimage.label(coverage > INK_LEVEL / 4)
    for index, rows in enumerate(ndimage.find_objects(labels), start=1):
        if rows is not None and rows[0].start >= ORNAMENT_TOP:
            out[labels == index] = 0.0
    return out


def straighten(value, baseline):
    """Undo the italic slant around the baseline (x' = x - SLANT * (baseline - y))."""
    height, width = value.shape
    out = np.zeros((height, width + 2 * STRAIGHTEN_PAD), np.float32)
    xs = np.arange(width + 2 * STRAIGHTEN_PAD, dtype=np.float32) - STRAIGHTEN_PAD
    for y in range(height):
        out[y] = np.interp(xs + SLANT * (baseline - y), np.arange(width), value[y], left=0, right=0)
    return out


def pieces(mask):
    """4-connected pieces as (y, x) pixel arrays."""
    labels, count = ndimage.label(mask)
    return [np.argwhere(labels == i) for i in range(1, count + 1)]


def x_height_band(value):
    """Rows where most columns with ink have ink, the x-height band of a name in small letters:
    its height and its bottom row (about the baseline)."""
    rows = (value > INK_LEVEL).sum(axis=1)
    band = np.nonzero(rows >= 0.5 * rows.max())[0]
    return band[-1] - band[0] + 1, band[-1] + 1


def cut_columns(columns, letters, x_height, widths):
    """Column ranges, one per letter, that cross little ink and fit the expected widths."""
    inked = np.nonzero(columns)[0]
    start, end = inked[0], inked[-1] + 1
    expected = np.array([widths.get(ch, 0.9) for ch in letters]) * x_height
    # Squeezed names: scale the expectations to the ink actually there.
    expected *= (columns[start:end] > 0).sum() / expected.sum()
    n, span = len(letters), end - start
    filled = np.concatenate([[0], np.cumsum(columns[start:end] > 0)])
    cost = np.full((n + 1, span + 1), np.inf)
    back = np.zeros((n + 1, span + 1), np.int32)
    cost[0, 0] = 0.0
    for i in range(1, n + 1):
        for b in range(i, span + 1):        # i letters need at least i columns
            if i == n and b != span:
                continue
            cut = columns[start + b] if start + b < len(columns) else 0.0
            width = filled[b] - filled[i - 1:b]
            options = cost[i - 1, i - 1:b] + WIDTH_WEIGHT * ((width - expected[i - 1]) / expected[i - 1]) ** 2
            best = int(np.argmin(options))
            cost[i, b] = options[best] + (cut if i < n else 0.0)
            back[i, b] = best + i - 1
    cuts, b = [], span
    for i in range(n, 0, -1):
        a = back[i, b]
        cuts.append((start + a, start + b))
        b = a
    return cuts[::-1]


def grow_along_ink(mask, owner):
    """Give every unowned ink pixel the owner nearest to it along the ink (8-connected)."""
    height, width = mask.shape
    frontier = deque(zip(*np.nonzero(owner >= 0)))
    while frontier:
        y, x = frontier.popleft()
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                ny, nx = y + dy, x + dx
                if 0 <= ny < height and 0 <= nx < width and mask[ny, nx] and owner[ny, nx] < 0:
                    owner[ny, nx] = owner[y, x]
                    frontier.append((ny, nx))


def letter_shapes(mask, letters, x_height, widths, baseline):
    """One pixel set per letter. Column ranges give each letter a core: the middle of its range,
    in the band from the baseline up one x-height. The rest of the ink goes to the core it is
    nearest to along the ink, so strokes reaching over a neighbour stay with their letter (a V's
    arm, the swash of K and k under the next letter, a T's bar). Ink no core reaches (an i dot)
    goes by column, and to the letter its piece mostly lies over."""
    cuts = cut_columns(mask.sum(axis=0).astype(np.float32), letters, x_height, widths)
    band_top, band_bottom = int(round(baseline - x_height)), int(round(baseline))
    owner = np.full(mask.shape, -1, np.int32)
    for index, (a, b) in enumerate(cuts):
        margin = (b - a) // 4
        core = np.zeros(mask.shape, bool)
        core[band_top:band_bottom, a + margin:max(b - margin, a + margin + 1)] = True
        owner[core & mask] = index
    grow_along_ink(mask, owner)
    by_column = np.full(mask.shape, -1, np.int32)
    for index, (a, b) in enumerate(cuts):
        by_column[:, a:b] = index
    for piece in pieces(mask & (owner < 0)):
        votes = np.bincount(by_column[piece[:, 0], piece[:, 1]] + 1, minlength=len(letters) + 1)[1:]
        owner[piece[:, 0], piece[:, 1]] = int(np.argmax(votes))
    return [np.argwhere(owner == index) for index in range(len(letters))]


def name_samples(local_dir, stem, text, squeezed, widths):
    path = next(Path(local_dir, "Eng", "ImgsMapName").glob(stem + ".*"))
    rgba = read_ozt(path)
    raw = without_ornament(fill_coverage(rgba[:min(NAME_ROWS, rgba.shape[0])]))
    ink_rows = np.nonzero((raw > INK_LEVEL).any(axis=1))[0]
    text_height = ink_rows[-1] - ink_rows[0] + 1
    value = straighten(raw, ink_rows[-1] - 0.2 * text_height)
    band, band_bottom = x_height_band(value)
    letters = text.replace(" ", "")
    shapes = letter_shapes(value > INK_LEVEL, letters, band, widths, band_bottom)
    bottoms = [s[:, 0].max() + 1 for s, ch in zip(shapes, letters) if ch in BASELINE_CHARS]
    baseline = float(np.median(bottoms))
    samples = []
    for order, (shape, ch) in enumerate(zip(shapes, letters)):
        own = np.zeros(value.shape, bool)
        own[shape[:, 0], shape[:, 1]] = True
        # Keep the anti-aliased rim, but none of a neighbour's pixels.
        grown = ndimage.binary_dilation(own, structure=np.ones((3, 3), bool), iterations=2)
        for other in shapes:
            if other is not shape:
                grown[other[:, 0], other[:, 1]] = False
        top, left = shape[:, 0].min(), shape[:, 1].min()
        bottom, right = shape[:, 0].max() + 1, shape[:, 1].max() + 1
        y0, x0 = max(top - 3, 0), max(left - 3, 0)
        y1, x1 = min(bottom + 3, value.shape[0]), min(right + 3, value.shape[1])
        samples.append(dict(char=ch, source=stem, text=text, order=order, squeezed=squeezed,
                            box=(top, left, bottom, right), origin=(y0, x0),
                            value=np.where(grown, value, 0)[y0:y1, x0:x1], baseline=baseline, xh_band=band))
    return samples


def learned_widths(samples):
    """Median ink width of each letter in x-heights, from unsqueezed names."""
    found = defaultdict(list)
    for s in samples:
        if not s["squeezed"]:
            top, left, bottom, right = s["box"]
            found[s["char"]].append((right - left) / s["xh_band"])
    widths = dict(WIDTH_GUESS)
    widths.update({ch: float(np.median(v)) for ch, v in found.items() if len(v) >= 2})
    return widths


def cut_letters(local_dir):
    """Every letter sample of every source name."""
    widths = dict(WIDTH_GUESS)
    for _ in range(WIDTH_PASSES):
        samples = [s for source in SOURCES for s in name_samples(local_dir, *source, widths)]
        widths = learned_widths(samples)
    return samples


# ---------------------------------------------------------------- one mask per letter

def cubic_resize(image, height, width):
    """Separable Keys cubic resampling."""
    def weights(size_in, size_out):
        centre = (np.arange(size_out) + 0.5) * (size_in / size_out) - 0.5
        taps = np.floor(centre).astype(int)[:, None] - 1 + np.arange(4)[None, :]
        t = np.abs(centre[:, None] - taps)
        a = -0.5
        w = np.where(t <= 1, (a + 2) * t ** 3 - (a + 3) * t ** 2 + 1,
                     np.where(t < 2, a * t ** 3 - 5 * a * t ** 2 + 8 * a * t - 4 * a, 0.0))
        return np.clip(taps, 0, size_in - 1), w / w.sum(axis=1, keepdims=True)

    rows, wr = weights(image.shape[0], height)
    cols, wc = weights(image.shape[1], width)
    tmp = (image[rows] * wr[:, :, None]).sum(axis=1)
    return (tmp[:, cols] * wc[None, :, :]).sum(axis=2)


def squeeze_factors(samples, widths):
    """How much each squeezed name was narrowed, from its letters' widths."""
    found = defaultdict(list)
    for s in samples:
        if s["squeezed"] and s["char"] in widths:
            top, left, bottom, right = s["box"]
            found[s["source"]].append((right - left) / s["xh_band"] / widths[s["char"]])
    return {source: float(np.median(v)) for source, v in found.items()}


def drop_stray_pieces(big, char):
    """Remove bits of a neighbour: small pieces, or ones beside the letter rather than part of it."""
    if char in KEEP_PIECES:
        return big
    found = pieces(big > INK_LEVEL)
    if not found:
        return big
    main = max(found, key=len)
    left_edge, right_edge = main[:, 1].min(), main[:, 1].max()
    for p in found:
        if p is main:
            continue
        overlaps = p[:, 1].max() >= left_edge and p[:, 1].min() <= right_edge
        if len(p) < 0.2 * len(main) or (not overlaps and len(p) < 0.6 * len(main)):
            big[p[:, 0], p[:, 1]] = 0
    return big


def enlarged(sample, squeeze):
    """The sample at the shared scale: image, baseline row, left ink column."""
    scale_y = TARGET_XH / sample["xh_band"] * UPSCALE
    scale_x = scale_y / squeeze
    value = sample["value"]
    big = cubic_resize(value, max(1, round(value.shape[0] * scale_y)), max(1, round(value.shape[1] * scale_x)))
    big = drop_stray_pieces(big, sample["char"])
    baseline = (sample["baseline"] - sample["origin"][0]) * scale_y
    left = (sample["box"][1] - sample["origin"][1]) * scale_x
    return big, baseline, left


def in_frame(image, baseline, left):
    canvas = np.zeros((BASELINE + BELOW, WIDTH), np.float32)
    dy, dx = int(round(BASELINE - baseline)), int(round(ANCHOR_X - left))
    ys, xs = np.nonzero(image > 0)
    ty, tx = ys + dy, xs + dx
    ok = (ty >= 0) & (ty < canvas.shape[0]) & (tx >= 0) & (tx < WIDTH)
    canvas[ty[ok], tx[ok]] = image[ys[ok], xs[ok]]
    return canvas


def averaged(entries):
    """Average the samples that agree with the most typical one."""
    canvases = [in_frame(*entry) for entry in entries]
    if len(canvases) <= 2:
        return canvases[0]
    masks = [c > INK_LEVEL for c in canvases]
    overlap = np.array([[(a & b).sum() / max((a | b).sum(), 1) for b in masks] for a in masks])
    typical = int(np.argmax(overlap.mean(axis=1)))
    chosen = [i for i in np.argsort(-overlap[typical]) if overlap[typical, i] >= AGREEMENT][:MAX_AVERAGED]
    return np.mean([canvases[i] for i in chosen], axis=0)


def blur(image, sigma):
    return ndimage.gaussian_filter(image, sigma, mode="constant")


def body_edges(mask, baseline_row, x_height_rows):
    """Left and right ink of a letter's body: from the baseline up one x-height, so swashes,
    tails and a T's bar don't count."""
    top = max(int(round(baseline_row - x_height_rows)), 0)
    cols = np.nonzero(mask[top:int(round(baseline_row))].any(axis=0))[0]
    if len(cols) == 0:
        cols = np.nonzero(mask.any(axis=0))[0]
    return cols[0], cols[-1] + 1


def neighbour_gaps(samples):
    """(left letter, right letter, gap) for every pair of touching letters in the plain names, and
    the gaps between words; gaps between letter bodies, in mask pixels."""
    by_source = defaultdict(list)
    for s in samples:
        by_source[s["source"]].append(s)
    pairs, word_gaps = [], []
    for letters in by_source.values():
        if letters[0]["squeezed"]:
            continue
        letters.sort(key=lambda s: s["order"])
        positions = [i for i, ch in enumerate(letters[0]["text"]) if ch != " "]
        unit = TARGET_XH / letters[0]["xh_band"] * UPSCALE
        edges = []
        for s in letters:
            left, right = body_edges(s["value"] > INK_LEVEL, s["baseline"] - s["origin"][0], s["xh_band"])
            edges.append((left + s["origin"][1], right + s["origin"][1]))
        for i in range(len(letters) - 1):
            gap = (edges[i + 1][0] - edges[i][1]) * unit
            if positions[i + 1] - positions[i] == 1:
                pairs.append((letters[i]["char"], letters[i + 1]["char"], gap))
            else:
                word_gaps.append(gap)
    return pairs, word_gaps


def side_bearings(pairs):
    """Each letter's own room on its left and right, so that right(a) + left(b) fits the gap of
    every pair a, b in the originals (least squares). Letters seen in few pairs stay near the
    typical half gap. Returns {letter: (left, right)} and the typical half gap, in mask pixels."""
    half = float(np.median([gap for _, _, gap in pairs])) / 2
    chars = sorted({c for a, b, _ in pairs for c in (a, b)})
    index = {c: i for i, c in enumerate(chars)}
    n = len(chars)
    rows, targets = [], []
    for a, b, gap in pairs:
        row_ = np.zeros(2 * n)
        row_[n + index[a]] = 1.0      # right side of a
        row_[index[b]] = 1.0          # left side of b
        rows.append(row_)
        targets.append(gap)
    for i in range(2 * n):            # pull towards the typical half gap
        row_ = np.zeros(2 * n)
        row_[i] = BEARING_PULL
        rows.append(row_)
        targets.append(BEARING_PULL * half)
    solution = np.linalg.lstsq(np.array(rows), np.array(targets), rcond=None)[0]
    return {c: (solution[index[c]], solution[n + index[c]]) for c in chars}, half


def letter_masks(samples):
    """One mask per letter (unsqueezed sources where there are any), each letter's side
    bearings, the typical half gap and the word gap, in mask pixels."""
    factors = squeeze_factors(samples, learned_widths(samples))
    by_char = defaultdict(list)
    for s in samples:
        by_char[s["char"]].append(s)
    masks = {}
    for ch, group in sorted(by_char.items()):
        plain = [s for s in group if not s["squeezed"]]
        entries = [enlarged(s, factors.get(s["source"], 1.0)) for s in (plain or group)]
        masks[ch] = blur(averaged(entries), SMOOTHING * UPSCALE) > INK_LEVEL
    pairs, word_gaps = neighbour_gaps(samples)
    bearings, half_gap = side_bearings(pairs)
    word_gap = float(np.median(word_gaps)) if word_gaps else None
    return masks, bearings, half_gap, word_gap
