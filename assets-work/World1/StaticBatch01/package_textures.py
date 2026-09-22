"""Downsample the retained paintings to engine budgets and wrap existing texture names."""

from pathlib import Path
import sys

from PIL import Image

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

TEXTURES = [('Candle01', 'diffuse', 'candle', 512), ('Candle01', 'flame', 'candle2', 128),
            ('TreasureChest01', 'diffuse', 'treasure_chest', 512), ('Tomb03', 'diffuse', 'tombstone', 512)]


def main():
    for prop, painting, name, size in TEXTURES:
        folder = ROOT.parent / prop
        with Image.open(folder / f'paintings/{painting}-master.png') as master:
            image = master.convert('RGB').resize((size, size), Image.Resampling.LANCZOS)
        image.save(folder / f'textures/{name}.png')
        jpeg = folder / f'textures/{name}.jpg'
        image.save(jpeg, quality=98, subsampling=0)
        target = folder / f'exports/{name}.OZJ'
        mu_texture.wrap_file(jpeg, target)
        errors = mu_texture.check_file(target)
        assert not errors, errors
        assert mu_texture.unwrap_bytes(target.read_bytes(), '.ozj') == jpeg.read_bytes()
        print(f'PASS {prop}/{target.name}: {size}x{size} RGB JPEG, exact wrapper payload')


if __name__ == '__main__':
    main()
