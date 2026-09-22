"""Reuse the reviewed full rig/action validator for the four owned modular models."""
import importlib.util
from pathlib import Path
import sys

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REFERENCE = ROOT.parent / 'StaticBatch01/validate_export.py'
NAMES = ('Fence01', 'Fence02', 'Fence03', 'Fence04')

spec = importlib.util.spec_from_file_location('static_validator', REFERENCE)
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)
validator.EXPECTED_TEXTURES = {name: ['tile_wood02.jpg' if name == 'Fence01' else 'joint.jpg'] for name in NAMES}
validator.EXPECTED_KEYS = {name: 1 for name in NAMES}

if __name__ == '__main__':
    for name in NAMES:
        validator.validate(ROOT / name)
