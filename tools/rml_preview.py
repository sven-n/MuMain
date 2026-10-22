#!/usr/bin/env python3
"""Preview an RmlUi document of the game in a web browser, without building the client.

Translates one RML document and its theme's RCSS into plain HTML/CSS:

- the document and its stylesheets resolve through the theme chain (theme.ini Base=), as the
  game resolves them; token() values come from the chain's tokens.ini files;
- `dp` scales by the window's dp ratio and `rem` is the native text size, both computed for the
  given window size as UI::Scaling computes them;
- data bindings (data-for, data-if, data-visible, data-class-*, data-style-*, data-attr-*,
  data-value, data-checked, data-rml, {{...}}) are evaluated against a JSON model;
- @spritesheet sprites and image()/ninepatch() decorators are drawn from the game's own
  textures (.OZT/.OZJ), gradients become CSS gradients, and the bundled fonts are used.

Optionally screenshots the result with a Chromium-based browser (Chrome, Edge, Chromium).

It is a preview, not RmlUi: the browser's layout engine runs, not RmlUi's. Live 3D content
(item models, render targets), scrollbars, font effects and C++-driven state that isn't in the
model are not shown. Use it to look at a layout or a theme change quickly, and check the result
in the game before relying on details.

The model is a JSON object with the document's binding names (the model's members), e.g.
    {"lines": [{"text": "Enter selling price.", "bold": false, "color": ""}], "has_input": true}
A document with several data models takes {"model_name": {...}, ...}. A file with
    {"states": {"zen": {...}, "jewel": {...}}}
renders one page per state. Missing names evaluate to nothing (with a warning), so a document
also renders without a model.

Usage: python tools/rml_preview.py DOCUMENT [--theme legacy] [--model FILE.json]
                                   [--window 1024x768] [--ui-scale 100] [--out DIR]
                                   [--png] [--browser PATH] [--background CSS]
DOCUMENT is a document name (generic_confirm_dialog) or an .rml path. Writes DIR/<state>.html
(and .png with --png); open the HTML in any browser.
Exit code 0 = rendered, 1 = error.
"""
import argparse
import base64
import html
import json
import math
import os
import pathlib
import platform
import re
import shutil
import struct
import subprocess
import sys
import xml.etree.ElementTree as ET
import zlib

sys.dont_write_bytecode = True
from rml_theme_inheritance import resolve_virtual, theme_chain


REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
RMLUI_ROOT = REPO_ROOT / "src/bin/Data/Interface/RmlUi"
FONTS_ROOT = REPO_ROOT / "src/bin/fonts"

# UI::Scaling (UITransform.cpp): the 640x480 reference screen, the panel scale's cap, and the
# normal font's point range, which grows with the scale up to kMaximumTypographyScale.
REFERENCE_WIDTH, REFERENCE_HEIGHT = 640, 480
MAXIMUM_PANEL_SCALE = 2.0
MAXIMUM_TYPOGRAPHY_SCALE = 2.25
NORMAL_FONT_POINT_RANGE = (11, 16)
# RmlUi's initial font-size, for documents without the native text size.
RMLUI_DEFAULT_FONT_SIZE = 12

warnings = []
missing_names = []


def warn(message):
    if message not in warnings:
        warnings.append(message)
        print(f"warning: {message}", file=sys.stderr)


def missing(name):
    """A name the model doesn't have: it evaluates to nothing, reported once at the end."""
    if name not in missing_names:
        missing_names.append(name)


# ---------------------------------------------------------------------------------------------
# Scaling


def dp_ratio(width, height, ui_scale_percent=100):
    """UI::Scaling::TypographyScale(): the dp ratio of the RmlUi context."""
    fits_window = min(width / REFERENCE_WIDTH, height / REFERENCE_HEIGHT)
    fit = min(max(fits_window, 1.0), MAXIMUM_PANEL_SCALE)
    return min(fit * ui_scale_percent / 100, max(fit, fits_window))


def native_text_size(ratio):
    """UI::Scaling::NativeTextPixelSize() of the normal font: 1rem of a "native-text" body."""
    minimum, maximum = NORMAL_FONT_POINT_RANGE
    growth = min(max((ratio - 1.0) / (MAXIMUM_TYPOGRAPHY_SCALE - 1.0), 0.0), 1.0)
    return round(minimum + (maximum - minimum) * growth)


def root_font_sizes(body_classes, uses_native_text, width, height, ui_scale_percent):
    """RmlNativeText.cpp's ApplyNativeTextSize(): (root font-size, font-size of .native-text elements)."""
    if not uses_native_text:
        return RMLUI_DEFAULT_FONT_SIZE, None
    text = native_text_size(dp_ratio(width, height, ui_scale_percent))
    if "scene-window-scale" in body_classes:  # SceneWindowScale(): the text's growth since 1024x768
        reference = native_text_size(dp_ratio(1024, 768, ui_scale_percent))
        return max(text / reference, 1.0), text
    if "scene-bar-scale" in body_classes:  # SceneBarScale()
        return min(max(min(width / 800, height / 600), 1.0), 2.0), text
    if "native-text" in body_classes:
        return text, None
    return RMLUI_DEFAULT_FONT_SIZE, None


def theme_setting(chain, section, key):
    """A theme.ini value, the derived theme's over its base's."""
    for directory in chain:
        path = directory / "theme.ini"
        if not path.is_file():
            continue
        current = ""
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            line = line.strip()
            if line.startswith("[") and line.endswith("]"):
                current = line[1:-1].strip().lower()
            elif current == section.lower() and "=" in line and not line.startswith((";", "#")):
                name, value = line.split("=", 1)
                if name.strip().lower() == key.lower():
                    return value.strip()
    return None


# ---------------------------------------------------------------------------------------------
# Textures: the game's .OZT (TGA behind a 4 byte header) and .OZJ (JPEG behind a 24 byte header)


def find_texture(path):
    """The file the game loads for an image path: .tga is stored as .OZT, .jpg as .OZJ."""
    alternatives = {".tga": ".ozt", ".jpg": ".ozj", ".jpeg": ".ozj"}
    suffix = path.suffix.lower()
    candidates = {suffix, alternatives.get(suffix, suffix)} - {""}
    directory = path.parent
    if directory.is_dir():
        for entry in directory.iterdir():
            if entry.stem.lower() == path.stem.lower() and entry.suffix.lower() in candidates and entry.is_file():
                return entry
    return None


def decode_tga(data):
    """Uncompressed or RLE true-color/grayscale TGA to (width, height, RGBA rows top-down)."""
    id_length, colormap_type, image_type = data[0], data[1], data[2]
    width, height, bpp, descriptor = struct.unpack_from("<HHBB", data, 12)
    if colormap_type != 0 or image_type not in (2, 3, 10, 11):
        raise ValueError(f"unsupported TGA type {image_type}")
    channels = bpp // 8
    offset = 18 + id_length
    count = width * height
    if image_type in (2, 3):
        pixels = data[offset:offset + count * channels]
    else:
        out = bytearray()
        while len(out) < count * channels:
            header = data[offset]
            offset += 1
            run = (header & 0x7F) + 1
            if header & 0x80:
                out += data[offset:offset + channels] * run
                offset += channels
            else:
                out += data[offset:offset + run * channels]
                offset += run * channels
        pixels = bytes(out[:count * channels])

    rgba = bytearray(count * 4)
    for i in range(count):
        p = pixels[i * channels:(i + 1) * channels]
        if channels == 1:
            rgba[i * 4:i * 4 + 4] = bytes((p[0], p[0], p[0], 255))
        elif channels == 2:  # 16 bit A1R5G5B5
            v = p[0] | p[1] << 8
            r, g, b = (v >> 10) & 31, (v >> 5) & 31, v & 31
            rgba[i * 4:i * 4 + 4] = bytes((r * 255 // 31, g * 255 // 31, b * 255 // 31, 255 if v & 0x8000 or bpp == 16 else 0))
        else:
            rgba[i * 4:i * 4 + 4] = bytes((p[2], p[1], p[0], p[3] if channels == 4 else 255))
    stride = width * 4
    rows = [bytes(rgba[y * stride:(y + 1) * stride]) for y in range(height)]
    if not descriptor & 0x20:  # origin at the bottom
        rows.reverse()
    return width, height, rows


def encode_png(width, height, rows):
    def chunk(kind, payload):
        return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload))

    raw = b"".join(b"\x00" + row for row in rows)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def jpeg_size(data):
    offset = 2
    while offset < len(data):
        marker, length = data[offset + 1], struct.unpack_from(">H", data, offset + 2)[0]
        if 0xC0 <= marker <= 0xCF and marker not in (0xC4, 0xC8, 0xCC):
            height, width = struct.unpack_from(">HH", data, offset + 5)
            return width, height
        offset += 2 + length
    raise ValueError("no JPEG frame header")


class Textures:
    """Converts the textures a document uses into browser images under out/img."""

    def __init__(self, out_dir):
        self.out_dir = out_dir
        self.images = {}  # source file -> (relative url, width, height, data uri)
        self.sprites = set()  # the cropped sprite files written this run
        (out_dir / "img").mkdir(parents=True, exist_ok=True)

    def load(self, path):
        source = find_texture(path)
        if source is None:
            warn(f"texture not found: {path}")
            return None
        if source in self.images:
            return self.images[source]
        data = source.read_bytes()
        suffix = source.suffix.lower()
        if suffix == ".ozt":
            data = data[4:]
        elif suffix == ".ozj":
            data = data[24:]
        if suffix in (".ozt", ".tga"):
            width, height, rows = decode_tga(data)
            data, mime, extension = encode_png(width, height, rows), "image/png", ".png"
        elif suffix in (".ozj", ".jpg", ".jpeg"):
            (width, height), mime, extension = jpeg_size(data), "image/jpeg", ".jpg"
        elif suffix == ".png":
            width, height = struct.unpack_from(">II", data, 16)
            mime, extension = "image/png", ".png"
        else:
            warn(f"unsupported texture format: {source}")
            return None
        name = f"{source.stem.lower()}{extension}"
        (self.out_dir / "img" / name).write_bytes(data)
        uri = f"data:{mime};base64,{base64.b64encode(data).decode()}"
        self.images[source] = (f"img/{name}", width, height, uri)
        return self.images[source]

    def crop(self, path, rect):
        """A sprite as an SVG image of its sheet rectangle, so any part of any sheet can be cropped."""
        image = self.load(path)
        if image is None:
            return None
        _, width, height, uri = image
        x, y, w, h = rect
        name = f"{path.stem.lower()}_{x}_{y}_{w}_{h}.svg"
        if name in self.sprites:
            return f"img/{name}"
        # Written once each run, as the sheets are: a sprite left from an earlier run would show the old art.
        self.sprites.add(name)
        (self.out_dir / "img" / name).write_text(
            f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" '
            f'width="{w}" height="{h}" viewBox="{x} {y} {w} {h}" preserveAspectRatio="none">'
            f'<image width="{width}" height="{height}" xlink:href="{uri}" style="image-rendering:auto"/></svg>')
        return f"img/{name}"


# ---------------------------------------------------------------------------------------------
# RCSS -> CSS


COMMENT_RE = re.compile(r"/\*.*?\*/", re.S)
SPRITESHEET_RE = re.compile(r"@spritesheet\s+[\w-]+\s*\{(.*?)\}", re.S)
SPRITE_RE = re.compile(r"([\w-]+)\s*:\s*(-?\d+(?:\.\d+)?)px\s+(-?\d+(?:\.\d+)?)px\s+(\d+(?:\.\d+)?)px\s+(\d+(?:\.\d+)?)px")
DECORATOR_RE = re.compile(r"(?<![\w-])decorator\s*:\s*([^;]+);?")
TOKEN_RE = re.compile(r"token\(\s*([\w-]+)\s*\)")
DP_RE = re.compile(r"(-?\d*\.?\d+)dp\b")
REM_RE = re.compile(r"(-?\d*\.?\d+)rem\b")
RGBA_RE = re.compile(r"rgba\(([^()]*)\)")
BORDER_RE = re.compile(r"(?<![\w-])(border(?:-top|-right|-bottom|-left)?)\s*:\s*([^;{}]+)")
BORDER_STYLES = {"none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset"}


def split_top_level(text, separator=","):
    parts, depth, current = [], 0, ""
    for char in text:
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
        if char == separator and depth == 0:
            parts.append(current.strip())
            current = ""
        else:
            current += char
    if current.strip():
        parts.append(current.strip())
    return parts


class StylesheetConverter:
    def __init__(self, chain, tokens, ratio, textures):
        self.chain = chain
        self.tokens = tokens
        self.ratio = ratio
        self.textures = textures
        self.sprites = {}  # name -> (sheet path, (x, y, w, h), display scale)
        self.rem = RMLUI_DEFAULT_FONT_SIZE  # the document's root font-size, which rem refers to

    def resolve(self, href, directory):
        """An asset path written relative to a theme file, resolved through the theme chain."""
        path = (directory / href).resolve()
        return resolve_virtual(path, RMLUI_ROOT.resolve(), self.chain) if path.is_relative_to(RMLUI_ROOT.resolve()) else path

    def length(self, value):
        return f"{value * self.ratio:.3f}".rstrip("0").rstrip(".") + "px"

    def convert(self, text, directory):
        text = COMMENT_RE.sub("", text)
        text = TOKEN_RE.sub(lambda m: self.token(m.group(1)), text)
        text = SPRITESHEET_RE.sub(lambda m: self.spritesheet(m.group(1), directory) or "", text)
        text = DECORATOR_RE.sub(lambda m: self.decorator(m.group(1).strip(), directory), text)
        return self.convert_values(text)

    def convert_values(self, text):
        """Units, colors and borders of declarations, e.g. of an inline style attribute."""
        text = TOKEN_RE.sub(lambda m: self.token(m.group(1)), text)
        text = DP_RE.sub(lambda m: self.length(float(m.group(1))), text)
        # rem as px: RmlUi's root size may be far below a browser's minimum font size.
        text = REM_RE.sub(lambda m: f"{float(m.group(1)) * self.rem:.3f}".rstrip("0").rstrip(".") + "px", text)
        text = RGBA_RE.sub(self.rgba, text)
        text = BORDER_RE.sub(self.border, text)
        return text

    def token(self, name):
        if name not in self.tokens:
            warn(f"unknown token: {name}")
            return "inherit"
        return self.tokens[name]

    def spritesheet(self, body, directory):
        src = re.search(r'src\s*:\s*"?([^";]+)"?', body)
        if not src:
            return None
        sheet = self.resolve(src.group(1).strip(), directory)
        scale = 1.0
        resolution = re.search(r"resolution\s*:\s*([\d.]+)x", body)
        if resolution:
            scale = 1.0 / float(resolution.group(1))
        for name, x, y, w, h in SPRITE_RE.findall(body):
            self.sprites[name] = (sheet, tuple(int(float(v)) for v in (x, y, w, h)), scale)
        return None

    @staticmethod
    def rgba(match):
        parts = [p.strip() for p in match.group(1).split(",")]
        if len(parts) != 4:
            return match.group(0)
        try:
            alpha = float(parts[3]) / 255  # RmlUi's alpha is 0..255
        except ValueError:
            return match.group(0)
        return f"rgba({parts[0]}, {parts[1]}, {parts[2]}, {alpha:.3f})"

    @staticmethod
    def border(match):
        # RmlUi borders are always solid; its shorthand has no style keyword.
        values = match.group(2).split()
        if any(v in BORDER_STYLES for v in values) or match.group(2).strip() in ("0", "none"):
            return match.group(0)
        return f"{match.group(1)}: {values[0]} solid {' '.join(values[1:])}".rstrip()

    def image_source(self, name, directory):
        """A sprite name or an image path -> (url, natural width, natural height) in CSS px."""
        if name in self.sprites:
            sheet, rect, scale = self.sprites[name]
            url = self.textures.crop(sheet, rect)
            return (url, rect[2] * self.ratio * scale, rect[3] * self.ratio * scale) if url else None
        image = self.textures.load(self.resolve(name.strip("\"'"), directory))
        return (image[0], image[1] * self.ratio, image[2] * self.ratio) if image else None

    def decorator(self, value, directory):
        if value == "none":
            return "background-image: none;"
        layers = []  # (image, size, position, repeat)
        for layer in split_top_level(value):
            function = re.match(r"([\w-]+)\((.*)\)(?:\s+(border-box|padding-box|content-box))?$", layer, re.S)
            if not function:
                warn(f"unsupported decorator: {layer}")
                continue
            kind, arguments = function.group(1), function.group(2).strip()
            if kind in ("linear-gradient", "radial-gradient", "repeating-linear-gradient", "repeating-radial-gradient"):
                layers.append((f"{kind}({arguments})", "100% 100%", "0 0", "no-repeat"))
            elif kind in ("image", "additive-image"):
                layers.extend(self.image_layer(kind, arguments, directory))
            elif kind == "ninepatch":
                layers.extend(self.ninepatch_layers(arguments, directory))
            else:
                warn(f"unsupported decorator: {kind}")
        if not layers:
            return "background-image: none;"
        image = ", ".join(l[0] for l in layers)
        size = ", ".join(l[1] for l in layers)
        position = ", ".join(l[2] for l in layers)
        repeat = ", ".join(l[3] for l in layers)
        return (f"background-image: {image}; background-size: {size}; background-position: {position}; "
                f"background-repeat: {repeat}; background-origin: padding-box; background-clip: padding-box;")

    def image_layer(self, kind, arguments, directory):
        words = arguments.split()
        if kind == "additive-image":  # additive-image(color path): drawn as a plain image here
            words = words[1:]
        if not words:
            return []
        source = self.image_source(words[0], directory)
        if source is None:
            return []
        url, width, height = source
        fit = next((w for w in words[1:] if w in ("fill", "contain", "cover", "scale-none", "scale-down",
                                                   "repeat", "repeat-x", "repeat-y")), "fill")
        align = [w for w in words[1:] if w in ("left", "center", "right", "top", "bottom") or re.match(r"-?[\d.]+(%|px|dp)$", w)]
        position = " ".join(self.align(a) for a in align) if align else "center" if fit != "fill" else "0 0"
        natural = f"{width:.3f}px {height:.3f}px"
        size, repeat = {
            "fill": ("100% 100%", "no-repeat"),
            "contain": ("contain", "no-repeat"),
            "cover": ("cover", "no-repeat"),
            "scale-none": (natural, "no-repeat"),
            "scale-down": (natural, "no-repeat"),  # approximation: never shrinks
            "repeat": (natural, "repeat"),
            "repeat-x": (natural, "repeat-x"),
            "repeat-y": (natural, "repeat-y"),
        }[fit]
        return [(f"url({url})", size, position, repeat)]

    def align(self, value):
        match = re.match(r"(-?[\d.]+)dp$", value)
        return self.length(float(match.group(1))) if match else value

    def ninepatch_layers(self, arguments, directory):
        names = split_top_level(arguments)
        if len(names) < 2 or names[0] not in self.sprites or names[1] not in self.sprites:
            warn(f"unsupported ninepatch: {arguments}")
            return []
        sheet, outer, scale = self.sprites[names[0]]
        _, inner, _ = self.sprites[names[1]]
        ox, oy, ow, oh = outer
        ix, iy, iw, ih = inner
        # Raw edge sizes in the sheet, and their size on screen (raw px x dp ratio x display scale).
        left, top = ix - ox, iy - oy
        right, bottom = ox + ow - (ix + iw), oy + oh - (iy + ih)
        f = self.ratio * scale
        L, T, R, B = left * f, top * f, right * f, bottom * f
        columns = [(ox, left, "left 0", f"{L:.3f}px"), (ix, iw, f"left {L:.3f}px", f"calc(100% - {L + R:.3f}px)"),
                   (ix + iw, right, "right 0", f"{R:.3f}px")]
        rows = [(oy, top, "top 0", f"{T:.3f}px"), (iy, ih, f"top {T:.3f}px", f"calc(100% - {T + B:.3f}px)"),
                (iy + ih, bottom, "bottom 0", f"{B:.3f}px")]
        layers = []
        for ry, rh, rpos, rsize in rows:
            for cx, cw, cpos, csize in columns:
                if cw <= 0 or rh <= 0:
                    continue
                url = self.textures.crop(sheet, (cx, ry, cw, rh))
                if url:
                    layers.append((f"url({url})", f"{csize} {rsize}", f"{cpos} {rpos}", "no-repeat"))
        return layers


def load_tokens(chain):
    tokens = {}
    for directory in reversed(chain):  # the base first, so a derived theme overrides it
        path = directory / "tokens.ini"
        if not path.is_file():
            continue
        section = ""
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            line = line.strip()
            if not line or line.startswith((";", "#")):
                continue
            if line.startswith("[") and line.endswith("]"):
                section = line[1:-1].strip().lower()
            elif section == "tokens" and "=" in line:
                key, value = line.split("=", 1)
                tokens[key.strip().lower()] = value.strip()
    return tokens


# ---------------------------------------------------------------------------------------------
# Data binding expressions (RmlUi's data expression syntax)


EXPRESSION_TOKEN_RE = re.compile(r"\s*(?:(\d+\.\d*|\.\d+|\d+)|('(?:\\.|[^'\\])*'|\"(?:\\.|[^\"\\])*\")|"
                                 r"([A-Za-z_][\w]*)|(&&|\|\||==|!=|<=|>=|[-+*/%!<>?:()\[\].,|]))")


class Expression:
    """Parses and evaluates an RmlUi data expression against a scope."""

    def __init__(self, text):
        self.text = text
        self.tokens = []
        position = 0
        while position < len(text):
            match = EXPRESSION_TOKEN_RE.match(text, position)
            if not match or match.end() == position:
                if text[position:].strip() == "":
                    break
                raise ValueError(f"cannot parse expression: {text}")
            number, string, name, operator = match.groups()
            if number is not None:
                self.tokens.append(("number", float(number)))
            elif string is not None:
                self.tokens.append(("string", string[1:-1].replace("\\'", "'").replace('\\"', '"')))
            elif name is not None:
                self.tokens.append(("name", name))
            else:
                self.tokens.append(("op", operator))
            position = match.end()
        self.index = 0

    def peek(self, value=None):
        if self.index >= len(self.tokens):
            return None
        token = self.tokens[self.index]
        return token if value is None or token == ("op", value) else None

    def take(self, value=None):
        token = self.peek(value)
        if token is None:
            raise ValueError(f"expected {value or 'a value'} in: {self.text}")
        self.index += 1
        return token

    # Precedence, lowest first: ?:, ||, &&, == !=, < > <= >=, + -, * / %, unary, postfix, pipes.
    def evaluate(self, scope):
        self.index = 0
        value = self.pipe(scope)
        if self.index != len(self.tokens):
            raise ValueError(f"unexpected token in: {self.text}")
        return value

    def pipe(self, scope):
        value = self.ternary(scope)
        while self.peek("|"):
            self.take("|")
            name = self.take()[1]
            arguments = []
            if self.peek("("):
                self.take("(")
                while not self.peek(")"):
                    arguments.append(self.ternary(scope))
                    if self.peek(","):
                        self.take(",")
                self.take(")")
            value = transform(name, value, arguments)
        return value

    def ternary(self, scope):
        condition = self.binary(scope, 0)
        if self.peek("?"):
            self.take("?")
            yes = self.ternary(scope)
            self.take(":")
            no = self.ternary(scope)
            return yes if truthy(condition) else no
        return condition

    LEVELS = [("||",), ("&&",), ("==", "!="), ("<", ">", "<=", ">="), ("+", "-"), ("*", "/", "%")]

    def binary(self, scope, level):
        if level == len(self.LEVELS):
            return self.unary(scope)
        left = self.binary(scope, level + 1)
        while self.peek() and self.peek()[0] == "op" and self.peek()[1] in self.LEVELS[level]:
            operator = self.take()[1]
            right = self.binary(scope, level + 1)
            left = apply(operator, left, right)
        return left

    def unary(self, scope):
        if self.peek("!"):
            self.take("!")
            return not truthy(self.unary(scope))
        if self.peek("-"):
            self.take("-")
            return -number(self.unary(scope))
        return self.postfix(scope)

    def postfix(self, scope):
        kind, value = self.take()
        if kind == "op" and value == "(":
            result = self.pipe(scope)
            self.take(")")
        elif kind in ("number", "string"):
            result = value
        elif kind == "name":
            if value in ("true", "false"):
                result = value == "true"
            elif value in scope:
                result = scope[value]
            else:
                missing(value)
                result = None
        else:
            raise ValueError(f"unexpected '{value}' in: {self.text}")
        while self.peek(".") or self.peek("["):
            if self.take()[1] == ".":
                member = self.take()[1]
                result = member_of(result, member)
            else:
                index = self.pipe(scope)
                self.take("]")
                result = member_of(result, int(number(index)))
        return result


def member_of(value, member):
    if member == "size" and isinstance(value, (list, str)):
        return len(value)
    if isinstance(value, dict):
        if member not in value:
            missing(member)
        return value.get(member)
    if isinstance(value, list) and isinstance(member, int):
        return value[member] if 0 <= member < len(value) else None
    return None


def truthy(value):
    if isinstance(value, str):
        return value not in ("", "0", "false")
    return bool(value)


def number(value):
    if isinstance(value, bool):
        return 1.0 if value else 0.0
    if isinstance(value, (int, float)):
        return float(value)
    try:
        return float(value)
    except (TypeError, ValueError):
        return 0.0


def to_text(value):
    if value is None:
        return ""
    if isinstance(value, bool):
        return "1" if value else "0"
    if isinstance(value, float):
        return f"{value:.6f}".rstrip("0").rstrip(".") if not math.isclose(value, round(value)) else str(int(round(value)))
    return str(value)


def apply(operator, left, right):
    if operator == "||":
        return truthy(left) or truthy(right)
    if operator == "&&":
        return truthy(left) and truthy(right)
    if operator in ("==", "!="):
        if isinstance(left, str) or isinstance(right, str):
            equal = to_text(left) == to_text(right)
        else:
            equal = number(left) == number(right)
        return equal if operator == "==" else not equal
    if operator == "+" and (isinstance(left, str) or isinstance(right, str)):
        return to_text(left) + to_text(right)
    a, b = number(left), number(right)
    return {
        "<": lambda: a < b, ">": lambda: a > b, "<=": lambda: a <= b, ">=": lambda: a >= b,
        "+": lambda: a + b, "-": lambda: a - b, "*": lambda: a * b,
        "/": lambda: a / b if b else 0.0, "%": lambda: math.fmod(a, b) if b else 0.0,
    }[operator]()


def transform(name, value, arguments):
    if name == "format":
        decimals = int(number(arguments[0])) if arguments else 0
        text = f"{number(value):.{decimals}f}"
        if len(arguments) > 1 and truthy(arguments[1]) and "." in text:
            text = text.rstrip("0").rstrip(".")
        return text
    if name == "to_upper":
        return to_text(value).upper()
    if name == "to_lower":
        return to_text(value).lower()
    if name == "round":
        return float(round(number(value)))
    warn(f"unknown transform function '{name}', passed through")
    return value


def evaluate(text, scope):
    try:
        return Expression(text).evaluate(scope)
    except (ValueError, IndexError, TypeError) as error:
        warn(str(error))
        return None


# ---------------------------------------------------------------------------------------------
# RML -> HTML


INTERPOLATION_RE = re.compile(r"\{\{(.*?)\}\}", re.S)
VOID_ELEMENTS = {"img", "input", "br", "hr", "link", "meta"}


def interpolate(text, scope):
    return INTERPOLATION_RE.sub(lambda m: to_text(evaluate(m.group(1), scope)), text) if text else text


ATTRIBUTE_VALUE = r"""(?:"[^"]*"|'[^']*'|[^\s"'>]+)"""
START_TAG_RE = re.compile(r"<[A-Za-z][\w:.-]*(?:\s+[^\s\"'>/=]+(?:\s*=\s*" + ATTRIBUTE_VALUE + r")?)*\s*/?>")
ATTRIBUTE_RE = re.compile(r"(\s+)([^\s\"'>/=]+)(\s*=\s*" + ATTRIBUTE_VALUE + r")?")


def give_attributes_values(tag):
    """RmlUi takes an attribute without a value (autofocus), XML doesn't: it becomes name=""."""
    name_end = re.match(r"<[A-Za-z][\w:.-]*", tag).end()
    attributes = ATTRIBUTE_RE.sub(lambda m: m.group(0) if m.group(3) else f'{m.group(1)}{m.group(2)}=""',
                                  tag[name_end:])
    return tag[:name_end] + attributes


def parse_rml(text):
    text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    text = START_TAG_RE.sub(lambda m: give_attributes_values(m.group(0)), text)
    text = re.sub(r"&(?!amp;|lt;|gt;|quot;|apos;|#)", "&amp;", text)
    # RmlUi accepts "<" in attribute values (data-if="a < b"), XML doesn't.
    text = re.sub(r'="([^"]*)"', lambda m: '="' + m.group(1).replace("<", "&lt;") + '"', text)
    return ET.fromstring(text)


def stylesheet_links(root):
    head = root.find("head")
    return [] if head is None else [link.get("href") for link in head.findall("link")
                                    if link.get("type") == "text/rcss" and link.get("href")]


def load_templates(root, chain):
    """The templates the document links (<link type="text/template">), by name."""
    templates = {}
    head = root.find("head")
    for link in [] if head is None else head.findall("link"):
        if link.get("type") != "text/template" or not link.get("href"):
            continue
        path = resolve_virtual(RMLUI_ROOT / link.get("href"), RMLUI_ROOT, chain)
        if not path.is_file():
            warn(f"template not found: {link.get('href')}")
            continue
        template = parse_rml(path.read_text(encoding="utf-8-sig"))
        templates[template.get("name")] = template
    return templates


def apply_template(root, templates):
    """Splices the document's body into its template's body at the template's content element."""
    body = root.find("body")
    name = body.get("template")
    if not name:
        return
    template = templates.get(name)
    if template is None:
        warn(f"body template '{name}' is not linked")
        return
    shell = ET.fromstring(ET.tostring(template.find("body")))
    content = next((e for e in shell.iter() if e.get("id") == template.get("content")), None)
    if content is None:
        warn(f"template '{name}' has no element with id '{template.get('content')}'")
        return
    content.text = body.text
    content.extend(list(body))
    attributes = {**shell.attrib, **{k: v for k, v in body.attrib.items() if k != "template"}}
    if "class" in shell.attrib and "class" in body.attrib:
        attributes["class"] = f"{shell.attrib['class']} {body.attrib['class']}"
    shell.attrib.clear()
    shell.attrib.update(attributes)
    root.remove(body)
    root.append(shell)


class DocumentRenderer:
    def __init__(self, models, image_url, convert_style):
        self.models = models
        self.image_url = image_url
        self.convert_style = convert_style

    def model_scope(self, name):
        if name in self.models and isinstance(self.models[name], dict):
            return dict(self.models[name])
        return dict(self.models)

    def bind(self, element, scope, parent):
        """Applies the data bindings of element and its children; returns False to drop it."""
        attributes = element.attrib
        if "data-model" in attributes:
            scope = self.model_scope(attributes.pop("data-model"))

        if "data-for" in attributes:
            declaration = attributes.pop("data-for")
            names, _, collection = declaration.rpartition(":")
            names = [n.strip() for n in names.split(",")] if names else ["it"]
            if len(names) == 1:
                names.append("it_index")  # RmlUi's index alias when the loop names only its item
            items = evaluate(collection.strip(), scope) or []
            position = list(parent).index(element)
            parent.remove(element)
            for index, item in enumerate(items):
                clone = ET.fromstring(ET.tostring(element))
                clone.tail = None
                item_scope = {**scope, names[0]: item}
                if len(names) > 1:
                    item_scope[names[1]] = index
                if self.bind(clone, item_scope, parent):
                    parent.insert(position, clone)
                    position += 1
            return None  # already placed

        if "data-if" in attributes and not truthy(evaluate(attributes.pop("data-if"), scope)):
            return False

        classes = attributes.get("class", "").split()
        styles = attributes.get("style", "")
        for name in list(attributes):
            value = attributes[name]
            if name.startswith("data-class-"):
                if truthy(evaluate(value, scope)):
                    classes.append(name[len("data-class-"):])
            elif name.startswith("data-style-"):
                styles += f";{name[len('data-style-'):]}: {to_text(evaluate(value, scope))}"
            elif name.startswith("data-attr-"):
                attributes[name[len("data-attr-"):]] = to_text(evaluate(value, scope))
            elif name == "data-visible":
                if not truthy(evaluate(value, scope)):
                    styles += ";display: none"
            elif name == "data-value":
                attributes["value"] = to_text(evaluate(value, scope))
            elif name == "data-checked":
                if truthy(evaluate(value, scope)):
                    attributes["checked"] = "checked"
            elif name == "data-rml":
                fragment = parse_rml(f"<div>{to_text(evaluate(value, scope))}</div>")
                element.text = fragment.text
                for child in list(element):
                    element.remove(child)
                element.extend(list(fragment))
            else:
                continue
            del attributes[name]
        for name in [n for n in attributes if n.startswith("data-")]:
            del attributes[name]  # events, aliases and the game's own markers have no preview
        if classes:
            attributes["class"] = " ".join(classes)
        if styles:
            attributes["style"] = self.convert_style(styles.strip(";"))
        if element.tag == "img" and "src" in attributes:
            attributes["src"] = self.image_url(attributes["src"]) or ""

        element.text = interpolate(element.text, scope)
        for child in list(element):
            child.tail = interpolate(child.tail, scope)
            if self.bind(child, scope, element) is False:
                tail = child.tail
                index = list(element).index(child)
                element.remove(child)
                if tail:
                    if index == 0:
                        element.text = (element.text or "") + tail
                    else:
                        previous = element[index - 1]
                        previous.tail = (previous.tail or "") + tail
        return True


def serialize(element):
    attributes = "".join(f' {name}="{html.escape(value)}"' for name, value in element.attrib.items())
    if element.tag in VOID_ELEMENTS:
        inner = ""
        result = f"<{element.tag}{attributes}>"
    else:
        inner = html.escape(element.text or "", quote=False) + "".join(serialize(child) for child in element)
        result = f"<{element.tag}{attributes}>{inner}</{element.tag}>"
    return result + html.escape(element.tail or "", quote=False)


# ---------------------------------------------------------------------------------------------
# Page


BASE_CSS = """
/* RmlUi has no user-agent stylesheet: start every element unstyled, as RmlUi does. */
html, body, div, span, p, h1, h2, h3, h4, h5, h6, button, input, textarea, select, option, img, progress {
  margin: 0; padding: 0; border: 0 solid currentColor; background: none; font: inherit; color: inherit;
  text-align: inherit; box-sizing: content-box; outline: none; appearance: none; -webkit-appearance: none;
}
div, p, h1, h2, h3, h4, h5, h6, body, html { display: block; }
span, button, input, textarea, select, img, progress { display: inline-block; }
"""


def font_faces(out_dir):
    """@font-face rules for the fonts the client bundles (src/bin/fonts), by their own family names."""
    rules = []
    (out_dir / "fonts").mkdir(parents=True, exist_ok=True)
    for path in sorted(FONTS_ROOT.glob("*")):
        if path.suffix.lower() not in (".ttf", ".otf"):
            continue
        family, style = font_names(path)
        if not family:
            continue
        shutil.copyfile(path, out_dir / "fonts" / path.name)
        weight = 700 if "bold" in style.lower() else 400
        italic = "italic" if "italic" in style.lower() or "oblique" in style.lower() else "normal"
        rules.append(f'@font-face {{ font-family: "{family}"; src: url("fonts/{path.name}"); '
                     f"font-weight: {weight}; font-style: {italic}; }}")
    return "\n".join(rules)


def font_names(path):
    """(family, subfamily) from the font's name table."""
    data = path.read_bytes()
    tables = struct.unpack_from(">H", data, 4)[0]
    for i in range(tables):
        tag, _, offset, _ = struct.unpack_from(">4sIII", data, 12 + i * 16)
        if tag != b"name":
            continue
        count, strings = struct.unpack_from(">HH", data, offset + 2)
        names = {}
        for j in range(count):
            platform_id, encoding, language, name_id, length, start = struct.unpack_from(">HHHHHH", data, offset + 6 + j * 12)
            if name_id not in (1, 2, 16, 17) or name_id in names:
                continue
            raw = data[offset + strings + start:offset + strings + start + length]
            names[name_id] = raw.decode("utf-16-be" if platform_id in (0, 3) else "latin-1", "replace")
        return names.get(16, names.get(1, "")), names.get(17, names.get(2, ""))
    return "", ""


def find_browser(explicit):
    if explicit:
        return explicit
    candidates = []
    playwright = os.environ.get("PLAYWRIGHT_BROWSERS_PATH")
    if playwright:
        candidates += sorted(pathlib.Path(playwright).glob("chromium-*/chrome-linux/chrome"))
        candidates += sorted(pathlib.Path(playwright).glob("chromium_headless_shell-*/chrome-linux/headless_shell"))
    for name in ("chromium", "chromium-browser", "google-chrome", "google-chrome-stable", "chrome", "msedge"):
        found = shutil.which(name)
        if found:
            candidates.append(pathlib.Path(found))
    for variable in ("PROGRAMFILES", "PROGRAMFILES(X86)", "LOCALAPPDATA"):
        root = os.environ.get(variable)
        if root:
            candidates += [pathlib.Path(root) / "Google/Chrome/Application/chrome.exe",
                           pathlib.Path(root) / "Microsoft/Edge/Application/msedge.exe"]
    candidates += [pathlib.Path("/Applications/Google Chrome.app/Contents/MacOS/Google Chrome")]
    return next((str(c) for c in candidates if c.is_file()), None)


def screenshot(browser, page, png, width, height, pixel_ratio):
    arguments = [browser, "--headless", "--hide-scrollbars", "--disable-gpu",
                 f"--window-size={width},{height}", f"--force-device-scale-factor={pixel_ratio}",
                 f"--screenshot={png}", page.resolve().as_uri()]
    if platform.system() == "Linux" and hasattr(os, "geteuid") and os.geteuid() == 0:
        arguments.insert(1, "--no-sandbox")
    subprocess.run(arguments, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=120)


def find_document(document, chain):
    path = pathlib.Path(document)
    if path.suffix.lower() == ".rml" and path.is_file():
        return path.resolve()
    name = path.name if path.suffix.lower() == ".rml" else f"{document}.rml"
    resolved = resolve_virtual(RMLUI_ROOT / name, RMLUI_ROOT, chain)
    if not resolved.is_file():
        raise FileNotFoundError(f"no document {name} in {RMLUI_ROOT} or the theme chain")
    return resolved


def render(document, theme, states, window, ui_scale, out_dir, background):
    width, height = window
    chain = theme_chain(RMLUI_ROOT / "themes" / theme)
    ratio = dp_ratio(width, height, ui_scale)
    textures = Textures(out_dir)
    converter = StylesheetConverter(chain, load_tokens(chain), ratio, textures)
    rml_path = find_document(document, chain)
    source = rml_path.read_text(encoding="utf-8-sig")
    root = parse_rml(source)
    templates = load_templates(root, chain)
    apply_template(root, templates)
    body_classes = root.find("body").attrib.get("class", "").split()
    uses_native_text = theme_setting(chain, "Capabilities", "NativeTextSize") == "1"
    rem, element_text = root_font_sizes(body_classes, uses_native_text, width, height, ui_scale)
    native_text_rule = f".native-text {{ font-size: {element_text}px; }}\n" if element_text else ""
    converter.rem = rem

    # The stylesheets, the templates' first, each resolved as the game resolves it: relative to
    # the document's virtual place (RmlUi/), through the theme chain.
    hrefs = [href for template in templates.values() for href in stylesheet_links(template)]
    css = []
    for href in hrefs + stylesheet_links(root):
        path = resolve_virtual((RMLUI_ROOT / href).resolve(), RMLUI_ROOT.resolve(), chain)
        if not path.is_file():
            path = (rml_path.parent / href).resolve()
        if not path.is_file():
            warn(f"stylesheet not found: {href}")
            continue
        css.append(f"/* {path.relative_to(REPO_ROOT).as_posix()} */\n"
                   + converter.convert(path.read_text(encoding="utf-8-sig"), path.parent))
    for inline in re.findall(r"<style[^>]*>(.*?)</style>", source, re.S):
        css.append(converter.convert(inline, rml_path.parent))

    faces = font_faces(out_dir)

    def image_url(src):
        # A source bound to a missing model value is empty: no image, as in the game.
        if not src.strip():
            return None
        if src in converter.sprites:
            sheet, rect, _ = converter.sprites[src]
            return textures.crop(sheet, rect)
        image = textures.load((rml_path.parent / src).resolve())
        return image[0] if image else None

    page_css = (f"{faces}\n{BASE_CSS}\n"
                f"html {{ font-size: {rem}px; font-family: \"Liberation Sans\", \"Noto Sans TC\", NanumGothic; }}\n"
                f"html, body {{ width: {width}px; height: {height}px; overflow: hidden; }}\n"
                f"body {{ position: relative; }}\n"
                + "\n".join(css) + "\n" + native_text_rule
                + f"\nhtml {{ background: {background}; }}\n")
    (out_dir / "preview.css").write_text(page_css, encoding="utf-8")

    pages = []
    for state, models in states.items():
        missing_names.clear()
        root = parse_rml(source)
        apply_template(root, templates)
        body = root.find("body")
        DocumentRenderer(models, image_url, converter.convert_values).bind(body, dict(models), root)
        title = root.findtext("head/title") or rml_path.stem
        page = out_dir / f"{state}.html"
        page.write_text(
            "<!doctype html>\n<html><head><meta charset=\"utf-8\">"
            f"<title>{html.escape(title)} ({html.escape(theme)}, {html.escape(state)})</title>"
            "<link rel=\"stylesheet\" href=\"preview.css\"></head>\n"
            f"{serialize(body)}\n</html>\n", encoding="utf-8")
        pages.append(page)
        if missing_names:
            print(f"note: {state}: not in the model, so empty or false: {', '.join(missing_names)}", file=sys.stderr)
    return pages, ratio, rem


def load_states(path):
    if path is None:
        return {"preview": {}}
    data = json.loads(pathlib.Path(path).read_text(encoding="utf-8"))
    if isinstance(data, dict) and isinstance(data.get("states"), dict):
        return data["states"]
    return {"preview": data}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("document", help="document name (generic_confirm_dialog) or .rml path")
    parser.add_argument("--theme", default="legacy", help="theme folder under RmlUi/themes (default: legacy)")
    parser.add_argument("--model", help="JSON file with the data model, or {\"states\": {...}}")
    parser.add_argument("--window", default="1024x768", help="window size WIDTHxHEIGHT (default: 1024x768)")
    parser.add_argument("--ui-scale", type=int, default=100, help="the UI scale option in percent (default: 100)")
    parser.add_argument("--out", default=str(REPO_ROOT / "build/rml-preview"), help="output directory (default: build/rml-preview)")
    parser.add_argument("--png", action="store_true", help="also screenshot each page with a Chromium-based browser")
    parser.add_argument("--pixel-ratio", type=float, default=1.0, help="device pixel ratio of the screenshots")
    parser.add_argument("--browser", help="path of Chrome, Edge or Chromium (default: searched)")
    parser.add_argument("--background", default="#14110d", help="CSS background behind the document")
    args = parser.parse_args()

    match = re.fullmatch(r"(\d+)x(\d+)", args.window)
    if not match:
        parser.error("--window must be WIDTHxHEIGHT")
    window = int(match.group(1)), int(match.group(2))
    out_dir = pathlib.Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    try:
        pages, ratio, rem = render(args.document, args.theme, load_states(args.model), window, args.ui_scale,
                                   out_dir, args.background)
    except (OSError, ValueError, ET.ParseError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    print(f"{window[0]}x{window[1]}: 1dp = {ratio:g}px, 1rem = {rem:g}px")
    for page in pages:
        print(page)

    if args.png:
        browser = find_browser(args.browser)
        if browser is None:
            print("error: no Chromium-based browser found; pass --browser", file=sys.stderr)
            return 1
        for page in pages:
            png = page.with_suffix(".png")
            try:
                screenshot(browser, page, png, window[0], window[1], args.pixel_ratio)
            except (OSError, subprocess.SubprocessError) as error:
                print(f"error: screenshot failed: {error}", file=sys.stderr)
                return 1
            print(png)
    return 0


if __name__ == "__main__":
    sys.exit(main())
