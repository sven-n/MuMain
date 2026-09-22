"""Assemble exact-size HUD replacements from retained imagegen components.

Run from the repository root. Writes this batch only; validation installs exports.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageEnhance

from common import (BUTTON_SIZE, CASH_PATH, EXP_PATHS, GAUGES, PANEL_PATHS, ROOT,
                    SKILL_PATHS, WORK, original, pilot_assembly)
from paint_parts import (LAYOUT, METAL_COLOR_STRENGTH, column_parts, experience,
                         gem_parts, painting, quiet_metal, slot)

JPEG_QUALITY = 100
XP_RAIL_COLORS = ((39, 46, 53), (109, 105, 88), (20, 26, 32),
                  (10, 15, 20), (10, 15, 20), (10, 15, 20), (10, 15, 20),
                  (20, 26, 32), (67, 73, 77), (7, 11, 16))
XP_SEPARATOR = (76, 75, 64)
TOP_RAIL = (115, 105, 75)


def save_asset(relative, master):
    path = Path(relative)
    locations = {kind: WORK / kind / path.with_suffix(suffix)
                 for kind, suffix in (("masters", ".png"), ("payloads", ".jpg"), ("exports", ".OZJ"))}
    for target in locations.values():
        target.parent.mkdir(parents=True, exist_ok=True)
    master = master.convert("RGB")
    master.save(locations["masters"])
    master.save(locations["payloads"], quality=JPEG_QUALITY, subsampling=0, optimize=False, progressive=False)
    editable = WORK / "source/editable" / path.with_suffix(".ora")
    editable.parent.mkdir(parents=True, exist_ok=True)
    pilot_assembly.write_ora(editable, original(relative), master.convert("RGBA"), master)
    subprocess.run([sys.executable, str(ROOT / "tools/mu_texture.py"), "wrap",
                    str(locations["payloads"]), "--out", str(locations["exports"])], check=True,
                   stdout=subprocess.DEVNULL)


def place_slots(canvas, spec):
    x, y = spec["start"]
    width, height = spec["cell"]
    for index, key in enumerate(spec["keys"]):
        canvas.paste(slot((width, height), key=key), (x + index * width, y))


def experience_trough(canvas):
    draw = ImageDraw.Draw(canvas)
    xp = LAYOUT["xp"]
    for row, color in enumerate(XP_RAIL_COLORS):
        y = xp["source_y"] + row
        draw.line((0, y, canvas.width - 1, y), fill=color)
    for x in xp["separator_x"]:
        draw.line((x, xp["source_y"] + 2, x, xp["source_y"] + 7), fill=XP_SEPARATOR)
    # Keep the existing final counter well separate from the 629px fill run.
    draw.rectangle((xp["counter_x"], xp["source_y"], canvas.width - 1, canvas.height - 1),
                   fill=(7, 11, 16), outline=XP_SEPARATOR)


def panel_strip(gems, columns):
    strip = quiet_metal(tuple(LAYOUT["hud_strip_size"]))
    ImageDraw.Draw(strip).line((0, 0, strip.width - 1, 0), fill=TOP_RAIL)
    place_slots(strip, LAYOUT["item_slots"])
    place_slots(strip, LAYOUT["skill_slots"])
    current = LAYOUT["current_skill"]
    strip.paste(slot(tuple(current["size"]), selected=True), tuple(current["position"]))
    experience_trough(strip)
    for name in ("life", "mana"):
        strip.paste(gems["empty"], tuple(LAYOUT[name]["position"]))
    for name in ("shield", "ability"):
        strip.paste(columns["empty"], tuple(LAYOUT[name]["position"]))
    return strip


def save_panels(gems, columns):
    strip = panel_strip(gems, columns)
    for relative, crop in zip(PANEL_PATHS, LAYOUT["panel_crops_xyxy"]):
        save_asset(relative, strip.crop(crop))
    spec = LAYOUT["alternate_slots"]
    alternate = quiet_metal(tuple(spec["size"]))
    ImageDraw.Draw(alternate).line((0, 0, alternate.width - 1, 0), fill=TOP_RAIL)
    place_slots(alternate, spec)
    save_asset("Interface/newui_menu02-03.OZJ", alternate)


def save_gauges(gems, columns):
    for name, info in GAUGES.items():
        save_asset(info["path"], (gems if name in gems else columns)[name])
    for name, parts in (("gem", gems), ("column", columns)):
        parts["empty"].save(WORK / f"source/{name}-empty.png")
        parts["mask"].save(WORK / f"source/{name}-energy-mask.png")
    for index, relative in enumerate(EXP_PATHS):
        save_asset(relative, experience(columns["SD"], master=bool(index)))


def save_controls():
    normal = ImageEnhance.Color(painting("cash", BUTTON_SIZE)).enhance(METAL_COLOR_STRENGTH)
    atlas = Image.new("RGB", original(CASH_PATH).size)
    for state, offset in enumerate(LAYOUT["cash"]["state_y"]):
        atlas.paste(pilot_assembly.button_state(normal, state, LAYOUT["cash"]["glyph_box"]), (0, offset))
    save_asset(CASH_PATH, atlas)
    for selected, relative in enumerate(SKILL_PATHS):
        save_asset(relative, slot(selected=bool(selected)))


def main():
    gems, columns = gem_parts(), column_parts()
    save_panels(gems, columns)
    save_gauges(gems, columns)
    save_controls()
    print("Assembled 14 exact-size HUD masters, editable layers, payloads and wrapped exports.")


if __name__ == "__main__":
    main()
