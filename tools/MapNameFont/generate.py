#!/usr/bin/env python3
"""Generate the map-name font from the client's map-name images.

    python tools/MapNameFont/generate.py --data "<MU client>/Data" --out src/bin/fonts/MuMapName-Italic.otf

--data is the client's Data folder (it needs Local/Eng/ImgsMapName). --preview writes a page that
shows every glyph next to the original images. See README.md.
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from font_writer import write_font  # noqa: E402
from letters import X_HEIGHT, cut_letters, letter_masks  # noqa: E402
from recipes import build  # noqa: E402
from shapes import top_v  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--data", required=True, type=Path, help="the client's Data folder")
    parser.add_argument("--out", required=True, type=Path, help="the .otf file to write")
    parser.add_argument("--preview", type=Path, help="folder for a preview page")
    args = parser.parse_args()

    local_dir = args.data / "Local"
    if not (local_dir / "Eng" / "ImgsMapName").is_dir():
        parser.error(f"{local_dir / 'Eng' / 'ImgsMapName'} not found")
    samples = cut_letters(local_dir)
    masks, bearings, half_gap, word_gap = letter_masks(samples)
    latin, figures, punctuation, accented, cyrillic, _ = build(masks)
    glyphs = {**latin, **figures, **punctuation, **accented, **cyrillic}
    cap_height = top_v(masks["B"]) * X_HEIGHT
    entries = write_font(glyphs, bearings, half_gap, word_gap, cap_height, args.out)
    print(f"{len(entries)} glyphs from {len(samples)} letter samples -> {args.out}")
    if args.preview:
        from preview import write_preview
        write_preview(args.preview, args.out, local_dir)
        print(f"preview: {args.preview / 'index.html'}")


if __name__ == "__main__":
    main()
