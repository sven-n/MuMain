# FireLight01 — Standing iron basket brazier

2026-09-22. ASTRA coordinator / FireProps01. Offline validation complete; client review pending.

26 actual World1 placements retained in placements.json. 48 → 80 triangles.
Bind bounds before: `[[-28.57, -28.47, -0.41], [28.61, 28.71, 175.23]]`. After: `[[-28.57, -28.47, -0.41], [28.61, 28.71, 175.23]]`.
Full BMD comparison: **DIFFERENT**. Skeleton/actions: **EQUIVALENT**. Explicit local pose matrices, bone names/order/parents and action metadata retained in validation.
All original Blender-space anchors survive within 0.000000000 game units. Material order, rigid weights and effect UVs are preserved.

Complete material/container dependencies:

- `light.tga`: `src/bin/Data/Object1/light.OZT`
- `light2.jpg`: `src/bin/Data/Object1/light2.OZJ`
- `light3.jpg`: `src/bin/Data/Object1/light3.OZJ`

New opaque paintings are 512×512; new light/fire_light_01 RGBA masks are 256×256. light3, fire_02 and shared tile_02 remain byte-identical at original resolution. Parent textures/original retains all untouched containers and decoded files.

Packed source.blend contains REF_ORIGINAL and the unchanged rig. Original source/info and validation SMD/action files are retained. Exports use the official importer/exporter and mu_texture wrapping/checks. review/comparison.jpg includes matching cameras, actual reimport, wireframe and reduced previews; the 190-unit bar is an offline scale reference.

Bonfire previews approximate documented BlendMesh1 additive shading on both versions; procedural particles and runtime lighting are absent. No image is client evidence. No runtime installation occurred.

See ../notes.md for reproduction and pending checks.
