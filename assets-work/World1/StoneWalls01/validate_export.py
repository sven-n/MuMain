"""Reuse the reviewed full rig/action validator for the five owned rough-stone wall models."""
import importlib.util
from pathlib import Path
import sys

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REFERENCE = ROOT.parent / 'StaticBatch01/validate_export.py'
NAMES = ('StoneWall01', 'StoneWall02', 'StoneWall03', 'StoneWall05', 'StoneWall06')

spec = importlib.util.spec_from_file_location('static_validator', REFERENCE)
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)
validator.EXPECTED_TEXTURES = {'StoneWall01': ['tile_01.jpg', 'tile_03.jpg', 'tile_02.jpg', 'bridge_01.jpg'], 'StoneWall02': ['tile_01.jpg', 'tile_03.jpg', 'tile_02.jpg', 'bridge_01.jpg'], 'StoneWall03': ['tile_01.jpg', 'horse_drawn_01.jpg', 'tile_03.jpg'], 'StoneWall05': ['tile_01.jpg', 'tile_03.jpg'], 'StoneWall06': ['badge_01.jpg', 'badge_03.tga', 'tile_01.jpg']}
validator.EXPECTED_KEYS = {'StoneWall01': 1, 'StoneWall02': 1, 'StoneWall03': 1, 'StoneWall05': 1, 'StoneWall06': 25}

if __name__ == '__main__':
    for name in NAMES:
        validator.validate(ROOT / name)
