"""Install only the three owned source-game files, proving all other files unchanged."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FILES = {'Grass07.bmd': 'Grass07/exports/Grass07.bmd',
         'Grass08.bmd': 'Grass08/exports/Grass08.bmd',
         'mushroom.OZJ': 'textures/final/mushroom.OZJ'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inventory():
    return {str(path.relative_to(ROOT)): digest(path)
            for directory in ('Object1', 'World1')
            for path in (ROOT / 'src/bin/Data' / directory).iterdir() if path.is_file()}


def main():
    before = inventory()
    changed = {}
    for name, exported in FILES.items():
        target = ROOT / 'src/bin/Data/Object1' / name
        relative = str(target.relative_to(ROOT))
        original = subprocess.check_output(['git', 'show', 'ac0f6dd8:' + relative], cwd=ROOT)
        desired = (HERE / exported).read_bytes()
        assert target.read_bytes() in (original, desired), 'Unowned existing edit: ' + relative
        changed[relative] = dict(export=exported, sha256=digest(HERE / exported))
    for name, exported in FILES.items():
        shutil.copy2(HERE / exported, ROOT / 'src/bin/Data/Object1' / name)
    after = inventory()
    assert all(after[path] == value for path, value in before.items() if path not in changed)
    (HERE / 'installed-files.json').write_text(json.dumps(dict(
        files=changed, protected={path: value for path, value in before.items() if path not in changed},
        client_verified=False, runtime_modified=False), indent=2) + '\n')


if __name__ == '__main__':
    main()
