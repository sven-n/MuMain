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
    draw.text((30, 25), f'{name} / CARVED RIVER BOAT', font=title, fill='#e1e2d3')
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
    text = [f'{original_count} → {report["triangles"]} triangles', '512 surface / 512x256 linen',
            'Original bind bounds retained', 'Bone order / parents / action: EQUIVALENT',
            'Geometry: ' + 'EQUIVALENT', 'Scale bar: 190 game units tall']
    for index, line in enumerate(text):
        draw.text((1210, 680 + 42*index), line, font=label, fill='#dedecf')
    draw.text((30, 1040), 'Sources include REF_ORIGINAL. Original 30-key action and alpha rigging retained. Offline diffuse/geometry study.', font=label, fill='#a8b5af')
    sheet.save(folder / 'review/comparison.jpg', quality=94)


for name, count in [('Ship01', 754)]:
    review(name, count)

folder = HERE / 'Ship01/review'
sheet = Image.new('RGB', (1800, 1530), BACKGROUND)
draw=ImageDraw.Draw(sheet)
font=ImageFont.truetype(FONT,26)
for row,view in enumerate(('reverse','pose-14','pose-29')):
    for col,stage in enumerate(('before','after')):
        draw.text((30+col*900,20+row*510), f'{view} / {stage} / OFFLINE',font=font,fill='#e0d8ba')
        paste(sheet,folder/(view+'-'+stage+'.png'),(col*900,55+row*510,900,450))
sheet.save(folder/'action-reverse-comparison.jpg',quality=94)
