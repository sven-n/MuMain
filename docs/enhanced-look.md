# Classic and Enhanced look

The client has two looks. **Classic** (the default) draws the game as it
always looked. **Enhanced** is a preset that switches on five rendering
features and 4x multisample anti-aliasing together. Players choose a look, not values: the values of the
Enhanced preset ship with the client in two data files, and every piece of it
can be tuned there, also per map, without touching the code (see "Tuning the
Enhanced preset" below).

| Feature | What it does | Guide |
|---|---|---|
| Texture filtering | mipmaps and anisotropic filtering for world textures | — |
| Enhanced lighting | per-pixel model lighting with rim light and, on metal armour, specular highlights | — |
| Dynamic model lights | candles, fires and spells light characters, monsters and NPCs | — |
| Light mood | darker baked map light, so the light sources stand out | [light-mood.md](light-mood.md) |
| Post-processing | bloom around light effects and per-map colour grading | [post-processing.md](post-processing.md) |
| Anti-aliasing | multisampled edges (4 samples, or what `Antialiasing` asks for), alpha-to-coverage for cut-out foliage and fences | — |

## Choosing the look

- **Options window**: the "Enhanced graphics" checkbox on the Graphics tab. Under
  it, Enhanced lighting, Dynamic lights, Darker map light (the light mood) and
  Bloom and colour grading switch those features of Enhanced off one by one;
  dynamic lights need Enhanced lighting. Ticked
  is Enhanced, unticked Classic; it applies at once and is saved.
- **Chat box**: `$look enhanced` and `$look classic`, in every build; saved too.
- **`config.ini`**, section `[Render]`:

  | Key | Values | Default |
  |---|---|---|
  | `Look` | `Enhanced` or `Classic` | `Classic` |
  | `Antialiasing` | samples of the Enhanced look: `0` (off), `2`, `4` or `8` | `4` |
  | `Anisotropy` | maximum anisotropic filtering, clamped to the GPU | `8` |
  | `EnhancedLighting`, `DynamicLights`, `LightMood`, `PostProcessing` | `0` switches that feature of Enhanced off; Classic ignores them | `1` |

  A missing or unknown `Look` counts as Classic, so the classic look stays
  until a player ticks "Enhanced graphics". Classic renders without
  anti-aliasing whatever `Antialiasing` says; Enhanced uses its count, lowered
  to what the GPU supports. The options window's Anti-aliasing row, or
  `$msaa off|2|4|8` in the chat box, changes and saves the count (on a slow GPU,
  Off keeps Enhanced without anti-aliasing). The Anisotropic filtering row sets
  `Anisotropy`.
  The anisotropy level applies to every texture with mip levels: world
  textures in Enhanced, and upscaled textures (an optional asset pack) in both
  looks, because they are always drawn smaller than they are stored.

The login and character selection scenes take the Enhanced features too,
except the light mood. The interface is the same in both looks.

## Tuning the Enhanced preset

Enhanced is a bundle of pieces that are tuned independently; nothing about
its look is fixed in the code. To change how Enhanced looks:

| To change… | Edit | Guide |
|---|---|---|
| model lighting (diffuse curve, rim, specular, highlight roll-off) | `lighting.*` in `Data/Render/Look.json` | below |
| how candles, fires and spells light models | `dynamic.*` in `Data/Render/Look.json` | below |
| bloom and colour grading | `bloom.*`, `grading.*` in `Data/Render/Look.json`; grading tables in `Data/Grading/` | [post-processing.md](post-processing.md) |
| how dark each map's baked light is | `Data/Maps/LightMood.json` | [light-mood.md](light-mood.md) |
| which armour shines | `material.specular` in the item-model JSON (`Data/Items/Models/`) | below |

Every value can be set for the whole game or for one map. Developer builds
change any value live and save it back to the files (`$look set`, `$look
save`, or the DevEditor's Look panel; see "Developer commands"), so a change
can be judged in the game before it is kept. A server or a player who wants a
different Enhanced ships different data files; the code stays the same.

## The Enhanced values: `Data/Render/Look.json`

The look profile holds the tuning values of the features; the light mood's
values are in `Data/Maps/LightMood.json` ([light-mood.md](light-mood.md)).
Classic uses neither file.

```json
{
  "formatVersion": 1,
  "default": { "bloom.strength": 1.0, "dynamic.effectCap": 2.5 },
  "maps": [ { "world": 10, "name": "Icarus", "bloom.strength": 0.3 } ]
}
```

`default` applies everywhere; a map entry names its world number and
overrides any value for that map (`name` is for the reader). A value left out
keeps its built-in value. One value out of its range or a broken file makes
the client log every problem once (`render` log) and use the built-in values
for the whole file.

| Value | Shipped | Built-in | Range | Meaning |
|---|---|---|---|---|
| `lighting.diffuse.scale` | 1.0 | 0.8 | 0–2 | model light curve: `max(scale × dot(n, L) + ambient, floor)` |
| `lighting.diffuse.ambient` | 0.3 | 0.4 | 0–1 | 〃 |
| `lighting.diffuse.floor` | 0.15 | 0.2 | 0–1 | 〃 |
| `lighting.highlightKnee` | 0.8 | 0.8 | 0.5–0.9 | lit colours above it approach white without reaching it |
| `lighting.rim.strength` | 0.4 | 0.35 | 0–1 | light along a model's silhouette |
| `lighting.rim.power` | 3 | 3 | 1–8 | how narrow the rim is |
| `lighting.specular.scale` | 1 | 1 | 0–2 | highlights on item models that declare specular (`material.specular` in the item-model JSON; the metal armour sets) |
| `lighting.specular.power` | 32 | 32 | 4–128 | how small the highlight is |
| `dynamic.wrap` | 0.3 | 0.5 | 0–0.5 | how far a point light reaches round a model |
| `dynamic.mapSmoothing` | 0.5 | 0 | 0–2 | seconds over which flickering map and character lights (candles, fires) ease to their new brightness |
| `dynamic.effectGain` | 2.5 | 1 | 0–4 | brightness factor of spell and effect lights |
| `dynamic.effectRange` | 3 | 1 | 0.5–4 | range factor of spell and effect lights |
| `dynamic.effectRangeMax` | 8 | 0 (none) | 0–16 | the most terrain cells the range factor may widen a light to |
| `dynamic.effectFade` | 0.5 | 0 | 0–2 | seconds over which a spell's light fades out after it ends |
| `dynamic.effectCap` | 2.5 | 0 (none) | 0–8 | the most light overlapping spell lights may add up to |
| `bloom.strength` | 1.0 (Icarus 0.3) | 0.6 | 0–2 | how strong the halo around light effects is |
| `bloom.levelFalloff` | 0.6 | 0.5 | 0.25–1 | how wide the halo spreads |
| `grading.strength` | 1 | 1 | 0.1–1 | how much of a map's grading table applies |

## Developer commands

Builds with the control socket (`ENABLE_CONTROL_SOCKET=ON`, the `-mueditor`
presets) or the DevEditor have commands for tuning. Players' builds have none
of them; there only `$look enhanced|classic` exists.

Live tuning of the look profile (session only until `$look save`):

| Command | Effect |
|---|---|
| `$look show [<name>]` | every effective value and where it comes from (render log) |
| `$look set [map] <name> <value>` | changes a value of the default profile, or of the current map's entry |
| `$look reset [<name>]` | back to the values loaded from the files |
| `$look load [<candidate>]` | loads `Data/Render/Candidates/<candidate>/Look.json` and `LightMood.json`, or the shipped files again |
| `$look save` | writes both files |

Shorter forms of `$look set`: `$lighting rim <v>`, `$lighting specular <v>`,
`$lighting diffuse <scale> <ambient> <floor>`, `$lightmood knee <k>`,
`$dynlights wrap <w>`, `$dynlights smooth <s>`,
`$dynlights spell <gain> <range> <fade>`, `$dynlights cap <c>`,
`$dynlights reach <cells>`, `$bloom falloff <f>`, `$bloom <strength>`.
The DevEditor has the same values in its Look panel.

Feature overrides, to compare one feature on and off. They last for the
session, are never saved, and choosing a look (checkbox, `$look`) or a
restart clears them:

| Command | Effect |
|---|---|
| `$texfilter on\|off`, `$lighting on\|off`, `$dynlights on\|off`, `$post on\|off` | one feature on or off |
| `$lightmood on\|off\|<0-100>` | the light mood, or its strength in percent |

Diagnostics: `$post view source|bloom|scene` (what feeds the bloom, the blurred
halo, back to the scene) and `$dynlights stats` (lights the light history
follows, render log).
