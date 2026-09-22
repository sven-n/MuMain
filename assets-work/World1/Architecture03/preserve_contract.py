"""Keep authored official mesh panels while restoring original protected corners and rig data.

Blender's export may slightly quantize normals and boundary coordinates.
The converter manifest path preserves exact poses and untouched mesh corners.
"""
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parent
CONVERTER = os.environ['MU_BMDCONV']
NAMES = ('HouseWall03', 'Bridge01', 'BridgeStone01')
POSITION_TOLERANCE = .01  # Identification only; matched corners are replaced with exact original data.
UV_TOLERANCE = .00001


def run(*args):
    result = subprocess.run([CONVERTER, *map(str, args)], capture_output=True, text=True, check=True)
    return result.stdout + result.stderr


def triangle_blocks(text):
    lines = text.split('triangles\n')[1].splitlines()
    return [lines[i:i+4] for i in range(0, len(lines)-1, 4)]


def same_triangle(a, b):
    if a[0] != b[0]:
        return False
    old = [list(map(float, line.split())) for line in a[1:]]
    new = [list(map(float, line.split())) for line in b[1:]]
    for shift in range(3):
        aligned = new[shift:] + new[:shift]
        if all(x[0] == y[0] and max(abs(v-w) for v, w in zip(x[1:4], y[1:4])) < POSITION_TOLERANCE
               and max(abs(v-w) for v, w in zip(x[7:9], y[7:9])) < UV_TOLERANCE for x, y in zip(old, aligned)):
            return True
    return False


def restore_panel_boundaries(new, available, original):
    old_corners = [line.split() for block in original for line in block[1:]]
    restored, maximum = 0, 0
    for triangle in available:
        for corner in range(1, 4):
            values = new[triangle][corner].split()
            candidates = [old for old in old_corners if old[0] == values[0]
                          and max(abs(float(old[i])-float(values[i])) for i in (1, 2, 3)) < POSITION_TOLERANCE
                          and max(abs(float(old[i])-float(values[i])) for i in (7, 8)) < UV_TOLERANCE]
            if not candidates:
                continue
            old = min(candidates, key=lambda old: sum((float(old[i])-float(values[i]))**2 for i in (1, 2, 3)))
            maximum = max(maximum, max(abs(float(old[i])-float(values[i])) for i in (1, 2, 3)))
            # Keep the authored bevel normal; only its immutable original anchor is restored.
            values[1:4], values[7:9] = old[1:4], old[7:9]
            new[triangle][corner] = ' '.join(values)
            restored += 1
    return dict(original_panel_boundary_corners=restored, maximum_boundary_correction=maximum)


def protected_mesh(folder, baseline, mesh):
    report = json.loads((folder / 'validation/blender.json').read_text())
    changed = {i for panel in report['panel_recesses'] for i in panel['original_face_pair']}
    new, available = triangle_blocks(mesh), set(range(len(triangle_blocks(mesh))))
    restored = 0
    for index, old in enumerate(triangle_blocks(baseline)):
        if index in changed:
            continue
        matches = [i for i in available if same_triangle(old, new[i])]
        assert matches, (folder.name, index, 'Protected original triangle lost in official export')
        target = matches[0]
        new[target] = old
        available.remove(target)
        restored += 1
    boundary = restore_panel_boundaries(new, available, triangle_blocks(baseline))
    (folder / 'validation/protected-packaging.json').write_text(json.dumps(dict(
        restored_original_triangles=restored, official_authored_triangles=len(available), panel_boundary_restoration=boundary,
        reason='Retain protected original positions, bone indices, UVs and normals despite Blender split-normal roundtrip quantization',
        original_bind_and_actions=True), indent=2) + '\n')
    return '\n'.join(line for block in new for line in block) + '\nend\n'


def preserve(name):
    folder = ROOT / name
    official = folder / 'validation/official-export'
    locked = folder / 'validation/locked-motion'
    official.mkdir(exist_ok=True)
    locked.mkdir(exist_ok=True)
    original = folder / 'original/smd'
    game = folder / f'exports/{name}.bmd'
    shutil.copy2(game, official / f'{name}.bmd')
    messages = [run('bmd2smd', game, official)]
    mesh = (official / f'{name}.smd').read_text()
    baseline = (original / f'{name}.smd').read_text()
    assert mesh.split('nodes\n')[1].split('\nend')[0] == baseline.split('nodes\n')[1].split('\nend')[0]
    # New panels come from official export. Untouched triangle corners remain the original data.
    final_mesh = protected_mesh(folder, baseline, mesh)
    (locked / f'{name}.smd').write_text(baseline.split('triangles\n')[0] + 'triangles\n' + final_mesh)
    for path in original.glob(f'{name}_a*.smd'):
        shutil.copy2(path, locked)
    shutil.copy2(original / f'{name}.actions.txt', locked)
    messages += [run('validate', locked / f'{name}.smd'), run('validate', locked / f'{name}_a00.smd', '--animation')]
    messages.append(run('smd2bmd', locked / f'{name}.smd', game, '--manifest', locked / f'{name}.actions.txt'))
    messages.append(run('compare', official / f'{name}.bmd', game))
    (folder / 'validation/contract-preservation.txt').write_text('\n'.join(messages))
    assert (locked / f'{name}_a00.smd').read_bytes() == (original / f'{name}_a00.smd').read_bytes()


if __name__ == '__main__':
    for name in NAMES:
        preserve(name)
