"""Export preserved vegetation geometry and prove skeleton, actions, bounds and UV identity."""
from pathlib import Path
import hashlib
import json
import math
import os
import shutil
import subprocess
import sys

import numpy as np

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
ASSETS = ('Grass01', 'Grass02', 'Grass05', 'Grass06')
TEXTURES = {'Grass01': 'tree_08', 'Grass02': 'tree_08', 'Grass05': 'tree_09', 'Grass06': 'tree_09'}
os.environ['MU_BMDCONV'] = CONVERTER
sys.path.insert(0, str(ROOT.parent / 'StaticBatch01'))
import validate_export as shared
shared.EXPECTED_KEYS = {name: 1 for name in ASSETS}


def export(name):
    folder = ROOT / name
    command = [BLENDER, '-b', str(folder / 'source.blend'), '--python',
               str(REPOSITORY / 'tools/blender/mu_bmd_export.py'), '--', '--out',
               str(folder / f'exports/{name}.bmd'), '--bmdconv', CONVERTER]
    result = subprocess.run(command, cwd=REPOSITORY, capture_output=True, text=True)
    (folder / 'validation/export.log').write_text(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    shutil.copy2(ROOT / f'textures/final/{TEXTURES[name]}.OZT', folder / f'exports/{TEXTURES[name]}.OZT')


def rotation_matrix(euler):
    x, y, z = euler
    cx, cy, cz = math.cos(x), math.cos(y), math.cos(z)
    sx, sy, sz = math.sin(x), math.sin(y), math.sin(z)
    rx = np.array(((1,0,0),(0,cx,-sx),(0,sx,cx)))
    ry = np.array(((cy,0,sy),(0,1,0),(-sy,0,cy)))
    rz = np.array(((cz,-sz,0),(sz,cz,0),(0,0,1)))
    return rz @ ry @ rx


def check_local_motion(original, replacement, name, reports):
    samples = []
    for filename in (f'{name}.smd', f'{name}_a00.smd'):
        nodes, frames, old = shared.pose_rows(original / filename)
        new_nodes, new_frames, new = shared.pose_rows(replacement / filename)
        assert nodes == new_nodes and frames == new_frames and len(old) == len(new)
        position, rotation = 0, 0
        for before, after in zip(old, new):
            assert before[0] == after[0]
            position = max(position, *(abs(a-b) for a,b in zip(before[1:4], after[1:4])))
            delta = np.abs(rotation_matrix(before[4:]) - rotation_matrix(after[4:]))
            rotation = max(rotation, float(delta.max()))
        assert position < .0001 and rotation < .00001
        samples.append(dict(file=filename, frames=len(frames), bone_samples=len(old),
                            max_local_position_component=position, max_rotation_matrix_component=rotation))
    record = dict(status='PASS', nodes='Bone names, order, parents identical', samples=samples,
                  rotation_check='Compare Rz*Ry*Rx matrices because equivalent Euler branches differ by pi',
                  semantics='All local keys retain equivalent position and rotation; Euler encoding may differ')
    (reports / 'local-motion.json').write_text(json.dumps(record,indent=2)+'\n')


def validate(name):
    folder = ROOT / name
    original, old_meta, old_messages = shared.extract(folder, 'original')
    replacement, new_meta, new_messages = shared.extract(folder, 'new')
    assert old_meta == new_meta
    check_local_motion(original, replacement, name, folder / 'validation')
    reports = folder / 'validation'
    (reports / 'smd-validation.txt').write_text('\n'.join(old_messages + new_messages))
    comparison = shared.run('compare', folder / f'original/{name}.bmd', folder / f'exports/{name}.bmd')
    assert 'EQUIVALENT' in comparison.stdout
    (reports / 'compare.txt').write_text(comparison.stdout)
    skeleton = shared.run('compare', original / 'skeleton.bmd', replacement / 'skeleton.bmd')
    assert 'EQUIVALENT' in skeleton.stdout
    (reports / 'skeleton-compare.txt').write_text(skeleton.stdout)
    info = shared.run('info', folder / f'exports/{name}.bmd').stdout
    (reports / 'info-after.txt').write_text(info)
    before = shared.engine_bounds((folder / 'original/info.txt').read_text())
    after = shared.engine_bounds(info)
    assert max(abs(a-b) for aa,bb in zip(before,after) for a,b in zip(aa,bb)) <= .01
    assert f'texture={TEXTURES[name]}.tga' in info and 'meshes: 1' in info
    texture_check = (ROOT / f'textures/final/{TEXTURES[name]}-check.txt').read_text()
    (reports / 'texture-check.txt').write_text(texture_check)
    summary = dict(status='PASS: offline', client_verified=False, geometry='Original cards retained; full BMD comparison EQUIVALENT',
                   skeleton_actions='EQUIVALENT', action_metadata=new_meta, bounds_before=before,
                   bounds_after=after, material=f'{TEXTURES[name]}.tga', meshes=1,
                   exports={p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in (folder/'exports').iterdir()})
    (reports / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(name, comparison.stdout, flush=True)


for asset in ASSETS:
    export(asset)
    validate(asset)
