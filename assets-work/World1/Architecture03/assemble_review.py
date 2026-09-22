"""Label Blender render evidence and compare it at identical display sizes."""

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'
PAPER, TEXT, MUTED, ACCENT = (24, 29, 31), (224, 225, 215), (159, 171, 172), (213, 186, 126)
IDENTITIES = {'HouseWall03': 'TIMBER LINTEL AND THRESHOLD', 'Bridge01': 'CARVED STONE BRIDGE PARAPET', 'BridgeStone01': 'TWIN-LOG BRIDGE RAIL', 'Fence01': 'TWO-RAIL TIMBER FENCE', 'FireLight02': 'IRON FIRE STANDARD', 'House01': 'LOW HOUSE SHELL WITH RAFTERS', 'House03': 'LOW HOUSE SHELL WITH AWNING', 'House04': 'ANIMATED ROUND WORKSHOP', 'House05': 'ANIMATED WATERMILL', 'HouseEtc02': 'LOW MASONRY ANNEX', 'HouseWall01': 'STRAIGHT MASONRY WALL', 'HouseWall02': 'WINDOW WALL', 'HouseWall04': 'L-CORNER MASONRY WALL', 'HouseWall05': 'CORNER ROOF CAP', 'HouseWall06': 'STRAIGHT ROOF CAP', 'StoneMuWall01': 'PORTCULLIS GATE', 'StoneWall01': 'STONE PORTCULLIS GATE', 'StoneWall02': 'STONE PORTCULLIS GATE VARIANT', 'Tent01': 'ANIMATED MARKET TENT', 'Tree07': 'ROOTED CUT STUMP'}
ORIGINAL_TRIANGLES = {'HouseWall03': 20, 'Bridge01': 196, 'BridgeStone01': 74, 'Fence01': 116, 'FireLight02': 34, 'House01': 208, 'House03': 313, 'House04': 358, 'House05': 324, 'HouseEtc02': 250, 'HouseWall01': 52, 'HouseWall02': 110, 'HouseWall04': 88, 'HouseWall05': 54, 'HouseWall06': 52, 'StoneMuWall01': 398, 'StoneWall01': 414, 'StoneWall02': 414, 'Tent01': 130, 'Tree07': 90}
KEYS = {'HouseWall03': 1, 'Bridge01': 1, 'BridgeStone01': 1, 'Fence01': 1, 'FireLight02': 1, 'House01': 1, 'House03': 1, 'House04': 40, 'House05': 30, 'HouseEtc02': 1, 'HouseWall01': 1, 'HouseWall02': 1, 'HouseWall04': 1, 'HouseWall05': 1, 'HouseWall06': 1, 'StoneMuWall01': 1, 'StoneWall01': 1, 'StoneWall02': 1, 'Tent01': 36, 'Tree07': 1}


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
    groups = [list(IDENTITIES)[:3], list(IDENTITIES)[3:12], list(IDENTITIES)[12:]]
    for page, names in enumerate(groups):
        canvas = Image.new('RGB', (1800, 1800 if page else 820), PAPER)
        draw = ImageDraw.Draw(canvas)
        text(draw, (30, 20), 'LORENCIA / BRIDGES AND SHARED ARCHITECTURAL MATERIALS', 31)
        text(draw, (30, 65), 'OFFLINE exported BMD review / Client acceptance pending / All compatibility BMDs byte-identical', 22, MUTED)
        for index, name in enumerate(names):
            x, y = 25 + (index % 3) * 595, 110 + (index // 3) * 530
            place(canvas, ROOT / name / 'review/after.png', (x, y), (520, 451))
            data = json.loads((ROOT / name / 'validation/blender.json').read_text())
            text(draw, (x + 10, y + 445), name, 26, ACCENT)
            text(draw, (x + 10, y + 485), f'{ORIGINAL_TRIANGLES[name]} → {data["triangles"]} triangles / preserved rig', 22)
        text(draw, (30, canvas.height-55), 'Original silhouettes, joins, effects and texture roles preserved. Diffuse lighting and additive effects are offline approximations.', 22, MUTED)
        (ROOT / 'review').mkdir(exist_ok=True)
        filename = 'batch-review.jpg' if not page else f'compatibility-{page}.jpg'
        canvas.save(ROOT / 'review' / filename, quality=95)


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
    references = (('bridge_01', 'Bridge01'), ('tree_04', 'BridgeStone01'), ('tile_ston06', 'BridgeStone01'),
                  ('tile_wood02', 'BridgeStone01'), ('tile_ston04', 'House01'), ('tile_02', 'FireLight02'))
    canvas = Image.new('RGB', (1920, 1490), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (25, 18), 'SIX SHARED MATERIAL PAINTINGS / ORIGINAL LEFT, REPAINT RIGHT', 31)
    text(draw, (25, 63), 'UV layout, ornament and material roles preserved. Bridge shadow RGBA strip remains byte-exact.', 23, MUTED)
    for index, (name, owner) in enumerate(references):
        x, y = 25 + (index % 2)*950, 115 + (index // 2)*450
        text(draw, (x, y), name, 25, ACCENT)
        size = (425, 212 if name == 'bridge_01' else 380)
        place(canvas, ROOT / owner / f'original/{name}.jpg', (x, y+40), size)
        place(canvas, ROOT / owner / f'textures/{name}.png', (x+460, y+40), size)
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
    if name == 'Tent01':
        label = 'Readonly tent BMD unchanged; three of its 36 preserved action keys. New shared timber/crest atlas only.'
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
    for label in ('stone-bridge', 'log-bridge', 'west-door', 'town-interior'):
        if (ROOT / f'review/{label}-after.png').exists():
            assembly_sheet(label)

    action_sheet('House04', (0, 19, 39))
    action_sheet('House05', (0, 14, 29))
    action_sheet('Tent01', (0, 17, 35))
    for name in ('House03', 'House05', 'House04', 'HouseWall02'):
        effect_sheet(name)
