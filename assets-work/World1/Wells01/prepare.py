"""Preserve complete original well and pottery dependencies before production."""
import json
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
ASSETS = ('Well01','Well02','Well03','Well04')
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BASELINE = 'ac0f6dd8'


def immutable(path, payload):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        assert path.read_bytes() == payload, str(path)
        return
    path.write_bytes(payload)


def original(relative):
    return subprocess.check_output(['git', 'show', BASELINE + ':' + relative], cwd=ROOT)


def prepare(name, record):
    folder = HERE / name
    for directory in ('original', 'exports', 'review', 'validation'):
        (folder / directory).mkdir(parents=True, exist_ok=True)
    immutable(folder / 'original' / (name + '.bmd'), original(record['path']))
    for paths in record['textures'].values():
        for path in paths:
            immutable(HERE / 'textures/original' / Path(path).name, original(path))
    info = subprocess.check_output([CONVERTER, 'info', str(folder / 'original' / (name + '.bmd'))], text=True)
    immutable(folder / 'original/info.txt', info.encode())
    (folder / 'placements.json').write_text(json.dumps(record['placements'], indent=2) + '\n')
    (folder / 'dependencies.json').write_text(json.dumps(record['textures'], indent=2) + '\n')


def import_model(name):
    folder = HERE / name / 'original'
    if (folder / 'source.blend').exists():
        return
    command = [BLENDER, '-b', '--python-exit-code', '1', '--python-expr', 'import sys; sys.dont_write_bytecode=True',
               '--python', str(ROOT / 'tools/blender/mu_bmd_import.py'), '--',
               '--bmd', str(folder / (name + '.bmd')), '--out', str(folder / 'source.blend'),
               '--textures', str(HERE / 'textures/original'), '--bmdconv', CONVERTER]
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    (folder / 'import.log').write_text(result.stdout + result.stderr)
    result.check_returncode()


if __name__ == '__main__':
    data = json.loads((ROOT / 'assets-work/World1/coordination/dependency-map.json').read_text())
    for asset in ASSETS:
        prepare(asset, data['models'][asset])
    subprocess.run([sys.executable, str(ROOT / 'tools/mu_texture.py'), 'unwrap-dir',
                    str(HERE / 'textures/original'), str(HERE / 'textures/original')], cwd=ROOT, check=True)
    for asset in ASSETS:
        import_model(asset)
