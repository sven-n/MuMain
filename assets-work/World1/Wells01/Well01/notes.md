# Well01 — Roofed fieldstone well, two casks and four ceramic vessels

2026-09-22. ASTRA coordinator, Wells01. Offline validation complete; client pending.

0 World1 placements; exact transforms retained in placements.json. 405 → 915 triangles.
Bind bounds before: `[[-154.76, -125.85, -0.07], [121.57, 93.46, 270.82]]`. After: `[[-154.76, -125.85, -0.07], [121.57, 93.46, 270.82]]`.
Full geometry comparison **DIFFERENT**; rig/actions **EQUIVALENT**. Original local bind/action records are restored after official mesh export through supported bmdconv SMD/manifest packaging. Bone names, order, parents and all one-key actions retained; explicit sample checks in validation/local-motion.json.

Original anchor maximum final-reimport drift: 0.000072479 units. Every authored vertex also appears at its intended named bone and position in the final reimport; maximum distance component 0.000072479. Source footprints and all original corners are unchanged. Pottery subdivision adds curved edge points constrained to each original bone AABB. Original constant-color UV cap faces remain intact: collapsed UV count 24 before and 24 after; geometric degeneracies remain zero. No newly collapsed UVs or geometry. Every vertex rigidly uses one original bone; mesh/material order and all original texture filenames retained.

well/jar_01 paintings are 512×512 RGB, tub is 512×256 RGB. Shared horse_drawn_01 is frozen. Complete original dependencies are archived and described in dependencies.json. Source.blend is packed with REF_ORIGINAL. Review comparison.jpg shows matching original/final cameras, wireframe and reduced previews; reverse views remain separate. These are offline Blender studies, not client evidence.

See ../notes.md for group ownership, exact changed paths and reproduction.
