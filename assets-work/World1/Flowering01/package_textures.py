"""Encode generated alpha artwork without color grading or changing its silhouette."""
from pathlib import Path
import json
import subprocess
import sys
import shutil

sys.dont_write_bytecode = True

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
SIZES = {'tree_04':(512,512),'tree_05':(512,512),'tree_01':(512,256)}
ALPHA_TEST = 64


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


def package_alpha(name, size):
    raw = Image.open(ROOT / f'textures/raw/{name}.png').convert('RGBA')
    assert raw.getchannel('A').getextrema() == (0, 255), 'Imagegen must supply real alpha'
    image = bleed_transparent_rgb(raw.resize(size, Image.Resampling.LANCZOS))
    folder = ROOT / 'textures/final'
    image.save(folder / f'{name}.png')
    image.save(folder / f'{name}.tga', compression=None, origin=0)
    command = [sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'wrap',
               str(folder / f'{name}.tga'), '--out', str(folder / f'{name}.OZT')]
    subprocess.run(command, cwd=REPOSITORY, check=True)
    result = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check',
                             str(folder / f'{name}.OZT')], cwd=REPOSITORY, capture_output=True,
                            text=True, check=True)
    (folder / f'{name}-check.txt').write_text(result.stdout)
    original = Image.open(ROOT / f'textures/original/{name}.tga').convert('RGBA')
    report = dict(original=alpha_statistics(original), generated=alpha_statistics(raw),
                  final=alpha_statistics(image), fringe='RGB beneath alpha<64 padded from visible alpha>=64; alpha unchanged')
    (folder / f'{name}-alpha.json').write_text(json.dumps(report, indent=2) + '\n')
    print(name, report, flush=True)


def package_opaque(name, size):
    folder = ROOT / 'textures/final'
    raw = Image.open(ROOT / f'textures/raw/{name}.png').convert('RGB')
    image = raw.resize(size, Image.Resampling.LANCZOS)
    image.save(folder / f'{name}.png')
    image.save(folder / f'{name}.jpg', quality=96, subsampling=0)
    subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'wrap',
                    str(folder / f'{name}.jpg'), '--out', str(folder / f'{name}.OZJ')], cwd=REPOSITORY, check=True)
    result = subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check',
                             str(folder / f'{name}.OZJ')], cwd=REPOSITORY, capture_output=True, text=True, check=True)
    (folder / f'{name}-check.txt').write_text(result.stdout)
    print(name, size, 'opaque RGB', flush=True)


for texture, dimensions in SIZES.items():
    if texture in ('tree_04','tree_05'):
        package_alpha(texture, dimensions)
    else:
        package_opaque(texture, dimensions)
