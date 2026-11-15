# Linear Lighting and Materials (WIP)

> **Working document.** Decision D1 of the lighting plan
> ([#990](https://github.com/sven-n/MuMain/issues/990), "D1 to D4 in detail"):
> the Enhanced look moves to linear light, glTF's metallic-roughness shading,
> the Khronos PBR Neutral tone map and colour grading before it. Classic does
> not change. The look's data is the [graphics look data
> design](2026-11-10-graphics-look-data-design.md) (area GL). Based on MuMain
> `main` @ `81643e40a`.

## Goal

The Enhanced look lights the painted art in the colour values it is stored in:
a texel of 0.5 times a light of 0.5 is 0.25, two glows of 0.5 add up to white,
and the highlights are cut off by two different knees. Light that adds up and
falls off like light, a shading model that GLB models are made for, and a tone
map that keeps painted colours as painted make the look consistent and let new
content (GLB models, materials, time of day) build on it instead of retuning
it.

D1 is also the point where the format of materials and colour grading is
settled, so that the rich GLB models, materials with textures and time of day
that come later are additions, not rework.

## Scope

| Step | What | Needs |
|---|---|---|
| LL1 | This design | — |
| LL2 | Linear colour: the developer switch, decoding by role, the HDR scene with Enhanced lighting, exposure, bloom before the tone map, PBR Neutral, effect gains, models drawn for the UI and the editor's previews | LL1 |
| LL3 | Colour grading: parameters on the look layers, baked with the tone map into one table, edited in the Graphics Look window; `World2.png` rebuilt as parameters | LL2 |
| LL4 | Shading and materials: glTF's BRDF with specular anti-aliasing and an ambient reflection, MU's terms outside it, `roughness` and `metallic` on item models, the item model form, debug views, the reference model check | LL2 |
| LL5 | Every Enhanced value retuned once, the user's sign-off (D4), the old path and the switch removed, performance measured, docs | LL3, LL4 |

**Not in D1:**
- reading GLB materials, texture slots (base colour factor, metallic-roughness, occlusion and emissive maps), emissive, shading models as data, glTF extensions (a rich-GLB design, and #1045 for the texture slots);
- point-light falloff (Stage 1);
- the light map's split into sun and ambient (Stage 3);
- auto exposure;
- HDR display output;
- render scale and upscalers (#1054, the temporal candidate);
- an intensity per effect type (until the Effect Browser edits);
- monster materials (they take the item models' field when the monster model form next changes);
- player settings beyond what the HDR scene forces (Stage 7, GL2.2).

## Current state

- **The Enhanced shaders mix stored colours.**
  - `skinned_lit.frag` (models on the GPU skinning path), `world_lit.frag` (the ground) and `basic_textured_fog.frag` (everything else drawn lit by its vertex colour) multiply the texel by the light as stored.
  - Light colours in the look and effect data are display colours.
- **Two knees.**
  - `skinned_lit.frag` rolls lit colours off above `lighting.highlightKnee` (`RollOffHighlight`).
  - `post_composite.frag` rolls the HDR scene off above its own knee.
  - Bloom is screen-blended after that, then the map's grading table (`Data/Grading/World<N>.png`, a 32³ display-space cube; only `World2.png` exists) and dither.
- **The HDR scene** (`R16G16B16A16_FLOAT`) exists only while post-processing is on (`SdlGpuPostProcessing::IsHdrScene`); otherwise the scene is drawn into the display image directly.
- **Models drawn for the UI** (inventory, shops; between `BeginScreenSpaceSampling` and `EndScreenSpaceSampling`) use `skinned_lit.frag` after the scene's boundary, into the display image.
- **Materials:**
  - an item model has one `material.specular` (0–1); 115 armour models set 0.6;
  - the Blinn-Phong highlight's power is `lighting.specular.power` (32);
  - meshes with skin or hair textures get no specular (`ZzzBMD.cpp`);
  - the GLB loader reads only the base-colour texture and the normal map.
- **Blend modes** (`BlendMode` in `MuRenderer.h`): Alpha, Additive, Subtract, InverseColor, Mixed, LightMap, Glow, Luminance.

## Decisions

The order of reasons in every decision: the hard constraints first (Classic
unchanged, D4; SDL_GPU has no texture views; texture slots wait for #1045;
D1's text), then what current engines do, performance, architecture, and what
keeps later work an addition (data, GLB). MU Editor operations decide a tie
and veto data no editor can set.

| # | Decision | Why |
|---|---|---|
| D-LL1 | **Rollout behind a developer switch.** `$look linear on/off` and a checkbox in the Graphics Look window, developer builds only, until LL5 removes the old path and the switch. Players never see two Enhanced looks. | The new and old look compared live in one build while tuning; small reviewable PRs. A player option would mean keeping two Enhanced looks tuned, against #990's "tune once". |
| D-LL2 | **Textures are decoded in the shader, by their role.** Base colour and legacy vertex colours are sRGB; normal maps and glTF `COLOR_0` are linear. The role, not the shader, carries the colour space. | `*_SRGB` texture formats are the standard and decode for free, but would change how Classic filters the same textures, and SDL_GPU cannot view one texture in two formats. With the colour space on the role, moving to those formats later is a loader change. The cost: filtering and mipmaps stay in stored values (see Colour). |
| D-LL3 | **The HDR scene and the tone map run whenever Enhanced lighting is on.** The Post-processing switch then covers bloom, grading and ambient occlusion only. | Linear light needs room above 1 and one place that compresses it; keeping the gamma path for "Enhanced without post-processing" would keep two Enhanced shading paths. The cost is the target and the full-screen pass post-processing already pays. |
| D-LL4 | **Colours in data are display colours; intensities are linear.** Light, fog and sky colours are decoded when packed into uniforms; a strength or gain multiplies linear light. | As colour pickers in Unity and Unreal work: an author picks a colour as seen, and brightness is a number. |
| D-LL5 | **Relative units: a light of 1 on a white surface facing it gives 1.** Our light values are glTF light intensities times π. | D1's "a sun of 1 lights white to display white". The Khronos renderer divides the diffuse by π and gives the light no π factor, so it shows our light of 1 as intensity π. Absolute units would be one scale factor later. |
| D-LL6 | **glTF 2.0's BRDF, exactly.** GGX distribution, height-correlated Smith visibility, Schlick Fresnel with 0.04 for non-metals, Lambert diffuse, α = roughness². | A GLB then shades here as it does in Blender or a glTF viewer; any difference would mean retuning every GLB later. |
| D-LL7 | **MU's terms stay outside the BRDF.** Rim, light wrap and the diffuse floor are added after it, each with a value that turns it off. | The painted look keeps its tools, and a GLB can be shown as authored. |
| D-LL8 | **Geometric specular anti-aliasing** (Tokuyoshi and Kaplanyan) widens the roughness where the normal changes within a pixel. | GGX highlights are sharper and shimmer; MSAA does not catch shading. It is the standard (Filament, Unreal) and removes D1's dependency on post-process AA (#1054 Step 2). |
| D-LL9 | **An ambient reflection without environment maps:** the ambient light reflected by roughness and angle (Karis's analytic approximation). | Metals have no diffuse; without any reflected ambient light, metal armour is black except at its highlight. Engines use an environment for this; until a reflection image exists (Stage 5), the approximation is the standard cheap form. |
| D-LL10 | **Materials with glTF's names:** `roughness` and `metallic`, on the item model entry for the whole model, with optional overrides per mesh. `specular` is converted once. Every surface reflects 4%; one without a material uses `material.defaultRoughness`. Later glTF fields (base colour factor, emissive, alpha mode, textures) are new fields, never renames. | glTF's shape keeps rich GLB an addition; per model fits the item model form as it is, per-mesh overrides keep glTF's per-primitive granularity possible. No field is reserved before something can use it. |
| D-LL11 | **Colour grading is parameters on the look layers,** baked with the tone map and the sRGB encode into one 32³ table, indexed by the ALEXA LogC encoding of the exposed linear colour, and rebaked when a value changes. Grading tables made outside the editor go. | All three engines grade scene-linear colour before the tone map (Unity's HDR mode, Unreal, Filament). Unity and Filament bake grading and tone map into one LogC 32³ table: one read per pixel. Parameters are edited live in the Graphics Look window and blend between layers, zones and time of day; tables do neither. HDR display output stays possible. |
| D-LL12 | **Effects are light in the linear scene,** with a gain per blend family (`effects.additiveGain` for Additive, `effects.glowGain` for Glow) tuned once. | Effects as HDR emissive light is the standard. In linear light, two 0.5 glows no longer add to white, so effects look softer until tuned. An intensity per effect type waits for an Effect Browser that edits. |
| D-LL13 | **Models drawn for the UI and the editor's previews render to texture:** an HDR atlas per frame, tone mapped once with the scene's exposure and tone map, not graded; the UI draws each item from its cell. | Glow passes and translucency must add up in linear light before the tone map, as in the world: two additive grey 0.4 passes give 0.886 tone mapped together, 1.0 when each is tone mapped and added. Render to texture is how Unreal (SceneCapture to UMG) and Unity (RenderTexture to a UI image) show 3D in the UI. The renderer's offscreen captures already replay before the frame, so it costs two passes per frame whatever the UI's draw order, and inventory items, RmlUi previews and the editor's thumbnails share one path. |
| D-LL14 | **Debug views:** lighting only, and the material values, beside the existing bloom views (`PostProcessingView`), selectable in the Graphics Look window. | Unreal's and Unity's standard views for tuning materials over painted textures. |

## The frame

The scene, at its size (the render scale later), in linear light:

1. Opaque and blended draws into the HDR scene. Every texel and colour is decoded by its role (D-LL2, D-LL4). Effects add their light with their gain (D-LL12).
2. Ambient occlusion multiplies the scene.
3. Exposure (`tone.exposure`) multiplies the scene.
4. Bloom adds its light, in linear, before the tone map.

The image, at the window's size:

5. `max(c, 0)`, then one read of the grading table (LL3): the exposed colour encoded as LogC, giving the graded, tone-mapped, sRGB-encoded colour. Before LL3, PBR Neutral and the encode run as functions, after the same clamp.
6. Dither.
7. The UI.

A spatial upscaler (#1054's FSR 1) goes between 5 and 7. A temporal upscaler goes between 4 and 5 and replaces MSAA and post-process AA. Steps 5 and 6 read the scene at whatever size it has.

**Signed values:** steps 1 to 4 keep the scene signed. Subtract and Inverse colour blending over light above 1 leave negative values, and later blends in the same pixel still add to them. The clamp at step 5 is the one boundary, as today's composite has it: LogC (whose logarithm is undefined below −0.0086) and PBR Neutral never see a negative value, in the function path, the table path and the atlas pass alike.

**Models drawn for the UI** (D-LL13):
- Each UI model's draws are an offscreen capture (`BeginOffscreenCapture`) into a cell of one HDR atlas (the scene's format), cleared to transparent black. Blended draws accumulate their coverage in its alpha.
- A cell is the model's screen rectangle plus a margin for glow that spills out of it. The model keeps its screen projection, with the viewport shifted onto the cell, so it looks as it would drawn in place.
- The captures replay before the frame, as they do today. One pass then applies exposure, the clamp, PBR Neutral and the encode to the whole atlas, into a display-format copy.
- Where the model was drawn, the UI draws its cell as premultiplied colour (`src + (1 − a) dst`), in the same place in the draw order.
- RmlUi's `RenderTarget` and the Map Editor's thumbnails are captures already: their targets become HDR and run the same tone-map pass into the image they publish.
- Where an item covers the window, the result is the world's. Where its glow or translucency lies over the window's background, or over another item's cell (a dragged item), the two meet in display colours, so the result there can only approximate the world.

**MSAA:** the scene is resolved before step 2, in linear light. Edges between very bright and dark surfaces can alias more than before. A resolve weighted by the tone map is the known fix, if it shows.

## Colour

**Roles:**

| Input | Colour space | Where it is decoded |
|---|---|---|
| Base colour texture (BMD, OZJ/OZT, GLB base colour) | sRGB | after sampling, in the shader |
| Legacy vertex colours, the light map in the ground's vertex colours, the object light, the terrain's sun-light grid | sRGB | in the fragment shader, after interpolation (painted gradients stay as painted) |
| glTF `COLOR_0` | linear | not |
| Normal map | linear | not |
| Light, fog, sky and sun colours in the look data; effect light colours | sRGB | when packed into uniforms |
| Strengths, gains, exposure | linear factors | not |

**Filtering stays in stored values.** The decode follows the sampler, and mipmaps are made from the stored values, as for Classic (D-LL2). A sample halfway between a black and a white texel decodes to 0.214 where glTF, which decodes before filtering, gives 0.5. Minified high-contrast textures therefore come out slightly darker than in a glTF viewer. This is the accepted deviation until the base colour moves to `*_SRGB` formats with sRGB-aware mip generation. Comparisons with a glTF viewer use constant-colour textures, or magnified ones at mip 0.

**Transfer functions** (Khronos Data Format Specification 1.3, §13.3, the citable form of IEC 61966-2-1):

```
decode:  c ≤ 0.04045:   c / 12.92          else ((c + 0.055) / 1.055)^2.4
encode:  c ≤ 0.0031308: c × 12.92          else 1.055 × c^(1/2.4) − 0.055
```

Not the pure 2.2 power the Khronos renderer uses for its output.

**Tone map:** Khronos PBR Neutral, from the reference
[`pbrNeutral.glsl`](https://github.com/KhronosGroup/ToneMapping/blob/main/PBR_Neutral/pbrNeutral.glsl):

```
startCompression = 0.8 - 0.04, desaturation = 0.15
x = min(r, g, b); offset = x < 0.08 ? x - 6.25 x² : 0.04; c -= offset
peak = max(r, g, b); if peak < startCompression: return c
d = 1 - startCompression; newPeak = 1 - d² / (peak + d - startCompression); c *= newPeak / peak
g = 1 - 1 / (desaturation (peak - newPeak) + 1); return mix(c, newPeak, g)
```

Its input is linear Rec. 709, non-negative (the clamp at step 5). Exposure multiplies the colour just before it, as in the Khronos renderer. Between 0.08 and 0.8 it only subtracts 0.04, the reflection of a non-metal, so a base colour in that range facing a light of 1 comes out exactly as painted.

## Shading

For a model on the GPU skinning path (`skinned_lit.frag`), with `n` the shading
normal (the normal map's when there is one), `v` towards the eye, `l` towards
the sun, `h` their half vector:

**Material:**
- `base` is the decoded texel;
- `roughness` and `metallic` come from the material (D-LL10);
- shading uses roughness ≥ 0.045, Filament's floor for 32-bit floats; the data keeps its value;
- α = roughness², widened by D-LL8;
- `f0 = mix(0.04, base, metallic)`;
- `diffuseColour = base × (1 − metallic)`.

**BRDF** (glTF 2.0 Appendix B: a dielectric and a metal response, mixed by `metallic`):

```
D(h)  = α² / (π ((n·h)² (α² − 1) + 1)²)
V     = 0.5 / ((n·l) sqrt((n·v)² (1 − α²) + α²) + (n·v) sqrt((n·l)² (1 − α²) + α²))
s     = (1 − |v·h|)^5
Fd    = 0.04 + 0.96 s                  dielectric
Fm    = base + (1 − base) s            metal
spec  = mix(Fd, Fm, metallic) D V      = (f0 + (1 − f0) s) D V
diff  = (1 − Fd) diffuseColour / π
```

Only the dielectric's Fresnel takes light from the diffuse. White at metallic 0.6 and normal incidence gives a diffuse of 0.384/π; the mixed Fresnel would give 0.1536/π.

**Guards,** since roughness 0 is valid data and makes D 0/0 at n·h = 1 on a flat surface, where specular anti-aliasing adds nothing:
- the roughness floor above, so D's denominator is at least π α⁴;
- n·v clamped to at least 1e-4, as Filament does;
- the BRDF evaluated only where n·l > 0, and V = 0 when its denominator is not positive, as the Khronos renderer does.

**Specular anti-aliasing** (Tokuyoshi and Kaplanyan, JCGT 2021, Listing 5), applied to α² after the floor and before shading, from the world normal's screen derivatives:

```
α² ← saturate(α² + min(2 σ² (|ddx n|² + |ddy n|²), κ)),  σ² = 1/(2π), κ = 0.18
```

**Light:**
- `object` is the decoded object light: the painted light at the model, as today.
- `sun = object × sunColour × lighting.diffuse.scale × shadow × clouds`
- `ambient = object × skyShade × lighting.diffuse.ambient`
- `points` are the point lights, with their current falloff (Stage 1 changes it).

```
direct(l, E) = π (diff + spec)(l) × E × saturate(n·l)
colour       = direct(sun direction, sun) + Σ direct(towards point, point colour × falloff)
             + ambient × (diffuseColour + EnvBRDFApprox(f0, roughness, n·v))
```

The factor π is D-LL5: a white non-metal facing a light of 1 gives 1.

`EnvBRDFApprox` is Karis's polynomial, as ported by Bevy:

```
r = roughness × (-1, -0.0275, -0.572, 0.022) + (1, 0.0425, 1.04, -0.04)
a = min(r.x², 2^(−9.28 (n·v))) r.x + r.y
EnvBRDFApprox = f0 × (−1.04 a + r.z) + (1.04 a + r.w)
```

**MU's terms** (D-LL7), added after `colour`:
- **Floor:** `max(colour, base × lighting.diffuse.floor × object)`. 0 turns it off.
- **Wrap:** point lights use `saturate(n·l × (1 − dynamic.wrap) + dynamic.wrap)` in place of `saturate(n·l)`. 0 turns it off.
- **Rim:** `lighting.rim.*` as today, in linear light. A strength of 0 turns it off.

`lighting.highlightKnee` and `lighting.specular.*` go in LL5: the tone map compresses, and the material shapes the highlight.

**The ground and everything lit by its vertex colour** decode their texel and colours and keep their current light. Specular on the ground comes with Stage 5 (wet surfaces, water).

## Materials

On an item model entry (`Data/Items/Models/*.json`):

```json
"material": { "roughness": 0.49, "metallic": 0.6, "meshes": { "2": { "metallic": 0 } } }
```

- `roughness` and `metallic` are 0–1. `meshes` maps a mesh index to overrides of the same fields.
- Left-out fields take the model's value, then `material.defaultRoughness` (a look value) and metallic 0.
- Meshes with skin or hair textures take the defaults, as they have no specular today.
- **Conversion (once, LL4):** a model with `specular: s` becomes `roughness = sqrt(sqrt(2 / (p + 2)))` with `p = lighting.specular.power` (32 → 0.49), and `metallic = s`.
  - The roughness comes from α = sqrt(2 / (p + 2)) (Walter et al. 2007), so roughness = sqrt(α).
  - The metal sets are then checked by eye in the item model form's preview.
- `specular` is reported as an unknown key afterwards.
- Format rules as for every content file: defaults left out, fields in a fixed order, saving unchanged data writes the same file, Data Checks reports values out of range.

## Colour grading

**Values on the look layers** (default → environment → map; later zones and time of day), in the order applied, after Filament's and Unity's:

| Value | Neutral | Applied |
|---|---|---|
| `grading.temperature`, `grading.tint` | 0, 0 | white balance, in linear light |
| `grading.slope.r/g/b`, `grading.offset.r/g/b`, `grading.power.r/g/b` | 1, 0, 1 | ASC CDL, in LogC |
| `grading.contrast` | 1 | in LogC, about middle grey |
| `grading.saturation` | 1 | in linear light, about luminance |
| `grading.strength` | 1 | the mix of the graded colour with the ungraded one, as today |

Further operations (shadows, midtones and highlights; curves; channel mixer)
are additions to the list.

**Bake:**
- The table is 32³, as Unity's and Filament's default.
- Each cell is a LogC (ALEXA EI 1000) code: `c × log10(a x + b) + d`, with `a = 5.555556`, `b = 0.047996`, `c = 0.244161`, `d = 0.386036`, as in Unity's `Color.hlsl` and Filament's `ColorSpaceUtils.h`.
- A cell decodes to a linear colour, clamped to non-negative (code 0 decodes to −0.004), which is then graded, PBR Neutral tone-mapped and sRGB-encoded. That colour is stored as RGBA16F.
- The bake runs on the CPU when a resolved grading value or the map changes: 32,768 cells, a few milliseconds, never per frame.
- Exposure stays outside the table, so changing it does not rebake.
- With every value neutral, the table is the tone map alone.

**Style choices** of Stage 7 (warmth, contrast, night darkness) become offsets on `grading.temperature`, `grading.contrast` and `tone.exposure`.

**`World2.png`** is rebuilt once as values on its map entry, side by side for the user's approval. The PNG loader goes in LL5.

## Look values

| Value | Range | Neutral | Step |
|---|---|---|---|
| `tone.exposure` | 0.25–4 | 1 | LL2 |
| `effects.additiveGain`, `effects.glowGain` | 0–4 | 1 | LL2 |
| `material.defaultRoughness` | 0.3–1 | 0.8 | LL4 |
| `grading.*` (above) | per value | neutral | LL3 |

Removed in LL5: `lighting.highlightKnee`, `lighting.specular.scale`,
`lighting.specular.power`. Their keys are then warned as unknown and dropped at
the next save (D-GL9). All new values are art direction: not player controls
(D-GL15, `enhanced-look.md`'s kinds).

## Editor

- **Graphics Look window:**
  - the developer switch;
  - the debug views;
  - the new values, grouped by prefix, live with drag (GL2.1), on every scope;
  - a grading change rebakes the table in the same frame.
- **Item model form:** Roughness and Metallic replace Specular, previewed live.
- **Effect Browser:** its world preview is where the effect gains are judged.
- **Previews** (item, monster, UI) are tone mapped as the world is (D-LL13).

## Tests

- **Shader functions mirrored in C++**, tested against reference numbers, as `EnhancedFogAmount` and `BlendGrading` are:
  - sRGB decode and encode (0.5 → 0.21404; the 0.04045 boundary);
  - PBR Neutral (the Khronos reference function on a grid; channels between 0.08 and 0.8 lose exactly 0.04);
  - the BRDF with specular anti-aliasing (against Appendix B and the Khronos renderer's functions, ported, on a grid of roughness, metallic and angles, within 1e-4), including metallic 0.6 (white at normal incidence: diffuse 0.384/π) and roughness 0 on a flat surface at n·h = 1 (finite);
  - the clamp: a negative channel gives the same result through the functions and the table, never NaN;
  - the atlas: two additive grey 0.4 draws in one cell give 0.886, as in the world; cells are placed with their margin and none overlap;
  - `EnvBRDFApprox`;
  - the grading bake (neutral values give the tone map alone; CDL and contrast against hand-computed cells).
- **Data:** the new look values and item material fields load, save unchanged as they were, and report bad values; the conversion script checks its own output.
- **Renderer:** which passes run for each combination of Enhanced lighting and post-processing; the gain chosen per blend mode.
- **In game:**
  - Classic `--compare` shows no difference;
  - the switch on and off in Lorencia, Devias, Dungeon and an effect-heavy fight;
  - the same item in the world and in a UI view under the same light: an opaque one, one with excellent or +13 glow passes, and a translucent one;
  - a darkening effect (Inverse colour or Subtract) over a glow above 1: black, as today;
  - a metal and rough sphere GLB, with constant-colour textures, beside the Khronos Sample Viewer, with its light at intensity π.
- **Performance:** Enhanced Medium and High on the reference scenes against D3, before LL5 merges; the atlas with a full inventory and a shop open, which also sets its size.

## References

- Khronos, [PBR Neutral](https://github.com/KhronosGroup/ToneMapping/tree/main/PBR_Neutral) (reference code and specification).
- Khronos, [glTF 2.0 specification, Appendix B](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation); [glTF Sample Renderer](https://github.com/KhronosGroup/glTF-Sample-Renderer) (`brdf.glsl`, `pbr.frag`, `punctual.glsl`, `tonemapping.glsl`).
- Khronos, [Data Format Specification 1.3, §13.3 sRGB](https://registry.khronos.org/DataFormat/specs/1.3/dataformat.1.3.html#TRANSFER_SRGB).
- Tokuyoshi and Kaplanyan, [Improved Geometric Specular Antialiasing](https://www.jp.square-enix.com/tech/library/pdf/ImprovedGeometricSpecularAA.pdf) (I3D 2019); [Stable Geometric Specular Antialiasing with Projected-Space NDF Filtering](https://jcgt.org/published/0010/02/02/paper.pdf) (JCGT 2021).
- Walter et al., [Microfacet Models for Refraction through Rough Surfaces](https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf) (EGSR 2007).
- Karis, Physically Based Shading on Mobile (Unreal Engine blog, 2014), `EnvBRDFApprox`, as ported in [Bevy](https://github.com/bevyengine/bevy/blob/main/crates/bevy_pbr/src/render/pbr_lighting.wesl).
- Google Filament, [`ColorGrading.h`](https://github.com/google/filament/blob/main/filament/include/filament/ColorGrading.h), [`surface_lighting.fs`](https://github.com/google/filament/blob/main/shaders/src/surface_lighting.fs), [Materials](https://github.com/google/filament/blob/main/docs_src/src_markdeep/Materials.md.html) (specular anti-aliasing defaults).
- Unity, [URP Asset: grading mode](https://docs.unity.com/en-us/engine/6000.0/manual/render-pipelines/universal-render-pipeline/urp-reference/universalrp-asset); [`Color.hlsl`](https://github.com/Unity-Technologies/Graphics/blob/master/Packages/com.unity.render-pipelines.core/ShaderLibrary/Color.hlsl) (LogC), `LutBuilderHdr.shader`.
- Epic Games, [Color Grading and the Filmic Tonemapper](https://dev.epicgames.com/documentation/en-us/unreal-engine/color-grading-and-the-filmic-tonemapper-in-unreal-engine).
