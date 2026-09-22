"""Wrap the retained boat paintings and preserve original rigging alpha fields."""
import json
from pathlib import Path
import shutil
import subprocess
import sys

from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FINAL = HERE / 'textures/final'
ORIGINAL = HERE / 'textures/original'
PAINTS = ('TileGround03', 'ship01', 'ship03', 'ship04', 'ship07')


def wrap(image, name):
    decoded = FINAL / name
    image.save(FINAL / (name + '.png'))
    if decoded.suffix == '.jpg':
        image.convert('RGB').save(decoded, quality=96, subsampling=0)
    else:
        image.save(decoded)
    container = FINAL / (decoded.stem + ('.OZT' if decoded.suffix == '.tga' else '.OZJ'))
    subprocess.run([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'wrap', str(decoded),
                    '--out', str(container)], cwd=ROOT, check=True)


def main():
    FINAL.mkdir(parents=True, exist_ok=True)
    for stem in PAINTS:
        image = Image.open(HERE / 'textures/raw' / (stem + '.png')).convert('RGB')
        wrap(image.resize((512, 512), Image.Resampling.LANCZOS), stem + '.jpg')
    linen = Image.open(HERE / 'textures/raw/ship05_linen.png').convert('RGB')
    wrap(linen.resize((512, 256), Image.Resampling.LANCZOS), 'ship05.jpg')
    source = Image.open(ORIGINAL / 'ship05.tga').convert('RGBA')
    alpha = source.getchannel('A').resize((512, 256), Image.Resampling.BILINEAR)
    ornament = Image.open(HERE / 'textures/raw/ship05_alpha.png').convert('RGBA')
    ornament = ornament.resize((512, 256), Image.Resampling.LANCZOS)
    ornament.putalpha(alpha)
    alpha.save(FINAL / 'ship05-alpha.png')
    wrap(ornament, 'ship05.tga')
    # Tiny rope cards and the end cap preserve their original low-resolution fields.
    for name in ('ship06.OZJ', 'ship06.OZT', 'ship07.OZT', 'ship06.jpg', 'ship06.tga', 'ship07.tga'):
        shutil.copy2(ORIGINAL / name, FINAL / name)
    containers = sorted(p for p in FINAL.iterdir() if p.suffix.upper() in ('.OZJ', '.OZT'))
    result = subprocess.check_output([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'check',
                                      *map(str, containers)], cwd=ROOT, text=True)
    (FINAL / 'loader-check.txt').write_text(result)
    (FINAL / 'alpha-preservation.json').write_text(json.dumps(dict(
        ship05='Original 64x32 alpha enlarged exactly bilinearly to 512x256',
        ship06='Original 4x16 RGBA texture container byte-identical, including graded alpha45..255',
        ship07='Original 8x32 RGBA texture container byte-identical',
        client_verified=False), indent=2) + '\n')


if __name__ == '__main__':
    main()
