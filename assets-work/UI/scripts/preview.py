#!/usr/bin/env python3
"""Offline HUD reconstruction from decoded exports and the inspected draw geometry.

This is a 2D sampler, not a client screenshot. Requires Pillow and numpy.
"""

from __future__ import annotations

import io
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[3]
WORK = ROOT / "assets-work/UI"
OUTPUT = WORK / "previews"
LAYOUT = json.loads((WORK / "source/layout.json").read_text())
FONT = ImageFont.load_default(size=15)
SMALL = ImageFont.load_default(size=12)
TITLE = ImageFont.load_default(size=30)
STATE_NAMES = LAYOUT["buttons"]["state_names"]
BUTTONS = LAYOUT["buttons"]["icons"]
DARK = (18, 22, 29)
LIGHT = (218, 216, 205)
ACCENT = (196, 173, 115)
CLASSIC_LAYOUT = False


def texture(relative, after=False):
    if after == "previous":
        previous = WORK / "previous" / Path(relative).with_suffix(".jpg")
        if previous.exists():
            return Image.open(previous).convert("RGBA")
        after = False
    export = WORK / "exports" / relative
    if after and export.exists():
        return Image.open(io.BytesIO(export.read_bytes()[24:])).convert("RGBA")
    payload = WORK / "original" / Path(relative).with_suffix(".jpg")
    return Image.open(payload).convert("RGBA")


def sample(image, source, size, inset=True):
    """Bilinear GL sampling including the engine's half-texel source insets."""
    sx, sy, sw, sh = source
    width, height = size
    padded_size = tuple(1 << (dimension - 1).bit_length() for dimension in image.size)
    padded = Image.new("RGBA", padded_size, (0, 0, 0, 255))
    padded.paste(image, (0, 0))
    image = padded
    # GL texel centers are n+0.5. Subtract 0.5 to address image array centers.
    offset = 0.0 if inset else -0.5
    extent_x, extent_y = (sw - 1, sh - 1) if inset else (sw, sh)
    xs = sx + offset + (np.arange(width) + 0.5) * extent_x / width
    ys = sy + offset + (np.arange(height) + 0.5) * extent_y / height
    xs, ys = np.clip(xs, 0, image.width - 1), np.clip(ys, 0, image.height - 1)
    x0, y0 = xs.astype(int), ys.astype(int)
    x1, y1 = np.minimum(x0 + 1, image.width - 1), np.minimum(y0 + 1, image.height - 1)
    fx, fy = (xs - x0)[None, :, None], (ys - y0)[:, None, None]
    pixels = np.asarray(image, dtype=np.float32)
    top = pixels[y0[:, None], x0] * (1 - fx) + pixels[y0[:, None], x1] * fx
    bottom = pixels[y1[:, None], x0] * (1 - fx) + pixels[y1[:, None], x1] * fx
    return Image.fromarray(np.clip(np.rint(top * (1 - fy) + bottom * fy), 0, 255).astype(np.uint8))


def transformed(rect, zone, canvas):
    width, height = canvas.size
    scale = max(1.0, min(2.0, width / 640, height / 480))
    x, y, w, h = rect
    offset_x = {"left": 0, "center": width / 2 - 320 * scale,
                "right": width - 640 * scale, "experience": 0}[zone]
    scale_x = width / 640 if zone == "experience" else scale
    if CLASSIC_LAYOUT:
        offset_x = width / 2 - 320 * scale
        scale_x = scale
    return tuple(round(value) for value in (x * scale_x + offset_x,
                 y * scale + height - 480 * scale, w * scale_x, h * scale))


def draw_texture(canvas, path, source, destination, zone, after=False, inset=True):
    x, y, width, height = transformed(destination, zone, canvas)
    patch = sample(texture(path, after), source, (width, height), inset)
    canvas.paste(patch, (x, y), patch.getchannel("A"))


def draw_frames(canvas, after):
    menu1, menu2 = "Interface/newui_menu01.OZJ", "Interface/newui_menu02.OZJ"
    for path, src, dst, zone in (
        (menu1, (0, 0, 152, 41), (0, 429, 152, 41), "left"),
        (menu1, (152, 0, 104, 41), (152, 429, 104, 41), "center"),
        (menu2, (0, 0, 128, 41), (256, 429, 128, 41), "center"),
        (menu1, (0, 41, 256, 10), (0, 470, 256, 10), "experience"),
        (menu2, (0, 41, 128, 10), (256, 470, 128, 10), "experience")):
        draw_texture(canvas, path, src, dst, zone)
    for region in LAYOUT["panel"]["regions"]:
        draw_texture(canvas, LAYOUT["panel"]["path"], region["source"], region["destination"],
                     region["transform"], after)


def draw_controls(canvas, after, state):
    for entry in BUTTONS:
        draw_texture(canvas, f"Interface/partCharge1/{entry['file']}", (0, state * 41, 30, 41),
                     (entry["x"], entry["y"], 30, 41), "right", after)
    draw_texture(canvas, "Interface/partCharge1/newui_menu_Bt05.OZJ", (0, state * 41, 30, 41),
                 (489, 429, 30, 41), "right")


def draw_gauges(canvas):
    # Illustrative half-full values. No numeric overlays or skill/item substitutes.
    for name, x, y, width in (("red", 158, 432, 45), ("blue", 437, 432, 45),
                              ("SD", 204, 431, 16), ("AG", 420, 431, 16)):
        missing, remaining = 19.5, 19.5
        draw_texture(canvas, f"Interface/newui_menu_{name}.OZJ", (0, missing, width, remaining),
                     (x, y + missing, width, remaining), "center", inset=False)
    draw_texture(canvas, "Interface/newui_exbar.OZJ", (0, 0, 6, 4), (2, 473, 314.5, 4),
                 "experience", inset=False)


def hud(size, after, state=0, background=DARK, gauges=True):
    canvas = Image.new("RGB", size, background)
    draw_frames(canvas, after)
    draw_controls(canvas, after, state)
    if gauges:
        draw_gauges(canvas)
    return canvas


def full_mockups(classic=False):
    global CLASSIC_LAYOUT
    CLASSIC_LAYOUT = classic
    mode = "classic" if classic else "anchored"
    for after in (False, True):
        name = "after" if after else "before"
        canvas = hud((1920, 1080), after)
        draw = ImageDraw.Draw(canvas)
        draw.text((56, 52), f"OFFLINE MOCKUP / {name.upper()}", font=TITLE, fill=ACCENT)
        draw.text((56, 106), "1920 x 1080 / contentScale=1 / actual engine HUD transforms / decoded OZJ textures",
                  font=FONT, fill="white")
        draw.text((56, 136), "Neutral canvas; no client scene captured. Skill/item models and dynamic text omitted. Gauges illustrated half full.",
                  font=FONT, fill="#a9b2bd")
        draw.text((56, 170), f"Revision 2 / {mode} layout / right panel + Character / Inventory / Friends / Menu. Other HUD textures unchanged.",
                  font=FONT, fill="#a9b2bd")
        layout_note = ("Classic option: continuous centered HUD at 2x scale, including experience strip." if classic else
                       "Default: HUD scale 2x; separate anchored bands. Experience strip scale 3x horizontally, 2x vertically.")
        draw.text((56, 895), layout_note,
                  font=FONT, fill=ACCENT)
        suffix = "-classic" if classic else ""
        canvas.save(OUTPUT / f"offline-1920x1080-{name}{suffix}.png")
    CLASSIC_LAYOUT = False


def revision_comparison():
    canvas = Image.new("RGB", (1120, 730), DARK)
    draw = ImageDraw.Draw(canvas)
    draw.text((28, 24), "HUD REVISION / actual 1080p control size", font=TITLE, fill=ACCENT)
    draw.text((28, 70), "OFFLINE MOCKUP / decoded game exports / 2x engine scale / unchanged 30x41 source cells", font=FONT, fill="white")
    for column, (version, title) in enumerate((("previous", "FIRST PASS"), (True, "REVISED"))):
        x = 28 + column * 550
        draw.text((x, 120), title, font=TITLE, fill="white")
        for state, label in enumerate(STATE_NAMES):
            y = 195 + state * 118
            draw.text((x, y - 25), label, font=SMALL, fill=ACCENT)
            for index, entry in enumerate(BUTTONS):
                tile = texture(f"Interface/partCharge1/{entry['file']}", version)
                tile = sample(tile, (0, state * 41, 30, 41), (60, 82))
                canvas.paste(tile, (x + index * 90, y))
    draw.text((28, 696), "Same positions and four states. Complete new button faces, restrained steel framing, readable filled symbols.",
              font=FONT, fill="white")
    canvas.save(OUTPUT / "offline-revision-comparison-1080p.png")


def native_comparison():
    canvas = Image.new("RGB", (900, 830), DARK)
    draw = ImageDraw.Draw(canvas)
    draw.text((28, 25), "OFFLINE MOCKUP / native 640 x 480 HUD geometry", font=TITLE, fill=ACCENT)
    draw.text((28, 67), "One texture texel = one logical pixel. All rows show the same geometry; states change only artwork.",
              font=SMALL, fill="white")
    for state, label in enumerate(STATE_NAMES):
        y = 120 + state * 170
        draw.text((28, y - 25), f"{label} / before", font=FONT, fill="white")
        canvas.paste(hud((640, 480), False, state).crop((0, 429, 640, 480)), (28, y))
        draw.text((700, y + 12), "original", font=FONT, fill="#abb4bf")
        canvas.paste(hud((640, 480), True, state).crop((0, 429, 640, 480)), (28, y + 61))
        draw.text((700, y + 72), "replacement", font=FONT, fill=ACCENT)
    draw.text((28, 810), "No separate disabled row exists. Selected = pressed. Alert flashes reuse hover / selected-hover.",
              font=SMALL, fill="white")
    canvas.save(OUTPUT / "offline-native-before-after.png")


def state_sheet():
    canvas = Image.new("RGB", (1440, 1060), DARK)
    draw = ImageDraw.Draw(canvas)
    draw.text((30, 25), "OFFLINE ATLAS REVIEW / all four states", font=TITLE, fill=ACCENT)
    draw.text((30, 68), "Native crops + 4x nearest-neighbor inspection. Original left / decoded export right within each pair.",
              font=FONT, fill="white")
    for column, entry in enumerate(BUTTONS):
        x = 35 + column * 350
        draw.text((x, 110), entry["label"], font=TITLE, fill="white")
        path = f"Interface/partCharge1/{entry['file']}"
        for state, name in enumerate(STATE_NAMES):
            y = 165 + state * 210
            draw.text((x, y), name, font=SMALL, fill=ACCENT)
            for version, after in enumerate((False, True)):
                patch = texture(path, after).crop((0, state * 41, 30, (state + 1) * 41))
                canvas.paste(patch.resize((120, 164), Image.Resampling.NEAREST), (x + version * 150, y + 25))
                canvas.paste(patch, (x + version * 150 + 125, y + 25))
    draw.text((30, 1030), "No new letters or numbers in textures. 30 x 41 per state, exactly aligned at y = 0 / 41 / 82 / 123.",
              font=FONT, fill="white")
    canvas.save(OUTPUT / "offline-controls-all-states.png")


def background_sheet():
    canvas = Image.new("RGB", (1440, 1040), DARK)
    draw = ImageDraw.Draw(canvas)
    draw.text((30, 25), "OFFLINE ALPHA / EDGE REVIEW", font=TITLE, fill=ACCENT)
    draw.text((30, 70), "All five files are RGB JPEG: alpha=255 everywhere. Black remains opaque; no cutout or color key.",
              font=FONT, fill="white")
    for row, background in enumerate((DARK, LIGHT)):
        for column, after in enumerate((False, True)):
            x, y = 30 + column * 710, 135 + row * 440
            label = ("AFTER" if after else "BEFORE") + (" / dark" if row == 0 else " / light")
            draw.text((x, y - 27), label, font=FONT, fill=ACCENT)
            block = Image.new("RGB", (680, 410), background)
            for state in range(4):
                for index, entry in enumerate(BUTTONS):
                    patch = texture(f"Interface/partCharge1/{entry['file']}", after)
                    patch = patch.crop((0, state * 41, 30, (state + 1) * 41)).resize((60, 82), Image.Resampling.NEAREST)
                    block.paste(patch, (25 + index * 160, 10 + state * 90), patch.getchannel("A"))
            panel = texture(LAYOUT["panel"]["path"], after)
            block.paste(panel, (25, 359), panel.getchannel("A"))
            canvas.paste(block, (x, y))
    canvas.save(OUTPUT / "offline-light-dark-alpha.png")


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    full_mockups()
    full_mockups(classic=True)
    revision_comparison()
    native_comparison()
    state_sheet()
    background_sheet()
    print("Created eight labeled offline preview PNGs from decoded exports.")


if __name__ == "__main__":
    main()
