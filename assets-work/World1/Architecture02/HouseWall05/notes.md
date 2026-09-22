# HouseWall05 — Corner roof cap with two shingled eaves

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-timber`.
Date: 2026-09-22. Offline production/validation complete; actual client acceptance pending.

Identity and World1 placement: Corner roof cap with two shingled eaves. 8 existing placements; exact positions, XYZ rotations, scale and tile coordinates in `original/placements.json`.

Geometry: 30 → 54 triangles. All sloped shingle faces, original corner outline, roof connection edges and exterior contacts remain unchanged. Only the flat deck and two inward-facing timber trim panels are dressed. Footprint, origin, orientation, bone pivots and all original corners retained. New recess depths are 0.18 units for ladder wear, 0.4 for roof interiors and 0.65 for stone panels. Explicit modified pairs/perimeters are recorded in `validation/blender.json`. This is conservative architectural edge refinement, with the primary visual gain supplied by the shared material paintings.

Material slots, unchanged order: tile_wood02.jpg, tile_wood03.jpg.
Actual material identities differ from filenames: tile_wood01 is coursed stone ashlar; tile_wood03 is green roof shingles. The paintings retain the original block/joint and staggered-shingle layout. The ladder's original reuse of the stone atlas is deliberately retained. Both paintings are 512×512 RGB, packaged through mu_texture as JPEG/OZJ; no material rename or new dependency.

Dimensions of every owned/frozen texture:

- `tile_wood02.jpg`: [128, 128], RGB
- `tile_wood03.jpg`: [512, 512], RGB

Frozen dependency containers retained byte for byte: tile_wood02.OZJ.
All other textures, including tile_wood02 timber, ston02 water, tile_ston04 and accepted Architecture01/Masonry paints, are unchanged.

Bind bounds before: `[[-259.78, -259.68, -45.47], [200.3, 200.29, 44.57]]`.
Bind bounds after: `[[-259.78, -259.68, -45.47], [200.3, 200.29, 44.57]]`.
Every original vertex position/bone survives final conversion within 0.000000000 units. 24 protected triangles retain positions, bone assignments and UVs. Maximum retained UV delta 0; normal component delta 0.000000000; normal direction delta 0.00000000 degrees. Original UV ranges: `[[-2.7853, 2.3599], [-0.676, 2.6927]]`.

Rig: `['Box01']`. Names, indices, order, parents and original one-action lock metadata unchanged; 1 action keys. Full local pose evidence: `[{'file': 'HouseWall05.smd', 'bone_samples': 1, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}, {'file': 'HouseWall05_a00.smd', 'bone_samples': 1, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}]`. All 1 hierarchical action matrices match with maximum component delta 0; every action-frame bound matches with maximum component delta 0. Per-frame matrices/bounds evidence is in `validation/matrices-and-posed-bounds.json`.
Full model comparison: DIFFERENT because selected panel geometry was remodeled. Isolated skeleton plus actual action comparison: EQUIVALENT. Geometry and rig equivalence are separate claims.

Special renderer contract: House05 mesh 2 ston02 water and House04 mesh 8 tile_space01 preserve original geometry/UVs and world-time V scroll. HouseWall02 mesh 4 light_02 keeps original additive brightness behavior. HouseWall05/06 keep entire-object HeroTile 4 fade; no new meshes or material slots are introduced. Source excerpts are retained in `../engine-contract.txt`.

Pipeline: official mu_bmd_import.py imports originals. Packed `source.blend` keeps export-excluded REF_ORIGINAL and editable REF_HIGH_POLY candidates, original rig/action and authored mesh. Official mu_bmd_export.py exports all six authored assets. `preserve_contract.py` then restores original protected corners, exact panel-boundary coordinates, bind and action data through supported bmdconv SMD/manifest packaging. This corrects Blender roundtrip quantization; new interior vertices and bevel normals stay authored. Raw official exports and final packing inputs/logs are retained, and raw-versus-final compare EQUIVALENT. Readonly compatibility BMDs are copied exactly and bypass export.

Evidence: full compare, skeleton compare, SMD validation, info, local-motion, matrices-and-posed-bounds, authored-bindings, raw-bone-bindings, modular-anchors, texture-check, final-contract and packed source-audit reports in `validation/`. Original BMD/container bytes independently match pinned Git baseline. Final game hashes are in `summary.json`.

Review: matching original versus actual re-imported final BMD, clear wireframe and reduced-scale previews in `review/comparison.png`. Mill and readonly workshop action sheets use frames 0/14/29 and 0/19/39 respectively. Effect sheets compare neutral diffuse versus labeled Blender additive approximation; illustrative scroll offsets are not engine captures. Actual unchanged World1 assemblies are in `../review/`. The town-interior assembly hides roof caps to illustrate HeroTile 4 target alpha 0; this is an offline approximation.

Sources: immutable game originals, unwrapped textures, official-import blend, bmdconv info/SMD/actions/manifest and placement records in `original/`; editable PNGs and final JPEGs in `textures/`; exact imagegen prompts and untouched generated master PNGs in `../paintings/`; final game files in `exports/`.

Pending: real client loading, lighting/filtering, water blending/scroll, mill motion, roof fade transition, ladder contacts, traversal and matching 1920×1080 screenshots. No runtime/client was used. Terrain placement, height, walk/collision, baked lighting and alpha strips are unchanged.
