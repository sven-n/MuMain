"""Package the imagegen painting as a diffuse texture; preserve frozen timber bytes."""
import json
from pathlib import Path
import shutil
import sys

sys.dont_write_bytecode = True
from PIL import Image

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

SIZE = (512, 512)


def main():
    master = ROOT / 'paintings/joint-master.png'
    image = Image.open(master).convert('RGB').resize(SIZE, Image.Resampling.LANCZOS)
    for name in ('Fence02', 'Fence03', 'Fence04'):
        folder = ROOT / name
        image.save(folder / 'textures/joint.png')
        image.save(folder / 'textures/joint.jpg', quality=95, subsampling=0)
        mu_texture.wrap_file(folder / 'textures/joint.jpg', folder / 'exports/joint.OZJ')
    shutil.copy2(ROOT / 'Fence01/original/tile_wood02.jpg', ROOT / 'Fence01/textures')
    shutil.copy2(ROOT / 'Fence01/original/tile_wood02.OZJ', ROOT / 'Fence01/exports')
    record = dict(tool='Built-in imagegen', prompt_file='prompt.txt', source='joint-master.png',
                  generated_source='/Users/webproduktion3/.codex/generated_images/01a0c96e-b7ee-72a1-9ad2-5f430b2130fb/exec-84010c0f-29a0-4277-9865-59daba614fc4.png',
                  processing='RGB, Lanczos resize to 512x512, JPEG quality 95 4:4:4, OZJ wrap',
                  texture='joint.jpg / joint.OZJ', alpha='opaque RGB; unchanged convention')
    (ROOT / 'paintings/generation.json').write_text(json.dumps(record, indent=2) + '\n')


if __name__ == '__main__':
    main()
