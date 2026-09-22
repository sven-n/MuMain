"""Preserve and label the original Object1 texture atlas inventory for shared-material review."""

import json
from pathlib import Path
import subprocess
import sys

from PIL import Image, ImageDraw

sys.dont_write_bytecode = True
HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import mu_texture

BASELINE = 'ac0f6dd8'
CELL = (190, 210)
COLUMNS = 7
ROWS = 5


def preserve(path):
    output = HERE / 'texture-baseline'
    output.mkdir(exist_ok=True)
    wrapped = output / Path(path).name
    data = subprocess.check_output(['git', 'show', BASELINE + ':' + path], cwd=ROOT)
    if wrapped.exists() and wrapped.read_bytes() != data:
        raise ValueError('Refusing changed texture baseline: ' + path)
    wrapped.write_bytes(data)
    suffix = '.jpg' if wrapped.suffix.lower() == '.ozj' else '.tga'
    decoded = wrapped.with_suffix(suffix)
    mu_texture.unwrap_file(wrapped, decoded)
    return decoded


def panel(path):
    source = Image.open(path).convert('RGBA')
    source.thumbnail((180, 180), Image.Resampling.NEAREST)
    target = Image.new('RGB', (180, 180), '#545955')
    x, y = (180 - source.width) // 2, (180 - source.height) // 2
    target.paste(source, (x, y), source)
    return target


def main():
    data = json.loads((HERE / 'dependency-map.json').read_text())
    records = [(path, preserve(path), len(users)) for path, users in sorted(data['texture_consumers'].items())]
    capacity = COLUMNS * ROWS
    for start in range(0, len(records), capacity):
        sheet = Image.new('RGB', (COLUMNS * CELL[0], ROWS * CELL[1] + 40), '#242b30')
        draw = ImageDraw.Draw(sheet)
        draw.text((12, 12), 'INTEGRATION BASELINE TEXTURES — original material roles / consumer counts', fill='white')
        for index, (path, decoded, consumers) in enumerate(records[start:start + capacity]):
            x, y = index % COLUMNS * CELL[0] + 5, index // COLUMNS * CELL[1] + 40
            sheet.paste(panel(decoded), (x, y))
            draw.text((x, y + 182), Path(path).name, fill='white')
            draw.text((x, y + 196), str(consumers) + ' model consumers', fill='#bbbbbb')
        sheet.save(HERE / f'texture-sheet-{start // capacity + 1:02}.jpg', quality=90)
    print(f'Preserved all {len(records)} texture containers and decoded sources.')


if __name__ == '__main__':
    main()
