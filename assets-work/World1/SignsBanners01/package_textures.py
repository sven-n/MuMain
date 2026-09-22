"""Package six imagegen paintings, preserving legacy scalar alpha and filtering fringe."""
from collections import deque
import io
import json
from pathlib import Path
import shutil
import sys
from xml.etree.ElementTree import Element, SubElement, tostring
from zipfile import ZipFile
from PIL import Image

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture
SPECS = {'notice': ('Sign01', '.jpg', (512, 512)),
         'signboard': ('Sign01', '.tga', (512, 256)),
         'doorknob': ('Sign01', '.tga', (128, 128)),
         'badge_01': ('Curtain01', '.jpg', (64, 512)),
         'badge_02': ('Curtain01', '.tga', (256, 512)),
         'badge_03': ('StoneWall04', '.tga', (512, 512))}


def png(image):
    stream = io.BytesIO()
    image.save(stream, format='PNG')
    return stream.getvalue()


def editable(name, color, alpha, final):
    document = Element('image', {'w': str(final.width), 'h': str(final.height), 'name': name})
    stack = SubElement(document, 'stack')
    layers = [('Final diffuse with original alpha', final.convert('RGBA'), 'visible'),
              ('Original scalar alpha resampled bilinearly', alpha.convert('RGBA'), 'hidden'),
              ('Generated color and filtering padding', color.convert('RGBA'), 'hidden')]
    with ZipFile(ROOT / 'paintings' / f'{name}.ora', 'w') as archive:
        archive.writestr('mimetype', 'image/openraster')
        for index, (label, image, visibility) in enumerate(layers):
            source = f'data/layer{index}.png'
            SubElement(stack, 'layer', {'name': label, 'src': source, 'opacity': '1.0',
                       'visibility': visibility, 'composite-op': 'svg:src-over', 'x': '0', 'y': '0'})
            archive.writestr(source, png(image))
        archive.writestr('stack.xml', tostring(document))
        archive.writestr('mergedimage.png', png(final))


def pad_fringe(color, alpha):
    """Extend nearest sufficiently opaque RGB into the fringe without changing alpha."""
    pixels, mask = list(color.getdata()), list(alpha.getdata())
    width, height = color.size
    seen = bytearray(value >= 128 for value in mask)
    queue = deque(index for index, valid in enumerate(seen) if valid)
    while queue:
        index = queue.popleft()
        x, y = index % width, index // width
        neighbors = []
        if x: neighbors.append(index-1)
        if x+1 < width: neighbors.append(index+1)
        if y: neighbors.append(index-width)
        if y+1 < height: neighbors.append(index+width)
        for neighbor in neighbors:
            if not seen[neighbor]:
                seen[neighbor] = 1
                pixels[neighbor] = pixels[index]
                queue.append(neighbor)
    result = Image.new('RGB', color.size)
    result.putdata(pixels)
    return result


def package(name, owner, extension, size):
    original = Image.open(ROOT / owner / 'original' / (name+extension))
    master = Image.open(ROOT / 'paintings' / (name+'-master.png')).convert('RGB')
    crop = None
    if name == 'badge_01':
        # The generator preserved the narrow atlas inside a wider white canvas.
        row = master.height // 2
        columns = [x for x in range(master.width) if min(master.getpixel((x, row))) < 235]
        crop = (min(columns), 0, max(columns)+1, master.height)
        master = master.crop(crop)
    color = master.resize(size, Image.Resampling.LANCZOS)
    alpha = Image.new('L', size, 255)
    if extension == '.tga':
        alpha = original.getchannel('A').resize(size, Image.Resampling.BILINEAR)
        color = pad_fringe(color, alpha)
    final = color.convert('RGBA' if extension == '.tga' else 'RGB')
    if extension == '.tga': final.putalpha(alpha)
    final.save(ROOT / 'paintings' / (name+'-final.png'))
    alpha.save(ROOT / 'paintings' / (name+'-alpha.png'))
    editable(name, color, alpha, final)
    for directory in ROOT.glob('*/original'):
        if not (directory / (name+extension)).exists(): continue
        folder = directory.parent
        target = folder / 'textures' / (name+extension)
        if extension == '.jpg': final.save(target, quality=96, subsampling=0)
        else: final.save(target, compression=None)
        container = folder / 'exports' / (name+('.OZJ' if extension == '.jpg' else '.OZT'))
        mu_texture.wrap_file(target, container)
    return dict(original_size=original.size, final_size=size, mode=final.mode,
                generated_master=f'{name}-master.png', prompt=f'{name}-prompt.txt', atlas_crop=crop,
                alpha='Exact bilinear original scalar field' if extension == '.tga' else 'Opaque RGB',
                fringe='Nearest opaque RGB extended into alpha<128; alpha itself unchanged',
                alpha_coverage_128=sum(v>=128 for v in alpha.getdata())/(size[0]*size[1]))


if __name__ == '__main__':
    records = {name: package(name, *spec) for name, spec in SPECS.items()}
    for original in ROOT.glob('*/original'):
        for path in original.iterdir():
            if path.suffix not in ('.OZJ', '.OZT') or path.stem in SPECS: continue
            shutil.copy2(path, original.parent / 'exports')
            suffix = '.jpg' if path.suffix == '.OZJ' else '.tga'
            shutil.copy2(original / (path.stem+suffix), original.parent / 'textures')
    (ROOT / 'paintings/packaging.json').write_text(json.dumps(records, indent=2)+'\n')
