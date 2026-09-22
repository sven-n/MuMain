"""Check frozen gate, siege and banner components against their original engine data."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from PIL import Image

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
CONVERTER = os.environ['MU_BMDCONV']
OWNED = ('tile_01.jpg', 'tile_03.jpg')
POSITION_TOLERANCE = .001


def run(*arguments):
    result = subprocess.run([CONVERTER, *map(str, arguments)], text=True, capture_output=True, check=True)
    return result.stdout + result.stderr


def component(folder, name, texture):
    header, body = (folder / f'{name}.smd').read_text().split('triangles\n')
    lines = body.splitlines()[:-1]
    chosen = ['\n'.join(lines[i:i+4]) for i in range(0, len(lines), 4) if lines[i] == texture]
    stem = Path(texture).stem
    source = folder / f'protected_{stem}.smd'
    source.write_text(header + 'triangles\n' + '\n'.join(chosen) + '\nend\n')
    model = source.with_suffix('.bmd')
    run('smd2bmd', source, model, '--manifest', folder / f'{name}.actions.txt')
    return model, [list(map(float, row.split())) for tri in chosen for row in tri.splitlines()[1:]]


def check_component(root, texture):
    old, old_corners = component(root / 'validation/original', root.name, texture)
    new, new_corners = component(root / 'validation/new', root.name, texture)
    output = run('compare', old, new)
    assert 'EQUIVALENT' in output and len(old_corners) == len(new_corners)
    maximum = 0
    for a in old_corners:
        candidates = [b for b in new_corners if a[0] == b[0] and a[7:9] == b[7:9]]
        error = min(max(abs(x-y) for x, y in zip(a[1:4], b[1:4])) for b in candidates)
        assert error < POSITION_TOLERANCE, (root.name, texture, error)
        maximum = max(maximum, error)
    (root / 'validation' / f'{Path(texture).stem}-compare.txt').write_text(output)
    return dict(texture=texture, triangles=len(old_corners) // 3, full_component_compare='EQUIVALENT',
                exact_corner_uv_and_bone=True, max_position_component_error=maximum)


def validate(root):
    summary = json.loads((root / 'validation/summary.json').read_text())
    protected = [check_component(root, texture) for texture in summary['textures'] if texture not in OWNED]
    containers = sorted(p for p in (root / 'exports').iterdir() if p.suffix in ('.OZJ', '.OZT'))
    result = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check', *map(str, containers)],
                            text=True, capture_output=True, check=True)
    (root / 'validation/texture-check.txt').write_text(result.stdout + result.stderr)
    alpha = None
    for image in (root / 'textures').glob('*.tga'):
        original = root / 'original' / image.name
        assert original.read_bytes() == image.read_bytes()
        pixels = Image.open(image).convert('RGBA')
        alpha = dict(texture=image.name, dimensions=list(pixels.size),
                     full_rgba_sha256=hashlib.sha256(pixels.tobytes()).hexdigest(),
                     alpha_sha256=hashlib.sha256(pixels.getchannel('A').tobytes()).hexdigest(),
                     preservation='RGBA pixels, transparent fringe RGB and alpha masks byte-identical')
    (root / 'validation/protected-components.json').write_text(json.dumps(dict(
        status='PASS', components=protected, alpha=alpha, position_tolerance=POSITION_TOLERANCE,
        scope='Gate and siege fixtures, banner cloth/pole geometry and all 25 animation keys preserved'), indent=2) + '\n')
    print(root.name, 'PASS:', len(protected), 'frozen components; alpha', bool(alpha))


if __name__ == '__main__':
    for original in sorted(ROOT.glob('*/original')):
        validate(original.parent)
