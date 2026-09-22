# HouseEtc02 — Low walled hut/platform, doorway, roof rim and rear static fixture

Owner: ASTRA fences/timber/masonry worker. Branch `codex/lorencia-masonry`.
Production date: 2026-09-22. Status: offline production and engine validation complete; coordinator acceptance and client review are separate.

Identity and placement: Low walled hut/platform, doorway, roof rim and rear static fixture. 1 unchanged World1 placements. Full positions, XYZ rotations, scales and tile coordinates are in `original/placements.json`.

Geometry: 226 → 250 triangles. 3 stone panels gain narrow beveled recesses, depth 0.65 units, inside the original panel perimeter. All original vertices, relief silhouettes, connection edges, openings, pivots and orientation are retained. The render-visible pass is primarily coordinated stone painting; the architecture's original outline is intentionally retained.

Material slots in original order: tile_wood02.jpg, tile_ston04.jpg, c_wall04.jpg, tile_wood03.jpg, tile_house01.jpg, tile_ston01.jpg, horse_drawn_01.jpg, tile_ston06.jpg.
All repainted textures are opaque RGB 512×512 JPEG/OZJ. Frozen dependency containers copied byte for byte for self-contained review: tile_wood03.OZJ, tile_wood02.OZJ, tile_ston06.OZJ, tile_house01.OZJ, tile_ston04.OZJ, horse_drawn_01.OZJ.
Exact dimensions for every owned/frozen image are in `validation/final-contract.json`.

Bind bounds before: `[[-302.53, -299.57, -0.15], [297.88, 408.2, 332.48]]`.
Bind bounds after: `[[-302.53, -299.57, -0.15], [297.88, 408.2, 332.48]]`.
All original corners survive export with maximum position component drift 0.000000000 units. 220 original triangles outside the explicitly listed panel pairs retain positions, original bone binding and UVs.

Rig proof: bone order `['ihix01', 'Box03', 'Mesh03']`. Bone names, indices and parents match exactly. Original one-frame action count and lock metadata retained. Full local translation/Euler values checked for bind and action keys: `[{'file': 'HouseEtc02.smd', 'bone_samples': 3, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}, {'file': 'HouseEtc02_a00.smd', 'bone_samples': 3, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}]`. Isolated skeleton plus actual action comparison is EQUIVALENT. Full model comparison is DIFFERENT because the stone panels were remodeled.

Source normals were copied, then normalized/requantized by the official import/export roundtrip. They are not claimed byte-equivalent: maximum retained normal component difference 0.001404000, maximum direction difference 0.081719 degrees (under 0.1 degree).
Original repeating UV range retained exactly: U `[-1.3555, 2.3555]`, V `[-0.7937, 1.7937]`. New UVs interpolate inside their original panels; no texture renaming or atlas reassignment. All exported corners have one original full-weight non-dummy bone and finite geometry/UV values.

Evidence: `validation/compare.txt`, `skeleton-compare.txt`, `local-motion.json`, `modular-anchors.json`, `smd-validation.txt`, `info-after.txt`, `texture-check.txt`, `source-audit.json`, `final-contract.json` and `summary.json`.
Review: `review/comparison.png` contains matched-camera actual before/after BMD renders, exported wireframe and reduced offline grass/190-unit figure study. The camera is an offline assumption. Batch assemblies are in `../review/` and use untouched decoded World1 transforms.

Sources: immutable original BMD, OZJs, unwrapped images, bmdconv info/SMD/actions and official-import Blender file in `original/`. Packed editable `source.blend` contains hidden export-excluded REF_ORIGINAL and REF_HIGH_POLY, the original rig/action, and final mesh. Optional high-poly bevel is a retained bake candidate, not an engine asset. Painted master PNGs and exact built-in imagegen prompts are in `../paintings/`; final editable PNG/JPEG copies are in `textures/`. Official exporter produced `exports/`.

Pending: actual client screenshots, lighting/readability, occlusion and traversal review. No runtime or client session was used. World1 placement, terrain height, walk/collision data, TerrainLight and alpha strips are unchanged.
