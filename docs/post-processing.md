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

The `Look` preset switches post-processing with the rest of the Enhanced look,
and the options window's "Bloom and colour grading" row switches it off on its
own. Its strengths are values of the look profile, `Data/Render/Look.json`:
`bloom.strength` (0 to 2, shipped 1.0, 0.3 in Icarus), `bloom.levelFalloff`
(0.25 to 1, how wide the halo spreads) and `grading.strength` (0 to 1, how much
of a map's table applies). A grading strength of 0 applies no grading at all
while the bloom stays on.

Players tune the three under that row (Bloom intensity, Bloom spread, Colour
grading strength, the last in percent); see "Tuning it yourself" in
[enhanced-look.md](enhanced-look.md). A value a player changed applies on every
map in place of the map's value until it is reset.

If the graphics device cannot build post-processing, the Enhanced look logs a
warning and the frame stays unprocessed.

Developer builds (control socket or DevEditor) have, for the session only:
`$post on|off` (post-processing alone, cleared by choosing a look),
`$bloom <value>` and `$bloom falloff <value>` (the look profile's values), and
for diagnosis `$post view source` (only what feeds the bloom: the additive
effects, black elsewhere), `$post view bloom` (the blurred glow) and
`$post view scene` (back). The views need post-processing on and a bloom
strength above 0.

## Edge smoothing

Edge smoothing (FXAA) is the last pass over the processed scene, after bloom,
grading and the HDR tone map and before the interface. It looks for edges by
the contrast in brightness, follows each edge to where it ends and blends
across it, and it softens single pixels that are much brighter or darker than
their neighbours. Multisampling smooths only the edges of triangles; edge
smoothing also catches what is drawn inside a surface: highlights on metal
armour, rim light and fine texture that shimmer while the camera moves. The
price is a little sharpness in fine texture.

It belongs to the Enhanced look and is off unless the player ticks "Edge
smoothing (FXAA)" under the Anti-aliasing row, or types `$fxaa on`
(`[Render] EdgeSmoothing` in `config.ini`). It works with any multisample count,
including none, and with bloom and colour grading switched off: the frame then
takes the post-processed path with a composite that changes nothing, so it
looks as without post-processing apart from the smoothed edges. The diagnostic
views (`$post view source` and `$post view bloom`) are shown unsmoothed.

The pass reads one full-size image in the frame's format, through one sampler,
so it adds a single sampler slot to the frame whatever the scene holds. If the
graphics device cannot build it, the log says so and frames stay unsmoothed.

## Render scale and sharpening

The render scale draws the 3D scene at a share of the window's size, from 50 to
100 percent, and upscales it to the window before the interface, which stays at
the window's size and as sharp as before. The upscale is AMD's FidelityFX Super
Resolution 1 edge-adaptive upsampling (EASU), and the sharpening its contrast
adaptive sharpening (RCAS); both are ported from `ffx_fsr1.h` (MIT licence) into
`post_fsr_easu.frag` and `post_fsr_rcas.frag`. Sharpening also works at 100
percent, on its own.

After the composite, the passes run in this order, each reading the image the
one before drew:

| Pass | Runs when | Size |
|---|---|---|
| Edge smoothing (FXAA) | ticked | the render size |
| Upscale (EASU) | the render scale is below 100 percent | from the render size to the window's |
| Sharpening (RCAS) | the sharpness is above 0 | the window's size |

The last of them draws inside the interface pass, the others into images of
their own. Each is one full-screen draw with one texture and one sampler, so
each adds one sampler slot to the frame.

At a render scale below 100 percent, what the scene draws into is that size:
the scene texture and its multisampled colour target, the scene's depth, the
bloom source and the bloom chain, the ground shadow mask, the ambient occlusion
and the composite. The viewports and scissors recorded for the window are
scaled to it while the scene's passes replay. The sun shadow map keeps its own
resolution, and the interface pass, RmlUi and the grading table stay at the
window's size. The interface then starts from a cleared depth of its own, since
the scene's depth is smaller: a 3D draw after the scene (an item preview, say)
no longer hides behind the world below 100 percent.

Both are player settings of the Enhanced look, like the anti-aliasing:
`[Render] RenderScale` (50 to 100, default 100) and `Sharpness` (0 to 100,
default 0) in `config.ini`, the Render scale and Sharpness rows under
Anti-aliasing in the options window, and `$renderscale <50-100>` and
`$sharpness <0-100>` in the chat box. At 100 percent and a sharpness of 0
nothing changes. Classic renders at the window's size without sharpening
whatever they say. If the graphics device cannot build a pass, the log says so
and frames go without it.

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
