# Sign01: Hanging notice board, animated wood bracket and iron ornament

Owner: ASTRA reviewer/production worker. Branch: `codex/lorencia-signs-banners`.
Status: offline production complete; coordinator visual review and integration pending.

World1 placements: 3; complete untouched transforms in `original/placements.json`.
Game path: `src/bin/Data/Object1/Sign01.bmd`.
Triangles: 30 → 46.
Before bind bounds: `[[-118.88, -4.64, 0.05], [0.92, 7.93, 81.77]]`.
After bind bounds: `[[-118.88, -4.64, 0.05], [0.92, 7.93, 81.77]]`.

Complete texture dependencies in preserved engine mesh order:

- `notice.OZJ` / `notice.jpg`: [512, 512], RGB; shared painted texture.
- `signboard.OZT` / `signboard.tga`: [512, 256], RGBA; shared painted texture.
- `doorknob.OZT` / `doorknob.tga`: [128, 128], RGBA; shared painted texture.

Original origins, orientation, footprint and every original vertex are retained.
Signs have 0.25-unit panel recesses inside unchanged original wood boundaries; the two panel interiors
gain explicit edge profiles. Iron ornament, suspension rings, poles, cloth, stone and attachment triangles
retain position, bone and UV corners. No placement, collision or terrain edits.

Full BMD comparison: **DIFFERENT**. Rig/actions: **EQUIVALENT**.
Bone names/order: `['Bone01', 'Bone02', 'Bone03', 'Cylinder01']`.
Action metadata: `['action 0 keys=21 lock=0']`.
Exact node-name/order/parent and action-count/key-count/lock checks pass.
Max exported original-corner component drift: 0.000000000 units.
Retained triangle count: 26; max retained UV error: 0.
Max retained normal direction change: 0.012105757 degrees.
Official SMD export normalizes/requantizes normals; no byte-equivalence of recomputed normals is claimed.

- `Sign01.smd`: 1 frames / 4 bone samples; max local position drift 0.000000000, max local rotation-matrix component drift 0.000000000.
- `Sign01_a00.smd`: 21 frames / 84 bone samples; max local position drift 0.000008000, max local rotation-matrix component drift 0.000000000.

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
