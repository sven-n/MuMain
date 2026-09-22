"""Paths and explicit ownership for this offline furniture batch."""

import os
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REPOSITORY = ROOT.parents[2]
DATA = REPOSITORY / 'src/bin/Data'
PRIMARY = REPOSITORY.parent / 'MuMain'
CONVERTER = Path(os.environ.get('BMD_CONVERTER', str(
    PRIMARY / 'out/build/macos-arm64/tools/bmdconv/Release/bmdconv')))
BLENDER = Path('/Applications/Blender.app/Contents/MacOS/Blender')
PROPS = {'Furniture03': 142, 'Furniture04': 143, 'Furniture05': 144}
TEXTURE = 'desk_big'
CLAIMED = {f'Object1/{name}.bmd' for name in PROPS} | {'Object1/desk_big.OZJ'}
MAX_TRIANGLES = 1500
