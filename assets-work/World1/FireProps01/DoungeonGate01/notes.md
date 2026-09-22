# DoungeonGate01 — Monumental stone cobra entrance with two flame plinths

2026-09-22. ASTRA coordinator / FireProps01. Offline validation complete; client review pending.

1 actual World1 placements retained in placements.json. 333 → 333 triangles.
Bind bounds before: `[[-199.6, -233.49, -7.97], [200.5, 227.89, 315.79]]`. After: `[[-199.6, -233.49, -7.97], [200.5, 227.89, 315.79]]`.
Full BMD comparison: **EQUIVALENT**. Skeleton/actions: **EQUIVALENT**. Explicit local pose matrices, bone names/order/parents and action metadata retained in validation.
All original Blender-space anchors survive within 0.001060992 game units. Material order, rigid weights and effect UVs are preserved.

Complete material/container dependencies:

- `copra_gate.jpg`: `src/bin/Data/Object1/copra_gate.OZJ`

New opaque paintings are 512×512; new light/fire_light_01 RGBA masks are 256×256. light3, fire_02 and shared tile_02 remain byte-identical at original resolution. Parent textures/original retains all untouched containers and decoded files.

Packed source.blend contains REF_ORIGINAL and the unchanged rig. Original source/info and validation SMD/action files are retained. Exports use the official importer/exporter and mu_texture wrapping/checks. review/comparison.jpg includes matching cameras, actual reimport, wireframe and reduced previews; the 190-unit bar is an offline scale reference.

Bonfire previews approximate documented BlendMesh1 additive shading on both versions; procedural particles and runtime lighting are absent. No image is client evidence. No runtime installation occurred.

See ../notes.md for reproduction and pending checks.
