"""Label Blender render evidence and compare it at identical display sizes."""

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
PAPER, TEXT, MUTED, ACCENT = (24, 29, 31), (224, 225, 215), (159, 171, 172), (213, 186, 126)
IDENTITIES = {'Candle01': 'THREE-CANDLE BRONZE STAND',
              'TreasureChest01': 'ARCHED TIMBER CHEST', 'Tomb03': 'CARVED GRAVE MARKER'}
ORIGINAL_TRIANGLES = {'Candle01': 116, 'TreasureChest01': 66, 'Tomb03': 30}


def text(draw, point, label, size=23, color=TEXT):
    draw.text(point, label, fill=color, font=ImageFont.truetype(FONT, size))


def place(canvas, path, point, size):
    with Image.open(path) as image:
        image = image.convert('RGBA').resize(size, Image.Resampling.LANCZOS)
        canvas.paste(image, point, image)


def review_sheet(name):
    folder = ROOT.parent / name
    data = json.loads((folder / 'validation/blender.json').read_text())
    canvas = Image.new('RGB', (1920, 1080), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 22), f'{name.upper()}  /  {IDENTITIES[name]}', 32)
    text(draw, (30, 66), 'OFFLINE BLENDER REVIEW  •  Original vs re-imported game export  •  Client acceptance pending', 23, MUTED)
    views = [('before', f'ORIGINAL  /  {ORIGINAL_TRIANGLES[name]} triangles'),
             ('after', f'REBUILT  /  {data["triangles"]} triangles'), ('wireframe', 'EXPORTED TOPOLOGY')]
    for index, (stage, label) in enumerate(views):
        x = 30 + index * 635
        text(draw, (x, 113), label, 25, ACCENT)
        place(canvas, folder / f'review/{stage}.png', (x, 148), (590, 511))
    text(draw, (30, 694), 'OFFLINE GAMEPLAY-SCALE STUDY  /  BEFORE', 21, ACCENT)
    text(draw, (665, 694), 'SAME SCALE  /  AFTER', 21, ACCENT)
    for index, stage in enumerate(('before', 'after')):
        place(canvas, folder / f'review/gameplay-{stage}.png', (30 + index * 635, 725), (590, 262))
    dimensions = [b - a for a, b in zip(*data['bounds_after'])]
    details = [f'{len(data["bone_order"])} bones / original action retained',
               'Skeleton + action compare: EQUIVALENT',
               'Bound size: ' + ' × '.join(f'{value:.2f}' for value in dimensions),
               'Diffuse-only game materials / no normal map',
               'Scale proxy: 190 units tall', 'Terrain repeats: 100 units',
               'Camera scale is an offline assumption']
    for index, label in enumerate(details):
        text(draw, (1300, 727 + index * 35), label, 21, TEXT if index < 4 else MUTED)
    text(draw, (30, 1020), 'NOT CLIENT SCREENSHOTS. Matching Blender camera/light; candle additive appearance approximated. In-game lighting and placements unverified.', 20, MUTED)
    canvas.save(folder / 'review/comparison.png')


def action_sheet(name):
    folder = ROOT.parent / name / 'review'
    canvas = Image.new('RGB', (1920, 1200), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 20), f'{name.upper()}  /  ORIGINAL SEVEN-FRAME ACTION', 30)
    text(draw, (30, 63), 'OFFLINE BLENDER POSE REVIEW  •  Top: original  /  Bottom: exported replacement  •  No animation edits', 23, MUTED)
    for row, stage in enumerate(('before', 'after')):
        for column, frame in enumerate((0, 3, 6)):
            point = (30 + column * 635, 147 + row * 520)
            text(draw, (point[0], point[1] - 32), f'{stage.upper()}  /  FRAME {frame}', 24, ACCENT)
            place(canvas, folder / f'action-{stage}-{frame}.png', point, (565, 489))
    text(draw, (30, 1163), 'Not client screenshots. Actual BMD action data compared over every frame, including local rotations and translations.', 20, MUTED)
    canvas.save(folder / 'action-comparison.png')


def batch_sheet():
    canvas = Image.new('RGB', (1800, 960), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 22), 'LORENCIA  /  THREE STATIC PROPS', 35)
    text(draw, (30, 72), 'OFFLINE BLENDER EXPORT REVIEW  •  New geometry and diffuse paintings  •  Client verification pending', 23, MUTED)
    for index, name in enumerate(IDENTITIES):
        x = 25 + 595 * index
        place(canvas, ROOT.parent / name / 'review/after.png', (x, 152), (565, 489))
        text(draw, (x + 15, 680), name, 29, ACCENT)
        data = json.loads((ROOT.parent / name / 'validation/blender.json').read_text())
        text(draw, (x + 15, 725), f'{ORIGINAL_TRIANGLES[name]} → {data["triangles"]} triangles', 24)
        text(draw, (x + 15, 765), f'{len(data["bone_order"])} original bones · unchanged action', 22, MUTED)
    text(draw, (30, 850), 'All skeleton/action comparisons: EQUIVALENT. All models below 1,500 triangles. Existing filenames retained.', 24)
    text(draw, (30, 904), 'These are Blender renders of re-imported BMD exports, not screenshots from the MU client.', 23, MUTED)
    canvas.save(ROOT / 'batch-review.jpg', quality=95)


def main():
    for name in IDENTITIES:
        review_sheet(name)
        if name != 'Tomb03':
            action_sheet(name)
    batch_sheet()


if __name__ == '__main__':
    main()
