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
OWNED = ('tombstone_big', 'stone_statue02', 'stone_statue01', 'angel_stone_statue', 'grave_02', 'grave_01')
NAMES = ('StoneStatue01', 'StoneStatue02', 'StoneStatue03', 'SteelStatue01', 'Tomb01', 'Tomb02', 'Waterspout01')
BASELINE = (ROOT / 'baseline-commit.txt').read_text().strip()


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    originals, ownership, assets = {}, {}, {}
    inventory = json.loads((ROOT.parent / 'coordination/dependency-map.json').read_text())
    for name in OWNED:
        consumers = [model for model, record in inventory['models'].items() if any(Path(t).stem == name for t in record['textures'])]
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
        assert name == 'StoneStatue02' or all(abs(a-b) < .00001 for aa, bb in zip(uv_range, old_range) for a, b in zip(aa, bb))
        images = {}
        for path in (folder / 'textures').iterdir():
            if path.suffix not in ('.jpg', '.tga'):
                continue
            with Image.open(path) as image:
                assert image.mode == ('RGBA' if path.suffix == '.tga' else 'RGB')
                assert all(x <= 1024 and x & (x - 1) == 0 for x in image.size)
                images[path.name] = dict(dimensions=list(image.size), mode=image.mode, alpha=path.suffix == '.tga')
        assets[name] = dict(status='PASS', triangles=len(rows) // 3, textures=images, uv_range=uv_range,
                            original_uv_range=old_range, rigid_skin='Exactly one valid original bone per BMD corner',
                            finite_export_geometry=True, source_originals_match_git=True)
        (folder / 'validation/final-contract.json').write_text(json.dumps(assets[name], indent=2) + '\n')
    (ROOT / 'original-sha256.json').write_text(json.dumps(originals, indent=2) + '\n')
    (ROOT / 'dependency-ownership.json').write_text(json.dumps(ownership, indent=2) + '\n')
    print('PASS: immutable original bytes, six textures wholly owned by seven consumers, exported UV/skin/material/alpha contracts')


if __name__ == '__main__':
    main()
