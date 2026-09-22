"""Reuse the reviewed full rig/action validator for the three owned timber/lantern models."""
import importlib.util
from pathlib import Path
import sys

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REFERENCE = ROOT.parent / 'StaticBatch01/validate_export.py'
NAMES = ('House02', 'TreasureDrum01', 'StreetLight01')

spec = importlib.util.spec_from_file_location('static_validator', REFERENCE)
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)
validator.EXPECTED_TEXTURES = {'House02': ['drum.jpg', 'steel.jpg'], 'TreasureDrum01': ['drum.jpg'],
                               'StreetLight01': ['streetlight.jpg', 'streetlight_brightness2.jpg']}
validator.EXPECTED_KEYS = {'House02': 1, 'TreasureDrum01': 1, 'StreetLight01': 21}

if __name__ == '__main__':
    for name in NAMES:
        validator.validate(ROOT / name)
