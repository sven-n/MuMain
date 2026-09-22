"""Package imagegen paintings with unchanged original effect fields and alpha masks."""
import json
from pathlib import Path
import shutil
import subprocess
import sys

from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ORIGINAL = HERE / 'textures/original'
FINAL = HERE / 'textures/final'
FINAL.mkdir(parents=True, exist_ok=True)


def wrap(image, stem, extension):
    image.save(FINAL / (stem + '.png'))
    decoded = FINAL / (stem + extension)
    if extension == '.jpg':
        image.convert('RGB').save(decoded, quality=96, subsampling=0)
    else:
        image.save(decoded)
    container = FINAL / (stem + ('.OZT' if extension == '.tga' else '.OZJ'))
    subprocess.run([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'wrap', str(decoded),
                    '--out', str(container)], cwd=ROOT, check=True)


def main():
    # Original effect textures and the shared metal bracket remain byte-identical.
    for stem in ('light3', 'fire_02', 'tile_02'):
        for suffix in ('.OZJ', '.jpg'):
            shutil.copy2(ORIGINAL / (stem + suffix), FINAL / (stem + suffix))
    iron = Image.open(HERE / 'textures/raw/iron-generated.png').convert('RGB')
    wrap(iron.resize((512, 512), Image.Resampling.LANCZOS), 'light2', '.jpg')
    alpha_report = {}
    for stem in ('light', 'fire_light_01'):
        source = Image.open(ORIGINAL / (stem + '.tga')).convert('RGBA')
        mask = source.getchannel('A').resize((256, 256), Image.Resampling.BILINEAR)
        color = iron.resize((256, 256), Image.Resampling.LANCZOS).convert('RGBA')
        color.putalpha(mask)
        mask.save(FINAL / (stem + '-alpha.png'))
        wrap(color, stem, '.tga')
        alpha_report[stem] = dict(original_size=source.size, final_size=color.size,
                                  mask='Exact bilinear resample of original scalar alpha; color-only repaint',
                                  original_coverage=sum(a >= 128 for a in source.getchannel('A').getdata()) / (source.width * source.height),
                                  final_coverage=sum(a >= 128 for a in mask.getdata()) / (mask.width * mask.height))
    for stem, raw in (('copra_gate', 'copra_gate-generated.png'), ('fire_01', 'wood-generated.png')):
        image = Image.open(HERE / 'textures/raw' / raw).convert('RGB')
        wrap(image.resize((512, 512), Image.Resampling.LANCZOS), stem, '.jpg')
    (FINAL / 'alpha-preservation.json').write_text(json.dumps(alpha_report, indent=2) + '\n')
    containers = sorted(p for p in FINAL.iterdir() if p.suffix.upper() in ('.OZJ', '.OZT'))
    result = subprocess.check_output([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'check',
                                      *map(str, containers)], cwd=ROOT, text=True)
    (FINAL / 'loader-check.txt').write_text(result)


if __name__ == '__main__':
    main()
