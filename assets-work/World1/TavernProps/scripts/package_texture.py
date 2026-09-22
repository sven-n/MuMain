"""Package the retained imagegen painting as editable atlas and RGB/OZJ exports."""

import io
import json
import shutil
import sys
from xml.etree.ElementTree import Element, SubElement, tostring
from zipfile import ZipFile, ZIP_DEFLATED, ZIP_STORED

from PIL import Image

sys.dont_write_bytecode = True
from config import PROPS, REPOSITORY, ROOT, TEXTURE
sys.path.insert(0, str(REPOSITORY / 'tools'))
import mu_texture

SIZE = 1024
REGIONS = {'Carved tabletop': (0, 0, 512, 512), 'Carved panel': (512, 0, 1024, 512),
           'Plain oak': (0, 512, 512, 1024), 'Forged iron': (512, 512, 1024, 1024)}


def png_bytes(image):
    output = io.BytesIO()
    image.save(output, format='PNG')
    return output.getvalue()


def editable_atlas(image, path):
    document = Element('image', w=str(SIZE), h=str(SIZE), name='Lorencia tavern diffuse')
    stack = SubElement(document, 'stack')
    with ZipFile(path, 'w', compression=ZIP_DEFLATED) as archive:
        archive.writestr('mimetype', 'image/openraster', compress_type=ZIP_STORED)
        for index, (name, box) in enumerate(REGIONS.items()):
            filename = f'data/layer{index}.png'
            archive.writestr(filename, png_bytes(image.crop(box)))
            SubElement(stack, 'layer', name=name, src=filename, x=str(box[0]), y=str(box[1]),
                       opacity='1.0', visibility='visible', **{'composite-op': 'svg:src-over'})
        archive.writestr('stack.xml', tostring(document))
        archive.writestr('mergedimage.png', png_bytes(image))


def main():
    texture_root = ROOT / 'Furniture03/textures'
    painting = texture_root / f'{TEXTURE}-generated.png'
    image = Image.open(painting).convert('RGB').resize((SIZE, SIZE), Image.Resampling.LANCZOS)
    image.save(texture_root / f'{TEXTURE}.png')
    image.save(texture_root / f'{TEXTURE}.jpg', quality=97, subsampling=0)
    editable_atlas(image, texture_root / f'{TEXTURE}.ora')
    prompt = (ROOT / 'generation-prompts.json').read_text()
    for name in PROPS:
        root = ROOT / name
        if name != 'Furniture03':
            for path in texture_root.iterdir():
                if path.is_file():
                    shutil.copy2(path, root / 'textures' / path.name)
        (root / 'textures/generation-prompt.json').write_text(prompt)
        source = root / 'textures' / f'{TEXTURE}.jpg'
        target = root / 'exports' / f'{TEXTURE}.OZJ'
        mu_texture.wrap_file(source, target)
        assert mu_texture.unwrap_bytes(target.read_bytes(), '.ozj') == source.read_bytes()
    print('Packaged identical 1024x1024 RGB atlas for all three consumers; lossless masters + ORA retained.')


if __name__ == '__main__':
    main()
