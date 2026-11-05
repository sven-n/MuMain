"""A page that shows the generated font: every glyph, map names in several languages styled like
the banner, and each original image next to the same name in the font. Serve the folder over HTTP
(python -m http.server) or open index.html in a browser."""
import html
import shutil

from letters import SOURCES
from ozt import over, read_ozt, write_png

GLYPHS = ("ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789 .,:;!?¡¿'\"‘’“”‚„-–—…()/ "
          "ÀÁÂÃÄÅÇÈÉÊËÌÍÎÏÑÒÓÔÕÖØÙÚÛÜÝ àáâãäåçèéêëìíîïñòóôõöøùúûüýÿß ĄĆĘŁŃŚŹŻČŠŽ ąćęłńśźżčšž "
          "АБВГҐДЕЁЄЖЗИІЇЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ абвгґдеёєжзиіїйклмнопрстуфхцчшщъыьэюя")
SAMPLES = [
    "Lorencia · Devias · Noria · Lost Tower · Kanturu Relics · GM Summon Zone · Balgass' Resting Place",
    "Verlorener Turm · Brüterei · Doppelgänger · Ruhestätte",
    "Torre perdida · Plaza del Diablo · Ícaro · La Cleón",
    "Praça do Diabo · Terra das Provações · Pântano da Paz",
    "Zaginiona Wieża · Kraina Prób · Miejsce spoczynku Balgassa",
    "Руины Кантуру · Подземелье · Искар · Крепость Криволка",
    "Руїни Кантуру · Підземелля · Загублена вежа · Інкубаторій",
]
STYLE = """
body { background: #181a1f; color: #ddd; font-family: sans-serif; margin: 12px; }
h3 { margin: 14px 0 2px; color: #888; font-size: 13px; }
td { padding: 2px 14px; vertical-align: middle; }
p { margin: 0; line-height: 1.25; }
.s46 { font-size: 46px; } .s80 { font-size: 80px; } .s100 { font-size: 100px; }
.plain, .silver { font-family: MuMapName, sans-serif; }
.plain { color: #fff; }
.silver { background: linear-gradient(#ffffff 15%, #8d8d8d 75%, #c9c9c9); -webkit-background-clip: text;
  background-clip: text; color: transparent;
  filter: drop-shadow(0 0 1px #000) drop-shadow(0 0 1px #000) drop-shadow(2px 2px 1px #000); }
img.x2 { width: 332px; image-rendering: pixelated; }
"""


def write_preview(folder, font_path, local_dir):
    folder.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(font_path, folder / "font.otf")
    rows = []
    for stem, text, _ in SOURCES:
        path = next((local_dir / "Eng" / "ImgsMapName").glob(stem + ".*"))
        write_png(folder / f"{stem}.png", over(read_ozt(path), (24, 26, 31)))
        rows.append(f'<tr><td><img class="x2" src="{stem}.png"></td>'
                    f'<td class="silver s100">{html.escape(text)}</td></tr>')
    samples = "".join(f'<p class="silver s46">{html.escape(line)}</p>' for line in SAMPLES)
    page = (f"<!doctype html><meta charset=utf-8><title>Map name font</title>"
            f"<style>@font-face {{ font-family: MuMapName; src: url(font.otf); }}{STYLE}</style>"
            f'<p class="plain s80">{html.escape(GLYPHS)}</p><h3>map names</h3>{samples}'
            f"<h3>originals (left, 2x) and the font</h3><table>{''.join(rows)}</table>")
    (folder / "index.html").write_text(page, encoding="utf-8")
