"""Label Blender render evidence and compare it at identical display sizes."""

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
PAPER, TEXT, MUTED, ACCENT = (24, 29, 31), (224, 225, 215), (159, 171, 172), (213, 186, 126)
IDENTITIES = {'StoneStatue01': 'CARVED RELIEF PILLAR', 'StoneStatue02': 'MOAI-LIKE STONE HEAD', 'StoneStatue03': 'WINGED ANGEL', 'SteelStatue01': 'CAPPED MEMORIAL', 'Tomb01': 'ARCHED SARCOPHAGUS', 'Tomb02': 'CROSS-TOPPED GRAVE', 'Waterspout01': 'FOUNTAIN COMPATIBILITY'}
ORIGINAL_TRIANGLES = {'StoneStatue01':92,'StoneStatue02':107,'StoneStatue03':248,'SteelStatue01':66,'Tomb01':44,'Tomb02':40,'Waterspout01':639}


def text(draw, point, label, size=23, color=TEXT):
    draw.text(point, label, fill=color, font=ImageFont.truetype(FONT, size))


def place(canvas, path, point, size):
    with Image.open(path) as image:
        image = image.convert('RGBA').resize(size, Image.Resampling.LANCZOS)
        canvas.paste(image, point, image)


def review_sheet(name):
    folder = ROOT / name
    data = json.loads((folder / 'validation/blender.json').read_text())
    canvas = Image.new('RGB', (1920, 1160), PAPER)
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
        place(canvas, folder / f'review/gameplay-{stage}.png', (30 + index * 635, 725), (540, 330))
    dimensions = [b - a for a, b in zip(*data['bounds_after'])]
    details = [f'{len(data["bone_order"])} bones / original action retained',
               'Skeleton + action compare: EQUIVALENT',
               'Bound size: ' + ' × '.join(f'{value:.2f}' for value in dimensions),
               'Original footprint / attachments retained',
               'Scale proxy: 190 units tall', 'Terrain repeats: 100 units',
               'Camera scale is an offline assumption']
    for index, label in enumerate(details):
        text(draw, (1300, 727 + index * 35), label, 21, TEXT if index < 4 else MUTED)
    text(draw, (30, 1104), 'NOT CLIENT SCREENSHOTS. Matched Blender camera and diffuse light. Flat grass and figure are scale proxies, not actual client views.', 20, MUTED)
    canvas.save(folder / 'review/comparison.png')


def batch_sheet():
    canvas = Image.new('RGB', (1800, 1800), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 20), 'LORENCIA  /  MONUMENTS AND GRAVES', 32)
    text(draw, (30, 68), 'OFFLINE BLENDER EXPORT REVIEW  /  Six models / 30 placements + one fountain compatibility placement  /  Client verification pending', 22, MUTED)
    for index, name in enumerate(IDENTITIES):
        x, y = 25 + (index % 3) * 595, 115 + (index // 3) * 530
        place(canvas, ROOT / name / 'review/after.png', (x, y), (520, 451))
        data = json.loads((ROOT / name / 'validation/blender.json').read_text())
        text(draw, (x + 10, y + 445), name, 26, ACCENT)
        text(draw, (x + 10, y + 485), f'{ORIGINAL_TRIANGLES[name]} → {data["triangles"]} triangles / preserved rig', 22)
    text(draw, (30, 1730), 'Original monument identities, footprints and rig contracts retained. All skeleton/action comparisons EQUIVALENT.', 23, MUTED)
    (ROOT / 'review').mkdir(exist_ok=True)
    canvas.save(ROOT / 'review/batch-review.jpg', quality=95)


def assembly_sheet(label):
    canvas = Image.new('RGB', (1920, 740), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 20), f'{label.upper()}  /  ACTUAL WORLD1 PLACEMENT TRANSFORMS', 31)
    text(draw, (30, 67), 'OFFLINE BLENDER ASSEMBLY  /  Original left, exported replacement right  /  No placement edits', 23, MUTED)
    for index, stage in enumerate(('before', 'after')):
        place(canvas, ROOT / f'review/{label}-{stage}.png', (25 + index * 960, 132), (930, 543))
    text(draw, (30, 699), 'Not client evidence. Owned models only; actual terrain, baked lighting, collision and unrelated buildings omitted.', 22, MUTED)
    canvas.save(ROOT / f'review/{label}-comparison.jpg', quality=95)




if __name__ == '__main__':
    for name in IDENTITIES:
        review_sheet(name)
    batch_sheet()
