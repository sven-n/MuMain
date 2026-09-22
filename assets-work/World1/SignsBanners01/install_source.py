"""Install exactly the four owned BMDs and six owned textures in this isolated source worktree."""
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
DATA = REPOSITORY / 'src/bin/Data/Object1'
OWNERS = {'Sign01': ('Sign01.bmd', 'notice.OZJ', 'signboard.OZT', 'doorknob.OZT'),
          'Sign02': ('Sign02.bmd',), 'Curtain01': ('Curtain01.bmd', 'badge_01.OZJ', 'badge_02.OZT'),
          'StoneWall04': ('StoneWall04.bmd', 'badge_03.OZT')}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    replacements = {DATA / name: ROOT / owner / 'exports' / name for owner, names in OWNERS.items() for name in names}
    baseline = {path: digest(path) for group in ('World1', 'Object1') for path in (DATA.parent / group).iterdir() if path.is_file()}
    report_path = ROOT / 'source-installation.json'
    previous = json.loads(report_path.read_text()).get('installed', {}) if report_path.exists() else {}
    shared = {}
    for owner, names in OWNERS.items():
        for name in names:
            relative = str((DATA / name).relative_to(REPOSITORY))
            allowed = (digest(ROOT / owner / 'original' / name), digest(ROOT / owner / 'exports' / name), previous.get(relative))
            assert digest(DATA / name) in allowed, name
        for texture in (p for p in (ROOT / owner / 'exports').iterdir() if p.suffix in ('.OZJ', '.OZT')):
            if texture.name in shared:
                assert shared[texture.name] == digest(texture), texture.name
            shared[texture.name] = digest(texture)
    for destination, source in replacements.items():
        shutil.copy2(source, destination)
    assert all(digest(path) == sha for path, sha in baseline.items() if path not in replacements)
    report = dict(status='PASS; isolated source worktree only', runtime_written=False, client_verified=False,
                  unchanged_world1_object1_files=len(baseline) - len(replacements),
                  installed={str(path.relative_to(REPOSITORY)): digest(path) for path in replacements},
                  shared_texture_exports_identical=shared)
    report_path.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
