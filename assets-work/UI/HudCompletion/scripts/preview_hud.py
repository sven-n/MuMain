"""CPU reconstructions of decoded HUD exports; these are not client captures."""

from __future__ import annotations

from PIL import Image, ImageDraw, ImageFont

from common import (ASSETS, CASH_PATH, EXP_PATHS, FULL_HD, GAUGES,
                    PANEL_PATHS, SKILL_PATHS, STATE_NAMES, WORK, decoded, pilot_preview)

OUT = WORK / "previews"
DARK = (18, 22, 29)
LIGHT = (218, 216, 205)
GOLD = (196, 173, 115)
FONT = ImageFont.load_default(size=16)
TITLE = ImageFont.load_default(size=27)
SMALL = ImageFont.load_default(size=13)
CLASSIC_CROP = (320, 978, 1600, 1080)
NATIVE_CROP = (0, 429, 640, 480)
GAUGE_LEVELS = (0, 0.1, 0.25, 0.5, 0.75, 1)
FRAME_REGIONS = (
    (0, (0, 0, 152, 41), (0, 429, 152, 41), "left"),
    (0, (152, 0, 104, 41), (152, 429, 104, 41), "center"),
    (1, (0, 0, 128, 41), (256, 429, 128, 41), "center"),
    (2, (0, 0, 104, 41), (384, 429, 104, 41), "center"),
    (2, (104, 0, 152, 41), (488, 429, 152, 41), "right"),
    (0, (0, 41, 256, 10), (0, 470, 256, 10), "experience"),
    (1, (0, 41, 128, 10), (256, 470, 128, 10), "experience"),
    (2, (0, 41, 256, 10), (384, 470, 256, 10), "experience"),
)


def blit(canvas, path, source, destination, zone, after, inset=True):
    x, y, width, height = pilot_preview.transformed(destination, zone, canvas)
    if width <= 0 or height <= 0:
        return
    patch = pilot_preview.sample(decoded(path, after), source, (width, height), inset)
    canvas.paste(patch, (x, y))


def draw_frames(canvas, after, alternate):
    for index, source, destination, zone in FRAME_REGIONS:
        blit(canvas, PANEL_PATHS[index], source, destination, zone, after)
    if alternate:
        blit(canvas, "Interface/newui_menu02-03.OZJ", (0, 0, 160, 40),
             (222, 429, 160, 40), "center", after)


def draw_controls(canvas, after, state):
    paths = [CASH_PATH] + [f"Interface/partCharge1/newui_menu_Bt{index:02d}.OZJ" for index in range(1, 5)]
    for index, path in enumerate(paths):
        blit(canvas, path, (0, state * 41, 30, 41), (489 + index * 30, 429, 30, 41), "right", after)


def draw_gauges(canvas, after, level, poison, master):
    names = ("green" if poison else "red", "blue", "SD", "AG")
    for name in names:
        item = GAUGES[name]
        x, y = item["position"]
        width, height = item["size"]
        missing, remaining = height * (1 - level), height * level
        blit(canvas, item["path"], (0, missing, width, remaining),
             (x, y + missing, width, remaining), "center", after, inset=False)
    blit(canvas, EXP_PATHS[int(master)], (0, 0, 6, 4), (2, 473, 629 * level, 4),
         "experience", after, inset=False)


def draw_skill_examples(canvas, after):
    # Unchanged texture cells illustrate overlap geometry, not a live character loadout.
    atlas = "Interface/newui_skill.OZJ"
    for index in range(5):
        x = 222 + index * 32
        if index == 2:
            blit(canvas, SKILL_PATHS[1], (0, 0, 32, 38), (x, 431, 32, 38), "center", after)
        blit(canvas, atlas, (index * 20, 0, 20, 28), (x + 6, 437, 20, 28), "center", False, inset=False)
    blit(canvas, atlas, (40, 0, 20, 28), (392, 437, 20, 28), "center", False, inset=False)


def hud(after, size=FULL_HD, classic=True, level=0.5, state=0,
        alternate=False, poison=False, master=False, examples=False, background=DARK):
    pilot_preview.CLASSIC_LAYOUT = classic
    canvas = Image.new("RGB", size, background)
    draw_frames(canvas, after, alternate)
    draw_controls(canvas, after, state)
    draw_gauges(canvas, after, level, poison, master)
    if examples:
        draw_skill_examples(canvas, after)
    return canvas


def heading(canvas, title, subtitle):
    draw = ImageDraw.Draw(canvas)
    draw.text((32, 22), title, font=TITLE, fill=GOLD)
    draw.text((32, 65), subtitle, font=FONT, fill="white")
    return draw


def full_screens():
    for classic in (False, True):
        mode = "classic" if classic else "anchored"
        for after in (False, True):
            version = "after" if after else "before"
            canvas = hud(after, classic=classic, examples=True)
            draw = heading(canvas, f"OFFLINE MOCKUP / {version.upper()} / {mode.upper()}",
                           "1920 x 1080 / contentScale=1 / existing engine transforms / decoded OZJ textures")
            draw.text((32, 100), "Illustrative 50% gauges and unchanged example skill icons. No client scene, item models or dynamic counters.", font=FONT, fill=GOLD)
            draw.text((32, 895), "The four approved right-side controls are unchanged. Source texture dimensions remain fixed.", font=FONT, fill=GOLD)
            canvas.save(OUT / f"offline-1920x1080-{version}-{mode}.png")


def comparison():
    canvas = Image.new("RGB", (1344, 455), DARK)
    draw = heading(canvas, "COMPLETE BOTTOM HUD / actual 1080p draw size",
                   "OFFLINE MOCKUP / original geometry / approved right-side controls retained")
    for after, y in ((False, 150), (True, 315)):
        draw.text((32, y - 28), "MERGED PILOT" if not after else "REMAINING HUD REPAINTED", font=FONT, fill=GOLD)
        canvas.paste(hud(after, examples=True).crop(CLASSIC_CROP), (32, y))
    canvas.save(OUT / "offline-hud-comparison-1080p.png")


def gauge_levels():
    canvas = Image.new("RGB", (1344, 1080), DARK)
    draw = heading(canvas, "OFFLINE FILL / STATE / KEY-BANK REVIEW",
                   "0 / 10 / 25 / 50 / 75 / 100% fills. No numerical values are baked into the gauge textures.")
    for row, level in enumerate(GAUGE_LEVELS):
        y = 133 + row * 150
        state = row % len(STATE_NAMES)
        alternate = row >= 3
        poison = row == 4
        master = row == 5
        label = f"{level:.0%} / {STATE_NAMES[state]} / {'keys 6-0' if alternate else 'keys 1-5'}"
        label += " / poison life" if poison else ""
        label += " / master XP" if master else ""
        draw.text((32, y - 24), label, font=FONT, fill=GOLD)
        canvas.paste(hud(True, level=level, state=state, alternate=alternate,
                         poison=poison, master=master).crop(CLASSIC_CROP), (32, y))
    canvas.save(OUT / "offline-fill-states.png")


def native_and_edges():
    canvas = Image.new("RGB", (1344, 760), DARK)
    draw = heading(canvas, "OFFLINE NATIVE / OPAQUE EDGE REVIEW",
                   "All 14 payloads are RGB JPEG: alpha=255 including black. Top two rows are 640px native geometry.")
    for index, after in enumerate((False, True)):
        y = 125 + index * 100
        draw.text((32, y - 25), "BEFORE" if not after else "AFTER", font=FONT, fill=GOLD)
        canvas.paste(hud(after, size=(640, 480)).crop(NATIVE_CROP), (32, y))
    for index, background in enumerate((DARK, LIGHT)):
        y = 390 + index * 170
        draw.text((32, y - 25), "AFTER / DARK" if not index else "AFTER / LIGHT", font=FONT, fill=GOLD)
        draw.rectangle((24, y - 8, 1320, y + 110), fill=background)
        canvas.paste(hud(True, background=background).crop(CLASSIC_CROP), (32, y))
    canvas.save(OUT / "offline-native-light-dark.png")


def slot_and_button_states():
    canvas = Image.new("RGB", (1000, 610), DARK)
    draw = heading(canvas, "OFFLINE CASH / SHARED SKILL SLOTS",
                   "Actual 2x display size. Skill slots are also used by MU Helper and the pet window.")
    for state, name in enumerate(STATE_NAMES):
        x = 40 + state * 240
        draw.text((x, 125), name, font=SMALL, fill=GOLD)
        for after, y in ((False, 160), (True, 270)):
            patch = pilot_preview.sample(decoded(CASH_PATH, after), (0, state * 41, 30, 41), (60, 82))
            canvas.paste(patch, (x, y))
    for index, path in enumerate(SKILL_PATHS):
        x = 40 + index * 280
        draw.text((x, 405), "NORMAL" if not index else "SELECTED", font=FONT, fill=GOLD)
        for after, offset in ((False, 0), (True, 90)):
            patch = pilot_preview.sample(decoded(path, after), (0, 0, 32, 38), (64, 76))
            canvas.paste(patch, (x + offset, 442))
    draw.text((40, 560), "Cash: before above, after below. Slots: before left, after right. No separate disabled sprite.", font=SMALL, fill="white")
    canvas.save(OUT / "offline-cash-skill-states.png")


def atlas_comparison():
    canvas = Image.new("RGB", (1380, 2060), DARK)
    draw = heading(canvas, "OFFLINE ATLAS CONTACT / all 14 replacements",
                   "Before left / after right. Each pair uses identical nearest-neighbor inspection scale.")
    for row, relative in enumerate(ASSETS):
        y = 125 + row * 135
        draw.text((32, y - 24), relative, font=FONT, fill=GOLD)
        for after, x in ((False, 32), (True, 720)):
            patch = decoded(relative, after).convert("RGB")
            factor = min(3, 620 / patch.width, 98 / patch.height)
            size = tuple(round(side * factor) for side in patch.size)
            canvas.paste(patch.resize(size, Image.Resampling.NEAREST), (x, y))
    canvas.save(OUT / "offline-atlas-before-after.png")


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    full_screens()
    comparison()
    gauge_levels()
    native_and_edges()
    slot_and_button_states()
    atlas_comparison()
    print("Created nine labeled offline mockups / review sheets from decoded game exports.")


if __name__ == "__main__":
    main()
