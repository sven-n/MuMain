# HouseWall02 — Timber and stone window-wall module with raised decorative shutter

Owner: ASTRA fences/timber/masonry/architecture worker. Branch `codex/lorencia-architecture`.
Date: 2026-09-22. Offline production and engine validation complete. Actual client verification pending.

Identity and placement: Timber and stone window-wall module with raised decorative shutter; 15 unchanged World1 placements. Actual coordinates, rotations, scales and tile coordinates are retained in `original/placements.json`.

Geometry: 70 → 110 triangles. 5 selected panel interiors gain narrow beveled recesses 0.25–0.65 units deep. All original perimeter corners, openings, roof edges and footings remain in place. Footprints, origins, orientation and modular boundaries are unchanged. `validation/blender.json` identifies every modified face pair and original perimeter. High-poly bevel candidates are retained for editing only, excluded from game export.

Material slot order: tile_ston06.jpg, tile_wood01.jpg, tile_wood02.jpg, tile_ston04.jpg, light_02.jpg.
Mesh 4 light_02.jpg; engine random additive brightness 0.4–0.7.
The indexed effect surfaces and original UVs are protected. Smithy awning geometry stays 12 triangles; Tent canopy stays 26 triangles with original per-vertex rigid bone assignments. No additive geometry was remodeled.

Texture dimensions and modes:

- `light_02.jpg`: [128, 128], RGB
- `tile_ston04.jpg`: [128, 128], RGB
- `tile_ston06.jpg`: [128, 128], RGB
- `tile_wood01.jpg`: [128, 128], RGB
- `tile_wood02.jpg`: [128, 128], RGB

Frozen dependency containers, copied unchanged for self-contained review: tile_wood01.OZJ, tile_wood02.OZJ, tile_ston06.OZJ, tile_ston04.OZJ.
The six owned paint files preserve material roles and exact filenames. tile_house01 keeps woven linen above and carved winged-scroll door panel below. tile_ston05 is muted rough tan limestone; tile_windows01 is aged oak; tile_ston07 keeps horizontal timber slats with the exact original alpha mask enlarged nearest4x. tile_space01 remains a restrained blue swirl; light_02 remains a narrow warm light-to-black additive gradient.

Bind bounds before: `[[-100.95, -110.97, 0.28], [101.14, 49.55, 264.48]]`.
Bind bounds after: `[[-100.95, -110.97, 0.28], [101.14, 49.55, 264.48]]`.
Every original vertex and bone assignment retained within 0.000008000 units. 60 protected triangles preserve positions and UVs. Maximum unchanged UV component delta: 0. Maximum protected normal component delta: 0.000000000; direction delta: 0.00000000 degrees.

Rig: `['sdfeew', 'Object01', 'Object03']`. Names, order, indices and parents exactly retained, together with one action and 1 keys plus original lock metadata. All local bind/action translations and Euler components are checked, not just key counts: `[{'file': 'HouseWall02.smd', 'bone_samples': 3, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}, {'file': 'HouseWall02_a00.smd', 'bone_samples': 3, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}]`. Skeleton plus actual action compare EQUIVALENT. Full model DIFFERENT because geometry was remodeled; no full-equivalence claim.

Official Blender exporter provides remodeled mesh data. `preserve_contract.py` then restores untouched original triangle corners and original bind/actions using the documented bmdconv SMD/manifest workflow. This fixes Blender split-normal quantization and House04 quaternion/Euler drift without changing new panel topology. The raw official BMD and conversion files remain in `validation/official-export/`; the final packing inputs remain in `validation/locked-motion/`. Their comparison is EQUIVALENT. The readonly annex bypasses this conversion and is copied exactly.

UV range retained: U `[-2.7466, 3.3092]`, V `[-3.4185, 0.9995]`. New UVs interpolate inside existing panel islands. Export has finite vertices/UVs, one original full-weight non-dummy bone per corner, original material order and unchanged texture names. Opaque images remain RGB; awning TGA is bottom-left uncompressed 32-bit RGBA with original alpha retained exactly. All OZJ/OZT wrappers pass mu_texture checks.

Evidence: `validation/compare.txt`, `skeleton-compare.txt`, `local-motion.json`, `modular-anchors.json`, `smd-validation.txt`, `info-after.txt`, `texture-check.txt`, `source-audit.json`, `final-contract.json`, `summary.json`. Immutable original bytes are independently checked against the pinned baseline commit.

Review: `review/comparison.png` contains matching-camera original and actual re-imported final BMD renders, exported wireframe and reduced-scale grass/190-unit figure study. `effect-comparison.jpg` exists for indexed-effect models; its neutral diffuse and approximate additive images are clearly labeled. House04 action frames 0/19/39 and Tent frames 0/17/35 are compared in `action-comparison.jpg`. House04 scroll offsets 0/-1/3/-2/3 are illustrative; the actual engine uses world time. All images are Blender offline evidence, never client screenshots.

Sources: original BMD/containers/unwrapped images/bmdconv info/SMD/manifest/official-import blend under `original/`. Packed editable `source.blend` contains REF_ORIGINAL and REF_HIGH_POLY, hidden and excluded from export, plus the original rig and final authored mesh. Painted masters and exact imagegen prompts are retained in `../paintings/`; editable PNGs and final images are in `textures/`; wrapped game exports in `exports/`.

Pending: real client lighting, alpha sorting/additive appearance, motion readability, occlusion and traversal checks at 1920×1080. No client or shared runtime was operated. No World1 placement, terrain, walk/collision, TerrainLight or alpha-strip data was edited.
