"""Run the official exporter sequentially, retaining complete per-model logs."""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'

for name in ('House02', 'TreasureDrum01', 'StreetLight01'):
    folder = ROOT / name
    command = [BLENDER, '-b', str(folder / 'source.blend'), '--python-expr', 'import sys; sys.dont_write_bytecode = True', '--python',
               str(REPOSITORY / 'tools/blender/mu_bmd_export.py'), '--',
               '--out', str(folder / f'exports/{name}.bmd'), '--bmdconv', os.environ['MU_BMDCONV']]
    result = subprocess.run(command, text=True, capture_output=True, check=True, env={**os.environ, 'PYTHONDONTWRITEBYTECODE': '1'})
    (folder / 'validation/export.txt').write_text(result.stdout + result.stderr)
