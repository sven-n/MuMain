"""Install nine validated owned files only into this isolated source worktree."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
from prepare import ASSETS, HERE, ROOT

TEXTURES = ('light.OZT', 'light2.OZJ', 'fire_light_01.OZT', 'copra_gate.OZJ', 'fire_01.OZJ')


def sha(payload):
    return hashlib.sha256(payload).hexdigest()


def main():
    assert subprocess.check_output(['git', 'branch', '--show-current'], cwd=ROOT, text=True).strip() == 'codex/lorencia-fire-props'
    files = {ROOT / 'src/bin/Data/Object1' / (n + '.bmd'): HERE / n / 'exports' / (n + '.bmd') for n in ASSETS}
    files.update({ROOT / 'src/bin/Data/Object1' / n: HERE / 'textures/final' / n for n in TEXTURES})
    for name in ASSETS:
        for report in ('summary.json', 'source-export-audit.json'):
            assert json.loads((HERE / name / 'validation' / report).read_text())['result'].startswith('PASS')
    protected = {}
    for directory in ('Object1', 'World1'):
        for path in sorted((ROOT / 'src/bin/Data' / directory).iterdir()):
            if not path.is_file() or path in files:
                continue
            expected = subprocess.check_output(['git', 'show', 'HEAD:' + str(path.relative_to(ROOT))], cwd=ROOT)
            assert path.read_bytes() == expected, str(path)
            protected[str(path.relative_to(ROOT))] = sha(expected)
    for target, source in files.items():
        before = subprocess.check_output(['git', 'show', 'HEAD:' + str(target.relative_to(ROOT))], cwd=ROOT)
        assert target.read_bytes() in (before, source.read_bytes()), str(target)
        shutil.copy2(source, target)
    record = dict(result='PASS', baseline='f8928697', runtime_modified=False, client_verified=False,
                  installed={str(p.relative_to(ROOT)): dict(export=str(s.relative_to(ROOT)), sha256=sha(p.read_bytes()))
                             for p,s in files.items()}, protected_count=len(protected), protected=protected)
    (HERE / 'installed-files.json').write_text(json.dumps(record, indent=2) + '\n')


if __name__ == '__main__':
    main()
