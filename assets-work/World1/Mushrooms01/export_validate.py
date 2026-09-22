"""Export the owned mushroom models and retain complete authoritative validation."""
import os
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
from prepare import ASSETS, BLENDER, CONVERTER, HERE, ROOT
os.environ['MU_BMDCONV'] = CONVERTER
sys.path.insert(0, str(HERE.parent / 'StaticBatch01'))
import validate_export as shared
shared.EXPECTED_KEYS = {name: 1 for name in ASSETS}
shared.EXPECTED_TEXTURES = {name: ['mushroom.jpg'] for name in ASSETS}


def logged(command, path):
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    path.write_text(result.stdout + result.stderr)
    result.check_returncode()


def export(name):
    folder = HERE / name
    logged([BLENDER, '--python-expr', 'import sys; sys.dont_write_bytecode = True', '-b', str(folder / 'source.blend'), '--python-exit-code', '1', '--python',
            str(ROOT / 'tools/blender/mu_bmd_export.py'), '--', '--out', str(folder / 'exports' / (name + '.bmd')),
            '--bmdconv', CONVERTER], folder / 'validation/export.log')
    shutil.copy2(HERE / 'textures/final/mushroom.OZJ', folder / 'exports/mushroom.OZJ')
    shared.validate(folder)
    logged([BLENDER, '--python-expr', 'import sys; sys.dont_write_bytecode = True', '-b', '--python-exit-code', '1', '--python', str(ROOT / 'tools/blender/mu_bmd_import.py'), '--',
            '--bmd', str(folder / 'exports' / (name + '.bmd')), '--out', str(folder / 'validation/reimported.blend'),
            '--textures', str(HERE / 'textures/final'), '--bmdconv', CONVERTER], folder / 'validation/reimport.log')


if __name__ == '__main__':
    for asset in ASSETS:
        export(asset)
