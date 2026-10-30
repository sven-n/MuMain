#!/usr/bin/env python3
"""List the real pixel size of MU's .OZJ/.OZT textures.

RmlUi loads a texture at its original size, but the files don't say it anywhere a theme author
can read: this prints the real size of every texture in a folder, to write the theme's
@spritesheet rects with (see docs/rmlui-ui-system/theming-and-modding.md).

.OZJ is a JPEG behind a 24-byte header; .OZT is a TGA behind a 4-byte header.

Usage: oz_texture_sizes.py <folder> [--recursive]
Output: tab-separated  path  format  width  height  power_of_two
"""
import argparse
import pathlib
import struct
import sys


def jpeg_size(data: bytes):
    i = 2  # skip SOI
    while i + 9 < len(data):
        if data[i] != 0xFF:
            i += 1
            continue
        marker = data[i + 1]
        if marker in (0xD8, 0x01) or 0xD0 <= marker <= 0xD7:
            i += 2
            continue
        length = struct.unpack(">H", data[i + 2:i + 4])[0]
        if 0xC0 <= marker <= 0xCF and marker not in (0xC4, 0xC8, 0xCC):
            height, width = struct.unpack(">HH", data[i + 5:i + 9])
            return width, height
        i += 2 + length
    return None


def texture_size(path: pathlib.Path):
    data = path.read_bytes()
    suffix = path.suffix.lower()
    if suffix == ".ozj":
        start = data.find(b"\xff\xd8", 24)
        return ("ozj", jpeg_size(data[start:])) if start >= 0 else ("ozj", None)
    if suffix == ".ozt":
        if len(data) < 4 + 18:
            return "ozt", None
        width, height = struct.unpack("<HH", data[4 + 12:4 + 16])
        return "ozt", (width, height)
    return None, None


def is_power_of_two(value: int) -> bool:
    return value > 0 and value & (value - 1) == 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("folder", type=pathlib.Path)
    parser.add_argument("--recursive", action="store_true")
    args = parser.parse_args()

    files = args.folder.rglob("*") if args.recursive else args.folder.iterdir()
    print("path\tformat\twidth\theight\tpower_of_two")
    for path in sorted(p for p in files if p.is_file()):
        fmt, size = texture_size(path)
        if fmt is None:
            continue
        relative = path.relative_to(args.folder).as_posix()
        if size is None:
            print(f"{relative}\t{fmt}\t?\t?\t?")
            continue
        width, height = size
        pot = "yes" if is_power_of_two(width) and is_power_of_two(height) else "no"
        print(f"{relative}\t{fmt}\t{width}\t{height}\t{pot}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
