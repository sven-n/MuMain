#!/usr/bin/env python3
"""Cut the map banner's ornament out of the client's map-name images: the left swirl, the right
swirl and each map's icon, as separate images.

    python tools/MapNameFont/ornament.py --data "<MU client>/Data" --out src/bin/Data/Interface/MapNameOrnament

--data is the client's Data folder (it needs Local/Eng, Por and Spn/ImgsMapName). --out gets
SwirlLeft.OZT, SwirlRight.OZT, one <banner>.OZT per banner with its icon, and Layout.json with where
each piece sat in the 166 x 90 banner. --preview writes a page that shows every banner rebuilt from
the pieces next to the original. See README.md.
"""
import argparse
import html
import json
import sys
from pathlib import Path

import numpy as np
from scipy import ndimage

sys.path.insert(0, str(Path(__file__).resolve().parent))

from ozt import over, read_ozt, write_ozt, write_png  # noqa: E402

LANGUAGES = ("Eng", "Por", "Spn")
NOT_A_BANNER = "mapnameaddstrife"   # the Gens battle-zone strip above the banner, no ornament
SWIRL_SOURCE = ("Por", "lorencia")  # the export of the swirls most banners share
SWIRLS = {"SwirlLeft": (60, 20), "SwirlRight": (60, 140)}   # a pixel inside each swirl there
SAME = 8            # visible difference (colour x opacity, 0-255) still counted as the same pixel
COVERED = 24        # a swirl pixel this different from the swirl has the icon over it
FILL_BRIGHT = 70    # silver fill: at least this bright (colour x opacity) and half opaque
ICON_TOP = 30       # the icon's fill pieces start this low; the name's start higher
SWIRL_ROW = 46      # ... and reach the swirls' top row; name pieces end above it
NEAR_ICON = 10      # px: pieces further from the icon than this are stray specks


def load_banners(local_dir):
    """{language: {stem in lower case: (stem as on disk, RGBA float image)}}"""
    banners = {}
    for language in LANGUAGES:
        folder = local_dir / language / "ImgsMapName"
        banners[language] = {path.stem.lower(): (path.stem, read_ozt(path).astype(np.float32))
                             for path in sorted(folder.iterdir())
                             if path.suffix.lower() == ".ozt" and path.stem.lower() != NOT_A_BANNER}
    return banners


def visible(image):
    """Colour times opacity, and opacity: what a pixel adds over any background."""
    return np.concatenate([image[..., :3] * image[..., 3:4] / 255.0, image[..., 3:4]], -1)


def piece_at(image, seed):
    """The connected piece of `image` around pixel `seed`, cropped, and its top-left corner."""
    labels, _ = ndimage.label(image[..., 3] > 0)
    mine = labels == labels[seed]
    rows, cols = np.nonzero(mine)
    box = (slice(rows.min(), rows.max() + 1), slice(cols.min(), cols.max() + 1))
    piece = np.where(mine[box][..., None], image[box], 0)
    return piece, (int(rows.min()), int(cols.min()))


def placed(piece, corner, shape):
    out = np.zeros(shape[:2] + (4,), np.float32)
    out[corner[0]:corner[0] + piece.shape[0], corner[1]:corner[1] + piece.shape[1]] = piece
    return out


def where_drawn(image, piece, corner):
    """The corner near `corner` where `image` shows `piece` best (one export sits a row lower)."""
    drawn = piece[..., 3] > 0
    best = None
    for dy in range(-2, 3):
        for dx in range(-2, 3):
            y, x = corner[0] + dy, corner[1] + dx
            window = image[y:y + piece.shape[0], x:x + piece.shape[1]]
            if y < 0 or x < 0 or window.shape[:2] != piece.shape[:2]:
                continue
            wrong = (np.abs(visible(window) - visible(piece))[drawn].max(-1) > SAME).sum()
            if best is None or wrong < best[0]:
                best = (wrong, (y, x))
    return best[1]


def shared_image(images):
    """The ornament is the same in every language and the name is not. Per pixel, the value of the
    language closest to the others (one language's letters over a pixel are outvoted), and whether
    another language shows it too."""
    if len(images) == 1:
        return images[0], np.ones(images[0].shape[:2], bool)
    shown = np.stack([visible(image) for image in images])
    apart = np.abs(shown[:, None] - shown[None, :]).max(-1)       # language x language x pixel
    pick = apart.sum(axis=1).argmin(axis=0)
    value = np.take_along_axis(np.stack(images), pick[None, ..., None], axis=0)[0]
    nearest_other = np.where(np.eye(len(images), dtype=bool)[..., None, None], np.inf, apart).min(axis=1)
    agreed = np.take_along_axis(nearest_other, pick[None], axis=0)[0] <= SAME
    return value, agreed


def fill_pieces(image, where):
    """Pieces of silver fill (grey to white, mostly opaque) inside `where`, and which of them are
    the icon's: they start below the name rows and reach the swirls' row."""
    fill = where & (visible(image)[..., :3].max(-1) >= FILL_BRIGHT) & (image[..., 3] >= 128)
    labels, count = ndimage.label(fill, np.ones((3, 3)))
    icon_piece = np.zeros(count + 1, bool)
    for index, box in enumerate(ndimage.find_objects(labels), start=1):
        icon_piece[index] = box[0].start >= ICON_TOP and box[0].stop - 1 >= SWIRL_ROW
    return labels, icon_piece


def icon_pixels(value, ink, images):
    """The icon's part of `ink`, which also holds bits of the name where two languages share them.

    The silver fill of a name and of an icon never touch, so each fill piece is told apart by where
    it lies. Outline, shadow and glow go with the fill they are connected to, the nearer one where
    both are: all fills grow a pixel at a time. Most of a name is in one language only, so each
    language's own name fill grows too, but only over pixels where that language shows the shared
    image: where it is the odd one out, the other two show what lies under its letters."""
    labels, icon_piece = fill_pieces(value, ink)
    icon_fill = icon_piece[labels] & (labels > 0)
    icon = icon_fill.copy()
    name = (labels > 0) & ~icon_fill
    shown = visible(value)
    own_names, shows_shared = [], []
    for image in images:
        own, own_icon = fill_pieces(image, image[..., 3] > 0)
        same = np.abs(visible(image) - shown).max(-1) <= SAME
        own_names.append((own > 0) & ~own_icon[own] & ~(same & ink))
        shows_shared.append(same)
    free = ink & ~icon & ~name
    step = np.ones((3, 3), bool)
    while True:
        to_icon = ndimage.binary_dilation(icon, step) & free
        to_name = ndimage.binary_dilation(name, step) & free & ~to_icon     # a tie goes to the icon
        for index, own in enumerate(own_names):
            grown = ndimage.binary_dilation(own, step) & free & shows_shared[index] & ~to_icon
            own_names[index] = own | grown
            to_name |= grown
        if not (to_icon.any() or to_name.any()):
            break
        icon |= to_icon
        name |= to_name
        free &= ~(to_icon | to_name)
    # Pieces without icon fill are bits of a name's edge; pieces far off are specks (Elbeland's
    # Portuguese and Spanish images have one in a corner).
    pieces, _ = ndimage.label(icon, step)
    icon = np.isin(pieces, np.unique(pieces[icon_fill & icon]))
    pieces, count = ndimage.label(icon, step)
    sizes = ndimage.sum(icon, pieces, range(1, count + 1))
    near = ndimage.binary_dilation(pieces == np.argmax(sizes) + 1, step, iterations=NEAR_ICON)
    return np.isin(pieces, np.unique(pieces[near & icon])) & icon


def cut_icon(images, swirls):
    """The icon of one banner (all its languages), in banner coordinates."""
    value, agreed = shared_image(images)
    under_swirls = np.zeros(value.shape[:2], bool)
    swirl_image = np.zeros_like(value)
    for piece, corner in swirls.values():
        here = placed(piece, where_drawn(value, piece, corner), value.shape)
        under_swirls |= here[..., 3] > 0
        swirl_image = np.where(here[..., 3:4] > 0, here, swirl_image)
    icon = icon_pixels(value, (value[..., 3] > 0) & agreed & ~under_swirls, images)
    # Where the icon overlaps a swirl (Atlans, Crywolf, Kalima), it keeps the pixels it changed.
    over_swirl = under_swirls & (np.abs(visible(value) - visible(swirl_image)).max(-1) > COVERED)
    while True:
        more = ndimage.binary_dilation(icon, np.ones((3, 3), bool)) & over_swirl & ~icon
        if not more.any():
            break
        icon |= more
    return np.where(icon[..., None], value, 0)


def cropped(image):
    rows, cols = np.nonzero(image[..., 3] > 0)
    box = (slice(rows.min(), rows.max() + 1), slice(cols.min(), cols.max() + 1))
    return image[box], (int(rows.min()), int(cols.min()))


def cut_ornament(local_dir):
    """{name: (RGBA uint8 piece, (top, left) in the banner)} for both swirls and every banner's
    icon, and the banner size."""
    banners = load_banners(local_dir)
    source = banners[SWIRL_SOURCE[0]][SWIRL_SOURCE[1]][1]
    swirls = {name: piece_at(source, seed) for name, seed in SWIRLS.items()}
    pieces = {name: (piece, corner) for name, (piece, corner) in swirls.items()}
    for key, (stem, _) in banners["Eng"].items():
        images = [banners[language][key][1] for language in LANGUAGES if key in banners[language]]
        pieces[stem] = cropped(cut_icon(images, swirls))
    pieces = {name: (np.clip(np.rint(piece), 0, 255).astype(np.uint8), corner)
              for name, (piece, corner) in pieces.items()}
    for piece, _ in pieces.values():
        piece[piece[..., 3] == 0] = 0
    return pieces, source.shape[:2]


def write_preview(folder, out_dir, layout, local_dir):
    """Every banner rebuilt from the written pieces next to the original."""
    folder.mkdir(parents=True, exist_ok=True)
    background = (24, 26, 31)
    height, width = layout["banner"]["height"], layout["banner"]["width"]

    def put(canvas, name, spot):
        piece = read_ozt(out_dir / f"{name}.OZT").astype(np.float32)
        area = canvas[spot["y"]:spot["y"] + piece.shape[0], spot["x"]:spot["x"] + piece.shape[1]]
        alpha = piece[..., 3:4] / 255.0
        area[:] = piece[..., :3] * alpha + area * (1 - alpha)

    rows = []
    for name, spot in layout["icons"].items():
        original = next((local_dir / "Eng" / "ImgsMapName").glob(name + ".*"))
        write_png(folder / f"{name}.png", over(read_ozt(original), background))
        canvas = np.empty((height, width, 3), np.float32)
        canvas[:] = background
        for swirl, swirl_spot in layout["swirls"].items():
            put(canvas, swirl, swirl_spot)
        put(canvas, name, spot)
        write_png(folder / f"{name}-rebuilt.png", canvas.astype(np.uint8))
        icon_only = np.empty((height, width, 3), np.float32)
        icon_only[:] = background
        put(icon_only, name, spot)
        write_png(folder / f"{name}-icon.png", icon_only.astype(np.uint8))
        rows.append(f"<tr><td>{html.escape(name)}</td><td><img src='{name}.png'></td>"
                    f"<td><img src='{name}-rebuilt.png'></td><td><img src='{name}-icon.png'></td></tr>")
    page = ("<!doctype html><meta charset=utf-8><title>Map name ornament</title><style>"
            "body { background: #181a1f; color: #ddd; font-family: sans-serif; margin: 12px; }"
            "td { padding: 2px 10px; } img { width: 332px; image-rendering: pixelated; }</style>"
            "<table><tr><th></th><th>original</th><th>swirls and icon</th><th>icon</th></tr>"
            f"{''.join(rows)}</table>")
    (folder / "index.html").write_text(page, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--data", required=True, type=Path, help="the client's Data folder")
    parser.add_argument("--out", required=True, type=Path, help="the folder to write the pieces to")
    parser.add_argument("--preview", type=Path, help="folder for a preview page")
    args = parser.parse_args()

    local_dir = args.data / "Local"
    for language in LANGUAGES:
        if not (local_dir / language / "ImgsMapName").is_dir():
            parser.error(f"{local_dir / language / 'ImgsMapName'} not found")
    pieces, (height, width) = cut_ornament(local_dir)
    args.out.mkdir(parents=True, exist_ok=True)
    layout = {"banner": {"width": width, "height": height}, "swirls": {}, "icons": {}}
    for name, (piece, (top, left)) in sorted(pieces.items(), key=lambda item: item[0].lower()):
        write_ozt(args.out / f"{name}.OZT", piece)
        group = "swirls" if name in SWIRLS else "icons"
        layout[group][name] = {"x": left, "y": top, "width": piece.shape[1], "height": piece.shape[0]}
    (args.out / "Layout.json").write_text(json.dumps(layout, indent=2) + "\n", encoding="utf-8")
    print(f"{len(layout['swirls'])} swirls and {len(layout['icons'])} icons -> {args.out}")
    if args.preview:
        write_preview(args.preview, args.out, layout, local_dir)
        print(f"preview: {args.preview / 'index.html'}")


if __name__ == "__main__":
    main()
