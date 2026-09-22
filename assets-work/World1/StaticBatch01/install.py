"""Install only seven validated batch files; verify every other World1/Object1 baseline."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
DATA = REPOSITORY / 'src/bin/Data'
RUNTIME = Path(os.environ.get('MU_RUNTIME_DATA',
                             REPOSITORY / 'out/build/macos-arm64/src/Release/Main.app/Contents/MacOS/Data'))
FILES = {'Candle01': ('Candle01.bmd', 'candle.OZJ', 'candle2.OZJ'),
         'TreasureChest01': ('TreasureChest01.bmd', 'treasure_chest.OZJ'),
         'Tomb03': ('Tomb03.bmd', 'tombstone.OZJ')}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def verify_protected():
    baseline = json.loads((ROOT / 'protected-baseline.json').read_text())
    allowed = {'Object1/' + name for names in FILES.values() for name in names}
    assert len(allowed) == 7
    for data in (DATA, RUNTIME):
        for name, expected in baseline.items():
            if name in allowed:
                continue
            assert digest(data / name) == expected, f'Protected file differs: {data / name}'
    return len(baseline) - len(allowed)


def validated_files():
    files = []
    for prop, names in FILES.items():
        folder = ROOT.parent / prop
        report = json.loads((folder / 'validation/summary.json').read_text())
        assert report['status'] == 'PASS: offline engine validation'
        for name in names:
            export = folder / 'exports' / name
            expected = report['export_sha256'][name]
            assert digest(export) == expected, f'Validation is stale: {export}'
            for data in (DATA, RUNTIME):
                assert (data / 'Object1' / name).is_file(), f'New game filename forbidden: {name}'
            files.append((export, 'Object1/' + name, expected))
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    protected = verify_protected()
    files = validated_files()
    if not args.install:
        print(f'PASS: {len(files)} validated exports; {protected} protected files match in source/runtime.')
        return
    for export, name, expected in files:
        for data in (DATA, RUNTIME):
            shutil.copy2(export, data / name)
            assert digest(data / name) == expected
    assert verify_protected() == protected
    record = dict(status='Installed; offline verified; client acceptance pending', client_verified=False,
                  protected_files_per_data_folder=protected,
                  files=[dict(file=name, sha256=expected) for _, name, expected in files])
    (ROOT / 'installed-files.json').write_text(json.dumps(record, indent=2) + '\n')
    print(f'Installed {len(files)} files in both Data folders; {protected} protected files unchanged in each.')


if __name__ == '__main__':
    main()
