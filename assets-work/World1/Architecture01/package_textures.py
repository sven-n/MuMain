"""Package owned artwork, preserving the original awning cutout mask and frozen dependencies."""
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

SIZES = {'tile_house01': 512, 'tile_ston05': 512, 'tile_windows01': 512,
         'tile_ston07': 512, 'tile_space01': 256, 'light_02': 128}
JPEG_QUALITY = 95


def package_owned(folder, name):
    size = SIZES[name]
    image = Image.open(ROOT / f'paintings/{name}-master.png').convert('RGB')
    image = image.resize((size, size), Image.Resampling.LANCZOS)
    extension, container = '.jpg', '.OZJ'
    if name == 'tile_ston07':
        # This is technical matte preservation, not new painted artwork: original holes stay exact.
        alpha = Image.open(folder / f'original/{name}.tga').getchannel('A')
        alpha.save(ROOT / 'paintings/tile_ston07-original-alpha.png')
        image.putalpha(alpha.resize((size, size), Image.Resampling.NEAREST))
        extension, container = '.tga', '.OZT'
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
                    alpha='Original 128px alpha retained exactly at nearest 4x scale' if name == 'tile_ston07' else 'Opaque RGB')
               for name, size in SIZES.items()]
    (ROOT / 'paintings/generation.json').write_text(json.dumps(records, indent=2) + '\n')


if __name__ == '__main__':
    main()
