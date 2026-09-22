# Waterspout01: Accepted dragon fountain; shared basin material compatibility only

Owner: ASTRA reviewer/production worker; branch `codex/lorencia-monuments`.
Offline production complete; coordinator visual review/integration and client observation pending.

Actual World1 placements: 1; exact original transforms archived in `original/placements.json`.
Game path: `src/bin/Data/Object1/Waterspout01.bmd`.
Triangles: 639 → 639.
Original engine bind bounds: `[[-199.06, -199.74, -0.14], [199.23, 198.55, 311.2]]`.
Final engine bind bounds: `[[-199.06, -199.74, -0.14], [199.23, 198.55, 311.2]]`.

Complete texture dependencies in preserved engine mesh order:

- `stone_statue02.OZJ` / `stone_statue02.jpg`: [256, 512], RGB; repainted.
- `reagon_waterspout.OZJ` / `reagon_waterspout.jpg`: [128, 128], RGB; frozen byte-identical.
- `ston01.OZJ` / `ston01.jpg`: [512, 512], RGB; frozen byte-identical.
- `ston02.OZJ` / `ston02.jpg`: [128, 128], RGB; frozen byte-identical.

The original footprint, origin, orientation and every source vertex are retained.
Selected panel face pairs: `[]`. Their interiors receive 0.4-unit recesses
inside unchanged original boundaries. Head/angel sculpture silhouette and topology remain intact.
Explicit UV repair faces: `[]`; only the Moai head top cap is
planarly mapped into the existing plain-stone atlas region to remove stretched legacy UV stripes.
Original/final collapsed UV triangles: 0 / 0. No new UV degeneracies.

Full bmdconv comparison output: **EQUIVALENT**. Rig/action comparison: **EQUIVALENT**.
This comparator assesses geometry/rig; its EQUIVALENT result does not imply identical UVs or texture bytes.
Bone names/order: `['OilTank01', 'Box04', 'Box01', 'Box02', 'Box03', 'QuadPatch01', 'Box05', 'Box06', 'Box07', 'Box08', 'QuadPatch02']`. Metadata: `['action 0 keys=21 lock=0']`.
Exact node-name/order/parent and action count/key count/lock checks pass.
Max original SMD-corner position-component drift: 0.000000000 units.
Max unchanged triangle UV component delta: 0.
Max retained normal direction delta: 0.000000000 degrees.
Every authored and final vertex matches bidirectionally with exact named bone; measured max evaluated
component delta 0.000000000 units, tolerance 0.001.
This additionally guards against converter position deduplication silently changing root ownership.

- `Waterspout01.smd`: 1 keys, 11 samples; local translation component delta 0.000000000, rotation-matrix component delta 0.000000000.
- `Waterspout01_a00.smd`: 21 keys, 231 samples; local translation component delta 0.000000000, rotation-matrix component delta 0.000000000.

Installed BMD is retained byte-identically. The official Blender export is archived separately as
`validation/official-roundtrip.bmd` and is not installed. Only stone_statue02 basin diffuse changes.
All 639 triangles, four mesh slots, 11 bones and 21 action keys remain exact. Mesh 3 water, V-scroll
behavior and particle anchors at bones 1 and 4 are protected. Dragon, stone base and water texture
containers are frozen, including accepted Rocks01 ston01. Offline renders do not emulate water UV
scrolling, runtime particles, additive water blending or actual client lighting.

Untouched original game files, decoded textures, bmdconv info/SMDs and official source import: `original/`.
Packed editable `source.blend` preserves exact excluded REF_ORIGINAL and optional REF_HIGH_POLY bake source.
Full comparison output, model/action validation, bone/anchor/UV/material audits: `validation/`.
Matching-camera actual BMD before/after, wireframe and reduced-scale views: `review/comparison.png`.
Six builtin imagegen master paintings, original references, prompts and editable OpenRaster files: `../paintings/`.
**Pending:** coordinator acceptance and serial client observation. No runtime install or client verification.
