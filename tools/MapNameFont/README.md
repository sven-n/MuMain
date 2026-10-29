# Map-name font generator

Builds `src/bin/fonts/MuMapName-Italic.otf`, a font of the client's map-name lettering: the silver
italic names the original client showed as one image per map and language
(`Data/Local/<language>/ImgsMapName`). With it the map banner can write any name in any language
in that lettering, sharp at every size. `ornament.py` cuts the ornament under the names into
separate images (see [The ornament](#the-ornament)).

## Run it

```bash
python -m venv .venv
.venv/Scripts/pip install -r tools/MapNameFont/requirements.txt
.venv/Scripts/python tools/MapNameFont/generate.py --data "<MU client>/Data" --out src/bin/fonts/MuMapName-Italic.otf --preview out/mapfont-preview
```

The packages are pinned to the versions that built the shipped font: with them the generator
writes the same font again (only the `head` table's creation time differs), so a one-glyph fix
gives a diff of that glyph. Raise them on purpose, together with a regenerated font.

`--data` is the client's `Data` folder; only `Local/Eng/ImgsMapName` is read. `--preview` writes a
page with every glyph, map names in several languages and each original image beside the same
name in the font (serve the folder with `python -m http.server`, or open `index.html`).

## How the letters are made

1. **Cut out** (`letters.py`). 24 of the 34 English images use the italic lettering. Each is
   straightened by the lettering's slant. The silver fill fades from white to grey, so its edge is
   found per pixel: halfway between the dark edge and the fill around it. The fill is cut into one
   piece per letter of the known name: column ranges give each letter a core, and the rest of the
   ink goes to the core it is connected to, so swashes under the next letter (K) and arms over it
   (V) stay with their letter. Long names were squeezed to fit the image; their letters are
   widened back.
2. **Averaged**. Every sample of a letter is scaled to one x-height, enlarged, lined up, and the
   samples that agree are averaged, which smooths out the source's pixel steps.
3. **Completed** (`recipes.py`). The images hold only the capitals A–F, I, K, L, N, S, T, V and the
   small letters except j and x. The rest are put together from parts of the traced letters or
   drawn with a broad-nib pen calibrated to their stroke widths (`shapes.py`):
   - P is B's stem and upper bowl; R adds K's leg; H is K's stem twice with a bar; F is E
     without its bottom arm (its only source is cut badly); k gets back its hairline swing under
     the next letter, too thin to cut out.
   - O, U, W, Y, Z enlarge o, u, w, y, z; G adds a spur to C; Q a tail to O.
   - j and J take i and I with a pen descender; x, M and the figures are drawn.
   - q is a with a drawn descender: its only source is a squeezed name whose neighbours run into
     it. y, from squeezed names too, has its wavy edges evened out. o gets its own ellipse as
     outline (averaged, its small flick made a lumpy shoulder); g and s are smoothed.
   - Where a drawn stroke carries on a traced one (P's bowl, k's leg, the bowl of Ь), it starts
     on the traced stroke's centre line and as wide, so the two meet without a step.
   - Accents, Polish and Latin-1 letters, punctuation, and Russian/Ukrainian Cyrillic in its
     italic forms (и is u, п is n, т is m).
   Chinese and Japanese stay on the bundled Noto fallback.
4. **Written** (`font_writer.py`). Each glyph is traced into curves (potrace) and slanted back to
   the original italic. Spacing: every pair of touching letters in the plain names gives a gap,
   and each letter's own room left and right is fitted to all of them (K, N and I have more on
   their right, joined letters like n and r less). Accented and Cyrillic letters take their base
   letter's room, and every letter gets a little extra.

Change a glyph in `recipes.py`, run the generator and look at the preview.

## The ornament

Under each name the original images have an ornament: a swirl on the left, one on the right, and
the map's icon between them (a leaf for Lorencia, snowflakes for Devias). The client has no
separate files for them; `ornament.py` cuts them out of the images:

```bash
.venv/Scripts/python tools/MapNameFont/ornament.py --data "<MU client>/Data" --out src/bin/Data/Interface/MapNameOrnament --preview out/ornament-preview
```

It writes `SwirlLeft.OZT`, `SwirlRight.OZT`, one icon per image named like the image
(`lorencia.OZT`), and `Layout.json` with where each piece sat in the 166 x 90 image, so the
original banner can be put back together. The game reads them as `.tga`, like its other OZT files.
The map banner is built from them; `MapIcons.json` beside them, kept by hand, says which icon
each map shows.

- **Swirls.** Every image has the same two swirls; a few exports differ only in their dark edge
  (Devias softer; Dungeon, Icarus, Tarkan, Balgass and Swamp of Peace harder in some languages;
  the English Raklion boss without it).
  They are taken from the export most images share (the Portuguese Lorencia). The right swirl is
  the left one's shape mirrored, but its shading is not, so it is a piece of its own.
- **Icons.** The icon is the same in the English, Portuguese and Spanish image of a map, and the
  name is not: per pixel the value of the language closest to the others is kept, so one
  language's letters over a pixel are outvoted. Where the name and the icon still touch (two
  languages share a name, or a descender reaches the icon), the silver fill decides: a fill piece
  that starts below the name rows and reaches the swirls belongs to the icon, the outline and glow
  go with the fill they are connected to, and a language's own letters only claim pixels where
  that language shows the shared image. Where an icon overlaps a swirl, it keeps the pixels it
  changes.

Maps that share an icon share a file's content (both Kalimas, Raklion and its boss, the two
Balgass maps).
