"""Builds the classic theme's derived textures from the original 0.97 ones.

Where 0.97 tiles or stretches a texture over a box of known size, the
theme ships the result instead, drawn unscaled:

  Nis_vframe_tall.ozj   nis_vframe (5x40) stacked 12 times: the chat window's sides.
  Message_box2_268.ozj  Message_box2 (213x5) stretched to 268: the chat window's top edge.
  Message_box2_230.ozj  the same stretched to 230: the in-game message box's edges.

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


def main():
    prefix, side = read_ozj("Nis_vframe.ozj")
    tall = Image.new("RGB", (side.width, side.height * 12))
    for i in range(12):
        tall.paste(side, (0, i * side.height))
    write_ozj("Nis_vframe_tall.ozj", prefix, tall)

    prefix, top = read_ozj("Message_box2.ozj")
    for width in (268, 230):
        write_ozj(f"Message_box2_{width}.ozj", prefix, top.resize((width, top.height), Image.BILINEAR))


if __name__ == "__main__":
    main()
