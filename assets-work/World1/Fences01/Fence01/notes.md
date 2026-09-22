# Fence01 — Two-post rustic timber fence with two sagging rails

Status: exported and validated offline; not verified in client. Owner: ASTRA fence worker,
branch `codex/lorencia-fences`; worktree `MuMain-lorencia-fences`.

Chamfered post caps and bases, correct longitudinal timber UV direction, and solid closed triangular rails. Original sag/bend points, rail/post interfaces, opening, dimensions and pivot retained.

## Actual World1 placement

180 instances, object type 81. Full untouched placement records are in
`original/placements.json`. Example tile: [96.0, 62.0]; world position:
[9600.0, 6200.0, 164.99998474121094]; rotation degrees: [0.0, 0.0, 810.0]; scale 1.0.

The names are loader identifiers, not inferred identities: Fence02–04 are 22.6887-unit-high
curb pieces. `MapManager.cpp:1059` loads Fence01–04 through MODEL_FENCE01+i; the four enums
are at `_enum.h:790`. Source search found no fence-specific mesh-index render code. One
material/mesh remains in all four, in the original slot.

## Counts, bounds and dependencies

- Triangles: 36 → 116 (prop budget 1500).
- Before bind minimum / maximum: -88.82341, -9.03141, -0.33463 / 88.93400, 11.97761, 100.83080.
- Authored minimum / maximum: -88.82341, -9.03141, -0.33461 / 88.93400, 11.97761, 100.83080.
- Engine-reported before bounds: -88.82000, -9.03000, -0.33000 / 88.93000, 11.98000, 100.83000.
- Engine-reported after bounds: -88.82000, -9.03000, -0.33000 / 88.93000, 11.98000, 100.83000.
- One original root bone, `Box07`, index 0, parent -1. Names, order and parents identical.
- One action, index 0, one key, lock=0. Bind and every local action translation/rotation unchanged.
- `src/bin/Data/Object1/tile_wood02.OZJ` (128x128) is frozen, byte-identical. Only geometry and UVs changed; no painted replacement was allowed for this shared texture.

## Validation

`validation/smd-validation.txt` retains full original/replacement reference and animation
validation output. `validation/info-after.txt`, `compare.txt`, `skeleton-compare.txt` and
`local-motion.json` retain engine output and every local-transform comparison.

Full model comparison (geometry intentionally differs):

```text
meshes 1 vs 1, bones 1 vs 1, actions 1 vs 1, triangles 36 vs 116
unmatched triangles: 36  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
DIFFERENT
```

Isolated skeleton plus actual action comparison:

```text
meshes 0 vs 0, bones 1 vs 1, actions 1 vs 1, triangles 0 vs 0
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

Full-model equivalence is not claimed. All local position and Euler component deviations
are zero in the retained converter output. One valid full-weight bone per vertex, no dummy
binding, one UVMap in [0,1], one opaque diffuse material, finite nondegenerate triangles,
identity export transforms and packed image dependencies pass. Texture loader checks pass.
No alpha texture or render flag was introduced.

`validation/modular-joins.json` checks original interface corners against re-imported final
BMD vertices: maximum error 0.001339736 units, tolerance 0.002.
The three curb assets retain their connection vertices exactly. Fence01's rotated imported
rig incurs below 0.002-unit Blender conversion drift, far below 0.00002 terrain tiles.
The source/export bounds match at bmdconv's printed precision.

`source.blend` retains the original rig/action, hidden `REF_ORIGINAL`, excluded bevel-ready
`REF_HIGH_POLY`, export geometry and packed images. `validation/source-audit.json` proves
packed images and reference exclusion. `original/` retains untouched BMD/container, decoded
texture, converter SMD/actions/info, official imported blend and inspection geometry/render.

## Review and pending checks

`review/comparison.png`: same-camera original versus re-imported game BMD, wireframe and
reduced-scale proxy. All images are **offline Blender evidence**, never client screenshots.
The scale study uses 100-unit ground repeats and a 190-unit proxy; it is not a measured
client camera. `../review/modular-joins.png` shows real unchanged World1 placement transforms
for a town corner and straight run, without unrelated terrain/buildings.

No runtime or client was operated. Client loading/logs, actual diffuse lighting, contact,
collision readability and 1920x1080 before/after screenshots remain pending under the user's
offline authorization. See `../notes.md` for reproduction and batch dependency scope.
