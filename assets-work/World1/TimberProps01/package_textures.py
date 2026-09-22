"""Package retained imagegen paintings into the owned diffuse texture containers."""
import json
from pathlib import Path
import sys
from PIL import Image

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

SPECS = {'drum': ('drum', 512, ('House02', 'TreasureDrum01')),
         'steel': ('steel', 512, ('House02',)),
         'streetlight': ('streetlight', 512, ('StreetLight01',)),
         'brightness': ('streetlight_brightness2', 128, ('StreetLight01',))}


def main():
    records = []
    for name, (filename, size, owners) in SPECS.items():
        image = Image.open(ROOT / f'paintings/{name}-master.png').convert('RGB').resize((size, size), Image.Resampling.LANCZOS)
        for owner in owners:
            folder = ROOT / owner
            image.save(folder / f'textures/{filename}.png')
            image.save(folder / f'textures/{filename}.jpg', quality=95, subsampling=0)
            mu_texture.wrap_file(folder / f'textures/{filename}.jpg', folder / f'exports/{filename}.OZJ')
        records.append(dict(name=filename, size=[size,size], owners=owners, tool='Built-in imagegen',
                            source=f'{name}-master.png', prompt=f'{name}-prompt.txt',
                            processing='RGB, Lanczos resize, JPEG quality95 4:4:4, OZJ wrap', alpha='Opaque RGB; existing additive lantern mesh retained'))
    (ROOT / 'paintings/generation.json').write_text(json.dumps(records, indent=2) + '\n')


if __name__ == '__main__':
    main()
