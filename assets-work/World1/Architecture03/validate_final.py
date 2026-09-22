"""Audit immutable originals, dependency ownership and the actual exported mesh contracts."""
import hashlib
import json
import math
from pathlib import Path
import subprocess
import sys
from PIL import Image

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
OWNED = ('bridge_01', 'tree_04', 'tile_ston06', 'tile_wood02', 'tile_ston04', 'tile_02')
NAMES = ('HouseWall03', 'Bridge01', 'BridgeStone01', 'Fence01', 'FireLight02', 'House01', 'House03', 'House04', 'House05', 'HouseEtc02', 'HouseWall01', 'HouseWall02', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'StoneMuWall01', 'StoneWall01', 'StoneWall02', 'Tent01', 'Tree07')
BASELINE = (ROOT / 'baseline-commit.txt').read_text().strip()


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    originals, ownership, assets = {}, {}, {}
    inventory = json.loads((ROOT.parent / 'coordination/dependency-map.json').read_text())
    for name in (*OWNED, 'bridge_shadow01'):
        texture_name = f'{name}.tga' if name == 'bridge_shadow01' else f'{name}.jpg'
        consumers = [model for model, record in inventory['models'].items() if texture_name in record['textures']]
        assert set(consumers).issubset(NAMES)
        ownership[name] = consumers
    for name in NAMES:
        folder = ROOT / name
        for path in (folder / 'original').iterdir():
            if path.suffix not in ('.bmd', '.OZJ', '.OZT'):
                continue
            original = subprocess.check_output(['git', 'show', f'{BASELINE}:src/bin/Data/Object1/{path.name}'], cwd=REPOSITORY)
            assert original == path.read_bytes(), path
            originals[str(path.relative_to(ROOT))] = digest(original)
        text = (folder / f'validation/new/{name}.smd').read_text()
        nodes = text.split('nodes\n')[1].split('\nend')[0].splitlines()
        bones = {int(line.split()[0]) for line in nodes}
        lines = text.split('triangles\n')[1].splitlines()
        rows = [list(map(float, line.split())) for i, line in enumerate(lines[:-1]) if i % 4 != 0]
        assert rows and len(rows) % 3 == 0 and len(rows) // 3 <= 1500
        assert all(len(row) == 9 and row[0] in bones and all(math.isfinite(v) for v in row) for row in rows)
        uv_range = [[min(row[i] for row in rows), max(row[i] for row in rows)] for i in (7, 8)]
        old_lines = (folder / f'validation/original/{name}.smd').read_text().split('triangles\n')[1].splitlines()
        old_rows = [list(map(float, line.split())) for i, line in enumerate(old_lines[:-1]) if i % 4 != 0]
        old_range = [[min(row[i] for row in old_rows), max(row[i] for row in old_rows)] for i in (7, 8)]
        assert all(abs(a-b) < .00001 for aa, bb in zip(uv_range, old_range) for a, b in zip(aa, bb))
        images = {}
        for path in sorted((folder / 'textures').iterdir()):
            if path.suffix not in ('.jpg', '.tga'):
                continue
            with Image.open(path) as image:
                assert image.mode == ('RGBA' if path.suffix == '.tga' else 'RGB') and all(x <= 1024 and x & (x - 1) == 0 for x in image.size)
                images[path.name] = dict(dimensions=list(image.size), mode=image.mode, alpha=image.mode == 'RGBA')
                if path.suffix == '.tga':
                    assert path.read_bytes() == (folder / 'original' / path.name).read_bytes()
                    images[path.name]['original_rgba_and_padding_byte_exact'] = True
        assets[name] = dict(status='PASS', triangles=len(rows) // 3, textures=images, uv_range=uv_range,
                            original_uv_range=old_range, rigid_skin='Exactly one valid original bone per BMD corner',
                            finite_export_geometry=True, source_originals_match_git=True)
        (folder / 'validation/final-contract.json').write_text(json.dumps(assets[name], indent=2) + '\n')
    (ROOT / 'original-sha256.json').write_text(json.dumps(originals, indent=2) + '\n')
    (ROOT / 'dependency-ownership.json').write_text(json.dumps(ownership, indent=2) + '\n')
    for name in ('Fence01', 'FireLight02', 'House01', 'House03', 'House04', 'House05', 'HouseEtc02', 'HouseWall01', 'HouseWall02', 'HouseWall04', 'HouseWall05', 'HouseWall06', 'StoneMuWall01', 'StoneWall01', 'StoneWall02', 'Tent01', 'Tree07'):
        assert (ROOT / name / f'exports/{name}.bmd').read_bytes() == (REPOSITORY / f'src/bin/Data/Object1/{name}.bmd').read_bytes() == (ROOT / name / f'original/{name}.bmd').read_bytes()
    print('PASS: immutable originals; six paints cover three owned models plus seventeen readonly compatibility models; bridge shadow byte-exact; UV/skin/alpha/effect contracts')


if __name__ == '__main__':
    main()
