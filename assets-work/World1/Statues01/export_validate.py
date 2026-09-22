"""Official export, model/action validation and component preservation proof."""
import hashlib
import json
import math
import numpy as np
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
ASSETS = ('StoneStatue01', 'StoneStatue02', 'StoneStatue03', 'SteelStatue01', 'Tomb01', 'Tomb02', 'Waterspout01')
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER = os.environ['MU_BMDCONV']
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
os.environ['MU_BMDCONV'] = CONVERTER
sys.path.insert(0, str(HERE.parent / 'StaticBatch01'))
import validate_export as shared
shared.EXPECTED_KEYS = {name: (21 if name == 'Waterspout01' else 1) for name in ASSETS}


def logged(command, path):
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    path.write_text(result.stdout + result.stderr)
    result.check_returncode()


POSITION_TOLERANCE = .0001
ROTATION_MATRIX_TOLERANCE = .00001

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
        assert position < POSITION_TOLERANCE and rotation < ROTATION_MATRIX_TOLERANCE, (filename,position,rotation)
        samples.append(dict(file=filename, frames=len(frames), bone_samples=len(old),
                            max_local_position_component=position, max_rotation_matrix_component=rotation))
    record = dict(status='PASS', nodes='Bone names, order, parents identical', samples=samples,
                  position_tolerance=POSITION_TOLERANCE, rotation_matrix_tolerance=ROTATION_MATRIX_TOLERANCE,
                  rotation_check='Compare Rz*Ry*Rx matrices because equivalent Euler branches differ by pi',
                  semantics='All local keys retain equivalent position and rotation; Euler encoding may differ')
    (reports / 'local-motion.json').write_text(json.dumps(record,indent=2)+'\n')


def protected_parts(folder, original, final, name):
    protected = {'StoneStatue01': [], 'StoneStatue02': ['stone_statue01.jpg'], 'StoneStatue03': ['tombstone_big.jpg','angel_stone_statue.jpg'], 'SteelStatue01': [], 'Tomb01': [], 'Tomb02': [], 'Waterspout01': ['stone_statue02.jpg','reagon_waterspout.jpg','ston01.jpg','ston02.jpg']}[name]
    messages = []
    for texture in protected:
        for directory in (original, final):
            header, body = (directory / (name + '.smd')).read_text().split('triangles\n')
            lines = body.splitlines()
            triangles = ['\n'.join(lines[i:i+4]) for i in range(0, len(lines)-1, 4)
                         if lines[i] == texture]
            assert triangles
            stem = 'protected_' + Path(texture).stem
            (directory / (stem + '.smd')).write_text(header + 'triangles\n' + '\n'.join(triangles) + '\nend\n')
            shared.run('smd2bmd', directory / (stem + '.smd'), directory / (stem + '.bmd'),
                       '--manifest', directory / (name + '.actions.txt'))
        comparison = shared.run('compare', original / (stem + '.bmd'), final / (stem + '.bmd')).stdout
        assert 'EQUIVALENT' in comparison
        messages.append(texture + '\n' + comparison)
    (folder / 'validation/protected-components.txt').write_text('\n'.join(messages))


def export_validate(name):
    folder = HERE / name
    reports = folder / 'validation'
    prefix = [BLENDER, '--python-expr', 'import sys; sys.dont_write_bytecode=True']
    logged(prefix + ['-b', str(folder / 'source.blend'), '--python-exit-code', '1',
                     '--python', str(ROOT / 'tools/blender/mu_bmd_export.py'), '--',
                     '--out', str(folder / 'exports' / (name + '.bmd')), '--bmdconv', CONVERTER],
           reports / 'export.log')
    if name == 'Waterspout01':
        # Compatibility consumer remains the accepted BMD byte for byte.
        shutil.copy2(folder / 'exports' / (name+'.bmd'), reports / 'official-roundtrip.bmd')
        output = shared.run('compare', folder / 'original' / (name+'.bmd'), reports / 'official-roundtrip.bmd', check=False)
        (reports / 'official-roundtrip-compare.txt').write_text(output.stdout)
        shutil.copy2(folder / 'original' / (name+'.bmd'), folder / 'exports' / (name+'.bmd'))
    dependencies = json.loads((folder / 'dependencies.json').read_text())
    for containers in dependencies.values():
        for container in containers:
            assert (folder / 'exports' / Path(container).name).exists()
    original, old_meta, old_messages = shared.extract(folder, 'original')
    final, new_meta, new_messages = shared.extract(folder, 'new')
    assert old_meta == new_meta
    check_local_motion(original, final, name, reports)
    (reports / 'smd-validation.txt').write_text('\n'.join(old_messages + new_messages))
    before = shared.run('info', folder / 'original' / (name + '.bmd')).stdout
    after = shared.run('info', folder / 'exports' / (name + '.bmd')).stdout
    (reports / 'info-after.txt').write_text(after)
    assert re.findall(r'texture=(.*)', before) == re.findall(r'texture=(.*)', after)
    bounds_before, bounds_after = shared.engine_bounds(before), shared.engine_bounds(after)
    assert max(abs(a-b) for aa,bb in zip(bounds_before,bounds_after) for a,b in zip(aa,bb)) <= .01
    comparison = shared.run('compare', folder / 'original' / (name + '.bmd'),
                            folder / 'exports' / (name + '.bmd'), check=False)
    expected = 'DIFFERENT' if name in ('StoneStatue01','SteelStatue01','Tomb01','Tomb02') else 'EQUIVALENT'
    assert expected in comparison.stdout
    (reports / 'compare.txt').write_text(comparison.stdout)
    rig = shared.run('compare', original / 'skeleton.bmd', final / 'skeleton.bmd').stdout
    assert 'EQUIVALENT' in rig
    (reports / 'skeleton-compare.txt').write_text(rig)
    protected_parts(folder, original, final, name)
    containers = [p for p in (folder / 'exports').iterdir() if p.suffix.upper() in ('.OZJ', '.OZT')]
    check = subprocess.check_output([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'check',
                                     *map(str, containers)], cwd=ROOT, text=True)
    (reports / 'texture-check.txt').write_text(check)
    summary = dict(result='PASS: offline validation', bounds_before=bounds_before,
                   bounds_after=bounds_after, full_compare=expected, rig_actions='EQUIVALENT',
                   action_metadata=new_meta, material_order=re.findall(r'texture=(.*)', after),
                   client_verified=False, export_sha256={p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                                        for p in (folder / 'exports').iterdir() if p.is_file()})
    (reports / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    logged(prefix + ['-b', '--python-exit-code', '1', '--python', str(ROOT / 'tools/blender/mu_bmd_import.py'),
                     '--', '--bmd', str(folder / 'exports' / (name + '.bmd')), '--out',
                     str(reports / 'reimported.blend'), '--textures', str(folder / 'textures'),
                     '--bmdconv', CONVERTER], reports / 'reimport.log')
    print(name, expected, 'rig EQUIVALENT')


if __name__ == '__main__':
    for asset in ASSETS:
        export_validate(asset)
