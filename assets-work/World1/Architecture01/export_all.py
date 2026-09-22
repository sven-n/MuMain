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

for name in ('House01', 'House03', 'House04', 'Tent01', 'HouseWall02'):
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

# The accepted annex is a readonly compatibility consumer, never re-exported.
shutil.copy2(ROOT / "HouseEtc02/original/HouseEtc02.bmd", ROOT / "HouseEtc02/exports/HouseEtc02.bmd")
