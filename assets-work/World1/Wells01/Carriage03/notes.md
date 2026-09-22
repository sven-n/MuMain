# Carriage03 — Accepted cask cart; unchanged BMD compatibility

2026-09-22. ASTRA coordinator, Wells01. Offline validation complete; client pending.

0 World1 placements; exact transforms retained in placements.json. 538 → 538 triangles.
Bind bounds before: `[[-160.09, -216.33, -1.17], [88.22, 220.78, 182.28]]`. After: `[[-160.09, -216.33, -1.17], [88.22, 220.78, 182.28]]`.
Full geometry comparison **EQUIVALENT**; rig/actions **EQUIVALENT**. Original local bind/action records are restored after official mesh export through supported bmdconv SMD/manifest packaging. Bone names, order, parents and all one-key actions retained; explicit sample checks in validation/local-motion.json.

Original anchor maximum final-reimport drift: 0.000000000 units. Every authored vertex also appears at its intended named bone and position in the final reimport; maximum distance component 0.000000000. Source footprints and all original corners are unchanged. Pottery subdivision adds curved edge points constrained to each original bone AABB. Original constant-color UV cap faces remain intact: collapsed UV count 0 before and 0 after; geometric degeneracies remain zero. No newly collapsed UVs or geometry. Every vertex rigidly uses one original bone; mesh/material order and all original texture filenames retained.

well/jar_01 paintings are 512×512 RGB, tub is 512×256 RGB. Shared horse_drawn_01 is frozen. Complete original dependencies are archived and described in dependencies.json. Source.blend is packed with REF_ORIGINAL. Review comparison.jpg shows matching original/final cameras, wireframe and reduced previews; reverse views remain separate. These are offline Blender studies, not client evidence.

This BMD is byte-identical to accepted CartHay01 commit93ca193e. It is archived and rendered to prove shared tub painting compatibility; no Carriage03 game file is installed by this batch.

See ../notes.md for group ownership, exact changed paths and reproduction.
