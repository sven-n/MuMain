"""Read-only independent checks of the completed tavern dependency group.

Run from the review worktree. All generated evidence stays beside this script.
"""

import hashlib
import json
import math
from pathlib import Path
import re
import runpy
import struct
import subprocess
import sys

OUTPUT = Path(__file__).resolve().parent
REVIEW_REPO = OUTPUT.parents[3]
ART_REPO = REVIEW_REPO.parent / 'MuMain-tavern-props'
ASSETS = ART_REPO / 'assets-work/World1/TavernProps'
CONVERTER = REVIEW_REPO.parent / 'MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
NAMES = ('Furniture03', 'Furniture04', 'Furniture05')
POSITION_TOLERANCE = 0.000001
TEXTURE_PREFIX_SIZE = 24


def command(arguments, log, allowed=(0,)):
    result = subprocess.run(list(map(str, arguments)), cwd=REVIEW_REPO,
                            text=True, capture_output=True)
    log.append({'command': list(map(str, arguments)), 'exit_code': result.returncode,
                'stdout': result.stdout, 'stderr': result.stderr})
    if result.returncode not in allowed:
        raise RuntimeError(result.stdout + result.stderr)
    return result.stdout


def convert_and_validate(model, destination, log):
    destination.mkdir(parents=True, exist_ok=True)
    command([CONVERTER, 'bmd2smd', model, destination], log)
    for smd in sorted(destination.glob('*.smd')):
        if smd.stem == 'skeleton':
            continue
        flags = [] if smd.stem == model.stem else ['--animation']
        command([CONVERTER, 'validate', smd, *flags], log)
    text = (destination / f'{model.stem}.smd').read_text()
    skeleton = destination / 'skeleton.smd'
    skeleton.write_text(text.split('triangles\n')[0] + 'triangles\nend\n')
    command([CONVERTER, 'smd2bmd', skeleton, destination / 'skeleton.bmd',
             '--manifest', destination / f'{model.stem}.actions.txt'], log)
    return text


def vertices(smd):
    lines = smd.split('triangles\n')[1].splitlines()[:-1]
    return [list(map(float, line.split())) for index, line in enumerate(lines) if index % 4]


def geometry_checks(original, replacement, name):
    old, new = vertices(original), vertices(replacement)
    points = [[row[1:4] for row in rows] for rows in (old, new)]
    bounds = [[[fn(point[axis] for point in group) for axis in range(3)]
               for fn in (min, max)] for group in points]
    assert bounds[0] == bounds[1], (name, bounds)
    assert all(row[0] == 0 for row in new)
    assert all(len(row) == 9 for row in new)
    assert all(math.isfinite(number) for row in new for number in row)
    assert all(0 <= number <= 1 for row in new for number in row[7:9])
    min_area, min_facing = float('inf'), float('inf')
    for index in range(0, len(new), 3):
        a, b, c = new[index:index + 3]
        ab = [b[axis] - a[axis] for axis in range(1, 4)]
        ac = [c[axis] - a[axis] for axis in range(1, 4)]
        normal = [ab[1]*ac[2] - ab[2]*ac[1], ab[2]*ac[0] - ab[0]*ac[2], ab[0]*ac[1] - ab[1]*ac[0]]
        assert sum(value*value for value in normal) > 0
        facing = sum(normal[axis] * (a[axis + 4] + b[axis + 4] + c[axis + 4]) for axis in range(3))
        area = abs((b[7] - a[7])*(c[8] - a[8]) - (b[8] - a[8])*(c[7] - a[7])) / 2
        assert area > 0 and facing > 0
        min_area, min_facing = min(min_area, area), min(min_facing, facing)
    return {'triangles': len(new) // 3, 'bounds_before_after': bounds,
            'all_vertices_bind_root_0': True, 'all_uvs_in_unit_square': True,
            'minimum_uv_triangle_area': min_area, 'minimum_winding_normal_dot': min_facing,
            'connection_points': joining_points(points, name)}


def joining_points(points, name):
    if name == 'Furniture03':
        return {'applicable': False}
    old, new = points
    axis = 1 if name == 'Furniture04' else 0
    planes = [max(point[axis] for point in old)]
    if name == 'Furniture05':
        planes.append(min(point[axis] for point in old))
    anchors = sorted(set(tuple(point) for point in old
                         if min(abs(point[axis] - edge) for edge in planes) < POSITION_TOLERANCE))
    deviations = [min(max(abs(a - b) for a, b in zip(anchor, point)) for point in new) for anchor in anchors]
    assert max(deviations) == 0
    return {'applicable': True, 'axis': axis, 'planes': planes,
            'unique_original_anchors': anchors, 'maximum_coordinate_deviation': max(deviations)}


def compare_rigs(original, replacement, name):
    assert pose_sections(original) == pose_sections(replacement)
    old_folder, new_folder = (OUTPUT / name / label for label in ('original', 'replacement'))
    old_files = sorted(path.name for path in old_folder.glob(f'{name}_a*.smd'))
    new_files = sorted(path.name for path in new_folder.glob(f'{name}_a*.smd'))
    assert old_files == new_files
    manifests = [(folder / f'{name}.actions.txt').read_text().splitlines()
                 for folder in (old_folder, new_folder)]
    metadata = [[line for line in lines if line.startswith(('action ', 'texture '))] for lines in manifests]
    assert metadata[0] == metadata[1]
    for filename in old_files:
        assert pose_sections((old_folder / filename).read_text()) == pose_sections((new_folder / filename).read_text())
    return {'nodes_bind_pose': 'identical nodes and numeric bind poses', 'actions': old_files,
            'action_smds': 'identical numeric frame poses, including negative-zero normalization',
            'action_texture_metadata': metadata[0], 'client_verified': False}


def pose_sections(text):
    nodes = text.split('nodes\n')[1].split('\nend')[0]
    skeleton = text.split('skeleton\n')[1].split('\nend')[0].splitlines()
    frames = [line for line in skeleton if line.startswith('time ')]
    poses = [list(map(float, line.split())) for line in skeleton if not line.startswith('time ')]
    return nodes, frames, poses


def inspect_asset(name):
    log = []
    root = ASSETS / name
    old_bmd = root / 'original' / f'{name}.bmd'
    new_bmd = root / 'exports' / f'{name}.bmd'
    assert old_bmd.read_bytes() == (REVIEW_REPO / 'src/bin/Data/Object1' / f'{name}.bmd').read_bytes()
    original = convert_and_validate(old_bmd, OUTPUT / name / 'original', log)
    replacement = convert_and_validate(new_bmd, OUTPUT / name / 'replacement', log)
    comparison = command([CONVERTER, 'compare', old_bmd, new_bmd], log, (0, 2))
    assert 'DIFFERENT' in comparison
    for label in ('original', 'replacement'):
        model = old_bmd if label == 'original' else new_bmd
        command([CONVERTER, 'info', model], log)
    skeleton = command([CONVERTER, 'compare', OUTPUT / name / 'original/skeleton.bmd',
                        OUTPUT / name / 'replacement/skeleton.bmd'], log)
    assert 'EQUIVALENT' in skeleton
    command([sys.executable, REVIEW_REPO / 'tools/mu_texture.py', 'check', root / 'exports/desk_big.OZJ'], log)
    report = geometry_checks(original, replacement, name)
    report.update(compare_rigs(original, replacement, name))
    report['full_model_compare'] = comparison
    report['skeleton_compare'] = skeleton
    for filename in (f'{name}.bmd', 'desk_big.OZJ'):
        assert (root / 'exports' / filename).read_bytes() == (ART_REPO / 'src/bin/Data/Object1' / filename).read_bytes()
    texture = (root / 'exports/desk_big.OZJ').read_bytes()
    assert texture[TEXTURE_PREFIX_SIZE:] == (root / 'textures/desk_big.jpg').read_bytes()
    report['export_sha256'] = {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                               for path in (root / 'exports').iterdir()}
    (OUTPUT / name / 'commands.json').write_text(json.dumps(log, indent=2) + '\n')
    return report


def inspect_consumers():
    log, mapping = [], {}
    models = sorted((ART_REPO / 'src/bin/Data/Object1').glob('*.bmd'))
    for model in models:
        result = command([CONVERTER, 'info', model], log)
        for texture in re.findall(r'texture=(\S+)', result):
            mapping.setdefault(texture, []).append(model.name)
    assert mapping['desk_big.jpg'] == [f'{name}.bmd' for name in NAMES]
    (OUTPUT / 'texture-consumers.json').write_text(json.dumps(mapping, indent=2) + '\n')
    (OUTPUT / 'object1-info.json').write_text(json.dumps(log, indent=2) + '\n')
    return {'model_count': len(models), 'desk_big.jpg': mapping['desk_big.jpg']}


def inspect_placements():
    inventory = runpy.run_path(str(REVIEW_REPO / 'assets-work/World1/inventory.py'))
    encrypted = (REVIEW_REPO / 'src/bin/Data/World1/EncTerrain1.obj').read_bytes()
    data = inventory['decode_map'](encrypted)
    count = struct.unpack_from('<h', data, 2)[0]
    records = {name: [] for name in NAMES}
    type_names = {142: 'Furniture03', 143: 'Furniture04', 144: 'Furniture05'}
    for index in range(count):
        kind, *values = struct.unpack_from('<h7f', data, 4 + index * inventory['OBJECT_RECORD_BYTES'])
        if kind not in type_names:
            continue
        records[type_names[kind]].append({'position': values[:3], 'rotation': values[3:6],
                                          'scale': values[6], 'tile': [values[0] / 100, values[1] / 100]})
    for name, placements in records.items():
        assert placements == json.loads((ASSETS / name / 'original/placements.json').read_text())
    (OUTPUT / 'placements.json').write_text(json.dumps(records, indent=2) + '\n')
    return {name: len(placements) for name, placements in records.items()}


def main():
    report = {'reviewed_commits': ['fe69aa12', '45c82ea0'], 'scope': 'offline only',
              'assets': {name: inspect_asset(name) for name in NAMES},
              'dependencies': inspect_consumers(), 'placement_counts': inspect_placements()}
    (OUTPUT / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
