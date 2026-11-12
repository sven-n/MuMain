# Classic and Enhanced look

The client has two looks. **Classic** (the default) draws the game as it
always looked. **Enhanced** is a preset that switches on five rendering
features and 4x multisample anti-aliasing together. The values of the Enhanced
preset ship with the client in one data file, `Data/Render/Look.json`, and
every piece of it can be tuned there, also per map, without touching the code
(see "Tuning the Enhanced preset" below). Players can also tune its strengths
for themselves in the options window, without editing that file (see "Tuning
it yourself").

| Feature | What it does | Guide |
|---|---|---|
| Texture filtering | mipmaps and anisotropic filtering for world textures | — |
| Enhanced lighting | per-pixel model lighting with rim light and, on metal armour, specular highlights | — |
| Dynamic model lights | candles, fires and spells light characters, monsters, NPCs and world objects per pixel, and the ground as smooth pools instead of patches interpolated between terrain vertices; characters' shadows on the ground are see-through and merge where they overlap | — |
| Light mood | darker baked map light, so the light sources stand out | [light-mood.md](light-mood.md) |
| Post-processing | bloom around light effects and per-map colour grading | [post-processing.md](post-processing.md) |
| Anti-aliasing | multisampled edges (4 samples, or what `Antialiasing` asks for), alpha-to-coverage for cut-out foliage and fences | — |

## Choosing the look

- **Options window**: the "Enhanced graphics" checkbox on the Graphics tab. Under
  it, Enhanced lighting, Dynamic lights, Sunlight, Darker map light (the light mood) and
  Bloom and colour grading switch those features of Enhanced off one by one;
  dynamic lights need Enhanced lighting, and Sunlight needs dynamic lights, because its ground shadows draw in their per-pixel ground pass. Ticked
  is Enhanced, unticked Classic; it applies at once and is saved.
- **Chat box**: `$look enhanced` and `$look classic`, in every build; saved too.
- **`config.ini`**, section `[Render]`:

  | Key | Values | Default |
  |---|---|---|
  | `Look` | `Enhanced` or `Classic` | `Classic` |
  | `Antialiasing` | samples of the Enhanced look: `0` (off), `2`, `4` or `8` | `4` |
  | `Anisotropy` | maximum anisotropic filtering, clamped to the GPU | `8` |
  | `EnhancedLighting`, `DynamicLights`, `Sunlight`, `LightMood`, `PostProcessing` | `0` switches that feature of Enhanced off; Classic ignores them | `1` |

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

## Tuning it yourself

Under each of the five rows below "Enhanced graphics" (Enhanced lighting,
Dynamic lights, Sunlight, Darker map light, Bloom and colour grading), the options
window has that feature's own sliders. Clicking a row's name or its arrow opens
or closes them; only the checkbox switches the feature. All five start closed,
and the specialist sliders of a group wait under its own "More settings".

Each slider shows its name, its current value with its unit, its smallest and
largest value, whether it uses the shipped settings ("Defaults"), is your own
value ("Custom") or is not saved yet ("Not saved"), and a Reset. Hover a slider for
what it does. Drag it, click on its track, or click it and use the arrow keys
(Page Up/Down: ten steps, Home/End: the ends). A click always sets the step
under the pointer, the track's ends included. The game shows the change on the
next frame (Mood strength rebuilds the map light at most once per frame); it
is saved when you let go of the slider or the key, click elsewhere or close the
window.

| Group | Slider | Value | Range | Step | Default | Where |
|---|---|---|---|---|---|---|
| Enhanced lighting | Directional light | `lighting.diffuse.scale` | 0–2 | 0.05 | 1 | common |
| Enhanced lighting | Rim intensity | `lighting.rim.strength` | 0–1 | 0.05 | 0.4 | common |
| Enhanced lighting | Metallic shine | `lighting.specular.scale` | 0–2 | 0.05 | 0.8 | common |
| Enhanced lighting | Ambient fill | `lighting.diffuse.ambient` | 0–1 | 0.05 | 0.3 | More settings |
| Enhanced lighting | Minimum light | `lighting.diffuse.floor` | 0–1 | 0.05 | 0.15 | More settings |
| Enhanced lighting | Highlight knee | `lighting.highlightKnee` | 0.5–0.9 | 0.01 | 0.8 | More settings |
| Enhanced lighting | Rim concentration | `lighting.rim.power` | 1–8 | 0.1 | 3 | More settings |
| Enhanced lighting | Shine concentration | `lighting.specular.power` | 4–128 | 1 | 32 | More settings |
| Dynamic lights | Skill intensity | `dynamic.effectGain` | 0–4× | 0.1× | 0.5× | common |
| Dynamic lights | Skill radius | `dynamic.effectRange` | 0.5–4× | 0.1× | 2× | common |
| Dynamic lights | Skill fade | `dynamic.effectFade` | 0–2 s | 0.05 s | 0.5 s | common |
| Dynamic lights | Overlapping-light cap | `dynamic.effectCap` | 0–8 | 0.1 | 2.5 | common |
| Dynamic lights | Shadow darkness | `dynamic.shadowOpacity` | 0–100 % | 5 % | 60 % | common |
| Dynamic lights | Light shadows | `dynamic.shadowLights` | 0–4 | 1 | 1 (indoors 2) | common |
| Dynamic lights | Light wrap | `dynamic.wrap` | 0–0.5 | 0.05 | 0.3 | More settings |
| Dynamic lights | Map-light smoothing | `dynamic.mapSmoothing` | 0–2 s | 0.05 s | 0.5 s | More settings |
| Dynamic lights | Maximum skill radius | `dynamic.effectRangeMax` | 0–16 cells | 1 cell | 8 cells | More settings |
| Sunlight | Sun shadows | `sun.shadow` | 0–100 % | 5 % | 100 % | common |
| Sunlight | Sun shadow softness | `sun.softness` | 0–2 | 0.1 | 1 | More settings |
| Sunlight | Sun shadow resolution | `sun.resolution` | 1024–4096 | 1024 | 2048 | More settings |
| Sunlight | Flat sun shadow | `dynamic.flatSunShadow` | 0–100 % | 5 % | 100 % (0 indoors) | More settings |
| Sunlight | Cloud shadows | `clouds.strength` | 0–100 % | 5 % | 35 % | common |
| Sunlight | Cloud size | `clouds.scale` | 500–10000 | 250 | 3000 | More settings |
| Sunlight | Cloud speed | `clouds.speed` | 0–300 | 5 | 40 | More settings |
| Sunlight | Cloud cover | `clouds.cover` | 0–100 % | 5 % | 45 % | More settings |
| Sunlight | Sky-blue shade | `sky.fill` | 0–100 % | 5 % | 50 % | common |
| Sunlight | Rim light follows the sun | `lighting.rim.sunFollow` | 0–100 % | 5 % | 100 % | More settings |
| Darker map light | Mood strength | `mood.strengthPercent` | 0–100 % | 1 % | 100 % | common |
| Bloom and colour grading | Bloom intensity | `bloom.strength` | 0–2 | 0.05 | 1 (Icarus 0.3) | common |
| Bloom and colour grading | Bloom spread | `bloom.levelFalloff` | 0.25–1 | 0.05 | 0.6 | common |
| Bloom and colour grading | Colour grading strength | `grading.strength` | 0–100 % | 1 % | 100 % | common |
| Bloom and colour grading | Contact shadows | `ao.strength` | 0–2 | 0.05 | 0.8 | common |
| Bloom and colour grading | Contact shadow reach | `ao.radius` | 10–300 | 5 | 60 | common |

The defaults are the shipped look profile's (and each map's own, where it has
one); they are defaults, not a promise that every combination of extremes looks
good. What some ends mean:

- **No cap**: an Overlapping-light cap or a Maximum skill radius of 0 removes
  that limit. It does not switch skill lights off.
- **Immediate**: a Skill fade or Map-light smoothing of 0 changes the light at
  once instead of easing it.
- A Skill intensity of 0 removes the light skills cast on models and the
  ground; the skill effects themselves are still drawn. Skill intensity is not
  the only source of light during a fight: map lights, and the darker map
  light's own boost of every light (Mood strength), add to it.
- A Colour grading strength of 0 % applies no grading at all while the bloom
  stays on.
- A Mood strength of 0 % is the map's original light; the rest of the Enhanced
  look stays on.

**Defaults and your values.** Set a slider once: your choice applies globally
on every map, including after travel and restart. There are no separate
per-map player settings. "Defaults" means the shipped settings apply; these
can adapt automatically to a map (for example, untouched Bloom intensity is
1 in Lorencia and 0.3 in Icarus). Moving that slider sets your chosen value on
every map until Reset restores the shipped settings. Each group has "Reset these settings" for its
sliders, and "Reset all tuning" below the five groups resets every slider,
Mood strength included. Resets never change which features are switched on,
the look, Anti-aliasing or Anisotropic filtering. Opening and closing the window
changes nothing: a map value between two slider steps stays as it is until you
move that slider.

**Switched off.** While Classic is chosen, a feature is switched off, the
graphics device cannot draw it, or (for Dynamic lights) Enhanced lighting is off,
that group's sliders cannot be moved and say why; your values are kept and
apply again when the feature is back, and Reset still works. A switched-off
feature's values do not apply anywhere meanwhile, also when the graphics
device cannot draw it: with Enhanced lighting off or unavailable, the darker
map light uses the map's Highlight knee, not yours. Classic ignores
them entirely.

**Where they are saved.** Only the sliders you moved are written, to the
`[EnhancedTuning]` section of `config.ini`, one line per value, named as in the
table above:

```ini
[EnhancedTuning]
dynamic.effectGain=1.2
bloom.strength=0.5
mood.strengthPercent=70
```

A missing line follows the map. A line with an unknown name, a value that is
not a number or one outside its range is ignored on its own, with a warning in
the `render` log; the other lines still apply. The section is written through
a temporary file that replaces `config.ini` only when it is complete: if the
save fails, the window says that the change is shown but not saved, and
`config.ini` keeps what it had. Your tuning never changes `Data/Render/Look.json`.

## Tuning the Enhanced preset

Enhanced is a bundle of pieces that are tuned independently; nothing about
its look is fixed in the code. To change how Enhanced looks:

| To change… | Edit | Guide |
|---|---|---|
| model lighting (diffuse curve, rim, specular, highlight roll-off) | `lighting.*` in `Data/Render/Look.json` | below |
| how candles, fires and spells light models | `dynamic.*` in `Data/Render/Look.json` | below |
| bloom and colour grading | `bloom.*`, `grading.*` in `Data/Render/Look.json`; grading tables in `Data/Grading/` | [post-processing.md](post-processing.md) |
| how dark each map's baked light is | `mood.*` in `Data/Render/Look.json` | [light-mood.md](light-mood.md) |
| which armour shines | `material.specular` in the item-model JSON (`Data/Items/Models/`) | below |

Every value can be set for the whole game or for one map; a player's own
value (see "Tuning it yourself") then applies over it. Developer builds
change any value live and save it back to the file (`$look set`, `$look
save`, or the DevEditor's Look panel; see "Developer commands"), so a change
can be judged in the game before it is kept. A server or a player who wants a
different Enhanced ships different data files; the code stays the same.

## The Enhanced values: `Data/Render/Look.json`

The look profile holds the tuning values of the features and the light
mood's values (`mood.*`, [light-mood.md](light-mood.md)). Classic doesn't use
it.

```json
{
  "formatVersion": 2,
  "default": {
    "bloom.strength": 1,
    "mood.base": 0.6
  },
  "environments": {
    "enclosed": {"sun.shadow": 0, "mood.base": 0.45},
    "fixed": {"mood.sun": 1},
    "sky": {"mood.background": 0.7}
  },
  "maps": [
    {"maps": [1], "name": "Dungeon", "environment": "enclosed"},
    {"maps": [10], "name": "Icarus", "environment": "sky", "bloom.strength": 0.3},
    {"maps": [11, 12, 13, 14, 15, 16, 17, 52], "name": "Blood Castle", "environment": "fixed"}
  ]
}
```

- **Layers.** A value comes from the first of these that names it: the
  map's entry, its environment, `default`, the built-in value. A player's own
  value (see "Tuning it yourself") applies over all of them.
- **Environments** are kinds of place (see below). A map entry picks one by
  name with `"environment"`; an environment does not list its maps.
- **Map entries** list their maps by the number of the world the client
  loads. That is the server's map number, except Devil Square 5–7: the server
  sends 32, the client loads world 9 (Devil Square 1–4) and takes world 9's
  entry, so an entry for 32 is not read. One entry holds the variants of one
  map, such as Blood Castle 1–8, which are alike. Maps that are only alike in
  kind share an environment, not an entry. A map is in at most one entry;
  `name` is for the reader. Which maps are variants of one map is not checked
  yet (the map data has no groups so far), only that every number is a map
  from 0 to 255 and in one entry.
- **Rules between values** are checked on the result: on the default, on each
  environment over the default and on each map, `mood.figures` is not below
  `mood.base`.
- **Errors.** An unknown name, a value out of its range, a map in two entries,
  a stage instead of a map number, an unknown environment or a broken file
  make the client log every problem once (`render` log) and use the built-in
  values for the whole file. Data Checks shows the same problems.
- **Saving** (`$look save`, the Dev Editor) writes a fixed layout: the
  default one value per line, each environment and each map entry on one
  line, environments by name, map entries by their first map, values in the
  order of the tables here and in [light-mood.md](light-mood.md), numbers in
  their shortest form (`1`, not `1.0`). Saving without changes writes the
  same file.
- **Version 1** files still load: a version 1 `Look.json` is read together
  with the `Data/Maps/LightMood.json` beside it, which held the mood values
  (a `mood.*` value in a version 1 `Look.json` makes it unusable). The mood's default becomes `mood.*` values on `default`, and each map keeps
  every mood value its entry named, even one equal to the default, so a later
  change of the default leaves it as it was. Saving writes version 2 into
  `Look.json` alone. Without a valid `LightMood.json` beside it, a version 1
  `Look.json` still applies, with the Classic mood, but cannot be saved:
  that would make the Classic mood the file's. Restore the mood file, or
  write `Look.json` as version 2 by hand with its `mood.*` values. A
  `LightMood.json` beside a version 2 `Look.json` is not read; Data Checks
  warns about it ("is no longer read") so it can be deleted.

| Value | Shipped | Built-in | Range | Meaning |
|---|---|---|---|---|
| `lighting.diffuse.scale` | 1.0 | 0.8 | 0–2 | model light curve: `max(scale × dot(n, L) + ambient, floor)` |
| `lighting.diffuse.ambient` | 0.3 | 0.4 | 0–1 | 〃 |
| `lighting.diffuse.floor` | 0.15 | 0.2 | 0–1 | 〃 |
| `lighting.highlightKnee` | 0.8 | 0.8 | 0.5–0.9 | lit colours above it approach white without reaching it |
| `lighting.rim.strength` | 0.4 | 0.35 | 0–1 | light along a model's silhouette |
| `lighting.rim.power` | 3 | 3 | 1–8 | how narrow the rim is |
| `lighting.specular.scale` | 0.8 | 1 | 0–2 | highlights on item models that declare specular (`material.specular` in the item-model JSON; the metal armour sets) |
| `lighting.specular.power` | 32 | 32 | 4–128 | how small the highlight is |
| `dynamic.wrap` | 0.3 | 0.5 | 0–0.5 | how far a point light reaches round a model |
| `dynamic.mapSmoothing` | 0.5 | 0 | 0–2 | seconds over which flickering map and character lights (candles, fires) ease to their new brightness |
| `dynamic.effectGain` | 0.5 | 1 | 0–4 | brightness factor of spell and effect lights |
| `dynamic.effectRange` | 2 | 1 | 0.5–4 | range factor of spell and effect lights |
| `dynamic.effectRangeMax` | 8 | 0 (none) | 0–16 | the most terrain cells the range factor may widen a light to |
| `dynamic.effectFade` | 0.5 | 0 | 0–2 | seconds over which a spell's light fades out after it ends |
| `dynamic.effectCap` | 2.5 | 0 (none) | 0–8 | the most light overlapping spell lights may add up to |
| `dynamic.shadowOpacity` | 0.6 | 0.6 | 0–1 | how much characters' shadows darken the ground's own light; overlapping shadows darken it once, and point lights still light shadowed ground |
| `dynamic.shadowLights` | 1 (`enclosed` 2) | 0 | 0–4 | how many of the strongest point lights near a character cast its shadow away from them; each darkens the point light on the ground by its share of the character's point light, and a light at the character casts none |
| `dynamic.shadowAngle` | 135 | 180 | 0–360 | not a player slider, since painted shadows cannot follow it; which way characters' shadows fall, in degrees counter-clockwise from world +x; 135 falls away from the sun the maps' baked light was made for, 180 is the Classic direction. The sun turns with it: its shadow map, the terrain's slope shading and the models' light all follow, each keeping the offset the map's baked light has from that sun, so at 135 every map looks as it always did |
| `bloom.strength` | 1.0 (Icarus 0.3) | 0.6 | 0–2 | how strong the halo around light effects is |
| `bloom.levelFalloff` | 0.6 | 0.5 | 0.25–1 | how wide the halo spreads |
| `grading.strength` | 1 | 1 | 0–1 | how much of a map's grading table applies; 0 grades nothing while the bloom stays |
| `ao.strength` | 0.8 | 0 | 0–2 | how much screen-space ambient occlusion darkens corners, wall bases and the ground under objects; additive effects are left alone. With anti-aliasing on, the scene's solid parts are drawn once more for the depth it reads |
| `ao.radius` | 60 | 60 | 10–300 | how far, in world units, nearby geometry still darkens a surface |
| `sun.shadow` | 1 | 0 | 0–1 | how dark the sun's shadows are, on top of `dynamic.shadowOpacity`. The flat shadow, clouds, sky-blue shade and sun rim fade with it, so low values change the look gradually. Everything solid (the terrain, buildings, trees, characters) is drawn from the sun into a shadow map, so shadows fall onto slopes, walls and other characters. The sun stands opposite `dynamic.shadowAngle`, as high as the flat character shadows were long. On models it takes away only the direct sunlight, not the ambient light. Characters' see-through parts (wings, capes, auras) cast a lighter shadow by their opacity, or glowing, their brightness. Above 0, characters cast no flat sun shadow; point-light shadows stay flat. Needs dynamic lights; drawn with anti-aliasing too |
| `sun.softness` | 1 | 1 | 0–2 | how many shadow-map texels the sun shadows' edges blur over |
| `dynamic.flatSunShadow` | 1 (Dungeon, Lost Tower, Kanturu Relics, Refinery Tower: 0) | 1 | 0–1 | how dark the flat shadow under characters is while sun shadows are off (`sun.shadow` 0); it falls away from the frame's sun, as long as the sun's slant (at most 3 units per unit of height), on top of the shadow's own darkness. The indoor and underground maps set both to 0: no sun there, so only torches and spells cast character shadows. Classic always draws it |
| `clouds.strength` | 0.35 | 0 | 0–1 | how much a passing cloud dims the sun's light on the ground and characters; drifting procedural clouds whose shadows fall slanted by the sun. Only with sun shadows |
| `clouds.scale` | 3000 | 3000 | 500–10000 | the cloud pattern's size in world units |
| `clouds.speed` | 40 | 40 | 0–300 | how fast the clouds drift along a fixed wind, in world units per second |
| `clouds.cover` | 0.45 | 0.45 | 0–1 | the share of the sky the clouds cover |
| `sky.fill` | 0.5 | 0 | 0–1 | how far shade (from shadows and clouds) turns towards the sky's blue while sunlit places keep the sun's colour; the light mood's `shadeCool` keeps the share (1 - this value) on painted shade while sun shadows are on. Only with sun shadows |
| `lighting.rim.sunFollow` | 1 | 0 | 0–1 | how much the characters' edge light comes from the sun: brighter and sun-coloured on its side, gone in its shadow; 0 the even Enhanced rim |
| `sun.resolution` | 2048 | 2048 | 1024–4096 | the sun shadow map's size in texels (1024, 2048 or 4096): sharper shadows for more graphics memory and time |

## Environments

`environments` names kinds of place; a map entry picks one
with `"environment"`. Their values apply between the default and the map's
own. The shipped ones:

| Environment | What it sets | Maps |
|---|---|---|
| `outdoor` | nothing yet | Lorencia, Devias, Noria, Tarkan, Valley of Loren, Land of Trials, Crywolf, Elbeland, Loren Market, Karutan |
| `outdoorDense` | models lit fully by the sun (`mood.sun` 1) | Aida, Kanturu Ruins, Swamp of Calmness, Raklion, Vulcanus |
| `sky` | lighter baked light, darker background | Icarus |
| `underwater` | softer sun shadows, lighter baked light | Atlans |
| `enclosed` | no sun shadows, character shadows from two point lights, baked light darkened evenly | Dungeon, Lost Tower, Devil Square, Chaos Castle, Kanturu Relics, Refinery Tower, Balgass Refuge, Illusion Temple, Raklion boss cave, Doppelganger, Imperial Guardian |
| `fixed` | models lit fully by the sun (`mood.sun` 1) | Arena, Blood Castle, Kalima, Barracks of Balgass, Santa Village, Duel Arena |

A map's own entry can still differ from its environment: Aida, Raklion, Arena,
Santa Village and Duel Arena keep the default's `mood.sun`. The file also has
an entry for map 32 (Devil Square 5–7) with the default's `mood.sun`, but the
client shows Devil Square 5–7 with world 9's entry (see "Map entries" above).

[enhanced-look-map-environments.md](enhanced-look-map-environments.md) lists
every map with its world number and why it has its environment.

## Shadows and their switches

Characters and objects get their sun shadow from one of two places:

- **The shadow map**, while `sun.shadow` is above 0 and the graphics device can draw it.
- **The flat shadow**, otherwise. As `sun.shadow` rises from 0 to 100 %, the flat shadow fades out while the shadow
  map, clouds, sky-blue shade and sun rim fade in, so low values have no sudden change.

The older switches also apply to the shadow map:

| Switch | Effect |
|---|---|
| Disable Shadows | models stop casting body shadows and leave the shadow map; the terrain keeps casting, and the sunlight, clouds and sky-blue shade stay |
| Render full effects, off | the same as Disable Shadows |
| Disable wing shadow | wings and capes stop casting, in both kinds of shadow; the rest of the character still casts |

These switches change what casts, not what is drawn: ambient occlusion still sees every model.

Alpha-tested and alpha-to-coverage cut-outs (foliage, fences, hair) cast shadows through the same holes they show.
Fading objects are not cut-outs: a fading object casts no shadow, except that a fading character part casts a partial
one, as blended wings and capes do.

If an optional pass cannot be drawn, the game falls back without losing geometry:

| Unavailable | Fallback |
|---|---|
| Sun shadow map or its pipelines | flat shadows return from the next frame; choosing another `sun.resolution` tries again |
| Ground shadow mask | sun shadows draw as recorded, and point-light shadows are left out |
| Scene depth or ambient occlusion | no ambient occlusion |
| HDR scene | the scene renders in the frame's own format |
| Post-processing targets | the frame renders directly, without bloom or ambient occlusion |

## Limits

These are accepted limits of the current look, with the stage of the [lighting plan](https://github.com/sven-n/MuMain/issues/990)
that takes them up. They are not claims of full 3D local-light occlusion.

- **Lighting is in gamma space.** Textures are not decoded from sRGB, and the HDR scene is headroom for bright effects,
  not linear light. The colour pipeline is decision D1, and the look values are tuned for gamma space until then.
- **Point-light shadows are terrain-projected.** A character's shadow from a nearby light is flattened onto the ground.
  It does not fall on walls or other characters, and world objects cast none. Per-light visibility is Stage 2, and
  occlusion by world objects is Stage 6.
- **Ambient occlusion darkens direct light too.** The composite darkens the whole scene except additive effects, so
  sunlit contact areas darken as well as shaded ones. Limiting it to ambient light is Stage 2.
- **Lighting clamps remain.** The ground clips at full brightness while models use a soft roll-off. One roll-off for
  both belongs to the light model in Stage 1, after the colour pipeline decision.
- **Ambient occlusion is tied to the bloom switch.** Turning off "Bloom and colour grading" removes ambient occlusion
  too, until Stage 4 separates them.
- **Direct3D 12 can crash or hang the GPU.** With Enhanced on, a frame can need more sampler descriptors than one
  heap of SDL's D3D12 backend holds (2048), and SDL's switch to a new heap mid-frame is faulty: the client may exit, or
  Windows may reset the GPU. Vulkan is not affected. Use `[Render] Backend=vulkan` until this is resolved: `default`
  picks Vulkan where it works, but falls back to Direct3D 12 on systems without a working Vulkan driver.

## Developer commands

Builds with the control socket (`ENABLE_CONTROL_SOCKET=ON`, the `-mueditor`
and `-in-game-tests` presets) or the DevEditor have commands for tuning. Players' builds have none
of them; there only `$look enhanced|classic` exists.

Live tuning of the look profile (session only until `$look save`). A value
changed this way wins over the player's own value of it where the change
applies: `$look set` on every map without its own entry for that value,
`$look set map` on the active map's entry, so on every map that entry lists
(the reply names them; a map without an entry gets one of its own). `$look reset <name>` clears the change
in the default and the active map's entry (another map's `set map` change
stays), `$look reset` and `$look load` clear every change; `$look show` names
a player's value `player`. `$look save` writes the file's own values, never a
player's:

| Command | Effect |
|---|---|
| `$look show [<name>]` | every effective value and where it comes from (render log) |
| `$look set [map] <name> <value>` | changes a value of the default profile, or of the current map's entry; when the entry lists several maps, the reply names them ("the entry is shared, maps … all get it") and the Dev Editor's Look panel shows them under "Shared by maps" |
| `$look reset [<name>]` | back to the values loaded from the file |
| `$look load [<candidate>]` | loads `Data/Render/Candidates/<candidate>/Look.json` (a version 1 candidate with its `LightMood.json`; without one it shows the shipped moods and cannot be saved), or the shipped file again |
| `$look save` | writes `Data/Render/Look.json` |

Shorter forms of `$look set`: `$lighting rim <v>`, `$lighting specular <v>`,
`$lighting diffuse <scale> <ambient> <floor>`, `$lightmood knee <k>`,
`$dynlights wrap <w>`, `$dynlights smooth <s>`,
`$dynlights spell <gain> <range> <fade>`, `$dynlights cap <c>`,
`$dynlights reach <cells>`, `$dynlights shadow <opacity>`, `$dynlights shadowangle <degrees>`, `$dynlights shadowlights <n>`, `$bloom falloff <f>`, `$bloom <strength>`, `$ao <strength>`, `$ao radius <r>`, `$sun <strength>`, `$sun softness <f>`, `$sun resolution <texels>`, `$dynlights flatsun <f>`, `$clouds <strength>`, `$clouds scale <units>`, `$clouds speed <units/s>`, `$clouds cover <f>`, `$sky <f>`, `$rim sun <f>`.
The DevEditor has the same values in its Look panel.

Feature overrides, to compare one feature on and off. They last for the
session, are never saved, and choosing a look (checkbox, `$look`) or a
restart clears them:

| Command | Effect |
|---|---|
| `$texfilter on\|off`, `$lighting on\|off`, `$dynlights on\|off`, `$sun on\|off`, `$post on\|off` | one feature on or off |
| `$lightmood on\|off\|<0-100>` | the light mood, or its strength in percent |

Diagnostics: `$post view source|bloom|scene` (what feeds the bloom, the blurred
halo, back to the scene) and `$dynlights stats` (lights the light history
follows, render log).
