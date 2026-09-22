# Waterspout01 — Animated dragon fountain; shared rock-base surface compatibility pass

Owner: ASTRA worker `/root/reviewer`, branch `codex/lorencia-rocks`.

World1 placements: **1**. Full positions, rotations, scales and tile coordinates: `original/placements.json`.
Game model: `src/bin/Data/Object1/Waterspout01.bmd`.
Complete ordered texture dependencies: `stone_statue02.jpg`, `reagon_waterspout.jpg`, `ston01.jpg`, `ston02.jpg`.

Triangles: **639 → 639**. One UV set, one original bone per vertex, finite positions/UVs/normals, no zero-area triangles.
Full BMD comparison: **EQUIVALENT**. Skeleton/actions-only comparison: **EQUIVALENT**.

Bind bounds from actual original / final BMD conversions:

- Before: `[[-199.061005, -199.743622, -0.1378], [199.234314, 198.551697, 311.204803]]`
- After: `[[-199.061005, -199.743622, -0.1378], [199.234314, 198.551697, 311.204803]]`
- Largest exported component drift: `0.000000` game units.
- Saved-source extrema are exactly unchanged; measured export drift is SMD/rotated-root floating-point roundtrip noise.

Bone names, order, parent indices and every bind/action local translation and rotation are compared in `validation/summary.json`. Full node lists and all frame indices are retained there. Maximum local rotation delta is zero. Action index/order, lock flags, key counts and original mesh/texture order are exact.

Original bone order: `['OilTank01', 'Box04', 'Box01', 'Box02', 'Box03', 'QuadPatch01', 'Box05', 'Box06', 'Box07', 'Box08', 'QuadPatch02']`.
Action metadata: `{'Waterspout01_a00': {'index': 0, 'lock': False}}`.

Artwork: shared `../textures/ston01.jpg` is 512×512 opaque cool slate with restrained moss. Stone01/02 additionally use `../textures/ston02.tga`, 512×512 RGBA dry grass, with the original 32px graded alpha enlarged bilinearly. Grass card geometry, UVs and weights are equivalent; all hidden RGB pixels retain botanical color to avoid black/white fringes.

Packed `source.blend` contains immutable `REF_ORIGINAL`; rock models additionally retain editable `REF_HIGH_POLY`, excluded from export. Official importer/exporter were used. Original BMDs, containers, unpacked textures, info and SMD/action files are in `original/`.

Validation: `validation/commands.json`, `compare.txt`, `skeleton-compare.txt`, `summary.json`, `source-audit.json` and `preservation.json`. Matching-camera renders reimport actual final BMDs and texture containers; `review/comparison.png` includes wireframe and reduced-scale images. `review/reverse-offline.png` checks the opposite side.

The installed/exported fountain BMD is **byte-identical to the original**. Only shared `ston01.OZJ` changes. The separate official export is `validation/official-roundtrip.bmd`, validated and compared EQUIVALENT in `official-roundtrip-check.txt`, and deliberately not installed.

Mesh order remains `stone_statue02.jpg`, `reagon_waterspout.jpg`, `ston01.jpg`, `ston02.jpg`. Mesh 3 water geometry, UVs, weights and all 11 bones / 21 action keys remain exact. `ston02.OZJ` is frozen fountain WATER; `ston02.OZT` is the changed grass texture used only by Stone01/02. `stone_statue02.OZJ` and `reagon_waterspout.OZJ` are also byte-identical; hashes are in `../installed-files.json`.

Original/final action poses 0, 10 and 20 are shown in `review/action-comparison.png`. The original fountain has 12 triangles whose averaged normals oppose geometric winding; final data preserves these exactly. No new such triangles were introduced. This historical normal convention is not an export failure.

**Pending:** serial client review at placed scale, engine vertex lighting and alpha testing. No runtime installation or client observation occurred; offline renders are not client evidence. Fountain water scrolling and engine particles are not emulated by these renders.
