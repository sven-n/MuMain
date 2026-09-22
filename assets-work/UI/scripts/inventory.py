#!/usr/bin/env python3
"""Index untouched Interface payloads and create labeled reference sheets.

Run after mu_texture.py unwrap-dir. Requires Pillow; never writes game data.
"""

from __future__ import annotations

import csv
import hashlib
import json
import shutil
import textwrap
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[3]
WORK = ROOT / "assets-work/UI"
ORIGINAL = WORK / "original/Interface"
OUTPUT = WORK / "inventory"
CELL = (240, 180)
COLUMNS = 5
ROWS = 6
PER_PAGE = COLUMNS * ROWS
FONT = ImageFont.load_default(size=12)
PAYLOADS = {".ozj": ".jpg", ".ozt": ".tga", ".ozb": ".bmp"}
RAW_IMAGES = {".jpg", ".tga", ".bmp"}
PILOT = ["newui_menu03"] + [f"newui_menu_Bt{i:02d}" for i in range(1, 5)]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_index():
    result = {}
    for path in sorted((ROOT / "src/source").rglob("*")):
        if path.suffix not in {".cpp", ".h"}:
            continue
        for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
            if ".jpg" in line.lower() or ".tga" in line.lower():
                result.setdefault(path.relative_to(ROOT).as_posix(), []).append((number, line))
    return result


def matching_references(stem, source):
    needle = (stem + ".").lower()
    return [f"{path}:{number}: {line.strip()}" for path, lines in source.items()
            for number, line in lines if needle in line.lower()]


def describe(path, source):
    relative = path.relative_to(ROOT / "src/bin/Data/Interface")
    extension = path.suffix.lower()
    payload = ORIGINAL / relative.with_suffix(PAYLOADS.get(extension, extension))
    if extension in RAW_IMAGES and not payload.exists():
        payload.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, payload)
    with Image.open(payload) as image:
        alpha = image.convert("RGBA").getchannel("A")
        histogram = alpha.histogram()
        result = {"path": path.relative_to(ROOT).as_posix(), "container": path.suffix,
                  "payload": payload.relative_to(WORK).as_posix(), "mode": image.mode,
                  "width": image.width, "height": image.height,
                  "alpha_min_max": list(alpha.getextrema()),
                  "transparent_pixels": histogram[0], "partial_alpha_pixels": sum(histogram[1:255]),
                  "opaque_pixels": histogram[255], "container_sha256": digest(path),
                  "payload_sha256": digest(payload), "references": matching_references(path.stem, source)}
    if extension in {".ozt", ".tga"}:
        header = payload.read_bytes()[:18]
        result["tga_header"] = {"type": header[2], "bpp": header[16], "descriptor": header[17]}
    return result


def checker(size):
    image = Image.new("RGB", size, "#30333b")
    draw = ImageDraw.Draw(image)
    tile = 12
    for y in range(0, size[1], tile):
        for x in range(0, size[0], tile):
            if (x // tile + y // tile) % 2:
                draw.rectangle((x, y, x + tile - 1, y + tile - 1), fill="#555861")
    return image


def draw_cell(sheet, entry, index):
    x, y = (index % COLUMNS) * CELL[0], 48 + (index // COLUMNS) * CELL[1]
    image = Image.open(WORK / entry["payload"]).convert("RGBA")
    image.thumbnail((CELL[0] - 16, CELL[1] - 70))
    background = checker(image.size)
    background.paste(image, mask=image.getchannel("A"))
    sheet.paste(background, (x + 8, y + 8))
    name = entry["path"].split("Interface/", 1)[1]
    name_lines = "\n".join(textwrap.wrap(name, width=34))
    label = f"{name_lines}\n{entry['width']}x{entry['height']} {entry['mode']} A={entry['alpha_min_max']}"
    draw = ImageDraw.Draw(sheet)
    draw.multiline_text((x + 8, y + CELL[1] - 58), label, font=FONT, fill="#e9e4d8", spacing=1)


def contact_sheets(entries):
    for start in range(0, len(entries), PER_PAGE):
        page = start // PER_PAGE + 1
        sheet = Image.new("RGB", (CELL[0] * COLUMNS, 48 + CELL[1] * ROWS), "#141820")
        ImageDraw.Draw(sheet).text((12, 14), f"UNTOUCHED UI ORIGINALS / page {page:02d} / thumbnails only",
                                  fill="white", font=FONT)
        for index, entry in enumerate(entries[start:start + PER_PAGE]):
            draw_cell(sheet, entry, index)
        sheet.save(OUTPUT / f"contact-{page:02d}.png")


def pilot_sheet():
    sheet = Image.new("RGB", (1100, 1040), "#171b23")
    draw = ImageDraw.Draw(sheet)
    draw.text((20, 15), "PILOT ORIGINAL ATLAS REFERENCE / 4x inspection view", font=FONT, fill="white")
    panel = Image.open(ORIGINAL / "partCharge1/newui_menu03.jpg")
    panel.resize((1024, 204), Image.Resampling.NEAREST).save(OUTPUT / "panel-reference.png")
    sheet.paste(panel.resize((1024, 204), Image.Resampling.NEAREST), (20, 60))
    draw.text((20, 38), "newui_menu03 256x51: content y=0..40 / XP y=41..50 / loader pads to 256x64",
              font=FONT, fill="white")
    for index, name in enumerate(PILOT[1:]):
        image = Image.open(ORIGINAL / f"partCharge1/{name}.jpg")
        x = 40 + index * 270
        sheet.paste(image.resize((120, 656), Image.Resampling.NEAREST), (x, 345))
        draw.text((x, 324), name, font=FONT, fill="white")
        for state in range(4):
            draw.text((x + 136, 350 + state * 164), f"row {state}\ny={state * 41}\n30x41", font=FONT, fill="white")
    sheet.save(OUTPUT / "pilot-original-atlases.png")


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    snapshot = OUTPUT / "interface.json"
    if snapshot.exists():
        # Retained original metadata must not be replaced with installed exports.
        entries = json.loads(snapshot.read_text())
        for entry in entries:
            if digest(WORK / entry["payload"]) != entry["payload_sha256"]:
                raise RuntimeError(f"Original was modified: {entry['payload']}")
        contact_sheets(entries)
        pilot_sheet()
        print(f"Regenerated sheets from {len(entries)} verified original payloads.")
        return
    source = source_index()
    files = sorted(path for path in (ROOT / "src/bin/Data/Interface").rglob("*")
                   if path.suffix.lower() in PAYLOADS or path.suffix.lower() in RAW_IMAGES)
    entries = [describe(path, source) for path in files]
    (OUTPUT / "interface.json").write_text(json.dumps(entries, indent=2) + "\n")
    columns = ["path", "container", "payload", "mode", "width", "height", "alpha_min_max",
               "transparent_pixels", "partial_alpha_pixels", "opaque_pixels", "container_sha256", "payload_sha256"]
    with (OUTPUT / "interface.csv").open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=columns, extrasaction="ignore", lineterminator="\n")
        writer.writeheader()
        writer.writerows(entries)
    contact_sheets(entries)
    pilot_sheet()
    print(f"Indexed {len(entries)} textures; wrote {(len(entries) + PER_PAGE - 1) // PER_PAGE} contact sheets.")


if __name__ == "__main__":
    main()
