"""Builds the classic theme's derived textures from the original 0.97 ones.

Where 0.97 tiles or stretches a texture over a box of known size, the
theme ships the result instead, drawn unscaled:

  Nis_vframe_tall.ozj   nis_vframe (5x40) stacked 12 times: the chat window's sides.
  Message_box2_268.ozj  Message_box2 (213x5) stretched to 268: the chat window's top edge.
  Message_box1_60x5.ozj Message_box1 (213x40) stretched to 60 and stacked 5 times: the Season 2
                        Chaos Goblin menu's stone (213x300).
  Message_box1_40x7.ozj Message_box1 (213x40) stacked 7 times, unscaled: the Season 2 trainer's
                        and Lahap's stones (213x200, 213x240 and 213x280, cut from it).
  Message_box1_440x300.ozj  Message_box1 widened to 440 for the settings window: its 5-texel sides
                        kept at their width, the stone between them stretched in proportion, in
                        rows down to 300.
  Message_box2_440.ozj  Message_box2 widened to 440 the same way: the settings window's edges.
  s1/DuelWindow_add.ozt Season 1's duel crest (a JPEG on black), which Season 1 drew additively:
                        as a TGA whose alpha is each texel's brightest channel and colour that
                        channel scaled back up, so plain alpha blending lands close to the
                        additive draw and the black square disappears.
  Progress_add.ozt      0.97's progress fill (an orange line on black), which Season 1's message
                        box drew additively over Progress_back: the same treatment.

Usage: python tools/classic_theme/make_classic_derived_textures.py
"""
import io
import pathlib

from PIL import Image

TEXTURES = pathlib.Path(__file__).resolve().parents[2] / "src/bin/Data/Interface/Classic"
# An OZJ is a 24-byte prefix followed by the JPEG.
OZJ_PREFIX = 24


def read_ozj(name):
    raw = (TEXTURES / name).read_bytes()
    return raw[:OZJ_PREFIX], Image.open(io.BytesIO(raw[OZJ_PREFIX:])).convert("RGB")


def write_ozj(name, prefix, image):
    out = io.BytesIO()
    image.save(out, "JPEG", quality=95, subsampling=0)
    (TEXTURES / name).write_bytes(prefix + out.getvalue())
    print(f"{name}: {image.width}x{image.height}")


# An OZT is a 4-byte prefix followed by an uncompressed TGA.
OZT_PREFIX = bytes([0, 0, 2, 0])


def write_ozt_rgba(name, image):
    header = (bytes([0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0]) + image.width.to_bytes(2, "little")
              + image.height.to_bytes(2, "little") + bytes([32, 8]))
    pixels = bytearray()
    for y in range(image.height - 1, -1, -1):  # bottom-up, as the originals
        for x in range(image.width):
            r, g, b, a = image.getpixel((x, y))
            pixels += bytes([b, g, r, a])
    (TEXTURES / name).write_bytes(OZT_PREFIX + header + bytes(pixels))
    print(f"{name}: {image.width}x{image.height}")


def additive_to_alpha(image):
    out = Image.new("RGBA", image.size)
    for y in range(image.height):
        for x in range(image.width):
            r, g, b = image.getpixel((x, y))
            a = max(r, g, b)
            if a == 0:
                out.putpixel((x, y), (0, 0, 0, 0))
            else:
                out.putpixel((x, y), (min(255, r * 255 // a), min(255, g * 255 // a), min(255, b * 255 // a), a))
    return out


def widen_box_part(part, width, height):
    """Widens a 0.97 box part (213 wide, 5-texel sides) to width without widening its sides: the
    sides keep their width and the middle stretches, both scaled in height by the middle's factor,
    and the result stacked in rows down to height."""
    side = 5
    middle = part.crop((side, 0, part.width - side, part.height))
    row_height = round(part.height * (width - 2 * side) / middle.width)
    row = Image.new("RGB", (width, row_height))
    row.paste(part.crop((0, 0, side, part.height)).resize((side, row_height), Image.BILINEAR), (0, 0))
    row.paste(middle.resize((width - 2 * side, row_height), Image.BILINEAR), (side, 0))
    row.paste(part.crop((part.width - side, 0, part.width, part.height)).resize((side, row_height), Image.BILINEAR),
              (width - side, 0))
    if height == part.height:
        return row.resize((width, height), Image.BILINEAR)
    out = Image.new("RGB", (width, height))
    for y in range(0, height, row_height):
        out.paste(row, (0, y))
    return out


def main():
    prefix, side = read_ozj("Nis_vframe.ozj")
    tall = Image.new("RGB", (side.width, side.height * 12))
    for i in range(12):
        tall.paste(side, (0, i * side.height))
    write_ozj("Nis_vframe_tall.ozj", prefix, tall)

    prefix, top = read_ozj("Message_box2.ozj")
    write_ozj("Message_box2_268.ozj", prefix, top.resize((268, top.height), Image.BILINEAR))

    prefix, stone = read_ozj("Message_box1.ozj")
    strip = stone.resize((stone.width, 60), Image.BILINEAR)
    stack = Image.new("RGB", (strip.width, strip.height * 5))
    for i in range(5):
        stack.paste(strip, (0, i * strip.height))
    write_ozj("Message_box1_60x5.ozj", prefix, stack)

    rows = Image.new("RGB", (stone.width, stone.height * 7))
    for i in range(7):
        rows.paste(stone, (0, i * stone.height))
    write_ozj("Message_box1_40x7.ozj", prefix, rows)

    write_ozj("Message_box1_440x300.ozj", prefix, widen_box_part(stone, 440, 300))
    prefix, edge = read_ozj("Message_box2.ozj")
    write_ozj("Message_box2_440.ozj", prefix, widen_box_part(edge, 440, edge.height))

    _, crest = read_ozj("s1/DuelWindow.OZJ")
    write_ozt_rgba("s1/DuelWindow_add.ozt", additive_to_alpha(crest))

    _, fill = read_ozj("Progress.ozj")
    write_ozt_rgba("Progress_add.ozt", additive_to_alpha(fill))


if __name__ == "__main__":
    main()
