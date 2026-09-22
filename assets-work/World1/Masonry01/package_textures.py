"""Package four imagegen paintings; copy all frozen dependencies byte for byte."""
import json
from pathlib import Path
import shutil
import sys
from PIL import Image

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

OWNED = ('c_wall04', 'c_wall05', 'c_wall06', 'tile_ston01')

for root in sorted(ROOT.glob('*/original')):
    for texture in root.glob('*.OZJ'):
        name = texture.stem
        folder = root.parent
        if name in OWNED:
            image = Image.open(ROOT / f'paintings/{name}-master.png').convert('RGB')
            image = image.resize((512, 512), Image.Resampling.LANCZOS)
            image.save(folder / f'textures/{name}.png')
            image.save(folder / f'textures/{name}.jpg', quality=95, subsampling=0)
            mu_texture.wrap_file(folder / f'textures/{name}.jpg', folder / f'exports/{name}.OZJ')
        else:
            shutil.copy2(texture, folder / 'exports')
            shutil.copy2(root / f'{name}.jpg', folder / 'textures')

records = [dict(name=name, size=[512, 512], tool='Built-in imagegen',
                source=f'{name}-master.png', prompt=f'{name}-prompt.txt',
                processing='RGB, Lanczos resize, JPEG quality95 4:4:4, OZJ wrap',
                alpha='Opaque RGB; no transparency added') for name in OWNED]
(ROOT / 'paintings/generation.json').write_text(json.dumps(records, indent=2) + '\n')
