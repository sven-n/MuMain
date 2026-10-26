# Map-name font generator

Builds `src/bin/fonts/MuMapName-Italic.otf`, a font of the client's map-name lettering: the silver
italic names the original client showed as one image per map and language
(`Data/Local/<language>/ImgsMapName`). With it the map banner can write any name in any language
in that lettering, sharp at every size.

## Run it

```bash
python -m venv .venv
.venv/Scripts/pip install -r tools/MapNameFont/requirements.txt
.venv/Scripts/python tools/MapNameFont/generate.py --data "<MU client>/Data" --out src/bin/fonts/MuMapName-Italic.otf --preview out/mapfont-preview
```

`--data` is the client's `Data` folder; only `Local/Eng/ImgsMapName` is read. `--preview` writes a
page with every glyph, map names in several languages and each original image beside the same
name in the font (serve the folder with `python -m http.server`, or open `index.html`).

## How the letters are made

1. **Cut out** (`letters.py`). 24 of the 34 English images use the italic lettering. Each is
   straightened by the lettering's slant, the silver fill is separated from the dark edge by
   brightness, and the fill is cut into one piece per letter of the known name. Long names were
   squeezed to fit the image; their letters are widened back.
2. **Averaged**. Every sample of a letter is scaled to one x-height, enlarged, lined up, and the
   samples that agree are averaged, which smooths out the source's pixel steps.
3. **Completed** (`recipes.py`). The images hold only the capitals A–F, I, K, L, N, S, T, V and the
   small letters except j and x. The rest are put together from parts of the traced letters or
   drawn with a broad-nib pen calibrated to their stroke widths (`shapes.py`):
   - P is B's stem and upper bowl; R adds K's leg; H is K's stem twice with a bar.
   - O, U, W, Y, Z enlarge o, u, w, y, z; G adds a spur to C; Q a tail to O.
   - j and J take i and I with a pen descender; x, M and the figures are drawn.
   - Accents, Polish and Latin-1 letters, punctuation, and Russian/Ukrainian Cyrillic in its
     italic forms (и is u, п is n, т is m).
   Chinese and Japanese stay on the bundled Noto fallback.
4. **Written** (`font_writer.py`). Each glyph is traced into curves (potrace), slanted back to the
   original italic and spaced like the originals.

Change a glyph in `recipes.py`, run the generator and look at the preview.
