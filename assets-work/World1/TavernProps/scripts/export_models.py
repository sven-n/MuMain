"""Export each packed source with the unmodified repository Blender exporter."""

import os
import subprocess
import sys

sys.dont_write_bytecode = True
from config import BLENDER, CONVERTER, PROPS, REPOSITORY, ROOT


def export(name):
    root = ROOT / name
    command = [str(BLENDER), '-b', str(root / 'source.blend'), '--python-exit-code', '1',
               '--python', str(REPOSITORY / 'tools/blender/mu_bmd_export.py'), '--',
               '--out', str(root / f'exports/{name}.bmd'), '--bmdconv', str(CONVERTER)]
    environment = dict(os.environ, PYTHONDONTWRITEBYTECODE='1')
    result = subprocess.run(command, capture_output=True, text=True, env=environment)
    lines = (result.stdout + result.stderr).splitlines()
    (root / 'validation/export-blender.txt').write_text('\n'.join(line.rstrip() for line in lines) + '\n')
    result.check_returncode()
    print(f'Exported {name}')


for prop in PROPS:
    export(prop)
