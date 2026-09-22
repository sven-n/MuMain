"""Package generated artwork with exact container casing and original translucency semantics."""
import json
import subprocess
import sys
sys.dont_write_bytecode = True
from pathlib import Path
import numpy as np
from PIL import Image
from config import CONTAINERS, ROOT, REPOSITORY
ALPHA_TEST = 64
SIZE = (512, 512)
def alpha_statistics(image):
    pixels = np.asarray(image)
    alpha = pixels[:, :, 3]
    return dict(size=list(image.size), alpha_min=int(alpha.min()), alpha_max=int(alpha.max()),
                visible_fraction=float(np.mean(alpha >= ALPHA_TEST)),
                fully_transparent_fraction=float(np.mean(alpha == 0)),
                alpha_test_used_for_report=ALPHA_TEST)


def bleed_transparent_rgb(image):
    """Visible RGB fills alpha-test-discarded texels; generated alpha stays identical."""
    pixels = np.asarray(image).copy()
    transparent = pixels[:, :, 3] < ALPHA_TEST
    known = ~transparent
    while not known.all():
        previous = known.copy()
        for axis, offset in ((0, 1), (0, -1), (1, 1), (1, -1)):
            neighbors = np.roll(previous, offset, axis=axis)
            edge = [slice(None), slice(None)]
            edge[axis] = 0 if offset > 0 else -1
            neighbors[tuple(edge)] = False
            fill = ~known & neighbors
            shifted = np.roll(pixels, offset, axis=axis)
            pixels[fill, :3] = shifted[fill, :3]
            known[fill] = True
    result = Image.fromarray(pixels)
    assert result.getchannel('A').tobytes() == image.getchannel('A').tobytes()
    return result



def package(name, material, container):
    folder = ROOT / 'textures/final'
    image = Image.open(ROOT / f'textures/raw/{name}.png').convert('RGBA').resize(SIZE, Image.Resampling.LANCZOS)
    old = Image.open(ROOT / 'textures/original' / material).convert('RGBA')
    if name == 'bottle':
        # Preserve the existing translucent glass opacity field; only RGB art is rebuilt.
        image.putalpha(old.getchannel('A').resize(SIZE, Image.Resampling.NEAREST))
    elif material.endswith('.tga'):
        assert image.getchannel('A').getextrema() == (0, 255)
        image = bleed_transparent_rgb(image)
    else:
        image = image.convert('RGB')
    image.save(folder / f'{name}.png')
    if material.endswith('.tga'):
        image.save(folder / material, compression=None, origin=0)
    else:
        image.save(folder / material, quality=96, subsampling=0)
    subprocess.run([sys.executable, str(REPOSITORY/'tools/mu_texture.py'), 'wrap',
                    str(folder/material), '--out', str(folder/container)], cwd=REPOSITORY, check=True)
    report = dict(original=alpha_statistics(old), final=alpha_statistics(image.convert('RGBA')),
                  processing='Generated RGB retained; encode only. Alpha-tested transparent RGB padded below cutoff64.',
                  bottle_alpha='Original 181..255 opacity field preserved exactly by nearest resampling' if name=='bottle' else None)
    (folder/f'{name}-audit.json').write_text(json.dumps(report,indent=2)+'\n')


def main():
    for material, container in CONTAINERS.items():
        package(Path(material).stem, material, container)
    result = subprocess.run([sys.executable, str(REPOSITORY/'tools/mu_texture.py'), 'check',
                             *[str(ROOT/'textures/final'/n) for n in CONTAINERS.values()]],
                            cwd=REPOSITORY, capture_output=True, text=True, check=True)
    (ROOT/'textures/final/texture-check.txt').write_text(result.stdout)


if __name__ == '__main__':
    main()
