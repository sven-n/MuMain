"""Label Blender render evidence and compare it at identical display sizes."""

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
PAPER, TEXT, MUTED, ACCENT = (24, 29, 31), (224, 225, 215), (159, 171, 172), (213, 186, 126)
IDENTITIES = {'House01': 'STONE HOUSE WITH CARVED DOOR', 'House03': 'THREE-CHIMNEY SMITHY',
              'House04': 'ANIMATED ROUND WORKSHOP', 'Tent01': 'WOVEN CANOPY',
              'HouseWall02': 'TIMBER WINDOW WALL', 'HouseEtc02': 'READONLY MASONRY ANNEX'}
ORIGINAL_TRIANGLES = {'House01': 176, 'House03': 289, 'House04': 342,
                      'Tent01': 66, 'HouseWall02': 70, 'HouseEtc02': 250}
KEYS = {'House01': 1, 'House03': 1, 'House04': 40, 'Tent01': 36, 'HouseWall02': 1, 'HouseEtc02': 1}


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
    canvas = Image.new('RGB', (1800, 1270), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (30, 20), 'LORENCIA  /  HOUSES, WORKSHOP, CANOPY AND WALLS', 32)
    text(draw, (30, 68), 'OFFLINE BLENDER EXPORT REVIEW  /  Five models / 21 placements + readonly annex  /  Client verification pending', 22, MUTED)
    for index, name in enumerate(IDENTITIES):
        x, y = 25 + (index % 3) * 595, 115 + (index // 3) * 530
        place(canvas, ROOT / name / 'review/after.png', (x, y), (520, 451))
        data = json.loads((ROOT / name / 'validation/blender.json').read_text())
        text(draw, (x + 10, y + 445), name, 26, ACCENT)
        text(draw, (x + 10, y + 485), f'{ORIGINAL_TRIANGLES[name]} → {data["triangles"]} triangles / preserved rig', 22)
    text(draw, (30, 1200), 'Fixed footprints, openings, roof edges and effect surfaces. Shared atlas reviewed on all six consumers.', 23, MUTED)
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


def texture_sheet():
    canvas = Image.new('RGB', (1800, 1430), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (25, 18), 'ARCHITECTURE PAINTINGS  /  ORIGINAL LEFT, REPAINT RIGHT', 31)
    text(draw, (25, 63), 'Material roles and atlas splits retained. Awning uses exact original alpha, nearest 4x.', 23, MUTED)
    sources = [('tile_house01', 'House01', 'jpg'), ('tile_ston05', 'House01', 'jpg'),
               ('tile_windows01', 'House04', 'jpg'), ('tile_ston07', 'House03', 'tga'),
               ('tile_space01', 'House04', 'jpg'), ('light_02', 'House03', 'jpg')]
    for index, (name, owner, suffix) in enumerate(sources):
        x, y = 25 + index % 2 * 895, 118 + index // 2 * 410
        text(draw, (x, y), name, 25, ACCENT)
        place(canvas, ROOT / owner / f'original/{name}.{suffix}', (x, y + 40), (350, 350))
        place(canvas, ROOT / owner / f'textures/{name}.png', (x + 385, y + 40), (350, 350))
    text(draw, (25, 1370), '512px surfaces / 256px blue effect / 128px light effect. Imagegen masters and prompts retained.', 23, MUTED)
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
    label = 'Blue porthole: illustrative V offsets 0, -1/3, -2/3. Actual engine uses world-time scroll; additive shading is approximated.' if name == 'House04' else '36 original animation keys retained; canopy triangles and multi-bone cloth silhouette unchanged.'
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
    for label in ('house-pair', 'town-wall'):
        if (ROOT / f'review/{label}-after.png').exists():
            assembly_sheet(label)

    action_sheet('House04', (0, 19, 39))
    action_sheet('Tent01', (0, 17, 35))
    for name in ('House03', 'House04', 'HouseWall02'):
        effect_sheet(name)
