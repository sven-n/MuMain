"""Label baseline offline inspection renders as contact sheets for identity review."""

from pathlib import Path
from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
COLUMNS = 4
ROWS = 5
CELL = (400, 400)
HEADER = 44


def main():
    images = sorted((HERE / 'inspection').glob('*/baseline-offline.png'))
    for start in range(0, len(images), COLUMNS * ROWS):
        sheet = Image.new('RGB', (COLUMNS * CELL[0], ROWS * CELL[1] + HEADER), '#242b30')
        draw = ImageDraw.Draw(sheet)
        draw.text((16, 14), 'INTEGRATION BASELINE — OFFLINE BLENDER — NOT CLIENT EVIDENCE', fill='white')
        for cell, path in enumerate(images[start:start + COLUMNS * ROWS]):
            x, y = cell % COLUMNS * CELL[0], HEADER + cell // COLUMNS * CELL[1]
            render = Image.open(path).convert('RGBA')
            sheet.paste(render, (x, y), render)
            draw.text((x + 12, y + 368), path.parent.name, fill='white')
        sheet.save(HERE / f'inventory-sheet-{start // (COLUMNS * ROWS) + 1:02}.jpg', quality=90)


if __name__ == '__main__':
    main()
