"""Label action samples and repeated material previews; never modify painted artwork."""
from pathlib import Path
import sys
sys.dont_write_bytecode = True
from PIL import Image, ImageDraw
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from assemble_review import PAPER, MUTED, ACCENT, text, place


def action_sheet():
    canvas = Image.new('RGB', (1800, 1230), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (28, 20), 'STONEWALL06 / PRESERVED 25-KEY BANNER ACTION', 31)
    text(draw, (28, 68), 'OFFLINE actual BMD comparison / original above, export below / matched camera', 23, MUTED)
    for row, stage in enumerate(('before', 'after')):
        for col, frame in enumerate((0, 12, 24)):
            position = (30 + col*590, 140 + row*525)
            text(draw, (position[0], position[1]-33), f'{stage.upper()} / KEY {frame}', 24, ACCENT)
            place(canvas, ROOT / f'StoneWall06/review/action-{stage}-{frame}.png', position, (560, 485))
    text(draw, (28, 1200), 'All 25 keys checked numerically. Banner geometry, texture alpha and UVs preserved. Client review pending.', 22, MUTED)
    canvas.save(ROOT / 'review/banner-action-comparison.jpg', quality=95)


def tiling_sheet():
    canvas = Image.new('RGB', (1200, 750), PAPER)
    draw = ImageDraw.Draw(canvas)
    text(draw, (25, 20), 'ROUGH STONE / TWO-BY-TWO MATERIAL REPEATS', 29)
    text(draw, (25, 63), 'Final wrapped JPEG diffuse / 512 × 512 per tile / inspection only', 22, MUTED)
    for col, name in enumerate(('tile_01', 'tile_03')):
        x = 30 + col*590
        text(draw, (x, 109), name, 26, ACCENT)
        for row in range(2):
            for repeat in range(2):
                place(canvas, ROOT / f'StoneWall01/textures/{name}.jpg',
                      (x+repeat*280, 150+row*280), (280, 280))
    canvas.save(ROOT / 'review/texture-repeat.jpg', quality=95)


if __name__ == '__main__':
    action_sheet()
    tiling_sheet()
