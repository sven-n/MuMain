"""Install only the four claimed files into this worktree's source Data after validation."""

import argparse
import hashlib
import json
import shutil
import subprocess
import sys

sys.dont_write_bytecode = True
from config import CLAIMED, DATA, PROPS, REPOSITORY, ROOT


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def export_files():
    result = {}
    for name in PROPS:
        root = ROOT / name
        summary = json.loads((root / 'validation/summary.json').read_text())
        audit = json.loads((root / 'validation/source-audit.json').read_text())
        assert summary['status'] == 'PASS: offline engine validation' and audit['status'] == 'PASS'
        assert summary['skeleton_actions'] == 'EQUIVALENT'
        for filename, digest in summary['export_sha256'].items():
            path = root / 'exports' / filename
            assert sha256(path) == digest
            relative = 'Object1/' + filename
            if relative in result:
                assert sha256(result[relative]) == digest, 'Shared atlas diverged between consumers'
            result[relative] = path
    assert set(result) == CLAIMED
    return result


def protect_baseline(files):
    baseline = json.loads((ROOT / 'protected-baseline.json').read_text())
    for relative, digest in baseline.items():
        accepted = {digest}
        if relative in files:
            accepted.add(sha256(files[relative]))
        assert sha256(DATA / relative) in accepted, f'Unexpected source Data edit: {relative}'
    for name in PROPS:
        original = ROOT / name / 'original'
        for filename in (f'{name}.bmd', 'desk_big.OZJ'):
            assert sha256(original / filename) == baseline['Object1/' + filename]
    return len(baseline) - len(CLAIMED)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    branch = subprocess.check_output(['git', 'branch', '--show-current'], cwd=REPOSITORY, text=True).strip()
    assert branch == 'art/lorencia-tavern-props', branch
    files = export_files()
    protected_count = protect_baseline(files)
    if not args.install:
        print(f'Preflight PASS: four exports; {protected_count} protected files unchanged. Add --install to copy.')
        return
    for relative, source in files.items():
        shutil.copy2(source, DATA / relative)
        assert sha256(source) == sha256(DATA / relative)
    protect_baseline(files)
    report = dict(status='PASS', branch=branch, destination='this worktree src/bin/Data only',
        protected_unchanged_files=protected_count, runtime_written=False, client_launched=False,
        installed={relative: sha256(DATA / relative) for relative in sorted(files)})
    (ROOT / 'installed-files.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
