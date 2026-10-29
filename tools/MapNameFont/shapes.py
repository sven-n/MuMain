"""Building blocks for new glyphs: cut, move, mirror and scale traced letters, and a broad-nib pen.

All shapes are boolean masks in the letters' frame (letters.py): row BASELINE is the baseline, the
x-height is X_HEIGHT rows, and design coordinates (u, v) are in x-heights from column ORIGIN_X and
the baseline, v up. Shapes are upright; the italic slant is applied when outlines are written.
"""
import numpy as np
from scipy import ndimage

from letters import ANCHOR_X, BASELINE, BELOW, WIDTH, X_HEIGHT

ORIGIN_X = ANCHOR_X
SHAPE = (BASELINE + BELOW, WIDTH)


def empty():
    return np.zeros(SHAPE, bool)


def col(u):
    return ORIGIN_X + u * X_HEIGHT


def row(v):
    return BASELINE - v * X_HEIGHT


def ink_box(mask):
    rows = np.nonzero(mask.any(axis=1))[0]
    cols = np.nonzero(mask.any(axis=0))[0]
    return rows[0], rows[-1] + 1, cols[0], cols[-1] + 1


def extent_u(mask):
    """Left and right ink edges, in x-heights."""
    _, _, left, right = ink_box(mask)
    return (left - ORIGIN_X) / X_HEIGHT, (right - ORIGIN_X) / X_HEIGHT


def body_u(mask):
    """Left and right ink of the x-height band (from the baseline up one x-height)."""
    band = mask[BASELINE - X_HEIGHT:BASELINE]
    cols = np.nonzero(band.any(axis=0))[0]
    if len(cols) == 0:
        return extent_u(mask)
    return (cols[0] - ORIGIN_X) / X_HEIGHT, (cols[-1] + 1 - ORIGIN_X) / X_HEIGHT


def top_v(mask):
    return (BASELINE - ink_box(mask)[0]) / X_HEIGHT


def bottom_v(mask):
    return (BASELINE - ink_box(mask)[1]) / X_HEIGHT


def region(mask, u0=-9, u1=9, v0=-9, v1=9):
    """Keep only the ink inside the rectangle (design units)."""
    out = mask.copy()
    keep = empty()
    r0, r1 = int(round(max(row(v1), 0))), int(round(min(row(v0), SHAPE[0])))
    c0, c1 = int(round(max(col(u0), 0))), int(round(min(col(u1), SHAPE[1])))
    keep[r0:r1, c0:c1] = True
    return out & keep


def erase(mask, u0=-9, u1=9, v0=-9, v1=9):
    return mask & ~region(np.ones(SHAPE, bool), u0, u1, v0, v1)


def shift(mask, du=0.0, dv=0.0):
    return ndimage.shift(mask.astype(np.float32), (-dv * X_HEIGHT, du * X_HEIGHT), order=1) > 0.5


def place_left(mask, u=0.0):
    """Move the shape so its leftmost ink is at u."""
    return shift(mask, u - extent_u(mask)[0])


def mirror(mask):
    """Mirror left-right, keeping the left ink edge where it was."""
    left = extent_u(mask)[0]
    return place_left(mask[:, ::-1].copy(), left)


def flip(mask, centre_v=0.5):
    """Mirror top-bottom around the height centre_v."""
    flipped = mask[::-1].copy()
    # Row r maps to SHAPE[0]-1-r; move the centre back where it was.
    centre = row(centre_v)
    offset = 2 * centre - (SHAPE[0] - 1)
    return ndimage.shift(flipped.astype(np.float32), (offset, 0), order=1) > 0.5


def rotate180(mask, centre_v=0.5):
    return flip(mirror(mask), centre_v)


def scale(mask, sx, sy, sy_below=None, thin=0.0, anchor_u=None):
    """Scale around the baseline and the left ink edge (or anchor_u). sy_below scales the part
    under the baseline separately. thin: pixels taken off each stroke edge afterwards, to undo
    the weight gain of an enlargement."""
    sy_below = sy if sy_below is None else sy_below
    anchor = col(extent_u(mask)[0] if anchor_u is None else anchor_u)
    rows = np.arange(SHAPE[0], dtype=np.float32)[:, None]
    cols = np.arange(SHAPE[1], dtype=np.float32)[None, :]
    up = BASELINE - rows
    src_up = np.where(up >= 0, up / sy, up / sy_below)
    src_rows = BASELINE - src_up + 0 * cols
    src_cols = anchor + (cols - anchor) / sx + 0 * rows
    out = ndimage.map_coordinates(mask.astype(np.float32), [src_rows, src_cols], order=1) > 0.5
    if thin > 0:
        out = ndimage.distance_transform_edt(out) > thin
    return out


def thicken(mask, pixels):
    return ndimage.distance_transform_edt(~mask) <= pixels


def pieces(mask, min_size=0):
    labels, count = ndimage.label(mask)
    return [labels == i for i in range(1, count + 1) if (labels == i).sum() >= min_size]


def largest(mask):
    parts = pieces(mask)
    return max(parts, key=lambda p: p.sum()) if parts else mask


# ---------------------------------------------------------------- broad-nib pen

class Pen:
    """A broad nib: a flat edge `length` px long at `angle` degrees, `edge` px thick. A vertical
    stroke comes out length*cos(angle) wide, a horizontal one length*sin(angle) high."""

    def __init__(self, length, angle, edge):
        self.length, self.angle, self.edge = length, np.radians(angle), edge
        self._kernels = {}

    def kernel(self, pressure):
        key = round(pressure, 2)
        if key not in self._kernels:
            half = int(self.length / 2 + self.edge) + 2
            y, x = np.mgrid[-half:half + 1, -half:half + 1].astype(np.float32)
            y = -y
            along = x * np.cos(self.angle) + y * np.sin(self.angle)
            across = -x * np.sin(self.angle) + y * np.cos(self.angle)
            length = self.length * key
            self._kernels[key] = (np.abs(along) <= length / 2 + self.edge / 2) & (np.abs(across) <= self.edge / 2 + 0.5)
        return self._kernels[key]

    def stroke(self, points, start=1.0, end=1.0, mid=1.0, pressures=None):
        """Draw through the design points (Catmull-Rom curve). start/mid/end: nib length factors,
        for tapered entries and exits; or `pressures`, one factor per point."""
        path, where = _spline([(col(u), row(v)) for u, v in points])
        out = empty()
        n = len(path)
        for i, (x, y) in enumerate(path):
            t = i / max(n - 1, 1)
            if pressures is not None:
                pressure = float(np.interp(where[i], np.arange(len(points)), pressures))
            else:
                pressure = (start + (mid - start) * (t / 0.5)) if t < 0.5 else (mid + (end - mid) * ((t - 0.5) / 0.5))
            k = self.kernel(max(pressure, 0.05))
            half = k.shape[0] // 2
            r0, c0 = int(round(y)) - half, int(round(x)) - half
            r1, c1 = r0 + k.shape[0], c0 + k.shape[1]
            kr0, kc0 = max(0, -r0), max(0, -c0)
            r0c, c0c = max(r0, 0), max(c0, 0)
            r1c, c1c = min(r1, SHAPE[0]), min(c1, SHAPE[1])
            if r1c > r0c and c1c > c0c:
                out[r0c:r1c, c0c:c1c] |= k[kr0:kr0 + r1c - r0c, kc0:kc0 + c1c - c0c]
        return out

    def dot(self, u, v, size=1.0):
        """A round dot like the i dot, `size` stems across."""
        out = empty()
        y, x = np.mgrid[0:SHAPE[0], 0:SHAPE[1]]
        radius = self.length * np.cos(self.angle) * 0.62 * size
        out[(x - col(u)) ** 2 + (y - row(v)) ** 2 <= radius ** 2] = True
        return out


def _spline(points, step=1.0):
    """Points about `step` px apart along the curve, and where each lies: the index of the design
    point it starts from plus how far it is towards the next."""
    pts = np.array(points, np.float64)
    if len(pts) == 2:
        length = np.hypot(*(pts[1] - pts[0]))
        t = np.linspace(0, 1, max(int(length / step), 2))
        return pts[0] + (pts[1] - pts[0]) * t[:, None], t
    padded = np.vstack([2 * pts[0] - pts[1], pts, 2 * pts[-1] - pts[-2]])
    out, where = [], []
    for i in range(1, len(padded) - 2):
        p0, p1, p2, p3 = padded[i - 1:i + 3]
        length = np.hypot(*(p2 - p1))
        for t in np.linspace(0, 1, max(int(length / step), 2), endpoint=False):
            t2, t3 = t * t, t * t * t
            out.append(0.5 * ((2 * p1) + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t2
                              + (-p0 + 3 * p1 - 3 * p2 + p3) * t3))
            where.append(i - 1 + t)
    out.append(pts[-1])
    where.append(len(pts) - 1)
    return np.array(out), np.array(where)
