"""Read MU .ozt images (4 junk bytes + 32-bit TGA) and write simple PNG sheets."""
import struct
import zlib
from pathlib import Path

import numpy as np


def read_ozt(path):
    """RGBA uint8 array, top row first."""
    data = Path(path).read_bytes()[4:]
    id_len, image_type = data[0], data[2]
    width, height, bpp, descriptor = struct.unpack_from("<HHBB", data, 12)
    if image_type != 2 or bpp not in (24, 32):
        raise ValueError(f"{path}: TGA type {image_type}, {bpp} bpp not handled")
    step = bpp // 8
    pixels = np.frombuffer(data, np.uint8, width * height * step, 18 + id_len).reshape(height, width, step)
    if not descriptor & 0x20:
        pixels = pixels[::-1]
    rgba = np.empty((height, width, 4), np.uint8)
    rgba[..., 0], rgba[..., 1], rgba[..., 2] = pixels[..., 2], pixels[..., 1], pixels[..., 0]
    rgba[..., 3] = pixels[..., 3] if step == 4 else 255
    return rgba


def write_ozt(path, rgba):
    """Write RGBA uint8 (top row first) as the client's .ozt files are: 4 junk bytes, an
    uncompressed 32-bit TGA stored bottom row first, and the TGA 2.0 footer."""
    height, width = rgba.shape[:2]
    header = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, width, height, 32, 8)
    bgra = np.ascontiguousarray(rgba[::-1][..., [2, 1, 0, 3]], np.uint8)
    footer = struct.pack("<II", 0, 0) + b"TRUEVISION-XFILE.\x00"
    Path(path).write_bytes(b"\x00\x00\x02\x00" + header + bgra.tobytes() + footer)


def write_png(path, rgb):
    rgb = np.ascontiguousarray(rgb, np.uint8)
    height, width = rgb.shape[:2]
    if rgb.ndim == 2:
        rgb = np.repeat(rgb[..., None], 3, axis=2)
    raw = b"".join(b"\x00" + rgb[y].tobytes() for y in range(height))
    png = b"\x89PNG\r\n\x1a\n"
    for tag, body in ((b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)),
                      (b"IDAT", zlib.compress(raw, 9)), (b"IEND", b"")):
        png += struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF)
    Path(path).write_bytes(png)


def over(rgba, background=(40, 44, 52)):
    alpha = rgba[..., 3:4].astype(np.float32) / 255.0
    return (rgba[..., :3] * alpha + np.array(background, np.float32) * (1 - alpha)).astype(np.uint8)


def sheet(images, columns, scale=1, gap=4, background=(40, 44, 52)):
    """Grid of RGB images."""
    cell_h = max(i.shape[0] for i in images) * scale + gap
    cell_w = max(i.shape[1] for i in images) * scale + gap
    rows = (len(images) + columns - 1) // columns
    out = np.empty((rows * cell_h + gap, columns * cell_w + gap, 3), np.uint8)
    out[:] = background
    for index, image in enumerate(images):
        image = np.kron(image, np.ones((scale, scale, 1), np.uint8)) if scale > 1 else image
        y = gap + (index // columns) * cell_h
        x = gap + (index % columns) * cell_w
        out[y:y + image.shape[0], x:x + image.shape[1]] = image
    return out
