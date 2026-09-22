# Fences01 — Lorencia timber fence and ornamental curbs

2026-09-22. Owner: ASTRA fence worker. Branch `codex/lorencia-fences`, isolated worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-fences`.
Status: four assets exported and validated offline; coordinator review/integration and
client verification pending. This directory is the worker's complete deliverable.

| Asset | Actual identity | Placements | Triangles before → after | Texture |
|---|---|---:|---:|---|
| Fence01 | Two-post, two-rail timber fence | 180 | 36 → 116 | tile_wood02, frozen 128² |
| Fence02 | Low ornamental curb, L corner | 33 | 16 → 112 | joint, 512² |
| Fence03 | Low ornamental curb, 200-unit straight | 38 | 10 → 112 | joint, 512² |
| Fence04 | Low ornamental curb, 100-unit straight | 17 | 10 → 56 | joint, 512² |

The three shallow curb models are not standing fences. Their 22.6887-unit height,
sloped faces, exact external end profiles and L-corner miter are preserved. Individual
stone joints are inset within that original envelope. The old material's upper stone and
lower decorated dark band remain on front, rear and ends. Timber retains its recognizable
bent rails and openings; frozen shared timber limits this asset to geometry/UV improvement.

## Exact authorized game changes

- `src/bin/Data/Object1/Fence01.bmd`
- `src/bin/Data/Object1/Fence02.bmd`
- `src/bin/Data/Object1/Fence03.bmd`
- `src/bin/Data/Object1/Fence04.bmd`
- `src/bin/Data/Object1/joint.OZJ`

The complete dependency group for joint.OZJ is Fence02/Fence03/Fence04. Its original 32²
texture was repainted through the built-in imagegen tool. Exact prompt, raw editable PNG
master, provenance and deterministic size/wrap script are retained in `paintings/`.
Final diffuse is 512² RGB JPEG/OZJ; no alpha, PBR, normal map or render-flag change.

`tile_wood02.OZJ` is frozen because it is shared with these other consumers:
BridgeStone01, Fence01, House04, House05, HouseEtc02, HouseWall01, HouseWall02, HouseWall03, HouseWall04, HouseWall05, HouseWall06.
It is retained byte-identically in Fence01's original/export folders, and is not a game
replacement. No texture renaming or new game filename avoids ownership boundaries.

## Evidence

All four assets pass bmdconv reference/animation validation and texture loader checks.
All retain one original mesh/material, one original bone and one one-frame action, lock=0.
Bone names/order/parents and every bind/action local translation/rotation are unchanged;
isolated rig/action BMDs compare EQUIVALENT. Full compares are DIFFERENT for deliberate
geometry changes. All before/after engine bind bounds agree at printed precision.
New vertices each have one valid full-weight bone; UVs/materials/export exclusion checked.
Original curb connection vertices are exact; timber interfaces differ by at most 0.00135
units after the official Blender conversion, within a documented 0.002-unit tolerance.

Each asset folder includes packed source.blend with REF_ORIGINAL and excluded high-poly
source, untouched originals, exported game files, final editable textures, validation
logs, notes and labeled before/after/wireframe/reduced-scale comparison. All final review
images use a re-import of the exported BMD, not the authoring mesh.

Batch sheet: `review/batch-review.jpg`; per-asset sheets: `<asset>/review/comparison.png`.
`review/modular-joins.png` uses the original World1 position/rotation/scale of actual town
corner and straight-run instances. Full placement lists and render caveats are retained.
These are Blender renders, **not client evidence**. No runtime or client was touched.

## Reproduce

Run commands from this worktree or the integrated checkout. Set `MU_BMDCONV` to the built
converter and `PYTHONDONTWRITEBYTECODE=1`. Blender requires enabled Source Tools.
Python packaging/review assembly requires Pillow.

```sh
export PYTHONDONTWRITEBYTECODE=1
export MU_BMDCONV=/path/to/bmdconv
python3 assets-work/World1/Fences01/package_textures.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Fences01/build_source.py
python3 assets-work/World1/Fences01/export_all.py
python3 assets-work/World1/Fences01/validate_export.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Fences01/render_exports.py
/Applications/Blender.app/Contents/MacOS/Blender -b --python assets-work/World1/Fences01/render_joins.py
python3 assets-work/World1/Fences01/assemble_review.py
python3 assets-work/World1/Fences01/write_notes.py
python3 assets-work/World1/Fences01/install_source.py
```

`prepare_originals.py` is baseline-only and refuses to overwrite original folders.
`install_source.py` writes only the listed five owned source game files; it never touches
the shared runtime. It checks all other World1/Object1 files and frozen timber unchanged.
The installation report is `source-installation.json`.

Pending: coordinator integration and combined dependency verification; actual client load,
lighting, zoom/readability, contacts/collision and serial 1920x1080 before/after captures.
No engine/CMake, UI, character, terrain placement/height/walk/light/alpha-strip files changed.
