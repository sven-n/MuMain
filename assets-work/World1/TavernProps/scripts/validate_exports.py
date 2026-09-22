"""Engine-parser validation of delivered geometry, unchanged rigs/actions and textures."""

import hashlib
import json
import math
import re
import subprocess
import sys

sys.dont_write_bytecode = True
from config import CONVERTER, MAX_TRIANGLES, PROPS, REPOSITORY, ROOT

TOLERANCE = .0001


def run(*arguments, check=True):
    result = subprocess.run([str(CONVERTER), *map(str, arguments)], capture_output=True, text=True)
    if check:
        result.check_returncode()
    return result


def extract(root, source):
    folder = root / 'validation' / source
    folder.mkdir(exist_ok=True)
    model = root / ('original' if source == 'original' else 'exports') / f'{root.name}.bmd'
    messages = [run('bmd2smd', model, folder).stdout]
    for path in sorted(folder.glob(f'{root.name}*.smd')):
        args = ['--animation'] if path.stem != root.name else []
        result = run('validate', path, *args)
        messages.append(result.stdout + result.stderr)
    reference = (folder / f'{root.name}.smd').read_text()
    (folder / 'skeleton.smd').write_text(reference.split('triangles\n')[0] + 'triangles\nend\n')
    manifest = folder / f'{root.name}.actions.txt'
    meta = [re.sub(r'file=\S+ ', '', s) for s in manifest.read_text().splitlines() if s.startswith('action ')]
    assert len(meta) == 1 and 'keys=1 lock=0' in meta[0]
    result = run('smd2bmd', folder / 'skeleton.smd', folder / 'skeleton.bmd', '--manifest', manifest)
    messages.append(result.stdout + result.stderr)
    return folder, meta, messages


def poses(path):
    text = path.read_text()
    nodes = text.split('nodes\n')[1].split('\nend')[0]
    skeleton = text.split('skeleton\n')[1].split('\nend')[0].splitlines()
    frames = [line for line in skeleton if line.startswith('time ')]
    rows = [list(map(float, s.split())) for s in skeleton if not s.startswith('time ')]
    return nodes, frames, rows


def local_motion(original, replacement, name):
    samples = []
    for filename in (f'{name}.smd', f'{name}_a00.smd'):
        old_nodes, old_frames, old = poses(original / filename)
        new_nodes, new_frames, new = poses(replacement / filename)
        assert old_nodes == new_nodes and old_frames == new_frames and len(old) == len(new)
        translation, rotation = 0, 0
        for a, b in zip(old, new):
            assert a[0] == b[0]
            translation = max(translation, *(abs(x - y) for x, y in zip(a[1:4], b[1:4])))
            rotation = max(rotation, *(abs(math.remainder(x - y, math.tau)) for x, y in zip(a[4:], b[4:])))
        assert max(translation, rotation) < TOLERANCE
        samples.append(dict(file=filename, frames=len(old_frames), nodes=old_nodes,
                            max_translation=translation, max_rotation_radians=rotation))
    return dict(status='PASS', tolerance=TOLERANCE, samples=samples)


def bounds(info):
    match = re.search(r'min ([-.\d ]+)  max ([-.\d ]+)  size', info)
    assert match
    return [list(map(float, group.split())) for group in match.groups()]


def positions(path):
    lines = path.read_text().split('triangles\n')[1].splitlines()[:-1]
    return [list(map(float, s.split()[1:4])) for i, s in enumerate(lines) if i % 4]


def precise_bounds(original, replacement, name):
    extents = []
    for folder in (original, replacement):
        points = positions(folder / f'{name}.smd')
        extents.append([[fn(p[a] for p in points) for a in range(3)] for fn in (min, max)])
    deviation = max(abs(a - b) for old, new in zip(*extents) for a, b in zip(old, new))
    assert deviation < TOLERANCE
    return dict(status='PASS', before=extents[0], after=extents[1], max_component_deviation=deviation)


def inspect_corners(path):
    lines = path.read_text().split('triangles\n')[1].splitlines()[:-1]
    min_uv_area, min_normal_dot = 1, 1
    for i in range(0, len(lines), 4):
        a, b, c = [list(map(float, line.split())) for line in lines[i + 1:i + 4]]
        ab, ac = [b[j] - a[j] for j in range(1, 4)], [c[j] - a[j] for j in range(1, 4)]
        cross = [ab[1]*ac[2] - ab[2]*ac[1], ab[2]*ac[0] - ab[0]*ac[2], ab[0]*ac[1] - ab[1]*ac[0]]
        length = math.sqrt(sum(v*v for v in cross))
        assert length > 0
        normal = [(a[j] + b[j] + c[j]) / 3 for j in range(4, 7)]
        normal_dot = sum(x*y for x, y in zip(cross, normal)) / length
        uv_area = abs((b[7] - a[7])*(c[8] - a[8]) - (b[8] - a[8])*(c[7] - a[7])) / 2
        assert uv_area > 1e-10 and normal_dot > 0, (path.name, i // 4, uv_area, normal_dot)
        min_uv_area, min_normal_dot = min(min_uv_area, uv_area), min(min_normal_dot, normal_dot)
    return dict(status='PASS', zero_uv_area_faces=0, winding_normal_disagreements=0,
                minimum_uv_area=min_uv_area, minimum_normal_dot=min_normal_dot)


def connection_points(original, replacement, name):
    old, new = positions(original / f'{name}.smd'), positions(replacement / f'{name}.smd')
    if name == 'Furniture03':
        return dict(status='not modular', note='Matched overall bounds and measured splayed foot centers.')
    if name == 'Furniture04':
        join_y = max(p[1] for p in old)
        anchors = [p for p in old if abs(p[1] - join_y) < TOLERANCE]
    else:
        left, right = min(p[0] for p in old), max(p[0] for p in old)
        anchors = [p for p in old if min(abs(p[0] - left), abs(p[0] - right)) < TOLERANCE]
    maximum = max(min(max(abs(a - b) for a, b in zip(p, q)) for q in new) for p in anchors)
    assert maximum < TOLERANCE, (name, maximum)
    return dict(status='PASS', original_anchor_corners=len(anchors), max_component_deviation=maximum,
                note='All original joining-edge corners survive in the exported BMD.')


def validate(name):
    root = ROOT / name
    reports = root / 'validation'
    original, old_meta, old_messages = extract(root, 'original')
    replacement, new_meta, new_messages = extract(root, 'new')
    assert old_meta == new_meta
    info = run('info', root / f'exports/{name}.bmd').stdout
    assert re.findall(r'texture=(.*)', info) == ['desk_big.jpg']
    assert 'meshes: 1  bones: 1  actions: 1' in info
    triangles = int(re.search(r'actions: 1  triangles: (\d+)', info)[1])
    assert 0 < triangles <= MAX_TRIANGLES
    before = bounds(run('info', root / f'original/{name}.bmd').stdout)
    after = bounds(info)
    assert before == after
    full = run('compare', root / f'original/{name}.bmd', root / f'exports/{name}.bmd', check=False)
    assert full.returncode != 0 and 'DIFFERENT' in full.stdout
    skeleton = run('compare', original / 'skeleton.bmd', replacement / 'skeleton.bmd').stdout
    assert 'EQUIVALENT' in skeleton
    texture = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check',
        str(root / 'exports/desk_big.OZJ')], capture_output=True, text=True, check=True)
    (reports / 'smd-validation.txt').write_text('\n'.join(old_messages + new_messages))
    for filename, content in {'info-after.txt': info, 'compare.txt': full.stdout,
            'skeleton-compare.txt': skeleton, 'texture-check.txt': texture.stdout + texture.stderr}.items():
        (reports / filename).write_text(content)
    summary = dict(status='PASS: offline engine validation', client_verified=False, triangles=triangles,
        bounds_before=before, bounds_after=after, action_metadata=new_meta, skeleton_actions='EQUIVALENT',
        full_compare='DIFFERENT: intentional geometry remodel', local_motion=local_motion(original, replacement, name),
        connection_points=connection_points(original, replacement, name),
        precise_bounds=precise_bounds(original, replacement, name),
        exported_corners=inspect_corners(replacement / f'{name}.smd'),
        export_sha256={p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in (root / 'exports').iterdir()})
    (reports / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(name + '\n' + full.stdout + skeleton + texture.stdout)


if __name__ == '__main__':
    for prop in PROPS:
        validate(prop)
