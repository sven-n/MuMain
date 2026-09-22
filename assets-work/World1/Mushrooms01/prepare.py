"""Archive and import the two exclusively owned mushroom clusters."""
import json
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
BASELINE = 'ac0f6dd8'
ASSETS = ('Grass07', 'Grass08')
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'


def archive():
    inventory = json.loads((ROOT / 'assets-work/World1/coordination/dependency-map.json').read_text())
    original = HERE / 'textures/original'
    original.mkdir(parents=True, exist_ok=True)
    payload = subprocess.check_output(['git', 'show', BASELINE + ':src/bin/Data/Object1/mushroom.OZJ'], cwd=ROOT)
    if (original / 'mushroom.OZJ').exists():
        assert (original / 'mushroom.OZJ').read_bytes() == payload
    (original / 'mushroom.OZJ').write_bytes(payload)
    sys.path.insert(0, str(ROOT / 'tools'))
    import mu_texture
    (original / 'mushroom.jpg').write_bytes(mu_texture.unwrap_bytes(payload, '.ozj'))
    for name in ASSETS:
        folder = HERE / name
        for directory in ('original', 'exports', 'review', 'validation'):
            (folder / directory).mkdir(parents=True, exist_ok=True)
        game = 'src/bin/Data/Object1/' + name + '.bmd'
        (folder / 'original' / (name + '.bmd')).write_bytes(subprocess.check_output(['git', 'show', BASELINE + ':' + game], cwd=ROOT))
        report = subprocess.check_output([CONVERTER, 'info', str(folder / 'original' / (name + '.bmd'))], text=True)
        (folder / 'original/info.txt').write_text(report)
        (folder / 'placements.json').write_text(json.dumps(inventory['models'][name]['placements'], indent=2) + '\n')


def import_originals():
    for name in ASSETS:
        folder = HERE / name / 'original'
        command = [BLENDER, '--python-expr', 'import sys; sys.dont_write_bytecode = True', '-b', '--python', str(ROOT / 'tools/blender/mu_bmd_import.py'), '--',
                   '--bmd', str(folder / (name + '.bmd')), '--out', str(folder / 'source.blend'),
                   '--textures', str(HERE / 'textures/original'), '--bmdconv', CONVERTER]
        result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
        (folder / 'import.log').write_text(result.stdout + result.stderr)
        result.check_returncode()


if __name__ == '__main__':
    archive()
    import_originals()
