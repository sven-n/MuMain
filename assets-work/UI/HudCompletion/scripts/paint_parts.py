"""Prepare painted component variants while keeping shared fill geometry identical."""

from __future__ import annotations

import json

import numpy as np
from PIL import Image, ImageDraw, ImageEnhance, ImageFont

from common import CELL_SIZE, COLUMN_SIZE, GAUGE_SIZE, WORK, pilot_assembly

LAYOUT = json.loads((WORK / "source/layout.json").read_text())
OPAQUE = 255
NORMALIZED_SIZE = (1024, 1024)
METAL_COLOR_STRENGTH = 0.9
KEY_FONT = ImageFont.load_default(size=9)
KEY_COLOR = (192, 183, 147)
KEY_OFFSET = (3, 2)
SELECTED_TINT = (1.25, 1.06, 0.65)
SELECTED_RULE = (208, 172, 88)
EMPTY_TINT = (2, 5, 9)
EMPTY_BRIGHTNESS = 0.16
RED_MASK_THRESHOLD = 10
RED_MASK_SOFTNESS = 45
HUES = {"blue": 153, "green": 92, "AG": 195, "experience": 68, "master": 123}
XP_CORE = (4, 12, 12, 20)


def painting(name, size, crop=None):
    image = pilot_assembly.load_painting(WORK / f"source/{name}-generated.png")
    if crop:
        image = image.crop(crop)
    return image.resize(size, Image.Resampling.LANCZOS)


def rgb_array(image):
    return np.asarray(image, dtype=np.float32)


def blend(image, replacement, mask):
    pixels = rgb_array(image) * (1 - mask[..., None]) + replacement * mask[..., None]
    return Image.fromarray(np.clip(np.rint(pixels), 0, OPAQUE).astype(np.uint8))


def color_variant(image, hue, mask):
    hsv = np.asarray(image.convert("HSV")).copy()
    hsv[..., 0] = hue
    replacement = Image.fromarray(hsv, "HSV").convert("RGB")
    return blend(image, rgb_array(replacement), mask)


def empty_variant(image, mask):
    grey = np.asarray(image.convert("L"), dtype=np.float32)
    replacement = grey[..., None] * EMPTY_BRIGHTNESS + np.asarray(EMPTY_TINT)
    return blend(image, replacement, mask)


def gem_parts():
    red = painting("gem", GAUGE_SIZE, LAYOUT["gem_source_crop_xyxy"])
    pixels = rgb_array(red)
    redness = pixels[..., 0] - np.maximum(pixels[..., 1], pixels[..., 2])
    mask = np.clip((redness - RED_MASK_THRESHOLD) / RED_MASK_SOFTNESS, 0, 1)
    return {"red": red, "blue": color_variant(red, HUES["blue"], mask),
            "green": color_variant(red, HUES["green"], mask),
            "empty": empty_variant(red, mask), "mask": Image.fromarray(np.rint(mask * OPAQUE).astype(np.uint8))}


def column_parts():
    shield = painting("column", COLUMN_SIZE, LAYOUT["column_source_crop_xyxy"])
    mask = np.zeros((shield.height, shield.width), dtype=np.float32)
    x0, y0, x1, y1 = LAYOUT["column_energy_xyxy"]
    mask[y0:y1, x0:x1] = 1
    return {"SD": shield, "AG": color_variant(shield, HUES["AG"], mask),
            "empty": empty_variant(shield, mask), "mask": Image.fromarray((mask * OPAQUE).astype(np.uint8))}


def slot(size=CELL_SIZE, selected=False, key=None):
    image = ImageEnhance.Color(painting("slot", size)).enhance(METAL_COLOR_STRENGTH)
    if selected:
        image = Image.fromarray(np.clip(rgb_array(image) * SELECTED_TINT, 0, OPAQUE).astype(np.uint8))
        ImageDraw.Draw(image).line((3, size[1] - 2, size[0] - 4, size[1] - 2), fill=SELECTED_RULE)
    if key:
        ImageDraw.Draw(image).text(KEY_OFFSET, key, font=KEY_FONT, fill=KEY_COLOR)
    return image


def quiet_metal(size):
    source = painting("slot", NORMALIZED_SIZE)
    return source.crop(LAYOUT["slot_interior_normalized_1024"]).resize(size, Image.Resampling.LANCZOS)


def experience(column, master=False):
    # Sample broad painted energy shading; no tiny motif is stretched along XP.
    core = column.crop(XP_CORE).rotate(90).resize((6, 4), Image.Resampling.LANCZOS)
    mean_rows = rgb_array(core).mean(axis=1, keepdims=True)
    image = Image.fromarray(np.repeat(mean_rows, core.width, axis=1).astype(np.uint8))
    mask = np.ones((image.height, image.width), dtype=np.float32)
    return color_variant(image, HUES["master" if master else "experience"], mask)
