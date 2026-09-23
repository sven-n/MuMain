"""Reproduce official imports, authored pottery, exports and strict audits."""
from pathlib import Path
import json
import hashlib
import os
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
CONVERTER = os.environ['MU_BMDCONV']
BLENDER = os.environ['MU_BLENDER']
NAMES = ('Object29', 'Object28')


def converter(*arguments):
    result = subprocess.run([CONVERTER, *map(str, arguments)], capture_output=True, check=True)
    return result.stdout.decode('utf8', 'backslashreplace')


def blender(log, *arguments):
    result = subprocess.run([BLENDER, '-b', '--python-exit-code', '1', *map(str, arguments)],
                            capture_output=True, cwd=REPOSITORY)
    log.write_bytes(result.stdout + result.stderr)
    assert result.returncode == 0, str(log)


def prepare():
    for name in ('Object28', 'Object29', 'Object30'):
        folder = ROOT / name
        for texture in (folder / 'baseline').glob('*.OZJ'):
            subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'unwrap', str(texture),
                            '--out', str(folder / 'textures' / (texture.stem + '.jpg'))], check=True)
        blender(folder / 'validation/import-baseline.txt', '--python', ROOT / 'official_io.py', '--', 'import', name, 'baseline')
        shutil.copy2(folder / 'baseline/source.blend', folder / 'original/source.blend')


def validate(name):
    from raw_audit import raw, check
    from audit_exports import inspect
    folder = ROOT / name
    target = folder / 'exports' / (name + '.bmd')
    report = folder / 'validation'
    (report / 'new').mkdir(exist_ok=True)
    converter('bmd2smd', target, report / 'new')
    for path in (report / 'new').glob('*.smd'):
        arguments = ['validate', path]
        if '_a' in path.stem:
            arguments.append('--animation')
        (report / (path.stem + '-validate.txt')).write_text(converter(*arguments))
    assert raw.payload(target)[:38] == raw.payload(folder / 'baseline' / (name + '.bmd'))[:38], 'Name, mesh, bone or action count changed'
    old_actions = [line for line in (report / 'baseline' / (name + '.actions.txt')).read_bytes().splitlines() if line.startswith(b'action ')]
    new_actions = [line for line in (report / 'new' / (name + '.actions.txt')).read_bytes().splitlines() if line.startswith(b'action ')]
    assert old_actions == new_actions, 'Action order/key count/lock changed'
    hashes = json.loads((ROOT / 'baseline-sha256.json').read_text())
    for texture in folder.joinpath('exports').glob('*.OZJ'):
        assert hashlib.sha256(texture.read_bytes()).hexdigest() == hashes[texture.name]
        assert texture.read_bytes() == (REPOSITORY / 'src/bin/Data/Object2' / texture.name).read_bytes()
        subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check', str(texture)], check=True)
    before = (report / 'baseline' / (name + '_a00.smd')).read_text()
    after = (report / 'new' / (name + '_a00.smd')).read_text()
    assert before.split('skeleton\n')[0] == after.split('skeleton\n')[0], 'Rig names/order/parents changed'
    def numeric_pose(text):
        lines = text.split('skeleton\n')[1].split('\nend')[0].splitlines()
        return [(line if line.startswith('time ') else list(map(float, line.split()))) for line in lines]
    assert numeric_pose(before) == numeric_pose(after), 'One-key rig/action values changed'
    (report / 'rig-actions.json').write_text(json.dumps(dict(status='PASS', criterion='Exact nodes, frames and numeric pose; signed zero equivalent'), indent=2))
    info = converter('info', target)
    (report / 'info-after.txt').write_text(info)
    assert [line.split('texture=')[1] for line in info.splitlines() if 'texture=' in line] == ['flower_vase.jpg', 'deep_wall04.jpg']
    comparison = subprocess.run([CONVERTER, 'compare', str(folder / 'baseline' / (name + '.bmd')), str(target)], capture_output=True)
    (report / 'compare.txt').write_bytes(comparison.stdout)
    inspect(name)
    assert check(folder, 'new')['status'] == 'PASS'
    (report / 'raw-bindings.json').write_text(json.dumps(raw.audit(target), indent=2))
    blender(report / 'import-export.txt', '--python', ROOT / 'official_io.py', '--', 'import', name, 'exports')


def main():
    if sys.argv[1] == 'prepare':
        prepare()
        return
    if sys.argv[1] == 'build':
        blender(ROOT / 'build-log.txt', '--python', ROOT / 'build_source.py')
    for name in NAMES:
        folder = ROOT / name
        blender(folder / 'validation/export.txt', folder / 'source.blend', '--python', ROOT / 'official_io.py', '--', 'export', name)
        validate(name)
    blender(ROOT / 'review-log.txt', '--python', ROOT / 'review.py')


if __name__ == '__main__':
    main()
