"""Derive and compare every hierarchical SMD bone matrix and posed bounds."""
import json
from pathlib import Path
import shlex
import sys

sys.dont_write_bytecode = True
from mathutils import Euler, Matrix, Vector

ROOT = Path(__file__).resolve().parent
MATRIX_TOLERANCE = .0001
BOUNDS_TOLERANCE = .0002


def parse_smd(path):
    text = path.read_text()
    nodes = [shlex.split(line) for line in text.split('nodes\n')[1].split('\nend')[0].splitlines()]
    parents = {int(row[0]): int(row[2]) for row in nodes}
    frames = {}
    for line in text.split('skeleton\n')[1].split('\nend')[0].splitlines():
        values = line.split()
        if values[0] == 'time':
            frame = int(values[1])
            frames[frame] = {}
        else:
            transform = list(map(float, values[1:]))
            frames[frame][int(values[0])] = Matrix.Translation(Vector(transform[:3])) @ Euler(transform[3:], 'XYZ').to_matrix().to_4x4()
    return parents, frames


def world_matrices(parents, local):
    world = {}
    for index, matrix in local.items():
        parent = parents[index]
        world[index] = matrix if parent < 0 else world[parent] @ matrix
    return world


def mesh_points(path):
    rows = path.read_text().split('triangles\n')[1].splitlines()
    return [(int(values[0]), Vector(tuple(map(float, values[1:4]))))
            for i, row in enumerate(rows[:-1]) if i % 4 != 0 and (values := row.split())]


def posed_bounds(points, bind, world):
    deform = {index: world[index] @ bind[index].inverted() for index in bind}
    transformed = [deform[index] @ point for index, point in points]
    return [[min(point[i] for point in transformed) for i in range(3)],
            [max(point[i] for point in transformed) for i in range(3)]]


def compare_asset(folder):
    name = folder.name
    old_path, new_path = folder / f'validation/original/{name}.smd', folder / f'validation/new/{name}.smd'
    parents, old_bind = parse_smd(old_path)
    new_parents, new_bind = parse_smd(new_path)
    assert parents == new_parents
    old_world_bind = world_matrices(parents, old_bind[0])
    new_world_bind = world_matrices(parents, new_bind[0])
    old_points, new_points = mesh_points(old_path), mesh_points(new_path)
    _, old_frames = parse_smd(old_path.with_name(f'{name}_a00.smd'))
    _, new_frames = parse_smd(new_path.with_name(f'{name}_a00.smd'))
    assert old_frames.keys() == new_frames.keys()
    results, maximum, bound_delta = [], 0, 0
    for frame in old_frames:
        old_world, new_world = world_matrices(parents, old_frames[frame]), world_matrices(parents, new_frames[frame])
        delta = max(abs(a-b) for bone in parents for row_a, row_b in zip(old_world[bone], new_world[bone]) for a, b in zip(row_a, row_b))
        before, after = posed_bounds(old_points, old_world_bind, old_world), posed_bounds(new_points, new_world_bind, new_world)
        difference = max(abs(a-b) for aa, bb in zip(before, after) for a, b in zip(aa, bb))
        maximum, bound_delta = max(maximum, delta), max(bound_delta, difference)
        results.append(dict(frame=frame, matrix_component_delta=delta, bounds_before=before, bounds_after=after))
    assert maximum < MATRIX_TOLERANCE and bound_delta < BOUNDS_TOLERANCE, (name, maximum, bound_delta)
    report = dict(status='PASS', method='SMD local translation times XYZ Euler rotation, composed through original parents; bind-relative deformation of every exported triangle corner',
                  bones=len(parents), frames=len(old_frames), action_matrix_samples=len(parents)*len(old_frames),
                  max_world_matrix_component_delta=maximum, max_posed_bounds_component_delta=bound_delta, frames_checked=results)
    (folder / 'validation/matrices-and-posed-bounds.json').write_text(json.dumps(report, indent=2) + '\n')
    print(name, 'matrix and posed-bounds PASS', maximum, bound_delta)


if __name__ == '__main__':
    for original in sorted(ROOT.glob('*/original')):
        compare_asset(original.parent)
