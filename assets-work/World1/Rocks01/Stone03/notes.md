# Stone03 — Large rounded boulder

Owner: ASTRA worker `/root/reviewer`, branch `codex/lorencia-rocks`.

World1 placements: **49**. Full positions, rotations, scales and tile coordinates: `original/placements.json`.
Game model: `src/bin/Data/Object1/Stone03.bmd`.
Complete ordered texture dependencies: `ston01.jpg`.

Triangles: **80 → 186**. One UV set, one original bone per vertex, finite positions/UVs/normals, no zero-area triangles.
Full BMD comparison: **DIFFERENT**. Skeleton/actions-only comparison: **EQUIVALENT**.

Bind bounds from actual original / final BMD conversions:

- Before: `[[-128.785614, -73.919304, -69.808998], [102.019997, 103.592506, 124.828323]]`
- After: `[[-128.785629, -73.919228, -69.808533], [102.019974, 103.593681, 124.828018]]`
- Largest exported component drift: `0.001175` game units.
- Saved-source extrema are exactly unchanged; measured export drift is SMD/rotated-root floating-point roundtrip noise.

Bone names, order, parent indices and every bind/action local translation and rotation are compared in `validation/summary.json`. Full node lists and all frame indices are retained there. Maximum local rotation delta is zero. Action index/order, lock flags, key counts and original mesh/texture order are exact.

Original bone order: `['GeoSphere01']`.
Action metadata: `{'Stone03_a00': {'index': 0, 'lock': False}}`.

Artwork: shared `../textures/ston01.jpg` is 512×512 opaque cool slate with restrained moss. Stone01/02 additionally use `../textures/ston02.tga`, 512×512 RGBA dry grass, with the original 32px graded alpha enlarged bilinearly. Grass card geometry, UVs and weights are equivalent; all hidden RGB pixels retain botanical color to avoid black/white fringes.

Packed `source.blend` contains immutable `REF_ORIGINAL`; rock models additionally retain editable `REF_HIGH_POLY`, excluded from export. Official importer/exporter were used. Original BMDs, containers, unpacked textures, info and SMD/action files are in `original/`.

Validation: `validation/commands.json`, `compare.txt`, `skeleton-compare.txt`, `summary.json`, `source-audit.json` and `preservation.json`. Matching-camera renders reimport actual final BMDs and texture containers; `review/comparison.png` includes wireframe and reduced-scale images. `review/reverse-offline.png` checks the opposite side.

Modeling uses selected convex manifold rock ridges above 25° for narrow one-segment chamfers. Edges touching an original bound-extreme vertex are protected. No blind subdivision or grass edits. Rock-only dominant-face-axis UV projection replaces stretched vertical legacy UVs; seams follow changed rock planes. Selection counts and widths are recorded in `validation/source.json`.

**Pending:** serial client review at placed scale, engine vertex lighting and alpha testing. No runtime installation or client observation occurred; offline renders are not client evidence. Fountain water scrolling and engine particles are not emulated by these renders.
