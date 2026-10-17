# Light mood

The light mood darkens the even, baked light of each world map, so candles,
street lights, bonfires, torches and skill lights stand out. Their own
light keeps its strength; only the light the map is baked with gets darker.
It is part of the Enhanced look, at full strength, and off in
Classic ([enhanced-look.md](enhanced-look.md)).

## Turning it on

Players switch it with the look: the "Enhanced graphics" checkbox on the
options window's Video tab, `$look enhanced|classic` or `[Render] Look` in
`config.ini`.
It takes effect on the next frame, without a map reload. The login and
character selection scenes, the UI and effects that carry their own light are
never changed.

Developer builds (control socket or DevEditor) also have, for the session
only and cleared by choosing a look: `$lightmood on|off` and
`$lightmood <0-100>` (the strength: 0 looks Classic, 100 applies the map's
full mood, values between blend), and `$lightmood knee <k>` (the look
profile's `lighting.highlightKnee`).

## The moods: `Data/Maps/LightMood.json`

```json
{
  "formatVersion": 1,
  "default": { "base": 0.45, "figures": 0.7, "dynamicGain": 1.7, "background": 1.0, "sun": 0.8,
               "shadowKeep": 0.8, "brightScale": 0.8, "shadeCool": 0.1 },
  "maps": [ { "world": 7, "name": "Atlans", "base": 0.8, "figures": 0.8, "brightScale": 1.0, "sun": 1.0 } ]
}
```

`default` applies to every map without its own entry and has the five required values.
A map entry names its world number and overrides any of them; `name` is only
for the reader.

| Value | Scales | Range |
|---|---|---|
| `base` | the baked light of the terrain and of the map objects lit by it | 0.2 to 1 |
| `figures` | the baked light on characters, monsters and NPCs, and the fixed light of the player's own character | `base` to 1 |
| `dynamicGain` | the light candles, fires, skills and effects add each frame | 0.5 to 2 |
| `background` | the clear and fog colour behind the scene | 0 to 1 |
| `sun` | how much brighter a model face turned to the sun is than one turned away (1 is Classic, 0 leaves only the even fill light) | 0 to 1 |
| `shadowKeep` | optional: the baked light at which a place keeps its full brightness; `base` and `figures` then darken the shade only, fading out towards this light (sunlit ground stays day). 0 (the default) darkens everything evenly | 0 to 1.5 |
| `brightScale` | optional, with a shadow keep: the scale places at the shadow keep get, never below `base` (figures: never below `figures`) | 0.3 to 1 (default) |
| `shadeCool` | optional: how much bluer the darkened shade gets | 0 (default) to 0.5 |

One value out of its range, a broken file or a missing one makes the client
log every problem once (`render` log) and keep the whole world Classic; it
never applies part of a file.

The shipped values come from side-by-side review rounds over eleven scenes and
a survey of 26 places: base 0.45 with the shadow keep at
0.8, so the shade darkens while sunlit ground keeps most of its light
(`brightScale` 0.8), figures at 0.7 so characters stay readable, a slightly
cooler shade, and dynamic lights 1.7 times as strong. Maps that had a weaker
sun (indoors, underground, event maps) light models from the sun at 1.0. Maps that looked too dark
have their own, lighter values (Atlans, Icarus, Raklion, Vulcanus, Karutan);
Icarus draws no terrain, so its `background` of 0.7 darkens it instead.

## How the light is computed

Classic builds each cell's static light once per map load from the baked
light image and the slope towards the sun; every frame it copies that into
the frame's light and adds the dynamic lights on top, each fading out over a
few cells. Characters, monsters and NPCs take the frame's light under their
feet; lit map objects do too, self-lit ones take 10 % of it.

With the mood, at strength `s` each value `v` of the map's mood becomes
`1 + s × (v − 1)` (so strength 0 is exactly Classic), and then:

- the static light of each cell is `slope × baked × base`: built once per map
  load and again when the setting changes (well under a millisecond);
- a dynamic light adds `gain ×` its Classic amount; darkening lights add their
  Classic amount;
- a figure gets the frame's light plus `(figures / base − 1) ×` the static
  light under it, which turns its static part into `figures × static` and
  leaves its dynamic part as is; at `figures = base` nothing is added;
- the player's own character, which Classic lights with a fixed white light
  (tinted by its ability buffs) instead of the terrain, gets that light
  times `figures`. It still does not brighten near candles, as in Classic;
- the map's clear and fog colour are multiplied by `background`;
- models: Classic lights a model face with `max(0.8 × dot(n, L) + 0.4, 0.2)`
  times its light, with one fixed light vector `L = (1.3, 0, 2)` on every map,
  so a face pointing up gets twice its light and is cut off at full
  brightness. The mood multiplies `L` by `sun` (a face pointing up gets
  `1.6 × sun + 0.4`), and a lit model colour `v` above `k = 0.8` becomes
  `k + (1 − k)(1 − e^(−(v − k)/(1 − k)))`: it approaches full brightness
  without reaching it, so the tops of buildings, rocks and figures keep their
  shading instead of turning flat white. At strength 0 the knee is 1, the
  Classic cut-off.

Because the dynamic lights are added after the darkening, a candle's pool on
the floor is as bright above the darker ground as it was in Classic.
