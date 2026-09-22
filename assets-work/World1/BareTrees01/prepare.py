"""Reproduce immutable archives from their original or accepted historical revision."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
from config import ASSETS, CONVERTER, FROZEN, REPOSITORY, ROOT


def archive(filename, revision, target):
    data = subprocess.check_output(['git', 'show', revision + ':src/bin/Data/Object1/' + filename],
                                   cwd=REPOSITORY)
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists():
        assert target.read_bytes() == data, 'Never overwrite a different original: ' + filename
    else:
        target.write_bytes(data)


def main():
    for name in ASSETS:
        folder = ROOT / name / 'original'
        revision = 'ac16ffeb' if name == 'Tree11' else 'ac0f6dd8'
        archive(name + '.bmd', revision, folder / (name + '.bmd'))
        for subfolder in ('exports', 'validation', 'review'):
            (ROOT / name / subfolder).mkdir(parents=True, exist_ok=True)
        info = subprocess.check_output([CONVERTER, 'info', str(folder / (name + '.bmd'))],
                                       cwd=REPOSITORY, text=True)
        (folder / 'info.txt').write_text(info)
    for filename in ('tree_03.OZJ', *FROZEN):
        revision = 'ac16ffeb' if filename == 'tree_06.OZT' else 'ac0f6dd8'
        target = ROOT / 'textures/original' / filename
        archive(filename, revision, target)
        subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'unwrap',
                        str(target)], cwd=REPOSITORY, check=True)
    for filename in ('tree_03.OZJ', 'tree_03.jpg', 'tree_06.OZT', 'tree_06.tga'):
        shutil.copy2(ROOT / 'textures/original' / filename, ROOT / 'Tree11/original' / filename)
    frozen = {}
    for filename in ('Tree11.bmd', *FROZEN):
        frozen[filename] = hashlib.sha256((REPOSITORY / 'src/bin/Data/Object1' / filename).read_bytes()).hexdigest()
    previous = ROOT / 'frozen-files.json'
    if previous.exists():
        assert json.loads(previous.read_text()) == frozen
    previous.write_text(json.dumps(frozen, indent=2) + '\n')


if __name__ == '__main__':
    main()
