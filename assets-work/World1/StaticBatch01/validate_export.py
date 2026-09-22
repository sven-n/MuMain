"""Validate a remodeled BMD, its full rig/actions, bounds, textures and special flame mesh."""

import argparse
import hashlib
import json
import os
import math
from pathlib import Path
import re
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
CONVERTER = Path(os.environ.get('MU_BMDCONV',
                               REPOSITORY / 'out/build/macos-arm64/tools/bmdconv/Release/bmdconv'))
PROPS = ('Candle01', 'TreasureChest01', 'Tomb03')
EXPECTED_TEXTURES = {'Candle01': ['candle.jpg', 'candle2.jpg'],
                     'TreasureChest01': ['treasure_chest.jpg'], 'Tomb03': ['tombstone.jpg']}
EXPECTED_KEYS = {'Candle01': 7, 'TreasureChest01': 7, 'Tomb03': 1}


def run(*arguments, check=True):
    result = subprocess.run([str(CONVERTER), *map(str, arguments)], capture_output=True, text=True)
    if check and result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return result


def manifest_meta(text):
    return [re.sub(r'file=\S+ ', '', line) for line in text.splitlines() if line.startswith('action ')]


def extract(root, source):
    name = root.name
    folder = root / 'validation' / source
    folder.mkdir(exist_ok=True)
    model = root / ('original' if source == 'original' else 'exports') / f'{name}.bmd'
    messages = [run('bmd2smd', model, folder).stdout]
    for path in sorted(folder.glob(f'{name}*.smd')):
        args = ['--animation'] if path.stem != name else []
        messages.append(run('validate', path, *args).stdout)
    text = (folder / f'{name}.smd').read_text()
    skeleton = text.split('triangles\n')[0] + 'triangles\nend\n'
    (folder / 'skeleton.smd').write_text(skeleton)
    manifest = folder / f'{name}.actions.txt'
    metadata = manifest_meta(manifest.read_text())
    assert len(metadata) == 1 and f'keys={EXPECTED_KEYS[name]} lock=0' in metadata[0]
    result = run('smd2bmd', folder / 'skeleton.smd', folder / 'skeleton.bmd', '--manifest', manifest)
    messages.append(result.stdout + result.stderr)
    return folder, metadata, messages


def check_flames(original, replacement, reports):
    original_uvs = None
    for folder in (original, replacement):
        reference = (folder / 'Candle01.smd').read_text()
        header, body = reference.split('triangles\n')
        lines = body.splitlines()
        triangles = ['\n'.join(lines[index:index + 4])
                     for index in range(0, len(lines) - 1, 4) if lines[index] == 'candle2.jpg']
        assert len(triangles) == 6
        corner_uvs = [list(map(float, line.split())) for triangle in triangles
                      for line in triangle.splitlines()[1:]]
        if original_uvs is None:
            original_uvs = corner_uvs
        else:
            for corner in original_uvs:
                matches = [v for v in corner_uvs if v[0] == corner[0]
                           and max(abs(a - b) for a, b in zip(v[1:4], corner[1:4])) < .0001]
                assert any(max(abs(a - b) for a, b in zip(v[7:9], corner[7:9])) < .00001
                           for v in matches), 'Flame UVs or rigid binding changed'
        (folder / 'flames.smd').write_text(header + 'triangles\n' + '\n'.join(triangles) + '\nend\n')
        run('smd2bmd', folder / 'flames.smd', folder / 'flames.bmd',
            '--manifest', folder / 'Candle01.actions.txt')
    comparison = run('compare', original / 'flames.bmd', replacement / 'flames.bmd').stdout
    assert 'EQUIVALENT' in comparison
    (reports / 'flame-compare.txt').write_text(comparison)
    (reports / 'flame-uv.txt').write_text('PASS: all 18 original triangle corners retain their bone, position and UV.\n')


def engine_bounds(info):
    match = re.search(r'min ([-.\d ]+)  max ([-.\d ]+)  size', info)
    assert match, 'Missing engine bind-pose bounds'
    return [list(map(float, group.split())) for group in match.groups()]


def pose_rows(path):
    text = path.read_text()
    nodes = text.split('nodes\n')[1].split('\nend')[0]
    skeleton = text.split('skeleton\n')[1].split('\nend')[0].splitlines()
    frames = [line for line in skeleton if line.startswith('time ')]
    rows = [list(map(float, line.split())) for line in skeleton if not line.startswith('time ')]
    return nodes, frames, rows


def check_local_motion(original, replacement, name, reports):
    deviations = []
    for filename in (f'{name}.smd', f'{name}_a00.smd'):
        old_nodes, old_frames, old = pose_rows(original / filename)
        new_nodes, new_frames, new = pose_rows(replacement / filename)
        assert old_nodes == new_nodes and old_frames == new_frames and len(old) == len(new)
        position, rotation = 0, 0
        for a, b in zip(old, new):
            assert a[0] == b[0]
            position = max(position, *(abs(x - y) for x, y in zip(a[1:4], b[1:4])))
            rotation = max(rotation, *(abs(math.remainder(x - y, math.tau)) for x, y in zip(a[4:], b[4:])))
        assert position < .0001 and rotation < .0001, (filename, position, rotation)
        deviations.append(dict(file=filename, bone_samples=len(old), frames=len(old_frames),
                               max_position_component=position, max_euler_component_radians=rotation))
    (reports / 'local-motion.json').write_text(json.dumps(dict(
        status='PASS', nodes='names, indices and parents identical', tolerance=.0001,
        rotation_note='Equivalent Euler components compared modulo 2*pi', samples=deviations), indent=2) + '\n')


def validate(root):
    reports = root / 'validation'
    original, old_meta, old_messages = extract(root, 'original')
    replacement, new_meta, new_messages = extract(root, 'new')
    assert old_meta == new_meta
    check_local_motion(original, replacement, root.name, reports)
    (reports / 'smd-validation.txt').write_text('\n'.join(old_messages + new_messages))
    old_model, model = root / f'original/{root.name}.bmd', root / f'exports/{root.name}.bmd'
    info = run('info', model).stdout
    assert re.findall(r'texture=(.*)', info) == EXPECTED_TEXTURES[root.name], 'Mesh order/texture mismatch'
    before, after = engine_bounds(run('info', old_model).stdout), engine_bounds(info)
    assert max(abs(a - b) for aa, bb in zip(before, after) for a, b in zip(aa, bb)) <= .02
    (reports / 'info-after.txt').write_text(info)
    remodel = run('compare', old_model, model, check=False)
    assert remodel.returncode != 0 and 'DIFFERENT' in remodel.stdout
    (reports / 'compare.txt').write_text(remodel.stdout)
    skeleton = run('compare', original / 'skeleton.bmd', replacement / 'skeleton.bmd').stdout
    assert 'EQUIVALENT' in skeleton
    (reports / 'skeleton-compare.txt').write_text(skeleton)
    if root.name == 'Candle01':
        check_flames(original, replacement, reports)
    texture_check = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check',
                                   *map(str, sorted((root / 'exports').glob('*.OZJ')))],
                                  capture_output=True, text=True, check=True)
    (reports / 'texture-check.txt').write_text(texture_check.stdout)
    summary = dict(status='PASS: offline engine validation', client_verified=False,
                   action_metadata=new_meta, bounds_before=before, bounds_after=after,
                   full_compare='DIFFERENT: intentional remodel', skeleton_actions='EQUIVALENT',
                   textures=EXPECTED_TEXTURES[root.name])
    summary['export_sha256'] = {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                               for p in sorted((root / 'exports').iterdir()) if p.is_file()}
    (reports / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(root.name + '\n' + remodel.stdout + skeleton + texture_check.stdout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prop', choices=PROPS)
    args = parser.parse_args()
    for prop in (args.prop,) if args.prop else PROPS:
        validate(ROOT.parent / prop)


if __name__ == '__main__':
    main()
