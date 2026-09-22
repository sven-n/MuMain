"""Install exactly the five owned BMDs and six owned textures in this isolated source worktree."""
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
DATA = REPOSITORY / 'src/bin/Data/Object1'
OWNERS = {'House01': ('House01.bmd', 'tile_house01.OZJ', 'tile_ston05.OZJ'),
          'House03': ('House03.bmd', 'tile_ston07.OZT', 'light_02.OZJ'),
          'House04': ('House04.bmd', 'tile_windows01.OZJ', 'tile_space01.OZJ'),
          'Tent01': ('Tent01.bmd',), 'HouseWall02': ('HouseWall02.bmd',)}


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
        for texture in (ROOT / owner / 'exports').iterdir():
            if texture.suffix not in ('.OZJ', '.OZT'):
                continue
            if texture.name in shared:
                assert shared[texture.name] == digest(texture), texture.name
            shared[texture.name] = digest(texture)
    assert (DATA / 'HouseEtc02.bmd').read_bytes() == (ROOT / 'HouseEtc02/original/HouseEtc02.bmd').read_bytes()
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
