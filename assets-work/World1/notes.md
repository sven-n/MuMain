# World1 pilot handoff — 2026-09-22 — ASTRA

**Status: exported, validated offline and installed; client visual acceptance pending.**

Branch: `art/world1-pilot`, created from `main` at `9a8b2027`. The original client repeatedly
crashed during reference capture setup, before any asset changes. The user explicitly
authorized continuing offline and leaving client verification pending. No engine or CMake
files were changed. This pilot does not yet prove the end-to-end client result or establish
an accepted visual benchmark.

## Delivered

- [inventory.md](inventory.md): all 32 World1 texture containers, dimensions, loader uses,
  terrain-slot counts and legacy-file distinctions. All containers unwrapped into `original/`.
- [GroundTiles/source.blend](GroundTiles/source.blend): packed 512×512 diffuse masters,
  displayed as 3×3 repeat patches. Original generated paintings, final PNG/JPG textures,
  17 `.OZJ` exports and a [repeat review](GroundTiles/repeat-review.jpg) accompany it.
- [Beer01/source.blend](Beer01/source.blend): remodeled still life on the original rig,
  packed texture, `REF_ORIGINAL`, excluded `REF_HIGH_POLY`, and export geometry.
  [Prop notes](Beer01/notes.md) contain the bounds, comparison output and validation details.
- [Beer01/review/comparison.png](Beer01/review/comparison.png): matching-camera offline
  before/after, wireframe and reduced-scale study. This is **not a client screenshot**.
- [generation-manifest.json](generation-manifest.json): exact prompts and built-in imagegen
  provenance. All selected image outputs are copied into the repository; no external cached
  image is needed to open either packed Blender source.
- [validation/installed-files.json](validation/installed-files.json): checksums of all 19
  installed files, identical in `src/bin/Data` and the existing macOS runtime `Data` folders.

## Scope and decisions

The requested list contains 17 base texture filenames, although the brief calls them 14.
The terrain loader uses 14; `TileGround01x`, `TileMgrass` and `TileMstone` are additional
legacy images. All 17 named files were repainted at 512×512, preserving names and extensions.

The palette uses olive grass, dusty earth, warm worn paving, carved gothic stone, cool dark
rock, restrained teal water and aged timber. Rock07's legacy logo is replaced by plain dark
slate. Fine painted detail is subordinate to material masses at reduced viewing size.

The four alpha strips retain both their original bytes and dimensions. TerrainLight is
untouched baked data. Height, mapping, object placements, walk data, minimaps, particles and
all other World1 files are unchanged: **35 non-replaced World1 files match baseline hashes**.

`Beer01.bmd` is a tavern still life, not a barrel. The user confirmed rebuilding this actual
asset after seeing the identity discrepancy. It preserves the bottle, mug, bowls, purple
grapes and vine details. Its sole replacement atlas uses existing `plate2.jpg` / `plate2.OZJ`,
which is exclusive to Beer01 among Object1 models. Shared plate, winecup, pot3 and bottle
textures are unchanged. The static prop now uses one mesh instead of five; all five bones
remain and receive rigidly assigned vertices. There are no Beer01-specific mesh-index
render controls in the source references inspected.

## Verification

| Check | Result |
|---|---|
| Original texture unwrap | 32 converted, 0 failures |
| Original texture loader check | 32 OK |
| Replacement texture loader check | 18 OK: 17 terrain + one prop atlas |
| Terrain export dimensions / color | All 512×512, RGB JPEG, proper OZJ wrappers |
| Repeated terrain review | All 17 inspected as 3×3 patches at reduced scale |
| JPEG opposite-edge continuity | Worst mean 0.424/255; worst individual channel delta 5/255 |
| Prop geometry budget | 784 triangles, below 800 |
| Prop SMD and animation validation | Both OK using bmdconv |
| Prop bones/actions | Five bones, original order/names/parents, one one-frame action, lock=0 |
| Skeleton-only bmdconv compare | EQUIVALENT; maximum bone deviation 0.0000 |
| Full remodel compare | DIFFERENT as expected for new geometry; full output retained |
| Protected World1 data | 35 files byte-identical |
| Source/runtime installation | 19 file hashes match exports in both destinations |
| New assets observed in client | **Not verified** |

Blender checks additionally assert non-degenerate triangles, one UV set, atlas-range UVs,
one texture per export mesh and exactly one valid full-weight bone assignment per vertex.
The higher-resolution source and the original geometry are excluded by the established
`REF_*` mechanism. Geometry, skeleton and action SMD files are retained for inspection.

## Reproduce

Run from the repository root. Blender Source Tools must be enabled. Packaging needs Python
with Pillow and NumPy; both are available in the Codex bundled Python runtime on this Mac.

```sh
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/GroundTiles/build_source.py
/Applications/Blender.app/Contents/MacOS/Blender -b assets-work/World1/Beer01/original/source.blend --python assets-work/World1/Beer01/build_source.py
/Applications/Blender.app/Contents/MacOS/Blender -b assets-work/World1/Beer01/source.blend --python tools/blender/mu_bmd_export.py -- --out assets-work/World1/Beer01/exports/Beer01.bmd
python3 assets-work/World1/Beer01/validate_export.py
python3 assets-work/World1/package_assets.py --install
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Beer01/render_review.py
python3 assets-work/World1/Beer01/review/assemble_sheet.py
```

`package_assets.py` writes only the explicit 17 World1 textures and two Object1 files when
`--install` is passed. Omitting it packages and checks without installing. The inventory
script is intentionally guarded against rerunning on replaced game files.

## Remaining client review

The required town-center, grass-field and rocky-area screenshots at 1920×1080 were not
completed. No substitute images are presented as client evidence. See
[baseline crash details](validation/client-baseline-crashes.md).

After client stability is resolved separately:

1. Restore the original 19 files from baseline `9a8b2027` into the runtime only, preserving
   the candidate assets in this branch. Capture the three baseline views at 1920×1080.
2. Install these exports, restart the client and repeat the same viewpoints and camera zoom.
   Inspect tile boundaries, terrain-layer transitions, grass-strip color compatibility and
   diffuse contrast under the original TerrainLight data.
3. Inspect Beer01 at Lorencia approximately `(127.4, 128.4)`; placement is unchanged at
   `(12738.604, 12838.121, 260.000)`, rotation `(0, 0, 180)`, scale 1.
4. Capture matched prop screenshots, inspect new World1/Object1 load errors, and decide
   whether the pilot meets the visual benchmark. Baseline missing audio/Object74 errors
   should be distinguished from replacement asset failures.

Do not mark this pilot verified in client until those checks pass.
