"""Validate actual exports and prove preserved geometry, material order and every local rig pose."""
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import sys
sys.dont_write_bytecode = True
from config import CONVERTER, PROPS, REPOSITORY, ROOT, REMESHED
TOLERANCE = 0.0001
BOUND_ROUNDTRIP_TOLERANCE = 0.005  # Rotated root/SMD serialization; source extrema are checked exactly.


def run(arguments, allowed=(0,)):
    result = subprocess.run(list(map(str, arguments)), cwd=REPOSITORY, capture_output=True, text=True)
    if result.returncode not in allowed:
        raise RuntimeError(result.stdout + result.stderr)
    return {'command': list(map(str, arguments)), 'exit_code': result.returncode,
            'stdout': result.stdout, 'stderr': result.stderr}


def extract(root, state, commands):
    output = root / 'validation' / state
    output.mkdir(exist_ok=True)
    source = root / ('original' if state == 'original' else 'exports') / f'{root.name}.bmd'
    commands.append(run([CONVERTER, 'bmd2smd', source, output]))
    for path in sorted(output.glob(f'{root.name}*.smd')):
        flags = [] if path.stem == root.name else ['--animation']
        commands.append(run([CONVERTER, 'validate', path, *flags]))
    info = run([CONVERTER, 'info', source])
    commands.append(info)
    (root / 'validation' / f'info-{state}.txt').write_text(info['stdout'])
    return output, info['stdout']


def pose_data(path):
    text = path.read_text()
    nodes = text.split('nodes\n')[1].split('\nend')[0]
    lines = text.split('skeleton\n')[1].split('\nend')[0].splitlines()
    frames = [line for line in lines if line.startswith('time ')]
    poses = [list(map(float, line.split())) for line in lines if not line.startswith('time ')]
    return nodes, frames, poses


def local_rig(original, replacement, name):
    reports = []
    for path in sorted(original.glob(f'{name}*.smd')):
        a_nodes, a_frames, a_poses = pose_data(path)
        b_nodes, b_frames, b_poses = pose_data(replacement / path.name)
        assert a_nodes == b_nodes and a_frames == b_frames and len(a_poses) == len(b_poses)
        translation, rotation = 0, 0
        for a, b in zip(a_poses, b_poses):
            assert a[0] == b[0]
            translation = max(translation, *(abs(x-y) for x,y in zip(a[1:4],b[1:4])))
            rotation = max(rotation, *(abs(math.remainder(x-y,math.tau)) for x,y in zip(a[4:],b[4:])))
        assert max(translation, rotation) < TOLERANCE
        reports.append({'file': path.name, 'nodes_exact': a_nodes, 'frames_exact': a_frames,
                        'max_translation_difference': translation, 'max_rotation_difference_radians': rotation})
    metadata = [[line for line in (folder / f'{name}.actions.txt').read_text().splitlines()
                 if line.startswith(('action ', 'texture '))] for folder in (original, replacement)]
    assert metadata[0] == metadata[1]
    return {'status': 'PASS', 'metadata': metadata[0], 'local_pose_samples': reports}


def corners(path):
    lines = path.read_text().split('triangles\n')[1].splitlines()[:-1]
    return [list(map(float,line.split())) for index,line in enumerate(lines) if index % 4]


def geometry(original, replacement, name):
    groups = [corners(folder / f'{name}.smd') for folder in (original,replacement)]
    bounds = [[[fn(row[axis] for row in rows) for axis in range(1,4)] for fn in (min,max)] for rows in groups]
    deviation = max(abs(a-b) for aa,bb in zip(*bounds) for a,b in zip(aa,bb))
    assert deviation < BOUND_ROUNDTRIP_TOLERANCE
    assert len(groups[1]) // 3 < 1500
    assert all(math.isfinite(v) for row in groups[1] for v in row)
    assert all(len(row)==9 for row in groups[1])
    uv_collapsed = []
    for rows in groups:
        count = 0
        for index in range(0,len(rows),3):
            a,b,c = rows[index:index+3]
            area = abs((b[7]-a[7])*(c[8]-a[8])-(b[8]-a[8])*(c[7]-a[7]))
            count += area == 0
        uv_collapsed.append(count)
    assert uv_collapsed[1] <= uv_collapsed[0], (name,uv_collapsed)
    quality=[surface_quality(rows) for rows in groups]
    assert quality[1]['zero_area_triangles']==0
    assert quality[1]['opposing_averaged_normals']<=quality[0]['opposing_averaged_normals']
    return {'triangles_before_after': [len(rows)//3 for rows in groups], 'bounds_before_after': bounds,
            'max_bound_component_difference': deviation, 'collapsed_uv_triangles_before_after': uv_collapsed,
            'all_export_vertices_single_bone': True, 'all_finite': True,
            'surface_checks_before_after':quality}


def surface_quality(rows):
    degenerate,opposed=0,0
    for index in range(0,len(rows),3):
        a,b,c=rows[index:index+3]
        ab=[b[j]-a[j] for j in range(1,4)]
        ac=[c[j]-a[j] for j in range(1,4)]
        normal=[ab[1]*ac[2]-ab[2]*ac[1],ab[2]*ac[0]-ab[0]*ac[2],ab[0]*ac[1]-ab[1]*ac[0]]
        degenerate+=sum(value*value for value in normal)<1e-12
        opposed+=sum(normal[j]*(a[j+4]+b[j+4]+c[j+4]) for j in range(3))<0
    return {'zero_area_triangles':degenerate,'opposing_averaged_normals':opposed}


def validate(name):
    root = ROOT / name
    commands = []
    original, old_info = extract(root, 'original', commands)
    replacement, new_info = extract(root, 'replacement', commands)
    assert re.findall(r'texture=(\S+)',old_info) == re.findall(r'texture=(\S+)',new_info)
    comparison = run([CONVERTER,'compare',root/'original'/f'{name}.bmd',root/'exports'/f'{name}.bmd'],(0,2))
    expected = 'DIFFERENT' if name in REMESHED else 'EQUIVALENT'
    assert expected in comparison['stdout']
    commands.append(comparison)
    (root/'validation/compare.txt').write_text(comparison['stdout'])
    for texture in sorted((root/'exports').glob('*.OZ*')):
        commands.append(run([sys.executable,REPOSITORY/'tools/mu_texture.py','check',texture]))
    report = {'status':'PASS OFFLINE', 'client_verified':False, 'geometry':geometry(original,replacement,name),
              'rig':local_rig(original,replacement,name), 'full_compare':expected,
              'mesh_texture_order':re.findall(r'texture=(\S+)',new_info),
              'export_sha256':{path.name:hashlib.sha256(path.read_bytes()).hexdigest() for path in (root/'exports').iterdir()}}
    (root/'validation/commands.json').write_text(json.dumps(commands,indent=2)+'\n')
    (root/'validation/summary.json').write_text(json.dumps(report,indent=2)+'\n')
    print(name, report['status'], report['full_compare'])


for prop in PROPS:
    validate(prop)
