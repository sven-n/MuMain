# Lorencia static batch 01 — 2026-09-22 — ASTRA

**Offline asset work. Client acceptance is pending under the user's explicit authorization.**

## Selection

All 115 Object1 BMDs were inspected with `bmdconv info`, and eight manageable candidates were
imported through `tools/blender/mu_bmd_import.py` and rendered. See
[candidate sheet](candidates/contact-sheet.jpg), [inventory](inventory.json), and individual
geometry/texture/bone-bound reports in `candidates/`. The sheet is **Blender output**, not
client evidence. World1 placements were decoded read-only using the established map cipher;
model IDs were checked against `_enum.h` and the Lorencia loader in `MapManager.cpp`.

| Candidate | Observed identity | World1 placements | Texture decision |
|---|---|---:|---|
| **Candle01** | Three-arm bronze stand, three wax candles, animated crossed flame triangles | 6 | Selected; candle.jpg and candle2.jpg exclusive to this model |
| **TreasureChest01** | Arched-lid wooden chest with brass bindings | 3 | Selected; treasure_chest.jpg exclusive |
| **Tomb03** | Upright round-topped inscribed grave marker | 5 | Selected; tombstone.jpg exclusive |
| TreasureDrum01 | Wooden stave barrel | 64 | Deferred: drum.jpg also used by House02 |
| Furniture03 | Rectangular four-legged table | 5 | Deferred: desk_big.jpg shared by Furniture03/04/05 |
| Furniture04 | Half-round pedestal table | 3 | Same shared texture; a future coordinated furniture pass |
| Furniture05 | Tall rectangular tavern counter | 3 | Same shared texture |
| Bonfire01 | Crossed charred logs with a separate additive fire layer | 9 | Exclusive textures, but more overlap with existing fire effects |

Candle01 complements Beer01 and the tavern furniture; the chest adds a small outdoor timber
prop; Tomb03 gives the northern graveyard a stone detail that works with the completed
terrain. These are placed visible models, not hidden effect/interaction markers. Actual
visibility/occlusion at the recorded locations has **not** been observed in the client.

## Engine integration inspected

`MapManager.cpp` loads the selected models and their textures from Object1. A scan of every
Object1 model found no other users of the four replaced textures; searches for explicit
texture loads in engine/render/world source found no independent use of them. Other Data
folders have independent texture paths and are not modified.

`ZzzObject.cpp` sets Candle01's `BlendMesh = 1` and animation velocity 0.3. The two mesh slots
must remain `candle.jpg` then `candle2.jpg`; the six flame triangles, their original UVs and
bone assignments are preserved. Only their diffuse/emissive sprite painting changes. The
engine's terrain light emitter is unchanged. TreasureChest01 has velocity zero in Lorencia,
but its complete seven-frame opening action and lid binding are retained and reviewed.
No special mesh-index handling was found for Tomb03. None of these source files was edited.

## Files and reproduction

Each sibling prop folder contains preserved BMD/containers/unwrapped textures, the original
packed import, SMDs/info/placements, generated paintings and exact prompts, packed
`source.blend`, export files, validation reports and offline review sheets. Blender sources
retain `REF_ORIGINAL`, the original active rig/action and an excluded `REF_HIGH_POLY`
collection with editable higher-resolution bevel sources for future baking.

Run from the art branch repository root with Blender 5.2 + Source Tools. Python packaging/review-sheet
assembly needs Pillow (available in the bundled Codex Python runtime). The inventory and
original-preservation scripts are intentionally guarded against overwriting their baselines.

The shared primary checkout was switched to `main` externally during this session. The
protection check rejected installation there before any writes. Work continued on the
existing `art/world1-pilot` branch in the managed worktree
`/Users/webproduktion3/.codex/worktrees/world1-static-batch/MuMain`; only that source checkout
and the existing runtime received the seven files. Main was not changed or committed to.

This worktree reuses the existing converter/runtime without rebuilding engine code:

```sh
export MU_BMDCONV=/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv
export MU_RUNTIME_DATA=/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/src/Release/Main.app/Contents/MacOS/Data
export PYTHONDONTWRITEBYTECODE=1
python3 assets-work/World1/StaticBatch01/package_textures.py
# Repeat these commands with Candle01, TreasureChest01 and Tomb03:
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/Candle01/build_source.py
/Applications/Blender.app/Contents/MacOS/Blender -b assets-work/World1/Candle01/source.blend --python-exit-code 1 --python tools/blender/mu_bmd_export.py -- --out assets-work/World1/Candle01/exports/Candle01.bmd
python3 assets-work/World1/StaticBatch01/validate_export.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/StaticBatch01/audit_sources.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python-exit-code 1 --python assets-work/World1/StaticBatch01/render_review.py
python3 assets-work/World1/StaticBatch01/assemble_review.py
python3 assets-work/World1/StaticBatch01/install.py --install
```

Full `compare` reports DIFFERENT because the meshes were deliberately remodeled. Independent
zero-mesh BMD comparisons use the actual original/exported nodes and action manifests to
prove skeleton/action equivalence. Additional SMD checks compare every parent/name/index,
frame number, local translation and Euler component (modulo 2π) with 0.0001 tolerance;
this covers rotation as well as the converter's bone-origin comparison.

## Final verification and protected scope

Three model/reference SMDs and all three actions pass engine validation. All four texture
containers pass loader checks. All skeleton/action comparisons are EQUIVALENT, and Candle01's
preserved flame geometry separately compares EQUIVALENT with original UVs. Reopened packed
sources pass the reference-exclusion, common UV-set and packed-image audit. Render review
includes before/after, wireframe, scale proxies and original/exported action poses.

Seven files were installed to source/runtime with identical export hashes; the other **317
files in each World1/Object1 destination remain byte-identical to the session baseline**.
That includes all 52 World1 files, the completed Beer01 BMD/plate2 atlas, TerrainLight and all
original alpha strips. See [installed-files.json](installed-files.json). No engine or CMake
file, object placement, original game filename or out-of-scope asset was changed.

Client loading, actual lighting/blending, real-camera readability, placement contact and
matched screenshots remain unverified. These renders do not resolve the previously recorded
baseline crashes. Stability work remains separate.
