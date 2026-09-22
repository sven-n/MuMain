# StoneMuWall02 — Thin tall dragon buttress with below-ground extension

Owner: ASTRA fences/timber/masonry worker. Branch `codex/lorencia-masonry`.
Production date: 2026-09-22. Status: offline production and engine validation complete; coordinator acceptance and client review are separate.

Identity and placement: Thin tall dragon buttress with below-ground extension. 32 unchanged World1 placements. Full positions, XYZ rotations, scales and tile coordinates are in `original/placements.json`.

Geometry: 44 → 76 triangles. 4 stone panels gain narrow beveled recesses, depth 0.65 units, inside the original panel perimeter. All original vertices, relief silhouettes, connection edges, openings, pivots and orientation are retained. The render-visible pass is primarily coordinated stone painting; the architecture's original outline is intentionally retained.

Material slots in original order: c_wall06.jpg, c_wall05.jpg, c_wall04.jpg.
All repainted textures are opaque RGB 512×512 JPEG/OZJ. Frozen dependency containers copied byte for byte for self-contained review: None.
Exact dimensions for every owned/frozen image are in `validation/final-contract.json`.

Bind bounds before: `[[-50.01, -147.4, -174.17], [49.99, 49.92, 334.09]]`.
Bind bounds after: `[[-50.01, -147.4, -174.17], [49.99, 49.92, 334.09]]`.
All original corners survive export with maximum position component drift 0.000000000 units. 36 original triangles outside the explicitly listed panel pairs retain positions, original bone binding and UVs.

Rig proof: bone order `['Box01']`. Bone names, indices and parents match exactly. Original one-frame action count and lock metadata retained. Full local translation/Euler values checked for bind and action keys: `[{'file': 'StoneMuWall02.smd', 'bone_samples': 1, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}, {'file': 'StoneMuWall02_a00.smd', 'bone_samples': 1, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}]`. Isolated skeleton plus actual action comparison is EQUIVALENT. Full model comparison is DIFFERENT because the stone panels were remodeled.

Source normals were copied, then normalized/requantized by the official import/export roundtrip. They are not claimed byte-equivalent: maximum retained normal component difference 0.000179000, maximum direction difference 0.011214 degrees (under 0.1 degree).
Original repeating UV range retained exactly: U `[-1.339, 0.9995]`, V `[-0.8529, 1.6659]`. New UVs interpolate inside their original panels; no texture renaming or atlas reassignment. All exported corners have one original full-weight non-dummy bone and finite geometry/UV values.

Evidence: `validation/compare.txt`, `skeleton-compare.txt`, `local-motion.json`, `modular-anchors.json`, `smd-validation.txt`, `info-after.txt`, `texture-check.txt`, `source-audit.json`, `final-contract.json` and `summary.json`.
Review: `review/comparison.png` contains matched-camera actual before/after BMD renders, exported wireframe and reduced offline grass/190-unit figure study. The camera is an offline assumption. Batch assemblies are in `../review/` and use untouched decoded World1 transforms.

Sources: immutable original BMD, OZJs, unwrapped images, bmdconv info/SMD/actions and official-import Blender file in `original/`. Packed editable `source.blend` contains hidden export-excluded REF_ORIGINAL and REF_HIGH_POLY, the original rig/action, and final mesh. Optional high-poly bevel is a retained bake candidate, not an engine asset. Painted master PNGs and exact built-in imagegen prompts are in `../paintings/`; final editable PNG/JPEG copies are in `textures/`. Official exporter produced `exports/`.

Pending: actual client screenshots, lighting/readability, occlusion and traversal review. No runtime or client session was used. World1 placement, terrain height, walk/collision data, TerrainLight and alpha strips are unchanged.
