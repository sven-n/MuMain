"""Preserve and import the six owned modular masonry originals; never replace baselines."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
NAMES = ('HouseEtc01', 'StoneMuWall01', 'StoneMuWall02', 'StoneMuWall03', 'StoneMuWall04', 'HouseEtc02')
CONVERTER = os.environ['MU_BMDCONV']
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture


def preserve(name, record):
    folder = ROOT / name
    original = folder / 'original'
    if original.exists():
        raise RuntimeError(f'Refusing to overwrite {original}')
    for part in ('original', 'exports', 'textures', 'review', 'validation'):
        (folder / part).mkdir(parents=True)
    shutil.copy2(REPOSITORY / record['path'], original)
    (original / 'info.txt').write_text(record['info'])
    (original / 'placements.json').write_text(json.dumps(record['placements'], indent=2) + '\n')
    for image, containers in record['textures'].items():
        assert len(containers) == 1
        container = REPOSITORY / containers[0]
        shutil.copy2(container, original)
        mu_texture.unwrap_file(container, original / image)
    subprocess.run([CONVERTER, 'bmd2smd', str(original / f'{name}.bmd'), str(original / 'smd')], check=True)
    command = [BLENDER, '-b', '--python-expr', 'import sys; sys.dont_write_bytecode = True', '--python', str(REPOSITORY / 'tools/blender/mu_bmd_import.py'), '--',
               '--bmd', str(original / f'{name}.bmd'), '--out', str(original / 'source.blend'),
               '--bmdconv', CONVERTER]
    result = subprocess.run(command, capture_output=True, text=True, check=True, env={**os.environ, 'PYTHONDONTWRITEBYTECODE': '1'})
    (original / 'import.txt').write_text(result.stdout + result.stderr)


if __name__ == '__main__':
    inventory = json.loads((ROOT.parent / 'coordination/dependency-map.json').read_text())
    for name in NAMES:
        preserve(name, inventory['models'][name])
