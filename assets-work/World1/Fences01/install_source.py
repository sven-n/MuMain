"""Install only the five owned game replacements in this isolated source worktree."""
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
DATA = REPOSITORY / 'src/bin/Data/Object1'
NAMES = ('Fence01', 'Fence02', 'Fence03', 'Fence04')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    replacements = {DATA / f'{name}.bmd': ROOT / name / f'exports/{name}.bmd' for name in NAMES}
    replacements[DATA / 'joint.OZJ'] = ROOT / 'Fence02/exports/joint.OZJ'
    baseline = {path: digest(path) for group in ('World1', 'Object1') for path in (DATA.parent / group).iterdir() if path.is_file()}
    for name in NAMES:
        assert digest(DATA / f'{name}.bmd') in (digest(ROOT / name / f'original/{name}.bmd'), digest(replacements[DATA / f'{name}.bmd']))
    assert digest(DATA / 'tile_wood02.OZJ') == digest(ROOT / 'Fence01/original/tile_wood02.OZJ')
    assert len({digest(ROOT / name / 'exports/joint.OZJ') for name in NAMES[1:]}) == 1
    for destination, source in replacements.items():
        shutil.copy2(source, destination)
    assert all(digest(path) == value for path, value in baseline.items() if path not in replacements)
    report = dict(status='PASS; isolated source worktree only', runtime_written=False, client_verified=False,
                  unchanged_world1_object1_files=len(baseline) - len(replacements), frozen_shared_wood_unchanged=True,
                  installed={str(path.relative_to(REPOSITORY)): digest(path) for path in replacements})
    (ROOT / 'source-installation.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
