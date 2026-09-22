# StoneWall06: Accepted masonry pier with shared mounted-rider banner; compatibility only

Owner: ASTRA reviewer/production worker. Branch: `codex/lorencia-signs-banners`.
Status: offline production complete; coordinator visual review and integration pending.

World1 placements: 2; complete untouched transforms in `original/placements.json`.
Game path: `src/bin/Data/Object1/StoneWall06.bmd`.
Triangles: 88 → 88.
Before bind bounds: `[[-50.73, -49.66, -79.37], [49.75, 88.94, 448.76]]`.
After bind bounds: `[[-50.73, -49.66, -79.37], [49.75, 88.94, 448.76]]`.

Complete texture dependencies in preserved engine mesh order:

- `badge_01.OZJ` / `badge_01.jpg`: [64, 512], RGB; shared painted texture.
- `badge_03.OZT` / `badge_03.tga`: [512, 512], RGBA; shared painted texture.
- `tile_01.OZJ` / `tile_01.jpg`: [512, 512], RGB; frozen byte-identical.

Original origins, orientation, footprint and every original vertex are retained.
Signs have 0.25-unit panel recesses inside unchanged original wood boundaries; the two panel interiors
gain explicit edge profiles. Iron ornament, suspension rings, poles, cloth, stone and attachment triangles
retain position, bone and UV corners. No placement, collision or terrain edits.

Full BMD comparison: **EQUIVALENT**. Rig/actions: **EQUIVALENT**.
Bone names/order: `['bonee01', 'bonee02', 'bonee03', 'bonee04', 'bonee05', 'bonee06', 'Box24', 'Object04', 'Object05']`.
Action metadata: `['action 0 keys=25 lock=0']`.
Exact node-name/order/parent and action-count/key-count/lock checks pass.
Max exported original-corner component drift: 0.000000000 units.
Retained triangle count: 88; max retained UV error: 0.
Max retained normal direction change: 0.000000000 degrees.
Official SMD export normalizes/requantizes normals; no byte-equivalence of recomputed normals is claimed.

- `StoneWall06.smd`: 1 frames / 9 bone samples; max local position drift 0.000000000, max local rotation-matrix component drift 0.000000000.
- `StoneWall06_a00.smd`: 25 frames / 225 bone samples; max local position drift 0.000000000, max local rotation-matrix component drift 0.000000000.

Compatibility export is the accepted BMD from baseline 0800af23 (accepted StoneWalls01 revision 149780be),
retained byte-identically together with tile_01.OZJ. Its official Blender roundtrip is retained separately
as `validation/official-roundtrip.bmd`; it is evidence only and is not installed. Only shared banner/pole
textures change in this compatibility model. Source and matching render reflect the accepted 88-triangle pier.

`source.blend` is packed with excluded `REF_ORIGINAL` and `REF_HIGH_POLY` references.
Untouched originals, official original import, textures, info and SMDs are in `original/`.
Full comparisons, validated model/action SMDs, skin/UV/material checks, local motion and source audits
are in `validation/`. The exact original scalar alpha fields are bilinearly enlarged; four RGBA masks
are pixel-checked and keep padded hidden RGB. Editable artwork, prompts and raw generations are in `../paintings/`.

`review/comparison.png` contains matching-camera actual BMD before/after renders, wireframe and
reduced-scale views. Animated models also include `review/action-comparison.jpg` at beginning/middle/end.
Both original and final review materials approximate the engine alpha >0.25 test plus scalar blend.
This is offline Blender evidence, not proof of client shading, placement or animation playback.

**Pending:** serial client observation. No runtime installation or client verification performed.
