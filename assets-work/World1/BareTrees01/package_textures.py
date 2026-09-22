"""Encode the painted bark and archive byte-identical dependent textures."""
import json
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
import numpy as np
from PIL import Image
from config import FROZEN, REPOSITORY, ROOT


def main():
    folder = ROOT / 'textures/final'
    raw = Image.open(ROOT / 'textures/raw/tree_03.png').convert('RGB')
    image = raw.resize((512, 256), Image.Resampling.LANCZOS)
    image.save(folder / 'tree_03.png')
    image.save(folder / 'tree_03.jpg', quality=96, subsampling=0)
    subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'wrap',
                    str(folder / 'tree_03.jpg'), '--out', str(folder / 'tree_03.OZJ')],
                   cwd=REPOSITORY, check=True)
    for filename in FROZEN:
        extension = '.tga' if filename.endswith('.OZT') else '.jpg'
        for source in (filename, Path(filename).stem + extension):
            shutil.copy2(ROOT / 'textures/original' / source, folder / source)
    result = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'),
                             'check', *map(str, sorted(folder.glob('*.OZ*')))],
                            cwd=REPOSITORY, text=True, capture_output=True, check=True)
    (folder / 'texture-check.txt').write_text(result.stdout)
    pixels = np.array(image, dtype=float)
    report = {'size': list(image.size), 'mode': image.mode,
              'horizontal_edge_mean_absolute_difference': float(np.abs(pixels[:,0]-pixels[:,-1]).mean()),
              'vertical_edge_mean_absolute_difference': float(np.abs(pixels[0]-pixels[-1]).mean()),
              'processing': 'Resize, JPEG encoding and MU wrapper only; no painted modification',
              'alpha': 'Bark remains opaque RGB; frozen conifer alpha texture unchanged'}
    (folder / 'bark-audit.json').write_text(json.dumps(report, indent=2) + '\n')
    tile = Image.new('RGB', (1024, 512))
    for x in (0,512):
        for y in (0,256):
            tile.paste(image,(x,y))
    tile.save(folder / 'tiling-preview.jpg', quality=95)


if __name__ == '__main__':
    main()
