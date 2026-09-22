"""Read-only candidate inventory. Run before installing this batch, from the repo root."""

import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(ROOT.parent))
from inventory import decode_map

DATA = REPOSITORY / 'src/bin/Data'
CONVERTER = Path(os.environ.get('MU_BMDCONV',
                               REPOSITORY / 'out/build/macos-arm64/tools/bmdconv/Release/bmdconv'))
CANDIDATES = {
    'TreasureDrum01': 58, 'TreasureChest01': 59, 'Tomb03': 46,
    'Furniture03': 142, 'Furniture04': 143, 'Furniture05': 144,
    'Candle01': 150, 'Bonfire01': 52,
}
RECORD_BYTES = 30


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def placements_by_type():
    payload = decode_map((DATA / 'World1/EncTerrain1.obj').read_bytes())
    count = struct.unpack_from('<h', payload, 2)[0]
    assert len(payload) == 4 + RECORD_BYTES * count
    result = {}
    for offset in range(4, len(payload), RECORD_BYTES):
        kind, *values = struct.unpack_from('<h7f', payload, offset)
        record = dict(position=values[:3], rotation=values[3:6], scale=values[6],
                      tile=[value / 100 for value in values[:2]])
        result.setdefault(kind, []).append(record)
    return result


def model_inventory():
    reports, users = {}, {}
    for path in sorted((DATA / 'Object1').glob('*.bmd')):
        report = subprocess.check_output([str(CONVERTER), 'info', str(path)], text=True)
        reports[path.stem] = report.replace(str(REPOSITORY) + '/', '')
        for texture in re.findall(r'texture=(.*)', report):
            users.setdefault(texture.lower(), []).append(path.name)
    return reports, users


def main():
    target = ROOT / 'inventory.json'
    if target.exists():
        raise SystemExit('Stored baseline exists; refusing to overwrite it with installed assets.')
    reports, users = model_inventory()
    placements = placements_by_type()
    selected = {name: dict(type=kind, info=reports[name], placements=placements.get(kind, []),
                          textures={tex: users[tex.lower()] for tex in re.findall(r'texture=(.*)', reports[name])})
                for name, kind in CANDIDATES.items()}
    result = dict(baseline='8d22a912', model_count=len(reports), candidates=selected,
                  all_model_info=reports, texture_users=users)
    target.write_text(json.dumps(result, indent=2) + '\n')
    hashes = {str(path.relative_to(DATA)): sha256(path)
              for folder in ('World1', 'Object1') for path in sorted((DATA / folder).iterdir()) if path.is_file()}
    (ROOT / 'protected-baseline.json').write_text(json.dumps(hashes, indent=2) + '\n')
    print(f'Inventoried {len(reports)} Object1 models and {len(selected)} candidates.')


if __name__ == '__main__':
    main()
