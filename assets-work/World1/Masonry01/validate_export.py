"""Reuse the reviewed full rig/action validator for the six owned modular masonry models."""
import importlib.util
from pathlib import Path
import sys

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REFERENCE = ROOT.parent / 'StaticBatch01/validate_export.py'
NAMES = ('HouseEtc01', 'StoneMuWall01', 'StoneMuWall02', 'StoneMuWall03', 'StoneMuWall04', 'HouseEtc02')

spec = importlib.util.spec_from_file_location('static_validator', REFERENCE)
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)
validator.EXPECTED_TEXTURES = {
    'HouseEtc01': ['c_wall04.jpg', 'c_wall06.jpg'],
    'StoneMuWall01': ['c_wall04.jpg', 'c_wall05.jpg', 'c_wall06.jpg', 'tile_02.jpg', 'bridge_01.jpg'],
    'StoneMuWall02': ['c_wall06.jpg', 'c_wall05.jpg', 'c_wall04.jpg'],
    'StoneMuWall03': ['c_wall04.jpg', 'c_wall06.jpg'],
    'StoneMuWall04': ['c_wall04.jpg', 'c_wall06.jpg', 'horse_drawn_01.jpg'],
    'HouseEtc02': ['tile_wood02.jpg', 'tile_ston04.jpg', 'c_wall04.jpg', 'tile_wood03.jpg', 'tile_house01.jpg', 'tile_ston01.jpg', 'horse_drawn_01.jpg', 'tile_ston06.jpg'],
}
validator.EXPECTED_KEYS = {name: 1 for name in NAMES}

if __name__ == '__main__':
    for name in NAMES:
        validator.validate(ROOT / name)
