# HouseEtc03 — Freestanding iron cage built from transparent panels

Owner `/root/reviewer`; branch `codex/lorencia-ironwork`.
**Status: validated offline; no client observation.**

World1 placements: 8. Exact position/rotation/scale/tile records: `original/placements.json`.
Game model: `src/bin/Data/Object1/HouseEtc03.bmd`.
Complete ordered texture dependencies: `steel_barred_a.tga`, `steel_barred_door.tga`.

Triangles: **16 → 16**. Full original/final BMD compare: **EQUIVALENT**.
Skeleton/actions-only BMD compare: **EQUIVALENT**. Names/order/parents, material order and one-key action metadata retained.
Original bone order: `['Box03']`.
Action metadata: `{'HouseEtc03_a00': {'index': 0, 'lock': False}}`.

Actual converted BMD bind bounds:

- Before: `[[-99.784698, -99.549202, 3.0969], [100.215302, 100.262611, 246.613907]]`
- After: `[[-99.784698, -99.549202, 3.0969], [100.215302, 100.262611, 246.613907]]`
- Largest component drift: `0.000000` game units.
- All 14 original position/bone anchors retained; maximum final distance `0.000000` units.
- Saved-source original connection vertices and bind extrema remain exactly equal.

Physical bar middles use narrow eight-sided profiles, transitioning back to every original endpoint over 2 game units. The 0.35-unit corner cuts stay inside the original section. Endpoints, spearheads, nonrectangular corner rail joints and alpha panels are protected. No placement, collision, orientation or opening adjustment compensates for the model. HouseEtc03 is a deliberate texture-only pass because all of its geometry is alpha panels.

Shared artwork: `steel_barred_b.jpg` 512×512 opaque RGB; `steel_barred_a.tga` and `steel_barred_door.tga` 256×512 RGBA where present. The exact original 64×128 alpha fields are enlarged 4× bilinearly. Continuous dark metal RGB underneath zero-alpha pixels prevents new black/white matte fringes. BMD material filenames and OZJ/OZT conventions are unchanged.

Evidence: `validation/compare.txt`, `commands.json`, `skeleton-compare.txt`, `preservation.json`, `summary.json`, `joins.json`, and `source-audit.json`. Each protected alpha material is separately compared EQUIVALENT, including all corners, original rigid bone and UV coordinates. All final triangles have nonzero area, one UV set and a single original bone binding; no collapsed UVs. Small newly created chamfer UV degeneracies are corrected locally and counted in `validation/source.json`.

Untouched BMDs, all texture dependencies, info, official imported source, SMD/actions and placement records remain in `original/`. Packed `source.blend` contains exact `REF_ORIGINAL`, export geometry and excluded editable `REF_HIGH_POLY` for remodeled support profiles. The cage needs no high-poly source. Official Blender import/export tools and `mu_texture.py` are used.

`review/comparison.png` contains matching-camera original/final actual BMD renders, wireframe and reduced-scale views; `review/reverse-offline.png` checks the opposite side. Parent `review/` contains two modular assemblies reconstructed from the recorded World1 transforms. Those reconstructions are offline XYZ transform studies, with no client, terrain, collision or lighting simulation.

Pending: serial client review at actual scale, per-vertex lighting, alpha testing/sorting and real 1920×1080 capture. No shared runtime folder or client session was used.
