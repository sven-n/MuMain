# StoneWall01: Raised portcullis gateway with rough masonry piers

Owner: ASTRA review/ironwork/stone-wall worker.
Branch: `codex/lorencia-stone-walls`. Status: offline production complete; coordinator review required.

World1 placements: 8; exact transforms are retained in `original/placements.json`.
Game model: `src/bin/Data/Object1/StoneWall01.bmd`.
Triangles: 318 → 414.
Bind bounds before: `[[-298.04, -84.12, 0.78], [297.44, 88.56, 623.57]]`; after: `[[-298.04, -84.12, 0.78], [297.44, 88.56, 623.57]]`.

Complete material dependencies, in original engine mesh order:

- `tile_01.OZJ` / `tile_01.jpg`: [512, 512], RGB; repainted.
- `tile_03.OZJ` / `tile_03.jpg`: [512, 512], RGB; repainted.
- `tile_02.OZJ` / `tile_02.jpg`: [64, 64], RGB; frozen byte-identical.
- `bridge_01.OZJ` / `bridge_01.jpg`: [256, 128], RGB; frozen byte-identical.

Selected stone-panel interiors are recessed 0.65 units. Original boundary vertices, wall endpoints,
opening-facing jamb planes and protected fixture/banner triangles are retained. No footprint, origin,
orientation, placement, collision or terrain edits.

Original rig order: `['gate']`. Action metadata: `['action 0 keys=1 lock=0']`.
Full geometry comparison: **DIFFERENT**, intentional remodel. Skeleton/actions: **EQUIVALENT**.
Bone names, order, parents, frame counts and lock metadata are preserved.
Maximum exported original-corner component drift: 0.000000000 units.
Maximum retained UV component error: 0.
Maximum retained-normal direction difference: 0.039789053 degrees.
Sub-millimeter exported drift is official floating-point roundtrip precision; source anchors are unmoved.
The 0.001-unit corner tolerance is explicit in the anchor report; no full geometry equivalence claimed.

- `StoneWall01.smd`: 1 frames, 1 bone samples; max local translation 0.000000000, wrapped rotation 0.000000000 radians.
- `StoneWall01_a00.smd`: 1 frames, 1 bone samples; max local translation 0.000000000, wrapped rotation 0.000000000 radians.

Evidence: `validation/` retains full comparisons, bmdconv validation/info, source audit,
UV/skin/alpha/material checks, protected-component comparisons, frozen hashes and local motion.
`source.blend` is packed, with excluded `REF_ORIGINAL` and `REF_HIGH_POLY` plus export geometry.
`original/` contains untouched game files, unwrapped textures, bmdconv info/SMDs and official import.
`review/comparison.png` compares actual exported BMD against original with matching camera,
wireframe and reduced-scale previews. Painted masters and prompts are shared within `../paintings/`.

**Pending:** serial in-client inspection. No runtime installation or client verification performed.
