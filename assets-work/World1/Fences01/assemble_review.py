"""Create labeled comparison sheets from the matching-camera Blender exports."""
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
PAPER, TEXT, MUTED, ACCENT = (24, 29, 31), (224, 225, 215), (159, 171, 172), (213, 186, 126)
IDENTITIES = {'Fence01': 'RUSTIC TWO-RAIL TIMBER FENCE', 'Fence02': 'ORNAMENTAL CURB / L-CORNER',
              'Fence03': 'ORNAMENTAL CURB / 200-UNIT STRAIGHT', 'Fence04': 'ORNAMENTAL CURB / 100-UNIT STRAIGHT'}
ORIGINAL_TRIANGLES = {'Fence01': 36, 'Fence02': 16, 'Fence03': 10, 'Fence04': 10}


def label(draw, point, text, size=23, color=TEXT):
    draw.text(point, text, fill=color, font=ImageFont.truetype(FONT, size))


def place(canvas, path, point, size):
    with Image.open(path) as source:
        image = source.convert('RGBA')
        image.thumbnail(size, Image.Resampling.LANCZOS)
        offset = (point[0] + (size[0] - image.width) // 2, point[1] + (size[1] - image.height) // 2)
        canvas.paste(image, offset, image)


def asset_sheet(name):
    folder = ROOT / name
    data = json.loads((folder / 'validation/blender.json').read_text())
    canvas = Image.new('RGB', (1920, 1080), PAPER)
    draw = ImageDraw.Draw(canvas)
    label(draw, (30, 22), f'{name.upper()} / {IDENTITIES[name]}', 31)
    label(draw, (30, 68), 'OFFLINE BLENDER REVIEW / Original vs re-imported BMD / Client acceptance pending', 24, MUTED)
    for index, stage in enumerate(('before', 'after', 'wireframe')):
        x = 30 + index * 635
        caption = ('ORIGINAL', 'REBUILT EXPORT', 'EXPORTED TOPOLOGY')[index]
        label(draw, (x, 115), caption, 25, ACCENT)
        place(canvas, folder / f'review/{stage}.png', (x, 150), (590, 511))
    for index, stage in enumerate(('before', 'after')):
        x = 30 + index * 635
        label(draw, (x, 691), f'REDUCED-SCALE STUDY / {stage.upper()}', 22, ACCENT)
        place(canvas, folder / f'review/gameplay-{stage}.png', (x, 725), (590, 262))
    texture = 'Frozen shared timber 128x128' if name == 'Fence01' else 'New exclusive joint atlas 512x512'
    details = [f'{ORIGINAL_TRIANGLES[name]} -> {data["triangles"]} triangles', 'One original bone / one original action',
               'Skeleton + actions: EQUIVALENT', 'Full geometry compare: DIFFERENT', texture,
               '190-unit figure / 100-unit terrain tiles', 'Offline camera scale assumption']
    for index, text in enumerate(details):
        label(draw, (1300, 727 + index * 35), text, 21, TEXT if index < 5 else MUTED)
    label(draw, (30, 1020), 'NOT CLIENT SCREENSHOTS. Same Blender camera/light. Original positions and rig preserved; client lighting, collision and load checks pending.', 21, MUTED)
    canvas.save(folder / 'review/comparison.png')


def batch_sheet():
    canvas = Image.new('RGB', (1800, 1250), PAPER)
    draw = ImageDraw.Draw(canvas)
    label(draw, (30, 20), 'LORENCIA / TIMBER FENCE AND ORNAMENTAL CURBS', 34)
    label(draw, (30, 68), 'OFFLINE BLENDER EXPORT REVIEW / 268 World1 placements / Client verification pending', 24, MUTED)
    for index, name in enumerate(IDENTITIES):
        x, y = 25 + index % 2 * 890, 122 + index // 2 * 540
        place(canvas, ROOT / name / 'review/after.png', (x, y), (530, 460))
        data = json.loads((ROOT / name / 'validation/blender.json').read_text())
        label(draw, (x + 540, y + 125), name, 30, ACCENT)
        label(draw, (x + 540, y + 173), f'{ORIGINAL_TRIANGLES[name]} -> {data["triangles"]} triangles', 23)
        label(draw, (x + 540, y + 215), 'Original rig/actions', 22, MUTED)
        label(draw, (x + 540, y + 250), 'EQUIVALENT', 22, MUTED)
    label(draw, (30, 1210), 'Re-imported BMD game exports. No runtime installed or client verification claimed.', 24, MUTED)
    canvas.save(ROOT / 'review/batch-review.jpg', quality=95)


def joins_sheet():
    canvas = Image.new('RGB', (1920, 1180), PAPER)
    draw = ImageDraw.Draw(canvas)
    label(draw, (30, 20), 'LORENCIA / ACTUAL PLACEMENT TRANSFORMS', 32)
    label(draw, (30, 68), 'OFFLINE BLENDER ONLY / Unchanged World1 position, rotation and scale / Matching cameras', 24, MUTED)
    for row, group in enumerate(('town-corner', 'long-curb')):
        for column, stage in enumerate(('before', 'after')):
            x, y = 30 + column * 950, 135 + row * 490
            label(draw, (x, y), f'{group.upper()} / {stage.upper()}', 24, ACCENT)
            place(canvas, ROOT / f'review/{group}-{stage}.png', (x, y + 35), (660, 440))
    label(draw, (30, 1130), 'Only owned geometry shown; terrain and neighboring buildings omitted. Existing placement contacts preserved. Not client screenshots.', 22, MUTED)
    canvas.save(ROOT / 'review/modular-joins.png')


if __name__ == '__main__':
    for name in IDENTITIES:
        asset_sheet(name)
    batch_sheet()
    joins_sheet()
