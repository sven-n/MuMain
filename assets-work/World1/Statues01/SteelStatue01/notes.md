# SteelStatue01: Capped carved memorial with ochre plaque

Owner: ASTRA reviewer/production worker; branch `codex/lorencia-monuments`.
Offline production complete; coordinator visual review/integration and client observation pending.

Actual World1 placements: 4; exact original transforms archived in `original/placements.json`.
Game path: `src/bin/Data/Object1/SteelStatue01.bmd`.
Triangles: 66 → 98.
Original engine bind bounds: `[[-65.92, -65.01, -0.04], [65.87, 66.77, 321.58]]`.
Final engine bind bounds: `[[-65.92, -65.01, -0.04], [65.87, 66.77, 321.58]]`.

Complete texture dependencies in preserved engine mesh order:

- `tombstone_big.OZJ` / `tombstone_big.jpg`: [512, 512], RGB; repainted.

The original footprint, origin, orientation and every source vertex are retained.
Selected panel face pairs: `[[10, 11], [12, 13], [14, 15], [16, 17]]`. Their interiors receive 0.4-unit recesses
inside unchanged original boundaries. Head/angel sculpture silhouette and topology remain intact.
Explicit UV repair faces: `[]`; only the Moai head top cap is
planarly mapped into the existing plain-stone atlas region to remove stretched legacy UV stripes.
Original/final collapsed UV triangles: 0 / 0. No new UV degeneracies.

Full bmdconv comparison output: **DIFFERENT**. Rig/action comparison: **EQUIVALENT**.
This comparator assesses geometry/rig; its EQUIVALENT result does not imply identical UVs or texture bytes.
Bone names/order: `['Box02']`. Metadata: `['action 0 keys=1 lock=0']`.
Exact node-name/order/parent and action count/key count/lock checks pass.
Max original SMD-corner position-component drift: 0.000000000 units.
Max unchanged triangle UV component delta: 0.
Max retained normal direction delta: 0.002034515 degrees.
Every authored and final vertex matches bidirectionally with exact named bone; measured max evaluated
component delta 0.000000000 units, tolerance 0.001.
This additionally guards against converter position deduplication silently changing root ownership.

- `SteelStatue01.smd`: 1 keys, 1 samples; local translation component delta 0.000000000, rotation-matrix component delta 0.000000000.
- `SteelStatue01_a00.smd`: 1 keys, 1 samples; local translation component delta 0.000000000, rotation-matrix component delta 0.000000000.

Untouched original game files, decoded textures, bmdconv info/SMDs and official source import: `original/`.
Packed editable `source.blend` preserves exact excluded REF_ORIGINAL and optional REF_HIGH_POLY bake source.
Full comparison output, model/action validation, bone/anchor/UV/material audits: `validation/`.
Matching-camera actual BMD before/after, wireframe and reduced-scale views: `review/comparison.png`.
Six builtin imagegen master paintings, original references, prompts and editable OpenRaster files: `../paintings/`.
**Pending:** coordinator acceptance and serial client observation. No runtime install or client verification.
