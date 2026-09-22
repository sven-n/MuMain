# StoneWall06: Rough masonry pier with animated heraldic banner

Owner: ASTRA review/ironwork/stone-wall worker.
Branch: `codex/lorencia-stone-walls`. Status: offline production complete; coordinator review required.

World1 placements: 2; exact transforms are retained in `original/placements.json`.
Game model: `src/bin/Data/Object1/StoneWall06.bmd`.
Triangles: 56 → 88.
Bind bounds before: `[[-50.73, -49.66, -79.37], [49.75, 88.94, 448.76]]`; after: `[[-50.73, -49.66, -79.37], [49.75, 88.94, 448.76]]`.

Complete material dependencies, in original engine mesh order:

- `badge_01.OZJ` / `badge_01.jpg`: [8, 64], RGB; frozen byte-identical.
- `badge_03.OZT` / `badge_03.tga`: [64, 64], RGBA; frozen byte-identical.
- `tile_01.OZJ` / `tile_01.jpg`: [512, 512], RGB; repainted.

Selected stone-panel interiors are recessed 0.65 units. Original boundary vertices, wall endpoints,
opening-facing jamb planes and protected fixture/banner triangles are retained. No footprint, origin,
orientation, placement, collision or terrain edits.

Original rig order: `['bonee01', 'bonee02', 'bonee03', 'bonee04', 'bonee05', 'bonee06', 'Box24', 'Object04', 'Object05']`. Action metadata: `['action 0 keys=25 lock=0']`.
Full geometry comparison: **DIFFERENT**, intentional remodel. Skeleton/actions: **EQUIVALENT**.
Bone names, order, parents, frame counts and lock metadata are preserved.
Maximum exported original-corner component drift: 0.000366000 units.
Maximum retained UV component error: 0.
Maximum retained-normal direction difference: 0.008013944 degrees.
Sub-millimeter exported drift is official floating-point roundtrip precision; source anchors are unmoved.
The 0.001-unit corner tolerance is explicit in the anchor report; no full geometry equivalence claimed.

- `StoneWall06.smd`: 1 frames, 9 bone samples; max local translation 0.000053000, wrapped rotation 0.000000000 radians.
- `StoneWall06_a00.smd`: 25 frames, 225 bone samples; max local translation 0.000078000, wrapped rotation 0.000000000 radians.

Evidence: `validation/` retains full comparisons, bmdconv validation/info, source audit,
UV/skin/alpha/material checks, protected-component comparisons, frozen hashes and local motion.
`source.blend` is packed, with excluded `REF_ORIGINAL` and `REF_HIGH_POLY` plus export geometry.
`original/` contains untouched game files, unwrapped textures, bmdconv info/SMDs and official import.
`review/comparison.png` compares actual exported BMD against original with matching camera,
wireframe and reduced-scale previews. Painted masters and prompts are shared within `../paintings/`.

**Pending:** serial in-client inspection. No runtime installation or client verification performed.
