# StoneStatue02: Moai-like stone head

Owner: ASTRA reviewer/production worker; branch `codex/lorencia-monuments`.
Offline production complete; coordinator visual review/integration and client observation pending.

Actual World1 placements: 7; exact original transforms archived in `original/placements.json`.
Game path: `src/bin/Data/Object1/StoneStatue02.bmd`.
Triangles: 107 → 107.
Original engine bind bounds: `[[-134.3, -139.45, 0.6], [134.95, 106.07, 485.88]]`.
Final engine bind bounds: `[[-134.3, -139.45, 0.6], [134.95, 106.07, 485.88]]`.

Complete texture dependencies in preserved engine mesh order:

- `stone_statue01.OZJ` / `stone_statue01.jpg`: [512, 512], RGB; repainted.

The original footprint, origin, orientation and every source vertex are retained.
Selected panel face pairs: `[]`. Their interiors receive 0.4-unit recesses
inside unchanged original boundaries. Head/angel sculpture silhouette and topology remain intact.
Explicit UV repair faces: `[32, 33, 69, 70, 93, 94]`; only the Moai head top cap is
planarly mapped into the existing plain-stone atlas region to remove stretched legacy UV stripes.
Original/final collapsed UV triangles: 0 / 0. No new UV degeneracies.

Full bmdconv comparison output: **EQUIVALENT**. Rig/action comparison: **EQUIVALENT**.
This comparator assesses geometry/rig; its EQUIVALENT result does not imply identical UVs or texture bytes.
Bone names/order: `['Cylinder02']`. Metadata: `['action 0 keys=1 lock=0']`.
Exact node-name/order/parent and action count/key count/lock checks pass.
Max original SMD-corner position-component drift: 0.000000000 units.
Max unchanged triangle UV component delta: 0.
Max retained normal direction delta: 0.010665105 degrees.
Every authored and final vertex matches bidirectionally with exact named bone; measured max evaluated
component delta 0.000000000 units, tolerance 0.001.
This additionally guards against converter position deduplication silently changing root ownership.

- `StoneStatue02.smd`: 1 keys, 1 samples; local translation component delta 0.000000000, rotation-matrix component delta 0.000000000.
- `StoneStatue02_a00.smd`: 1 keys, 1 samples; local translation component delta 0.000000000, rotation-matrix component delta 0.000000000.

Untouched original game files, decoded textures, bmdconv info/SMDs and official source import: `original/`.
Packed editable `source.blend` preserves exact excluded REF_ORIGINAL and optional REF_HIGH_POLY bake source.
Full comparison output, model/action validation, bone/anchor/UV/material audits: `validation/`.
Matching-camera actual BMD before/after, wireframe and reduced-scale views: `review/comparison.png`.
Six builtin imagegen master paintings, original references, prompts and editable OpenRaster files: `../paintings/`.
**Pending:** coordinator acceptance and serial client observation. No runtime install or client verification.
