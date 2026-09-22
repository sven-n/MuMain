"""Package owned artwork, preserving stone/shingle identities and all frozen dependencies."""
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

SIZES = {'tile_wood01': 512, 'tile_wood03': 512}
JPEG_QUALITY = 95


def package_owned(folder, name):
    size = SIZES[name]
    image = Image.open(ROOT / f'paintings/{name}-master.png').convert('RGB')
    image = image.resize((size, size), Image.Resampling.LANCZOS)
    extension, container = '.jpg', '.OZJ'
    image.save(folder / f'textures/{name}.png')
    arguments = dict(quality=JPEG_QUALITY, subsampling=0) if extension == '.jpg' else dict(compression=None)
    image.save(folder / f'textures/{name}{extension}', **arguments)
    mu_texture.wrap_file(folder / f'textures/{name}{extension}', folder / f'exports/{name}{container}')


def main():
    for original in sorted(ROOT.glob('*/original')):
        for texture in list(original.glob('*.OZJ')) + list(original.glob('*.OZT')):
            name, folder = texture.stem, original.parent
            if name in SIZES:
                package_owned(folder, name)
            else:
                shutil.copy2(texture, folder / 'exports')
                suffix = '.jpg' if texture.suffix == '.OZJ' else '.tga'
                shutil.copy2(original / f'{name}{suffix}', folder / 'textures')
    records = [dict(name=name, size=[size, size], tool='Built-in imagegen',
                    source=f'{name}-master.png', prompt=f'{name}-prompt.txt',
                    packaging='Lanczos RGB resize; JPEG95 4:4:4 or RGBA uncompressed bottom-left TGA; official mu_texture wrap',
                    alpha='Opaque RGB')
               for name, size in SIZES.items()]
    (ROOT / 'paintings/generation.json').write_text(json.dumps(records, indent=2) + '\n')


if __name__ == '__main__':
    main()
