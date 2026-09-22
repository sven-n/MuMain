"""Compare preserved masks against light and dark backgrounds."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parent
sheet = Image.new('RGB', (1400, 700), (28, 34, 32))
draw = ImageDraw.Draw(sheet)
font = ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial.ttf', 24)
for row, stem in enumerate(['light', 'fire_light_01']):
    original = Image.open(root / 'textures/original' / (stem + '.tga')).convert('RGBA')
    final = Image.open(root / 'textures/final' / (stem + '.tga')).convert('RGBA')
    for col, (label, image, bg) in enumerate([
            ('Original', original, (130, 140, 132)), ('Final / light', final, (195, 204, 194)),
            ('Final / dark', final, (20, 25, 23))]):
        draw.text((30 + col*460, 20 + row*345), stem + ' / ' + label, fill='#dedeca', font=font)
        base = Image.new('RGBA', (256, 256), bg + (255,))
        base.alpha_composite(image.resize((256, 256), Image.Resampling.BILINEAR))
        sheet.paste(base.convert('RGB'), (30 + col*460, 65 + row*345))
sheet.save(root / 'textures/alpha-review.jpg', quality=95)
