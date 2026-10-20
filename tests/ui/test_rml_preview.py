#!/usr/bin/env python3
"""Tests of tools/rml_preview.py: its scaling, expressions, RCSS translation and a whole document."""

import pathlib
import re
import struct
import sys
import tempfile
import unittest

ROOT = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else pathlib.Path(__file__).resolve().parents[2]
sys.dont_write_bytecode = True
sys.path.insert(0, str(ROOT / "tools"))
import rml_preview  # noqa: E402


def evaluate(text, **scope):
    return rml_preview.Expression(text).evaluate(scope)


class ScalingTest(unittest.TestCase):
    def test_dp_ratio_fits_the_reference_screen_up_to_the_panel_cap(self):
        self.assertAlmostEqual(rml_preview.dp_ratio(640, 480), 1.0)
        self.assertAlmostEqual(rml_preview.dp_ratio(1024, 768), 1.6)
        self.assertAlmostEqual(rml_preview.dp_ratio(1920, 1080), 2.0)
        self.assertAlmostEqual(rml_preview.dp_ratio(320, 240), 1.0)

    def test_ui_scale_grows_only_while_the_reference_screen_fits(self):
        self.assertAlmostEqual(rml_preview.dp_ratio(1024, 768, 150), 1.6)
        self.assertAlmostEqual(rml_preview.dp_ratio(1280, 960, 120), 2.0)

    def test_native_text_grows_with_the_typography_scale(self):
        self.assertEqual(rml_preview.native_text_size(1.0), 11)
        self.assertEqual(rml_preview.native_text_size(1.6), 13)
        self.assertEqual(rml_preview.native_text_size(2.25), 16)

    def test_scene_window_root_size_is_the_text_growth_since_1024x768(self):
        self.assertEqual(rml_preview.root_font_sizes(["scene-window-scale"], True, 1024, 768, 100), (1.0, 13))
        self.assertEqual(rml_preview.root_font_sizes(["native-text"], True, 1024, 768, 100), (13, None))
        self.assertEqual(rml_preview.root_font_sizes(["native-text"], False, 1024, 768, 100),
                         (rml_preview.RMLUI_DEFAULT_FONT_SIZE, None))


class ExpressionTest(unittest.TestCase):
    def test_logic_and_ternary(self):
        self.assertTrue(evaluate("!a && b || c", a=False, b=True, c=False))
        self.assertFalse(evaluate("a && b", a=True, b=False))
        self.assertEqual(evaluate("line.color == '' ? '#ffffff' : line.color", line={"color": ""}), "#ffffff")
        self.assertEqual(evaluate("line.color == '' ? '#ffffff' : line.color", line={"color": "#ff0000"}), "#ff0000")

    def test_arithmetic_and_string_concatenation(self):
        self.assertEqual(rml_preview.to_text(evaluate("fraction * 100 + '%'", fraction=0.4)), "40%")
        self.assertEqual(evaluate("top + 'dp'", top=12), "12dp")
        self.assertEqual(evaluate("(a + 2) * 3", a=1), 9)

    def test_members_indices_and_size(self):
        self.assertEqual(evaluate("items[1].name", items=[{"name": "a"}, {"name": "b"}]), "b")
        self.assertEqual(evaluate("items.size", items=[1, 2, 3]), 3)

    def test_transforms(self):
        self.assertEqual(evaluate("value | format(2)", value=1.5), "1.50")
        self.assertEqual(evaluate("value | to_upper", value="ok"), "OK")

    def test_missing_names_evaluate_to_nothing(self):
        self.assertIsNone(evaluate("missing_name"))
        self.assertEqual(rml_preview.to_text(evaluate("missing_name")), "")


class StylesheetTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        textures = rml_preview.Textures(pathlib.Path(self.temp.name))
        self.converter = rml_preview.StylesheetConverter([], {"accent": "#ffd23c"}, 2.0, textures)
        self.converter.rem = 13

    def tearDown(self):
        self.temp.cleanup()

    def test_units_tokens_colors_and_borders(self):
        css = self.converter.convert_values("width: 10dp; font-size: 1rem; color: token(accent); "
                                            "background-color: rgba(0, 0, 0, 255); border: 1dp #4a3620;")
        self.assertIn("width: 20px", css)
        self.assertIn("font-size: 13px", css)
        self.assertIn("color: #ffd23c", css)
        self.assertIn("rgba(0, 0, 0, 1.000)", css)
        self.assertIn("border: 2px solid #4a3620", css)

    def test_gradient_decorator_becomes_a_background_layer(self):
        css = self.converter.convert(".a { decorator: linear-gradient(to right, #000, #fff); }", ROOT)
        self.assertIn("background-image: linear-gradient(to right, #000, #fff)", css)


def tga(rgba):
    """A 1x1 uncompressed 32-bit TGA of one RGBA pixel."""
    header = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, 1, 1, 32, 0)
    red, green, blue, alpha = rgba
    return header + bytes((blue, green, red, alpha))


class ParseTest(unittest.TestCase):
    def test_attributes_without_a_value_parse_as_empty(self):
        root = rml_preview.parse_rml(
            '<rml><body><input type="text" autofocus/><div data-covers-world class="a">x</div>'
            '<div data-if="a > b && c < d" hidden></div></body></rml>')
        body = root.find("body")
        self.assertEqual(body[0].attrib, {"type": "text", "autofocus": ""})
        self.assertEqual(body[1].attrib, {"data-covers-world": "", "class": "a"})
        self.assertEqual(body[2].get("data-if"), "a > b && c < d")
        self.assertEqual(body[2].get("hidden"), "")

    def test_documents_with_attributes_without_a_value_render(self):
        for document in ("char_make", "workspace"):
            for theme in ("legacy", "modern"):
                with self.subTest(document=document, theme=theme), tempfile.TemporaryDirectory() as temp:
                    pages, _, _ = rml_preview.render(document, theme, rml_preview.load_states(None), (1024, 768),
                                                     100, pathlib.Path(temp), "#000")
                    self.assertEqual(len(pages), 1)


class BindingTest(unittest.TestCase):
    def bind(self, rml, models):
        root = rml_preview.parse_rml(rml)
        body = root.find("body")
        rml_preview.DocumentRenderer(models, lambda src: None, lambda style: style).bind(body, dict(models), root)
        return body

    def test_a_loop_naming_only_its_item_still_has_it_index(self):
        body = self.bind('<rml><body><div data-for="label : labels" data-class-selected="it_index == chosen">'
                         '{{label}}</div></body></rml>', {"labels": ["a", "b", "c"], "chosen": 1})
        self.assertEqual([(div.text, div.get("class")) for div in body], [("a", None), ("b", "selected"), ("c", None)])

    def test_a_document_with_an_unbound_image_renders(self):
        with tempfile.TemporaryDirectory() as temp:
            pages, _, _ = rml_preview.render("map_name", "legacy", rml_preview.load_states(None), (1024, 768), 100,
                                             pathlib.Path(temp), "#000")
            self.assertEqual(len(pages), 1)


class TextureTest(unittest.TestCase):
    def test_a_directory_is_not_a_texture(self):
        with tempfile.TemporaryDirectory() as temp:
            (pathlib.Path(temp) / "images").mkdir()
            self.assertIsNone(rml_preview.find_texture(pathlib.Path(temp) / "images"))

    def test_a_sprite_is_cropped_again_from_a_changed_sheet(self):
        with tempfile.TemporaryDirectory() as temp:
            sheet, out = pathlib.Path(temp) / "sheet.tga", pathlib.Path(temp) / "out"
            sheet.write_bytes(tga((255, 0, 0, 255)))
            first = (out / rml_preview.Textures(out).crop(sheet, (0, 0, 1, 1))).read_text()
            sheet.write_bytes(tga((0, 0, 255, 255)))
            second = (out / rml_preview.Textures(out).crop(sheet, (0, 0, 1, 1))).read_text()
            self.assertNotEqual(first, second)

    def test_bottom_up_tga_is_flipped_to_top_down_rgba(self):
        header = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, 1, 2, 32, 0)
        bottom, top = bytes((0, 0, 255, 255)), bytes((255, 0, 0, 128))  # BGRA: red, then blue
        width, height, rows = rml_preview.decode_tga(header + bottom + top)
        self.assertEqual((width, height), (1, 2))
        self.assertEqual(rows, [bytes((0, 0, 255, 128)), bytes((255, 0, 0, 255))])

    def test_png_has_the_signature_and_size(self):
        png = rml_preview.encode_png(2, 1, [bytes(8)])
        self.assertTrue(png.startswith(b"\x89PNG\r\n\x1a\n"))
        self.assertEqual(struct.unpack_from(">II", png, 16), (2, 1))


class DocumentTest(unittest.TestCase):
    def test_example_model_renders_in_every_theme(self):
        states = rml_preview.load_states(ROOT / "tools/rml_preview_models/generic_confirm_dialog.json")
        for theme in ("legacy", "modern"):
            with self.subTest(theme=theme), tempfile.TemporaryDirectory() as temp:
                out = pathlib.Path(temp)
                pages, _, _ = rml_preview.render("generic_confirm_dialog", theme, states, (1024, 768), 100, out,
                                                 "#000")
                self.assertEqual(sorted(p.stem for p in pages), sorted(states))
                page = (out / "three_buttons.html").read_text(encoding="utf-8")
                self.assertIn("Decrease", page)
                self.assertIn("Strength +1", page)
                self.assertNotIn("{{", page)
                self.assertNotIn("data-", page)
                css = (out / "preview.css").read_text(encoding="utf-8")
                self.assertNotIn("dp;", css)
                self.assertNotIn("token(", css)
                for url in re.findall(r"url\((img/[^)]+)\)", css):
                    self.assertTrue((out / url).is_file(), url)


if __name__ == "__main__":
    unittest.main(argv=sys.argv[:1])
