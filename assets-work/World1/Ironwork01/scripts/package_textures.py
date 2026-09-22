"""Package painted color with the preserved legacy alpha field; never synthesize alpha."""
import io
import json
from pathlib import Path
import subprocess
import sys
from xml.etree.ElementTree import Element, SubElement, tostring
from zipfile import ZipFile
from PIL import Image
sys.dont_write_bytecode = True
from config import DIMENSION, REPOSITORY, ROOT, TEXTURES, OPAQUE


def png_bytes(image):
    buffer = io.BytesIO()
    image.save(buffer, format='PNG')
    return buffer.getvalue()


def editable_source(name, diffuse, mask, final):
    document = Element('image', {'w': str(final.width), 'h': str(final.height), 'name': name})
    stack = SubElement(document, 'stack')
    layers = [('Painted color with preserved alpha', final, 'visible'),
              ('Original alpha field - bilinear 4x' if name != OPAQUE else 'Opaque alpha - no cutouts', mask.convert('RGBA'), 'hidden'),
              ('Unmasked generated color - editable', diffuse.convert('RGBA'), 'hidden')]
    with ZipFile(ROOT / 'textures' / f'{name}.ora', 'w') as archive:
        archive.writestr('mimetype', 'image/openraster')
        for index, (label, image, visibility) in enumerate(layers):
            source = f'data/layer{index}.png'
            SubElement(stack, 'layer', {'name': label, 'src': source, 'opacity': '1.0',
                       'visibility': visibility, 'composite-op': 'svg:src-over', 'x': '0', 'y': '0'})
            archive.writestr(source, png_bytes(image))
        archive.writestr('stack.xml', tostring(document))
        archive.writestr('mergedimage.png', png_bytes(final))


def package(name):
    if name == OPAQUE:
        return package_opaque()
    original = Image.open(ROOT / 'original' / f'{name}.tga').convert('RGBA')
    raw = Image.open(ROOT / 'textures' / f'{name}-generated.png').convert('RGB')
    color = raw.resize((DIMENSION // 2, DIMENSION), Image.Resampling.LANCZOS)
    alpha = original.getchannel('A').resize(color.size, Image.Resampling.BILINEAR)
    final = color.convert('RGBA')
    final.putalpha(alpha)
    color.save(ROOT / 'textures' / f'{name}-diffuse.png')
    alpha.save(ROOT / 'textures' / f'{name}-alpha.png')
    final.save(ROOT / 'textures' / f'{name}.png')
    final.save(ROOT / 'textures' / f'{name}.tga', compression=None)
    editable_source(name, color, alpha, final)
    export = ROOT / 'textures' / f'{name}.OZT'
    subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'wrap',
                    str(ROOT / 'textures' / f'{name}.tga'), '--out', str(export)], check=True)
    subprocess.run([sys.executable, str(REPOSITORY / 'tools/mu_texture.py'), 'check', str(export)], check=True)
    assert Image.open(ROOT / 'textures' / f'{name}.tga').tobytes() == final.tobytes()
    fringe = [rgb for rgb, value in zip(color.getdata(), alpha.getdata()) if value < 255]
    return {'size': list(final.size), 'mode': final.mode, 'original_size': list(original.size),
            'alpha': 'Exact original alpha resized with bilinear filtering; no new cutouts',
            'all_channels_unpremultiplied': True, 'transparent_rgb_black_pixels': sum(max(pixel) == 0 for pixel in fringe),
            'transparent_rgb_white_pixels': sum(min(pixel) == 255 for pixel in fringe),
            'fringe_color_source': 'Continuous forged iron painting across entire canvas, including hidden pixels',
            'alpha_mean_original': sum(original.getchannel('A').getdata()) / (original.width * original.height),
            'alpha_mean_final': sum(alpha.getdata()) / (final.width * final.height)}


def package_opaque():
    color = Image.open(ROOT / 'textures/steel_barred_b-generated.png').convert('RGB')
    color = color.resize((DIMENSION, DIMENSION), Image.Resampling.LANCZOS)
    color.save(ROOT / 'textures/steel_barred_b.png')
    color.save(ROOT / 'textures/steel_barred_b.jpg', quality=97, subsampling=0)
    editable_source(OPAQUE, color, Image.new('L',color.size,255), color.convert('RGBA'))
    subprocess.run([sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'wrap',
                    str(ROOT/'textures/steel_barred_b.jpg'),'--out',str(ROOT/'textures/steel_barred_b.OZJ')],check=True)
    subprocess.run([sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'check',
                    str(ROOT/'textures/steel_barred_b.OZJ')],check=True)
    return {'size':list(color.size),'mode':'RGB','alpha':'opaque','original_size':[32,32]}


if __name__ == '__main__':
    report = {name: package(name) for name in TEXTURES}
    (ROOT / 'textures/alpha-report.json').write_text(json.dumps(report, indent=2) + '\n')
