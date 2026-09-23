"""Pair actual current and candidate export renders without changing their framing."""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent
LABEL_HEIGHT = 40
BACKGROUND = (53, 58, 53)


def pair_views(folder, label):
    for suffix in ('', '-reverse', '-small'):
        paths = [folder / (stage + suffix + '.png') for stage in ('current', 'candidate')]
        if not all(path.exists() for path in paths):
            continue
        images = [Image.open(path).convert('RGBA') for path in paths]
        width, height = images[0].size
        assert images[1].size == (width, height)
        sheet = Image.new('RGB', (2 * width, height + LABEL_HEIGHT), BACKGROUND)
        draw = ImageDraw.Draw(sheet)
        for i, (stage, image) in enumerate(zip(('Current', 'Candidate'), images)):
            sheet.paste(image, (i * width, LABEL_HEIGHT), image)
            draw.text((i * width + 12, 14), label + ' / ' + stage, fill='white')
        sheet.save(folder / ('comparison' + suffix + '.jpg'), quality=94)


if __name__ == '__main__':
    for name in ('Object04',):
        pair_views(ROOT / name / 'review', name)
    for folder in sorted((ROOT / 'review-assemblies').glob('*')):
        if folder.is_dir():
            pair_views(folder, folder.name + ' / actual placements')
