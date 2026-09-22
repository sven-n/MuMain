#!/usr/bin/env python3
"""Wrap and unwrap MU Online texture containers.

The client stores textures in thin containers around ordinary image files:

    .OZJ = 24-byte prefix + JPEG   (opaque textures; the prefix is the first 24 bytes of the JPEG)
    .OZT =  4-byte prefix + TGA    (textures with alpha; 32-bit uncompressed, bottom-left origin)
    .OZB =  4-byte prefix + BMP    (terrain height maps; 8-bit, 256x256)

The loader (src/source/Render/Sprites/GlobalBitmap.cpp) skips the prefix and reads the payload,
so "unwrap" gives you a normal .jpg/.tga/.bmp to edit and "wrap" turns it back. Game code refers
to textures by their .jpg/.tga name and swaps the extension to .OZJ/.OZT itself.

Usage:
    mu_texture.py unwrap <file.OZJ|OZT|OZB> [--out PATH]
    mu_texture.py wrap   <file.jpg|jpeg|tga|bmp> [--out PATH]
    mu_texture.py unwrap-dir <source-dir> <target-dir>     (recursive, keeps the folder layout)
    mu_texture.py wrap-dir   <source-dir> <target-dir>
    mu_texture.py check  <file...>                          (validate against the loader's rules)

Standard library only; no Pillow required.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

PREFIX_SIZES = {".ozj": 24, ".ozt": 4, ".ozb": 4}
PAYLOAD_EXTENSIONS = {".ozj": ".jpg", ".ozt": ".tga", ".ozb": ".bmp"}
CONTAINER_EXTENSIONS = {".jpg": ".OZJ", ".jpeg": ".OZJ", ".tga": ".OZT", ".bmp": ".OZB"}
MAX_TEXTURE_SIZE = 1024  # CGlobalBitmap::MAX_WIDTH / MAX_HEIGHT
TGA_HEADER_SIZE = 18
TGA_TRUECOLOR_UNCOMPRESSED = 2
TGA_TRUECOLOR_RLE = 10
TGA_ORIGIN_TOP_BIT = 0x20
JPEG_SOI = b"\xff\xd8"
JPEG_SOF_MARKERS = {0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF}


class TextureError(Exception):
    """A file that the client would refuse or misread."""


def is_power_of_two(value: int) -> bool:
    return value > 0 and (value & (value - 1)) == 0


def unwrap_bytes(data: bytes, container_extension: str) -> bytes:
    prefix = PREFIX_SIZES[container_extension]
    if len(data) <= prefix:
        raise TextureError(f"file is only {len(data)} bytes, shorter than the {prefix}-byte prefix")
    return data[prefix:]


def wrap_bytes(payload: bytes, container_extension: str) -> bytes:
    prefix = PREFIX_SIZES[container_extension.lower()]
    if len(payload) < prefix:
        raise TextureError(f"image is only {len(payload)} bytes, shorter than the {prefix}-byte prefix")
    # The original files repeat the first bytes of the payload as the prefix; the loader
    # never reads them, so this keeps new files indistinguishable from shipped ones.
    return payload[:prefix] + payload


def parse_tga(payload: bytes) -> tuple[int, int, int, int, int]:
    if len(payload) < TGA_HEADER_SIZE:
        raise TextureError("TGA header is truncated")
    id_length, colormap_type, image_type = struct.unpack_from("<BBB", payload, 0)
    width, height, bits, descriptor = struct.unpack_from("<HHBB", payload, 12)
    return image_type, width, height, bits, descriptor, id_length, colormap_type  # type: ignore[return-value]


def check_tga(payload: bytes, warnings: list[str]) -> None:
    image_type, width, height, bits, descriptor, id_length, colormap_type = parse_tga(payload)
    if image_type == TGA_TRUECOLOR_RLE:
        raise TextureError("TGA is RLE compressed; the loader only reads uncompressed true-color TGA (type 2)")
    if image_type != TGA_TRUECOLOR_UNCOMPRESSED or colormap_type != 0:
        raise TextureError(f"TGA image type {image_type} is not supported; export uncompressed 32-bit true color")
    if bits != 32:
        raise TextureError(f"TGA has {bits} bits per pixel; the loader requires 32 (RGBA)")
    check_dimensions(width, height, warnings)
    if descriptor & TGA_ORIGIN_TOP_BIT:
        warnings.append("TGA origin is top-left; the loader assumes bottom-left, so the image will appear flipped")
    expected = TGA_HEADER_SIZE + id_length + width * height * 4
    if len(payload) < expected:
        raise TextureError(f"TGA pixel data is truncated ({len(payload)} of {expected} bytes)")


def read_jpeg_dimensions(payload: bytes) -> tuple[int, int, int]:
    if not payload.startswith(JPEG_SOI):
        raise TextureError("payload does not start with a JPEG SOI marker")
    offset = 2
    while offset + 4 <= len(payload):
        if payload[offset] != 0xFF:
            offset += 1
            continue
        marker = payload[offset + 1]
        if marker in (0xD8, 0x01) or 0xD0 <= marker <= 0xD7:
            offset += 2
            continue
        (length,) = struct.unpack_from(">H", payload, offset + 2)
        if marker in JPEG_SOF_MARKERS:
            _precision, height, width, components = struct.unpack_from(">BHHB", payload, offset + 4)
            return width, height, components
        offset += 2 + length
    raise TextureError("no JPEG frame header found")


def check_jpeg(payload: bytes, warnings: list[str]) -> None:
    width, height, components = read_jpeg_dimensions(payload)
    if components == 4:
        raise TextureError("JPEG uses 4 components (CMYK); export RGB")
    check_dimensions(width, height, warnings)


def check_bmp(payload: bytes, warnings: list[str]) -> None:
    if payload[:2] != b"BM" or len(payload) < 30:
        raise TextureError("payload is not a BMP file")
    width, height = struct.unpack_from("<ii", payload, 18)
    bits = struct.unpack_from("<H", payload, 28)[0]
    if bits != 8:
        warnings.append(f"BMP has {bits} bits per pixel; terrain height maps are 8-bit grayscale")
    if abs(width) != 256 or abs(height) != 256:
        warnings.append(f"BMP is {width}x{height}; terrain height maps are 256x256")


def check_dimensions(width: int, height: int, warnings: list[str]) -> None:
    if width <= 0 or height <= 0 or width > MAX_TEXTURE_SIZE or height > MAX_TEXTURE_SIZE:
        raise TextureError(f"{width}x{height} exceeds the loader limit of {MAX_TEXTURE_SIZE}x{MAX_TEXTURE_SIZE}")
    if not (is_power_of_two(width) and is_power_of_two(height)):
        warnings.append(
            f"{width}x{height} is not a power of two; the loader pads to the next power of two and UVs will not cover the image"
        )


CHECKERS = {".jpg": check_jpeg, ".jpeg": check_jpeg, ".tga": check_tga, ".bmp": check_bmp}


def check_file(path: Path) -> list[str]:
    extension = path.suffix.lower()
    data = path.read_bytes()
    warnings: list[str] = []
    if extension in PREFIX_SIZES:
        payload = unwrap_bytes(data, extension)
        CHECKERS[PAYLOAD_EXTENSIONS[extension]](payload, warnings)
    elif extension in CHECKERS:
        CHECKERS[extension](data, warnings)
    else:
        raise TextureError(f"unknown extension {path.suffix}")
    return warnings


def unwrap_file(source: Path, target: Path | None) -> Path:
    extension = source.suffix.lower()
    if extension not in PREFIX_SIZES:
        raise TextureError(f"{source.name}: not an .OZJ/.OZT/.OZB container")
    destination = target or source.with_suffix(PAYLOAD_EXTENSIONS[extension])
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(unwrap_bytes(source.read_bytes(), extension))
    return destination


def wrap_file(source: Path, target: Path | None) -> Path:
    extension = source.suffix.lower()
    if extension not in CONTAINER_EXTENSIONS:
        raise TextureError(f"{source.name}: not a .jpg/.tga/.bmp image")
    container = CONTAINER_EXTENSIONS[extension]
    destination = target or source.with_suffix(container)
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(wrap_bytes(source.read_bytes(), container))
    return destination


def convert_tree(source_dir: Path, target_dir: Path, extensions: dict, convert) -> tuple[int, int]:
    converted = 0
    failed = 0
    for path in sorted(source_dir.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in extensions:
            continue
        relative = path.relative_to(source_dir)
        target = target_dir / relative.parent / (path.stem + extensions[path.suffix.lower()])
        try:
            convert(path, target)
            converted += 1
        except TextureError as error:
            failed += 1
            print(f"skip {relative}: {error}", file=sys.stderr)
    return converted, failed


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("unwrap", "wrap"):
        command = commands.add_parser(name)
        command.add_argument("file", type=Path)
        command.add_argument("--out", type=Path)
    for name in ("unwrap-dir", "wrap-dir"):
        command = commands.add_parser(name)
        command.add_argument("source", type=Path)
        command.add_argument("target", type=Path)
    check = commands.add_parser("check")
    check.add_argument("files", type=Path, nargs="+")
    args = parser.parse_args(argv)

    try:
        if args.command == "unwrap":
            print(unwrap_file(args.file, args.out))
        elif args.command == "wrap":
            print(wrap_file(args.file, args.out))
        elif args.command == "unwrap-dir":
            converted, failed = convert_tree(args.source, args.target, PAYLOAD_EXTENSIONS, unwrap_file)
            print(f"unwrapped {converted} textures, {failed} failed")
            return 1 if failed else 0
        elif args.command == "wrap-dir":
            container_by_extension = {ext: container for ext, container in CONTAINER_EXTENSIONS.items()}
            converted, failed = convert_tree(args.source, args.target, container_by_extension, wrap_file)
            print(f"wrapped {converted} textures, {failed} failed")
            return 1 if failed else 0
        elif args.command == "check":
            problems = 0
            for path in args.files:
                try:
                    warnings = check_file(path)
                    status = "OK" if not warnings else "WARN"
                    print(f"{status} {path}")
                    for warning in warnings:
                        print(f"     {warning}")
                except TextureError as error:
                    problems += 1
                    print(f"FAIL {path}: {error}")
            return 1 if problems else 0
    except TextureError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
