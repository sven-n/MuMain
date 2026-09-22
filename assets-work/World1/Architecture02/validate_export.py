"""Validate six architecture exports and three unchanged accepted consumers against complete original rigs/actions."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
NAMES = ('House05', 'HouseWall01', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'Stair01', 'House04', 'HouseWall02', 'HouseEtc02')
KEYS = {'House05': 30, 'HouseWall01': 1, 'HouseWall04': 1, 'HouseWall05': 1, 'HouseWall06': 1, 'Stair01': 1, 'House04': 40, 'HouseWall02': 1, 'HouseEtc02': 1}
READONLY = ('House04', 'HouseWall02', 'HouseEtc02')
EFFECTS = {'House05': (2, 'ston02.jpg'), 'House04': (8, 'tile_space01.jpg'), 'HouseWall02': (4, 'light_02.jpg')}
spec = importlib.util.spec_from_file_location('engine_validator', ROOT.parent / 'StaticBatch01/validate_export.py')
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)
validator.EXPECTED_KEYS = KEYS


def compare_rig(root):
    old, old_meta, old_messages = validator.extract(root, 'original')
    new, new_meta, new_messages = validator.extract(root, 'new')
    assert old_meta == new_meta
    validator.check_local_motion(old, new, root.name, root / 'validation')
    (root / 'validation/smd-validation.txt').write_text('\n'.join(old_messages + new_messages))
    result = validator.run('compare', old / 'skeleton.bmd', new / 'skeleton.bmd').stdout
    assert 'EQUIVALENT' in result
    (root / 'validation/skeleton-compare.txt').write_text(result)
    return new_meta


def check_model(root):
    old, new = root / f'original/{root.name}.bmd', root / f'exports/{root.name}.bmd'
    before_info, after_info = validator.run('info', old).stdout, validator.run('info', new).stdout
    textures = re.findall(r'texture=(.*)', after_info)
    assert textures == re.findall(r'texture=(.*)', before_info), 'Material slot order changed'
    if root.name in EFFECTS:
        index, filename = EFFECTS[root.name]
        assert textures[index] == filename
    before, after = validator.engine_bounds(before_info), validator.engine_bounds(after_info)
    assert max(abs(a-b) for aa, bb in zip(before, after) for a, b in zip(aa, bb)) <= .02
    (root / 'validation/info-after.txt').write_text(after_info)
    comparison = validator.run('compare', old, new, check=False)
    expected = 'EQUIVALENT' if root.name in READONLY else 'DIFFERENT'
    assert expected in comparison.stdout
    assert (comparison.returncode == 0) == (expected == 'EQUIVALENT')
    if root.name in READONLY:
        assert old.read_bytes() == new.read_bytes(), 'Accepted compatibility BMD must be byte-identical'
    (root / 'validation/compare.txt').write_text(comparison.stdout)
    return dict(bounds_before=before, bounds_after=after, full_compare=expected, textures=textures)


def validate(name):
    root = ROOT / name
    metadata = compare_rig(root)
    summary = check_model(root)
    containers = sorted(p for p in (root / 'exports').iterdir() if p.suffix in ('.OZJ', '.OZT'))
    check = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check', *map(str, containers)],
                           capture_output=True, text=True, check=True)
    (root / 'validation/texture-check.txt').write_text(check.stdout)
    summary.update(status='PASS: offline engine validation', client_verified=False, action_metadata=metadata,
                   skeleton_actions='EQUIVALENT', readonly_compatibility=name in READONLY,
                   export_sha256={p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in (root / 'exports').iterdir() if p.is_file()})
    (root / 'validation/summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(name, summary['full_compare'], 'rig/action EQUIVALENT', 'all texture checks PASS')


if __name__ == '__main__':
    for name in NAMES:
        validate(name)
