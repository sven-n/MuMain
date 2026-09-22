# Tomb01: Arched sarcophagus with offset lid

Owner: ASTRA reviewer/production worker; branch `codex/lorencia-monuments`.
Offline production complete; coordinator visual review/integration and client observation pending.

Actual World1 placements: 4; exact original transforms archived in `original/placements.json`.
Game path: `src/bin/Data/Object1/Tomb01.bmd`.
Triangles: 44 → 76.
Original engine bind bounds: `[[-117.53, -140.51, -0.08], [62.39, 121.84, 78.1]]`.
Final engine bind bounds: `[[-117.53, -140.51, -0.08], [62.39, 121.84, 78.1]]`.

Complete texture dependencies in preserved engine mesh order:

- `grave_02.OZJ` / `grave_02.jpg`: [512, 512], RGB; repainted.

The original footprint, origin, orientation and every source vertex are retained.
Selected panel face pairs: `[[2, 3], [4, 5], [6, 7], [8, 9]]`. Their interiors receive 0.4-unit recesses
inside unchanged original boundaries. Head/angel sculpture silhouette and topology remain intact.
Explicit UV repair faces: `[]`; only the Moai head top cap is
planarly mapped into the existing plain-stone atlas region to remove stretched legacy UV stripes.
Original/final collapsed UV triangles: 0 / 0. No new UV degeneracies.

Full bmdconv comparison output: **DIFFERENT**. Rig/action comparison: **EQUIVALENT**.
This comparator assesses geometry/rig; its EQUIVALENT result does not imply identical UVs or texture bytes.
Bone names/order: `['Box01', 'Cylinder02']`. Metadata: `['action 0 keys=1 lock=0']`.
Exact node-name/order/parent and action count/key count/lock checks pass.
Max original SMD-corner position-component drift: 0.002633000 units.
Max unchanged triangle UV component delta: 0.
Max retained normal direction delta: 0.069205547 degrees.
Every authored and final vertex matches bidirectionally with exact named bone; measured max evaluated
component delta 0.002639771 units, tolerance 0.005.
This additionally guards against converter position deduplication silently changing root ownership.

- `Tomb01.smd`: 1 keys, 2 samples; local translation component delta 0.000007000, rotation-matrix component delta 0.000000000.
- `Tomb01_a00.smd`: 1 keys, 2 samples; local translation component delta 0.000007000, rotation-matrix component delta 0.000000000.

The original slanted Cylinder02 root incurs 0.002633 units of SMD corner drift and 0.002639771
units in Blender reconstruction. Source anchors, named bone ownership and UVs remain intact. Printed
bind bounds match. The original local translation differs only 0.000007 units with unchanged rotation
matrices. This measured float reconstruction uses an explicit 0.005-unit tolerance; other models use 0.001.

Untouched original game files, decoded textures, bmdconv info/SMDs and official source import: `original/`.
Packed editable `source.blend` preserves exact excluded REF_ORIGINAL and optional REF_HIGH_POLY bake source.
Full comparison output, model/action validation, bone/anchor/UV/material audits: `validation/`.
Matching-camera actual BMD before/after, wireframe and reduced-scale views: `review/comparison.png`.
Six builtin imagegen master paintings, original references, prompts and editable OpenRaster files: `../paintings/`.
**Pending:** coordinator acceptance and serial client observation. No runtime install or client verification.
