"""Assemble a labeled inventory of the actual combined offline renders."""

import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
FONT = '/System/Library/Fonts/Supplemental/Arial.ttf'


def main():
    models = json.loads((HERE / 'dependency-map.json').read_text())['models']
    names = [name for name, model in sorted(models.items()) if not model['scope_exclusion']]
    identity = json.loads((HERE / 'identities.json').read_text())
    title, small = ImageFont.truetype(FONT, 25), ImageFont.truetype(FONT, 16)
    links = ['# Combined Lorencia offline review', '',
             'All 106 in-scope static assets, rendered from the integration game files and their current shared textures. Each image has a SHA-256 provenance record. These are Blender diffuse previews, not client captures. Per-batch matching-camera comparisons, wireframes, animations and placement assemblies remain in each asset deliverable.', '',
             'Black effect cards on candles, the bonfire and some window beams are visible in this neutral diffuse view because the client applies additive blending to those mesh slots. See the labeled per-batch effect previews for that approximation. This gallery does not simulate client additive/scroll effects, terrain lighting or collision.', '']
    for start in range(0, len(names), 20):
        selected = names[start:start+20]
        canvas = Image.new('RGB', (1920, 100 + 500*((len(selected)+3)//4)), '#242a2d')
        draw = ImageDraw.Draw(canvas)
        page = start//20 + 1
        draw.text((24, 18), f'LORENCIA / COMBINED ART PASS / {page:02d}', font=title, fill='#e8dcc5')
        draw.text((24, 55), 'OFFLINE BLENDER PREVIEW — actual integrated models + shared textures — client review pending', font=small, fill='#bac6c8')
        for index, name in enumerate(selected):
            folder = HERE / 'final-inspection' / name
            info = json.loads((folder / 'provenance.json').read_text())
            x, y = (index % 4)*480, 100+(index//4)*500
            image = Image.open(folder / 'final-offline.png').convert('RGBA')
            canvas.paste(image, (x, y), image)
            draw.text((x+14, y+439), f'{name} | {info["triangles"]} tri | {len(models[name]["placements"])} placements', font=small, fill='#e8dcc5')
            description = identity[name]
            draw.text((x+14, y+465), description[:55], font=small, fill='#bac6c8')
        path = HERE / f'final-sheet-{page:02d}.jpg'
        canvas.save(path, quality=92)
        links += [f'![Combined offline inventory {page}]({path.name})', '']
    (HERE / 'final-review.md').write_text('\n'.join(links) + '\n')


if __name__ == '__main__':
    main()
