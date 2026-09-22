# StoneWall04: Yellow and purple mounted-rider heraldic standard

Owner: ASTRA reviewer/production worker. Branch: `codex/lorencia-signs-banners`.
Status: offline production complete; coordinator visual review and integration pending.

World1 placements: 12; complete untouched transforms in `original/placements.json`.
Game path: `src/bin/Data/Object1/StoneWall04.bmd`.
Triangles: 36 → 36.
Before bind bounds: `[[-3.84, -21.56, -0.08], [98.57, 13.02, 179.99]]`.
After bind bounds: `[[-3.84, -21.56, -0.08], [98.57, 13.02, 179.99]]`.

Complete texture dependencies in preserved engine mesh order:

- `badge_01.OZJ` / `badge_01.jpg`: [64, 512], RGB; shared painted texture.
- `badge_03.OZT` / `badge_03.tga`: [512, 512], RGBA; shared painted texture.

Original origins, orientation, footprint and every original vertex are retained.
Signs have 0.25-unit panel recesses inside unchanged original wood boundaries; the two panel interiors
gain explicit edge profiles. Iron ornament, suspension rings, poles, cloth, stone and attachment triangles
retain position, bone and UV corners. No placement, collision or terrain edits.

Full BMD comparison: **EQUIVALENT**. Rig/actions: **EQUIVALENT**.
Bone names/order: `['bonee01', 'bonee02', 'bonee03', 'bonee04', 'bonee05', 'bonee06', 'Box24', 'Object04']`.
Action metadata: `['action 0 keys=25 lock=0']`.
Exact node-name/order/parent and action-count/key-count/lock checks pass.
Max exported original-corner component drift: 0.000130000 units.
Retained triangle count: 36; max retained UV error: 0.
Max retained normal direction change: 0.007987158 degrees.
Official SMD export normalizes/requantizes normals; no byte-equivalence of recomputed normals is claimed.

- `StoneWall04.smd`: 1 frames / 8 bone samples; max local position drift 0.000013000, max local rotation-matrix component drift 0.000000284.
- `StoneWall04_a00.smd`: 25 frames / 200 bone samples; max local position drift 0.000029000, max local rotation-matrix component drift 0.000000284.

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
