"""Label matched Blender review images and retain reduced-scale previews."""
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
BACKGROUND = (25, 31, 30)
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'


def paste(sheet, image_path, box):
    image = Image.open(image_path).convert('RGBA')
    image.thumbnail(box[2:])
    sheet.paste(image, box[:2], image)


def review(name, original_count):
    folder = HERE / name
    report = json.loads((folder / 'validation/blender.json').read_text())
    sheet = Image.new('RGB', (1800, 1120), BACKGROUND)
    draw = ImageDraw.Draw(sheet)
    title = ImageFont.truetype(FONT, 32)
    label = ImageFont.truetype(FONT, 24)
    draw.text((30, 25), f'{name} / {7 if name == "Grass07" else 3} WOODLAND MUSHROOMS', font=title, fill='#e1e2d3')
    draw.text((30, 72), 'OFFLINE BLENDER / Matching cameras on original and reimported BMD / Client review pending', font=label, fill='#a8b5af')
    for column, stage in enumerate(('before', 'after', 'wireframe')):
        draw.text((30 + column*590, 135), stage.upper(), font=label, fill='#cbbb85')
        paste(sheet, folder / 'review' / (stage + '.png'), (20 + column*590, 175, 580, 440))
    for column, stage in enumerate(('before', 'after')):
        draw.text((30 + column*590, 625), 'REDUCED / ' + stage.upper(), font=label, fill='#cbbb85')
        image = Image.open(folder / 'review' / (stage + '.png')).convert('RGBA')
        image.thumbnail((240, 180))
        image.save(folder / 'review' / ('readability-' + stage + '.png'))
        paste(sheet, folder / 'review' / ('readability-' + stage + '.png'), (30 + column*590, 665, 240, 180))
        paste(sheet, folder / 'review' / ('scale-' + stage + '.png'), (275 + column*590, 660, 310, 310))
    text = [f'{original_count} → {report["triangles"]} triangles', '512 × 512 opaque diffuse atlas',
            'Per-mushroom bind bounds retained', 'Bone order / parents / action: EQUIVALENT',
            'Full geometry comparison: DIFFERENT', 'Scale bar: 190 game units tall']
    for index, line in enumerate(text):
        draw.text((1210, 680 + 42*index), line, font=label, fill='#dedecf')
    draw.text((30, 1040), 'Sources include REF_ORIGINAL. Painted red caps and ivory stems preserve the original material roles.', font=label, fill='#a8b5af')
    sheet.save(folder / 'review/comparison.jpg', quality=94)


review('Grass07', 126)
review('Grass08', 54)
