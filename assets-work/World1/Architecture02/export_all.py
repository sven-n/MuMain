"""Run the official exporter sequentially, retaining complete per-model logs."""
import os
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'

for name in ('House05', 'HouseWall01', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'Stair01'):
    folder = ROOT / name
    command = [BLENDER, '-b', str(folder / 'source.blend'), '--python-expr', 'import sys; sys.dont_write_bytecode = True', '--python',
               str(REPOSITORY / 'tools/blender/mu_bmd_export.py'), '--',
               '--out', str(folder / f'exports/{name}.bmd'), '--bmdconv', os.environ['MU_BMDCONV']]
    result = subprocess.run(command, text=True, capture_output=True, check=True, env={**os.environ, 'PYTHONDONTWRITEBYTECODE': '1'})
    (folder / 'validation/export.txt').write_text(result.stdout + result.stderr)

# Keep original protected corner and action data through the supported converter workflow.
import preserve_contract
for name in preserve_contract.NAMES:
    preserve_contract.preserve(name)

# Accepted compatibility models are copied exactly, never re-exported.
for name in ('House04', 'HouseWall02', 'HouseEtc02'):
    shutil.copy2(ROOT / name / f'original/{name}.bmd', ROOT / name / f'exports/{name}.bmd')
