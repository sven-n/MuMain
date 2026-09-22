"""Preserve selected originals and importer output; refuses replacement of saved baselines."""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

PROPS = ('Candle01', 'TreasureChest01', 'Tomb03')
DATA = REPOSITORY / 'src/bin/Data/Object1'
CONVERTER = Path(os.environ.get('MU_BMDCONV',
                               REPOSITORY / 'out/build/macos-arm64/tools/bmdconv/Release/bmdconv'))


def preserve(name, record):
    root = ROOT.parent / name
    original = root / 'original'
    assert not original.exists(), f'Refusing to overwrite {original}'
    for folder in ('original', 'paintings', 'textures', 'exports', 'review', 'validation'):
        (root / folder).mkdir(parents=True)
    shutil.copy2(DATA / f'{name}.bmd', original)
    shutil.copy2(Path('/tmp/astra-world1-candidates') / name / 'source.blend', original)
    shutil.copy2(ROOT / f'candidates/{name}.json', original / 'mesh-inspection.json')
    shutil.copy2(ROOT / f'candidates/{name}.png', original / 'inspection.png')
    (original / 'info.txt').write_text(record['info'])
    for field in ('placements', 'textures'):
        (original / f'{field}.json').write_text(json.dumps(record[field], indent=2) + '\n')
    for texture in record['textures']:
        suffix = '.ozj' if texture.lower().endswith('.jpg') else '.ozt'
        wanted = Path(texture).with_suffix(suffix).name.lower()
        container = next(path for path in DATA.iterdir() if path.name.lower() == wanted)
        shutil.copy2(container, original)
        mu_texture.unwrap_file(container, original / texture)
    subprocess.run([str(CONVERTER), 'bmd2smd', str(original / f'{name}.bmd'),
                    str(original / 'smd')], check=True)


def main():
    inventory = json.loads((ROOT / 'inventory.json').read_text())
    for name in PROPS:
        preserve(name, inventory['candidates'][name])


if __name__ == '__main__':
    main()
