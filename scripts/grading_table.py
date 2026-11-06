#!/usr/bin/env python3
"""Writes colour grading lookup tables for post-processing (docs/post-processing.md).

A table is a 32x32x32 colour cube stored as a 1024x32 PNG strip: 32 slices of
32x32 side by side, red along x inside a slice, green along y, blue picking the
slice. The client reads src/bin/Data/Grading/World<N>.png for map N-1.

`identity` writes the table that changes nothing: the starting point for a
table graded in an image editor (grade a screenshot with the strip pasted into
it, then cut the strip out again). The named presets are the tables shipped in
src/bin/Data/Grading, generated here so they can be reproduced and tuned.

Every table written here obeys two rules, checked before the file is written:
it never makes a colour brighter (Rec. 709 luma), and it leaves highlights
alone (no tint above HIGHLIGHT_LUMA), so armour, glows and light pools keep
their Classic colour. Grading may cool and deepen shadows a little; real base
darkness belongs to the map's light, not to its grading.

Usage: scripts/grading_table.py identity|dungeon <output.png>
"""

import argparse
import struct
import sys
import zlib
from pathlib import Path

SIZE = 32
LUMA = (0.2126, 0.7152, 0.0722)


def luma(color):
    return sum(weight * channel for weight, channel in zip(LUMA, color))


def clamp(value):
    return min(max(value, 0.0), 1.0)


def identity(color):
    return color


# Above this luma a table must leave colours as they are.
HIGHLIGHT_LUMA = 0.6
# How far a channel may move above HIGHLIGHT_LUMA, and how much brighter any colour may get
# (one 8-bit level: rounding).
TOLERANCE = 1.0 / 255.0


def shadow_weight(y):
    """1 at black, falling smoothly to 0 at HIGHLIGHT_LUMA."""
    t = clamp(1.0 - y / HIGHLIGHT_LUMA)
    return t * t * (3.0 - 2.0 * t)


def dungeon(color):
    """Cold, slightly desaturated and a little deeper shadows; mid-tones and highlights untouched."""
    shadow_tint = (-0.025, 0.004, 0.030)
    desaturation = 0.25
    deepening = 0.12
    y = luma(color)
    w = shadow_weight(y)
    toned = [channel + (y - channel) * desaturation * w + tint * w for channel, tint in zip(color, shadow_tint)]
    # Brightness: the shadows a little deeper, everything else where it was. Clamping a channel
    # undoes part of the correction, so correct again until it holds.
    target = y * (1.0 - deepening * w)
    for _ in range(4):
        correction = target - luma(toned)
        toned = [clamp(channel + correction) for channel in toned]
    return toned


def check(grade):
    """The table rules: never brighter, highlights untouched. Returns the worst offences."""
    brighter = 0.0
    highlight_shift = 0.0
    steps = [index / (SIZE - 1) for index in range(SIZE)]
    for red in steps:
        for green in steps:
            for blue in steps:
                color = [red, green, blue]
                graded = grade(color)
                brighter = max(brighter, luma(graded) - luma(color))
                if luma(color) >= HIGHLIGHT_LUMA:
                    highlight_shift = max(highlight_shift, max(abs(g - c) for g, c in zip(graded, color)))
    return brighter, highlight_shift


PRESETS = {"identity": identity, "dungeon": dungeon}


def table_rows(grade):
    rows = []
    for green in range(SIZE):
        row = bytearray()
        for blue in range(SIZE):
            for red in range(SIZE):
                color = [red / (SIZE - 1), green / (SIZE - 1), blue / (SIZE - 1)]
                graded = grade(color)
                row += bytes(round(channel * 255) for channel in graded) + b"\xff"
        rows.append(bytes(row))
    return rows


def png_chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def write_png(path, rows):
    width = len(rows[0]) // 4
    header = struct.pack(">IIBBBBB", width, len(rows), 8, 6, 0, 0, 0)  # 8-bit RGBA
    raw = b"".join(b"\x00" + row for row in rows)
    data = (b"\x89PNG\r\n\x1a\n" + png_chunk(b"IHDR", header) + png_chunk(b"IDAT", zlib.compress(raw, 9))
            + png_chunk(b"IEND", b""))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("preset", choices=sorted(PRESETS))
    parser.add_argument("output", type=Path)
    arguments = parser.parse_args()
    grade = PRESETS[arguments.preset]
    brighter, highlight_shift = check(grade)
    if brighter > TOLERANCE or highlight_shift > TOLERANCE:
        print(f"refused: brightens a colour by {brighter * 255:.1f} levels, moves a highlight by "
              f"{highlight_shift * 255:.1f} levels (at most {TOLERANCE * 255:.0f} each)", file=sys.stderr)
        return 1
    write_png(arguments.output, table_rows(grade))
    return 0


if __name__ == "__main__":
    sys.exit(main())
