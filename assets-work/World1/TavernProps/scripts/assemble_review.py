"""Label offline renders and assemble matching-camera, reduced-scale and UV review sheets."""

import json
from pathlib import Path
import sys

from PIL import Image, ImageDraw, ImageFont

sys.dont_write_bytecode = True
from config import PROPS, ROOT

BACKGROUND = (27, 32, 32)
TEXT = (226, 225, 210)
MUTED = (153, 171, 164)
ACCENT = (202, 179, 124)
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
IDENTITIES = {'Furniture03': 'TRESTLE TABLE', 'Furniture04': 'HALF-ROUND TABLE',
              'Furniture05': 'TAVERN COUNTER'}


def label(draw, position, text, size=24, color=TEXT):
    draw.text(position, text, font=ImageFont.truetype(FONT, size), fill=color)


def composite(canvas, path, position, size):
    image = Image.open(path).convert('RGBA').resize(size, Image.Resampling.LANCZOS)
    canvas.paste(image, position, image)


def stamp_renders(root):
    for path in (root / 'review').glob('*-offline.png'):
        image = Image.open(path).convert('RGBA')
        draw = ImageDraw.Draw(image)
        draw.rectangle((0, 0, image.width, 34), fill=(*BACKGROUND, 255))
        label(draw, (12, 8), 'OFFLINE BLENDER RENDER  |  Not client evidence', 16, MUTED)
        image.save(path)


def comparison(root):
    summary = json.loads((root / 'validation/summary.json').read_text())
    canvas = Image.new('RGB', (1920, 1080), BACKGROUND)
    draw = ImageDraw.Draw(canvas)
    label(draw, (35, 24), f'{root.name.upper()} / LORENCIA {IDENTITIES[root.name]}', 32)
    label(draw, (35, 73), 'OFFLINE BLENDER • Matching camera and lighting • Client acceptance pending', 24, MUTED)
    old_count = {'Furniture03': 42, 'Furniture04': 38, 'Furniture05': 34}[root.name]
    titles = [f'ORIGINAL / {old_count} triangles', f'REBUILT / {summary["triangles"]} triangles', 'EXPORTED TOPOLOGY']
    for x, title, filename in zip((30, 660, 1290), titles,
                                  ('before-offline.png', 'after-offline.png', 'wireframe-offline.png')):
        label(draw, (x, 123), title, 24, ACCENT)
        composite(canvas, root / 'review' / filename, (x, 165), (600, 475))
    label(draw, (35, 668), 'REDUCED-SCALE READABILITY', 24, ACCENT)
    for x, state in ((90, 'before'), (450, 'after')):
        composite(canvas, root / f'review/{state}-offline.png', (x, 710), (275, 218))
        label(draw, (x + 80, 935), 'Original' if state == 'before' else 'Rebuilt', 22)
    lines = [f'{summary["triangles"]} / 1,500 triangle prop budget',
             '1 mesh • 1 bone • 1 action • 1 frame • lock=0',
             'Skeleton/actions: EQUIVALENT', 'Original bounds retained; original placement data untouched',
             '1024 × 1024 RGB diffuse / desk_big.OZJ']
    for i, text in enumerate(lines):
        label(draw, (860, 740 + i * 42), text, 24)
    label(draw, (35, 1030), 'After views use re-imported game exports. No PBR or normal maps. Runtime lighting and appearance remain unverified.', 21, MUTED)
    canvas.save(root / 'review/comparison.png')


def uv_sheet(root):
    canvas = Image.new('RGB', (1104, 1170), BACKGROUND)
    draw = ImageDraw.Draw(canvas)
    label(draw, (40, 20), f'{root.name} / EXPORTED UV ATLAS', 27)
    label(draw, (40, 60), 'OFFLINE • Shared islands intentionally reused • Opaque RGB, no alpha edges', 20, MUTED)
    canvas.paste(Image.open(root / 'textures/desk_big.jpg'), (40, 115))
    polygons = json.loads((root / 'review/exported-uv.json').read_text())
    for polygon in polygons:
        points = [(40 + u * 1024, 115 + (1 - v) * 1024) for u, v in polygon]
        draw.line(points + points[:1], fill=(154, 208, 186), width=1)
    canvas.save(root / 'review/uv-atlas.png')


def joining_sheet(root):
    if root.name == 'Furniture03':
        return
    canvas = Image.new('RGB', (1920, 920), BACKGROUND)
    draw = ImageDraw.Draw(canvas)
    label(draw, (35, 24), f'{root.name.upper()} / ORIGINAL PLACEMENT ARRANGEMENT', 32)
    label(draw, (35, 73), 'OFFLINE BLENDER • Exact stored positions, rotations and scales • No terrain or placement edits', 24, MUTED)
    for x, state in ((0, 'before'), (960, 'after')):
        label(draw, (x + 35, 128), 'ORIGINAL' if state == 'before' else 'REBUILT EXPORT', 25, ACCENT)
        composite(canvas, root / f'review/arrangement-{state}-offline.png', (x, 168), (960, 700))
    note = ('Straight joining edge and all four original corner coordinates retained; original offset between halves retained.'
            if root.name == 'Furniture04' else
            'All original end corners retained. 200-unit placement spacing and 3.4696-unit tabletop overlap are unchanged.')
    label(draw, (35, 881), note, 21, MUTED)
    canvas.save(root / 'review/joining-comparison.png')


def overview():
    canvas = Image.new('RGB', (1800, 1060), BACKGROUND)
    draw = ImageDraw.Draw(canvas)
    label(draw, (35, 28), 'LORENCIA / TAVERN FURNITURE', 38)
    label(draw, (35, 83), 'OFFLINE BLENDER REVIEW • Three rebuilt props • Client verification pending', 26, MUTED)
    for column, name in enumerate(PROPS):
        x = column * 600
        root = ROOT / name
        summary = json.loads((root / 'validation/summary.json').read_text())
        label(draw, (x + 30, 150), IDENTITIES[name], 25, ACCENT)
        composite(canvas, root / 'review/after-offline.png', (x, 195), (600, 475))
        label(draw, (x + 30, 679), f'{name} • {summary["triangles"]} triangles', 24)
        label(draw, (x + 30, 725), 'Original', 20, MUTED)
        composite(canvas, root / 'review/before-offline.png', (x + 30, 758), (230, 182))
        label(draw, (x + 288, 790), '1 original bone', 22)
        label(draw, (x + 288, 825), '1 unchanged action', 22)
        label(draw, (x + 288, 860), 'Exact overall bounds', 22)
    label(draw, (35, 995), 'One shared 1024² diffuse atlas • Original filenames and placements • Re-imported BMD/OZJ exports shown', 25, MUTED)
    canvas.save(ROOT / 'review-overview.jpg', quality=95)


for prop in PROPS:
    folder = ROOT / prop
    stamp_renders(folder)
    comparison(folder)
    uv_sheet(folder)
    joining_sheet(folder)
overview()
