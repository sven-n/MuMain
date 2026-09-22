"""Retain immutable merged-baseline inputs and a labeled contact sheet."""

from __future__ import annotations

import subprocess
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

from common import ASSETS, BASE, ROOT, WORK, digest, mu_texture, write_json

SHEET_SIZE = (1380, 1340)
COLUMNS = 2
COLUMN_STEP = 680
ROW_STEP = 185
THUMB_LIMIT = (640, 145)
MARGIN = 20
LABEL_HEIGHT = 28


def preserve(path, data):
    if path.exists():
        if path.read_bytes() != data:
            raise RuntimeError(f"Refusing to overwrite retained input: {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def retain_asset(relative):
    tracked = f"src/bin/Data/{relative}"
    raw = subprocess.check_output(["git", "show", f"{BASE}:{tracked}"], cwd=ROOT)
    container = WORK / "original/containers" / relative
    payload = WORK / "original" / Path(relative).with_suffix(".jpg")
    preserve(container, raw)
    preserve(payload, mu_texture.unwrap_bytes(raw, ".ozj"))
    with Image.open(payload) as image:
        size, mode = image.size, image.mode
    return {"path": relative, "size": size, "mode": mode, "alpha": [255, 255],
            "gpu_allocation": [1 << (side - 1).bit_length() for side in size],
            "container_sha256": digest(container), "payload_sha256": digest(payload),
            "baseline_warnings": mu_texture.check_file(container)}


def contact_sheet(entries):
    canvas = Image.new("RGB", SHEET_SIZE, (19, 23, 30))
    draw = ImageDraw.Draw(canvas)
    font = ImageFont.load_default(size=17)
    for index, entry in enumerate(entries):
        x = MARGIN + (index % COLUMNS) * COLUMN_STEP
        y = MARGIN + (index // COLUMNS) * ROW_STEP
        payload = WORK / "original" / Path(entry["path"]).with_suffix(".jpg")
        with Image.open(payload) as original:
            factor = min(4, THUMB_LIMIT[0] / original.width, THUMB_LIMIT[1] / original.height)
            size = tuple(round(side * factor) for side in original.size)
            canvas.paste(original.resize(size, Image.Resampling.NEAREST), (x, y + LABEL_HEIGHT))
        draw.text((x, y), f"{entry['path']} / {entry['size']}", font=font, fill="white")
    canvas.save(WORK / "inventory/baseline-contact.png")


def main():
    entries = [retain_asset(relative) for relative in ASSETS]
    write_json(WORK / "inventory/assets.json", {"base": BASE, "assets": entries})
    contact_sheet(entries)
    print(f"Retained {len(entries)} exact baseline containers/payloads and contact sheet.")


if __name__ == "__main__":
    main()
