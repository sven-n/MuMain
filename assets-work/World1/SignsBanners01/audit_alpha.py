"""Audit decoded final masks against the exact original scalar field and preview cutouts."""
import json
from pathlib import Path
import sys
sys.dont_write_bytecode = True
from PIL import Image, ImageChops, ImageDraw, ImageFont, ImageOps
ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT))
from package_textures import SPECS, pad_fringe
records = {}
font = '/System/Library/Fonts/Supplemental/Arial.ttf'
canvas = Image.new('RGB', (1800, 1550), (24, 29, 31))
draw = ImageDraw.Draw(canvas)
def label(x, y, text, size=22):
    draw.text((x, y), text, fill=(224, 225, 215), font=ImageFont.truetype(font, size))
label(30, 20, 'SIGNAGE / HERALDRY  •  PAINT AND ALPHA REVIEW', 32)
label(30, 65, 'Original scalar alpha retained exactly after bilinear enlargement. Cutout previews approximate engine alpha >0.25.')
for index, (stem, (owner, extension, size)) in enumerate(SPECS.items()):
    folder = ROOT / owner
    original = Image.open(folder / 'original' / (stem+extension)).convert('RGBA')
    final = Image.open(folder / 'textures' / (stem+extension)).convert('RGBA')
    expected = original.getchannel('A').resize(size, Image.Resampling.BILINEAR)
    actual = final.getchannel('A')
    assert ImageChops.difference(expected, actual).getbbox() is None
    if extension == '.tga':
        assert pad_fringe(final.convert('RGB'), actual).tobytes() == final.convert('RGB').tobytes()
    records[stem] = dict(status='PASS', dimensions=size, original_dimensions=original.size,
        exact_bilinear_original_alpha=True,
        coverage={str(cut): sum(v>cut for v in actual.getdata())/(size[0]*size[1]) for cut in (0, 63, 127, 191)},
        rgb_fringe='Nearest >=128 alpha RGB extends into lower alpha; no white/black matte inserted' if extension=='.tga' else 'Opaque RGB')
    x, y = 25+(index%2)*900, 130+(index//2)*455
    label(x, y, f'{stem}  /  {size[0]}×{size[1]}', 26)
    for col, (kind, image) in enumerate([('ORIGINAL', original), ('PAINTED', final), ('ALPHA', actual.convert('RGBA'))]):
        preview = Image.new('RGB', (275, 340), (77, 79, 78))
        if kind != 'ALPHA':
            alpha = image.getchannel('A').point(lambda v: v if v>63 else 0)
            image = image.copy(); image.putalpha(alpha)
        image = ImageOps.contain(image, (260, 320), Image.Resampling.LANCZOS)
        preview.paste(image, ((275-image.width)//2,(340-image.height)//2), image)
        canvas.paste(preview, (x+col*290, y+45))
        label(x+col*290, y+390, kind, 20)
(ROOT / 'paintings/alpha-audit.json').write_text(json.dumps(records,indent=2)+'\n')
canvas.save(ROOT / 'paintings/alpha-review.jpg',quality=95)
print('PASS: six exact original alpha fields, four padded RGBA cutouts; scalar alpha never thresholded in game textures')
