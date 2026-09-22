"""Assemble the labeled offline review sheet from the three Blender renders."""

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
WIDTH, HEIGHT = 1920, 1080
PAPER = (23, 26, 25)
TEXT = (210, 213, 203)
MUTED = (155, 165, 158)
ACCENT = (209, 194, 148)
FONT_PATH = Path("/System/Library/Fonts/Supplemental/Arial.ttf")


def font(size):
    if FONT_PATH.exists():
        return ImageFont.truetype(str(FONT_PATH), size)
    return ImageFont.load_default(size=size)


def place_image(canvas, name, position, size):
    with Image.open(ROOT / name) as image:
        canvas.paste(image.convert("RGB").resize(size, Image.Resampling.LANCZOS), position)


def main():
    triangles = json.loads((ROOT.parent / "validation/blender.json").read_text())["triangles"]
    canvas = Image.new("RGB", (WIDTH, HEIGHT), PAPER)
    draw = ImageDraw.Draw(canvas)
    title, label, small = font(32), font(24), font(19)
    draw.text((40, 25), "BEER01  /  LORENCIA TAVERN STILL LIFE", font=title, fill=TEXT)
    draw.text((40, 72), "Offline Blender comparison — matching camera and lighting — client verification pending", font=label, fill=MUTED)
    views = [("before.png", "ORIGINAL  ·  216 TRIANGLES"),
             ("after.png", f"REBUILT  ·  {triangles} TRIANGLES"),
             ("wireframe.png", "EXPORTED TOPOLOGY  ·  1 ATLAS")]
    for index, (name, heading) in enumerate(views):
        x = 40 + index * 630
        place_image(canvas, name, (x, 150), (590, 443))
        draw.text((x, 112), heading, font=label, fill=ACCENT)
    draw.text((40, 628), "REDUCED-SCALE READABILITY STUDY", font=label, fill=ACCENT)
    for index, name in enumerate(("before.png", "after.png")):
        x = 220 + index * 420
        place_image(canvas, name, (x, 695), (200, 150))
        draw.text((x, 850), "Original" if index == 0 else "Rebuilt", font=label, fill=TEXT)
    details = ["Same 5 bones · same 1-frame action · lock=0",
               "Original bounds: 91.20 × 58.57 × 61.80 units",
               "Rebuilt bounds: 91.19 × 58.48 × 61.80 units",
               "512×512 diffuse atlas: plate2.jpg"]
    for index, text in enumerate(details):
        draw.text((1070, 695 + index * 42), text, font=label, fill=TEXT)
    draw.text((40, 961), "This sheet is not a game screenshot. Per-vertex lighting, scale and placement require review in the running client.", font=small, fill=MUTED)
    draw.text((40, 997), "Reference location: Lorencia ≈ (127.4, 128.4). Skeleton-only bmdconv comparison: EQUIVALENT.", font=small, fill=MUTED)
    canvas.save(ROOT / "comparison.png")


if __name__ == "__main__":
    main()
