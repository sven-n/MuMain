"""Package the two stone paintings and preserve every frozen texture container."""
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

OWNED = ('tile_01', 'tile_03')
TEXTURE_SIZE = (512, 512)


def package(folder):
    for texture in (folder / 'original').iterdir():
        if texture.suffix not in ('.OZJ', '.OZT'):
            continue
        if texture.stem in OWNED:
            image = Image.open(ROOT / f'paintings/{texture.stem}-master.png').convert('RGB')
            image = image.resize(TEXTURE_SIZE, Image.Resampling.LANCZOS)
            image.save(folder / f'textures/{texture.stem}.png')
            jpeg = folder / f'textures/{texture.stem}.jpg'
            image.save(jpeg, quality=95, subsampling=0)
            mu_texture.wrap_file(jpeg, folder / 'exports' / texture.name)
            continue
        shutil.copy2(texture, folder / 'exports')
        suffix = '.jpg' if texture.suffix == '.OZJ' else '.tga'
        shutil.copy2(folder / 'original' / (texture.stem + suffix), folder / 'textures')


if __name__ == '__main__':
    for original in sorted(ROOT.glob('*/original')):
        package(original.parent)
    records = [dict(name=name, size=TEXTURE_SIZE, tool='Built-in imagegen',
                    source=f'{name}-master.png', prompt=f'{name}-prompt.txt',
                    processing='RGB, Lanczos 512px, JPEG quality95 4:4:4, official OZJ wrap',
                    alpha='Opaque; frozen banner alpha container copied byte-identical') for name in OWNED]
    (ROOT / 'paintings/generation.json').write_text(json.dumps(records, indent=2) + '\n')
