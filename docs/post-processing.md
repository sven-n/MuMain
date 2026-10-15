# Post-processing

Post-processing works on the finished 3D scene before the interface is drawn on
top of it. It adds two things to the Classic look:

- **Bloom**: spells, fires, torches and portals spread a soft halo around
  themselves. Only effects drawn as light (additive blending) glow; bright
  surfaces such as sky or snow do not. Characters - players, monsters, NPCs,
  their equipment, wings, pets and mounts - and items on the ground never glow:
  they look exactly as without post-processing, however shiny their gear. The
  halo is screen-blended, so it lights dark surroundings without washing bright
  ones out to white, and it stays close to its effect.
- **Colour grading**: a map can carry its own colour mood through a lookup
  table. A map without a table is not graded. A table never makes anything
  brighter and leaves highlights (armour, glows, light pools) untouched; it
  cools and deepens shadows.

The HUD, windows, text, the cursor and item previews in windows are never
processed: they look exactly as without post-processing.

Post-processing is part of the Enhanced look and off in Classic
([enhanced-look.md](enhanced-look.md)). With it on, a bloom strength of 0 and
no grading table, the frame looks the same as with it off.

## Settings

Players do not set post-processing on its own: the `Look` preset switches it.
Its strengths are values of the look profile, `Data/Render/Look.json`:
`bloom.strength` (0 to 2, shipped 1.0, 0.3 in Icarus), `bloom.levelFalloff`
(how wide the halo spreads) and `grading.strength` (how much of a map's table
applies).

If the graphics device cannot build post-processing, the Enhanced look logs a
warning and the frame stays unprocessed.

Developer builds (control socket or DevEditor) have, for the session only:
`$post on|off` (post-processing alone, cleared by choosing a look),
`$bloom <value>` and `$bloom falloff <value>` (the look profile's values), and
for diagnosis `$post view source` (only what feeds the bloom: the additive
effects, black elsewhere), `$post view bloom` (the blurred glow) and
`$post view scene` (back). The views need post-processing on and a bloom
strength above 0.

## Grading tables

A map's table is `Data/Grading/World<N>.png`, where `N` is the map number plus
one (Lorencia, map 0, is `World1.png`; Dungeon is `World2.png`). The client
loads it when the map is entered and logs which table it uses, or that the map
has none. A file that cannot be used is logged with the reason and the map is
not graded.

A table is a 1024×32 PNG: a 32×32×32 colour cube cut into 32 slices of 32×32
placed side by side. Inside a slice red runs left to right and green top to
bottom; the slice number is blue. Each entry holds the colour that the colour
at its position becomes.

`scripts/grading_table.py identity <file>` writes the table that changes
nothing. To make a new table, paste that strip into a screenshot of the map,
grade the whole image in an image editor, and cut the strip out again. A
table must never brighten a colour and must leave colours above 60 %
brightness as they are; `scripts/grading_table.py` refuses to write one that
does. Strong darkening belongs to the map's lighting, not its grading. The
shipped `World2.png` (Dungeon: colder, slightly desaturated and a little deeper
shadows, mid-tones and highlights unchanged) comes from
`scripts/grading_table.py dungeon`.
