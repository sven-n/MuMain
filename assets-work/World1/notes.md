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


## Static batch 01 — 2026-09-22

**Three more props exported, validated offline and installed; client acceptance pending.**
Inventory covered all 115 Object1 models, with eight candidates imported and visually
inspected. The [selection report](StaticBatch01/README.md) records identities, placements,
shared texture exclusions and source render controls. Selected:

| Prop | Observed identity | Triangles before → after | Textures before → after | Placements |
|---|---|---:|---|---:|
| [Candle01](Candle01/notes.md) | Three-candle bronze stand | 116 → 1,018 | candle 32² → 512²; candle2 16² → 128² | 6 |
| [TreasureChest01](TreasureChest01/notes.md) | Arched timber chest | 66 → 1,174 | treasure_chest 128² → 512² | 3 |
| [Tomb03](Tomb03/notes.md) | Upright carved grave marker | 30 → 200 | tombstone 128² → 512² | 5 |

All replaced textures are exclusive to their selected model. The barrel was deferred because
its drum texture is shared with House02; the three tables/counter share desk_big and belong
in a coordinated future pass. Candle01 complements the tavern still life directly; the chest
and grave marker add wood/stone detail to the completed terrain surroundings.

The per-prop folders retain original BMDs, containers, unwrapped textures, info/SMD/action
files and placements; packed original/rebuilt Blender sources; original references and
excluded higher-resolution geometry; imagegen paintings/prompts; final game exports;
comparison, wireframe, scale and action renders; and complete validation reports.
[Batch preview](StaticBatch01/batch-review.jpg) is explicitly **offline Blender output**.
The final review images use re-imported BMD exports, with original matched camera/lighting.
Scale studies use 100-unit terrain repeats and a 190-unit proxy; they are not actual client
camera captures. Candle additive rendering in Blender is an approximation.

Three reference models and their actions pass bmdconv validation; four textures pass loader
checks. Full compares report DIFFERENT for intentional geometry changes. Skeleton/action-only
compares report **EQUIVALENT** for all three, with zero reported bone-origin deviations.
Additional checks verify bone indices/names/parents, all local translations/rotations, action
order, 7/7/1 frame counts and lock=0. Candle01 retains its two mesh slots, original six flame
triangles, weights and UVs, with a separate EQUIVALENT comparison. New geometry has exactly
one valid bone per vertex. Bounds match, except for the chest keyhole's 0.0036-unit projection.
All three sources reopen with packed images and excluded REF collections.

[Installation checksums](StaticBatch01/installed-files.json) cover seven new replacements
(three BMDs, four OZJs), installed in source and the existing macOS runtime. The 317 other
World1/Object1 files remain byte-identical in both destinations, including all completed
terrain, TerrainLight, alpha strips and Beer01. No engine/CMake changes or filenames changed.

During validation another process switched the shared primary checkout to main. The
installation guard caught the resulting baseline mismatch and wrote nothing there. This
batch then continued on **art/world1-pilot** in
`/Users/webproduktion3/.codex/worktrees/world1-static-batch/MuMain`. The art source checkout
and existing runtime contain the replacements; main was not committed to or overwritten.

Pending client checks: asset loads/error log, candle blend/flicker and terrain light,
chest/stone material readability at real zoom, footprint/contact/orientation, and matched
before/after screenshots. Start with Candle01 near `(126.58,128.25)`, TreasureChest01 near
`(185.17,140.06)`, and Tomb03 near `(130.50,215.00)`. Full placements and bounds are in each
prop's notes. The user's offline authorization remains in force; no client verification is
claimed and no stability work was attempted.


## 2026-09-22 — Complete Lorencia static art integration

All 106 in-scope static models are accepted offline on `art/lorencia-rebuild` in the sibling `MuMain-lorencia-rebuild` worktree. The 17 terrain paintings and four completed pilot props above remain unchanged. Use the [consolidated handoff](coordination/notes.md), [asset board](coordination/asset-board.md), [exact changed game files](coordination/changed-game-files.md) and [combined offline gallery](coordination/final-review.md) for current ownership, commits and evidence. There is no remaining static production batch. Client verification and real 1920×1080 captures remain pending; this rebuild task performed no shared-runtime installation or client launch.
