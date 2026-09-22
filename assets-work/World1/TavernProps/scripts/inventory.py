"""Read-only inventory and immutable original preservation; run once before production."""

import hashlib
import json
import re
import shutil
import struct
import subprocess
import sys

sys.dont_write_bytecode = True
from config import CONVERTER, DATA, PROPS, REPOSITORY, ROOT, TEXTURE

sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

MAP_KEY = bytes.fromhex('d17352f6d29acb273eaf593137b3e7a2')
RECORD_BYTES = 30


def decode_map(data):
    result, rolling = bytearray(), 0x5E
    for index, value in enumerate(data):
        result.append(((value ^ MAP_KEY[index % len(MAP_KEY)]) - rolling) & 0xFF)
        rolling = (value + 0x3D) & 0xFF
    return result


def placements():
    payload = decode_map((DATA / 'World1/EncTerrain1.obj').read_bytes())
    count = struct.unpack_from('<h', payload, 2)[0]
    assert len(payload) == 4 + count * RECORD_BYTES
    result = {}
    for offset in range(4, len(payload), RECORD_BYTES):
        kind, *values = struct.unpack_from('<h7f', payload, offset)
        result.setdefault(kind, []).append(dict(position=values[:3], rotation=values[3:6],
            scale=values[6], tile=[v / 100 for v in values[:2]]))
    return result


def preserve(name, report, locations):
    root = ROOT / name
    for folder in ('original', 'textures', 'exports', 'review', 'validation'):
        (root / folder).mkdir(parents=True, exist_ok=True)
    original = root / 'original'
    for filename in (f'{name}.bmd', f'{TEXTURE}.OZJ'):
        shutil.copy2(DATA / 'Object1' / filename, original / filename)
    mu_texture.unwrap_file(original / f'{TEXTURE}.OZJ', original / f'{TEXTURE}.jpg')
    (original / 'info.txt').write_text(report)
    (original / 'placements.json').write_text(json.dumps(locations, indent=2) + '\n')
    subprocess.run([str(CONVERTER), 'bmd2smd', str(original / f'{name}.bmd'),
                    str(original / 'smd')], check=True, capture_output=True)


def main():
    target = ROOT / 'inventory.json'
    if target.exists():
        raise SystemExit('Preserved inventory exists; refusing to replace baseline.')
    reports, users = {}, {}
    for path in sorted((DATA / 'Object1').glob('*.bmd')):
        report = subprocess.check_output([str(CONVERTER), 'info', str(path)], text=True)
        reports[path.stem] = report.replace(str(REPOSITORY) + '/', '')
        for texture in re.findall(r'texture=(.*)', report):
            users.setdefault(texture.lower(), []).append(path.name)
    assert users[f'{TEXTURE}.jpg'] == [f'{name}.bmd' for name in PROPS]
    locations = placements()
    selected = {name: dict(type=kind, info=reports[name], placements=locations[kind])
                for name, kind in PROPS.items()}
    target.write_text(json.dumps(dict(baseline='9a8b2027', models=len(reports),
        selected=selected, texture_users=users, all_model_info=reports), indent=2) + '\n')
    hashes = {str(path.relative_to(DATA)): hashlib.sha256(path.read_bytes()).hexdigest()
              for folder in ('Object1', 'World1') for path in sorted((DATA / folder).iterdir())
              if path.is_file()}
    (ROOT / 'protected-baseline.json').write_text(json.dumps(hashes, indent=2) + '\n')
    for name, record in selected.items():
        preserve(name, record['info'], record['placements'])
        print(record['info'])


if __name__ == '__main__':
    main()
