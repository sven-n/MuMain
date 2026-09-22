"""Install exactly three owned BMDs and six owned paintings in this isolated source worktree."""
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
DATA = REPOSITORY / 'src/bin/Data/Object1'
OWNERS = {'HouseWall03': ('HouseWall03.bmd', 'tile_wood02.OZJ'), 'Bridge01': ('Bridge01.bmd', 'bridge_01.OZJ'), 'BridgeStone01': ('BridgeStone01.bmd', 'tree_04.OZJ', 'tile_ston06.OZJ'), 'House01': ('tile_ston04.OZJ',), 'FireLight02': ('tile_02.OZJ',)}


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
    for original in ROOT.glob('*/original'):
        for texture in (original.parent / 'exports').iterdir():
            if texture.suffix not in ('.OZJ', '.OZT'):
                continue
            if texture.name in shared:
                assert shared[texture.name] == digest(texture), texture.name
            shared[texture.name] = digest(texture)
    for name in ('Fence01', 'FireLight02', 'House01', 'House03', 'House04', 'House05', 'HouseEtc02', 'HouseWall01', 'HouseWall02', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'StoneMuWall01', 'StoneWall01', 'StoneWall02', 'Tent01', 'Tree07'):
        assert (DATA / f'{name}.bmd').read_bytes() == (ROOT / name / f'original/{name}.bmd').read_bytes()
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
