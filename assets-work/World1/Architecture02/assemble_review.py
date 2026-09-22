"""Label Blender render evidence and compare it at identical display sizes."""

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
PAPER, TEXT, MUTED, ACCENT = (24, 29, 31), (224, 225, 215), (159, 171, 172), (213, 186, 126)
IDENTITIES = {'House05': 'ANIMATED WATERMILL', 'HouseWall01': 'STRAIGHT MASONRY WALL',
              'HouseWall04': 'L-CORNER MASONRY WALL', 'HouseWall05': 'CORNER ROOF CAP',
              'HouseWall06': 'STRAIGHT ROOF CAP', 'Stair01': 'NINE-RUNG LADDER',
              'House04': 'READONLY ROUND WORKSHOP', 'HouseWall02': 'READONLY WINDOW WALL',
              'HouseEtc02': 'READONLY MASONRY ANNEX'}
ORIGINAL_TRIANGLES = {'House05': 276, 'HouseWall01': 36, 'HouseWall04': 56,
                      'HouseWall05': 30, 'HouseWall06': 28, 'Stair01': 78,
                      'House04': 358, 'HouseWall02': 110, 'HouseEtc02': 250}
KEYS = {'House05': 30, 'HouseWall01': 1, 'HouseWall04': 1, 'HouseWall05': 1,
        'HouseWall06': 1, 'Stair01': 1, 'House04': 40, 'HouseWall02': 1, 'HouseEtc02': 1}


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
             ('after', f'CURRENT  /  {data["triangles"]} triangles'), ('wireframe', 'EXPORTED TOPOLOGY')]
    for index, (stage, label) in enumerate(views):
        x = 30 + index * 635
        text(draw, (x, 113), label, 25, ACCENT)
        place(canvas, folder / f'review/{stage}.png', (x, 148), (590, 511))
    text(draw, (30, 694), 'OFFLINE GAMEPLAY-SCALE STUDY  /  BEFORE', 21, ACCENT)
    text(draw, (665, 694), 'SAME SCALE  /  AFTER', 21, ACCENT)
    for index, stage in enumerate(('before', 'after')):
        place(canvas, folder / f'review/gameplay-{stage}.png', (30 + index * 635, 725), (540, 330))
    dimensions = [b - a for a, b in zip(*data['bounds_after'])]
    details = [f'{len(data["bone_order"])} bones / {KEYS[name]} action keys',
               'Skeleton + action compare: EQUIVALENT',
               'Bound size: ' + ' × '.join(f'{value:.2f}' for value in dimensions),
               'All original connection vertices retained',
               'Scale proxy: 190 units tall', 'Terrain repeats: 100 units',
               'Camera scale is an offline assumption']
    for index, label in enumerate(details):
        text(draw, (1300, 727 + index * 35), label, 21, TEXT if index < 4 else MUTED)
    text(draw, (30, 1104), 'NOT CLIENT SCREENSHOTS. Matched Blender camera; additive effects approximated. Flat grass and figure are scale proxies, not actual client views.', 20, MUTED)
    canvas.save(folder / 'review/comparison.png')


def batch_sheet():
    canvas = Image.new('RGB', (1800, 1800), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 20), 'LORENCIA  /  WATERMILL, WALLS, ROOF CAPS AND LADDER', 32)
    text(draw, (30, 68), 'OFFLINE BLENDER EXPORT REVIEW  /  Six models / 27 placements + three readonly consumers  /  Client verification pending', 22, MUTED)
    for index, name in enumerate(IDENTITIES):
        x, y = 25 + (index % 3) * 595, 115 + (index // 3) * 530
        place(canvas, ROOT / name / 'review/after.png', (x, y), (520, 451))
        data = json.loads((ROOT / name / 'validation/blender.json').read_text())
        text(draw, (x + 10, y + 445), name, 26, ACCENT)
        text(draw, (x + 10, y + 485), f'{ORIGINAL_TRIANGLES[name]} → {data["triangles"]} triangles / preserved rig', 22)
    text(draw, (30, 1740), 'Fixed footprints, openings, roof edges and effect surfaces. Shared stone/shingle atlases reviewed on all nine consumers.', 23, MUTED)
    (ROOT / 'review').mkdir(exist_ok=True)
    canvas.save(ROOT / 'review/batch-review.jpg', quality=95)


def assembly_sheet(label):
    canvas = Image.new('RGB', (1920, 740), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 20), f'{label.upper()}  /  ACTUAL WORLD1 PLACEMENT TRANSFORMS', 31)
    subtitle = 'OFFLINE BLENDER: roof caps omitted to illustrate HeroTile 4 alpha target 0 / Original left, replacement right' if label == 'town-interior' else 'OFFLINE BLENDER ASSEMBLY / Original left, exported replacement right / No placement edits'
    text(draw, (30, 67), subtitle, 23, MUTED)
    for index, stage in enumerate(('before', 'after')):
        place(canvas, ROOT / f'review/{label}-{stage}.png', (25 + index * 960, 132), (930, 543))
    text(draw, (30, 699), 'Not client evidence. Assigned and compatibility models only; terrain, lighting, collision and unowned buildings omitted.', 22, MUTED)
    canvas.save(ROOT / f'review/{label}-comparison.jpg', quality=95)


def texture_sheet():
    canvas = Image.new('RGB', (1800, 1030), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (25, 18), 'STONE AND SHINGLE PAINTINGS / ORIGINAL ABOVE, REPAINT BELOW', 31)
    text(draw, (25, 63), 'Original material identities retained despite wood filenames. Final textures 512 x 512, opaque RGB.', 23, MUTED)
    for index, (name, owner) in enumerate((('tile_wood01', 'House05'), ('tile_wood03', 'HouseWall05'))):
        x = 225 + index * 850
        text(draw, (x, 112), name, 25, ACCENT)
        place(canvas, ROOT / owner / f'original/{name}.jpg', (x, 155), (400, 400))
        place(canvas, ROOT / owner / f'textures/{name}.png', (x, 590), (400, 400))
    canvas.save(ROOT / 'review/texture-comparison.jpg', quality=95)


def action_sheet(name, frames):
    canvas = Image.new('RGB', (1800, 1180), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (25, 18), f'{name.upper()} / ORIGINAL ACTION POSES', 31)
    text(draw, (25, 65), 'OFFLINE REIMPORTED GAME EXPORTS / Original above, replacement below / Every local key preserved', 22, MUTED)
    for row, stage in enumerate(('before', 'after')):
        for index, frame in enumerate(frames):
            x, y = 25 + index * 590, 112 + row * 490
            text(draw, (x, y), f'{stage.upper()} / FRAME {frame}', 25, ACCENT)
            place(canvas, ROOT / name / f'review/action-{stage}-{frame}.png', (x, y + 38), (560, 450))
    label = 'Blue porthole: illustrative V offsets 0, -1/3, -2/3. Actual engine uses world-time scroll; additive shading is approximated.' if name == 'House04' else 'Mill wheel: all 30 keys retained. Original water texture/geometry, illustrative V offsets 0, -1/3, -2/3; additive approximation.'
    text(draw, (25, 1110), label, 21, MUTED)
    canvas.save(ROOT / name / 'review/action-comparison.jpg', quality=95)


def effect_sheet(name):
    canvas = Image.new('RGB', (1800, 660), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (25, 18), f'{name.upper()} / PRESERVED INDEXED EFFECT SURFACE', 31)
    text(draw, (25, 63), 'OFFLINE exported BMD. Neutral diffuse left; additive approximation right. Actual engine/client review pending.', 22, MUTED)
    for index, stage in enumerate(('neutral-after', 'after')):
        place(canvas, ROOT / name / f'review/{stage}.png', (125 + index * 900, 106), (620, 537))
    canvas.save(ROOT / name / 'review/effect-comparison.jpg', quality=95)


if __name__ == '__main__':
    for name in IDENTITIES:
        review_sheet(name)
    batch_sheet()
    texture_sheet()
    for label in ('town-roof', 'town-interior', 'west-corners'):
        if (ROOT / f'review/{label}-after.png').exists():
            assembly_sheet(label)

    action_sheet('House04', (0, 19, 39))
    action_sheet('House05', (0, 14, 29))
    for name in ('House05', 'House04', 'HouseWall02'):
        effect_sheet(name)
