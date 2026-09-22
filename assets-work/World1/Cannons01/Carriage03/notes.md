# Carriage03 — cart carrying wooden tubs

Owner `/root/groundcover`, branch `codex/lorencia-cannons`. 0 unchanged World1 placements;
exact coordinates/rotations/scales in original/placements.json. Original BMD revision
0bb845121f92e451a5d61afff38a4abf948a637a. Status: completed offline; client verification pending.

Triangles 538 → 538. Full comparison: EQUIVALENT; actual BMD bytes unchanged. Skeleton/actions EQUIVALENT:
bone names, order, parents, action order/count, frames and lock flags unchanged. Every local key
is checked by position and Rz*Ry*Rx rotation matrix. Original action metadata ['action 0 keys=1 lock=0'].

Bind bounds before [[-160.09, -216.33, -1.17], [88.22, 220.78, 182.28]]; after [[-160.09, -216.33, -1.17], [88.22, 220.78, 182.28]].
Every original material/bone AABB remains within .005 units. All 1 action keys retain
posed bounds within 0.00000000 units. Original origin/orientation retained.
Authored source→actual exported vertex and intended bone audit max deviation
0.00000049 units. Retained timber,
contacts, platforms, steps and rope preserve all original UVs within .000002 and positions within .005.

Mesh/material order: tub.jpg, horse_drawn_01.jpg.
Full dependencies: tub.OZJ, horse_drawn_01.OZJ. Dimensions {'tub.jpg': [512, 256], 'horse_drawn_01.jpg': [1024, 1024]}.
Only horse_drawn_01.OZJ is repainted, an opaque RGB 1024² diffuse atlas serving twelve models.
Other dependency bytes are unchanged. Well01 and its well/jar/tub textures are pinned at
46daddf2; they are archived only in this batch and never installed to unowned game paths.

Production cannon changes: sixteen-sided tapered barrel, real recessed bore/lip and rounded
hemisphere shot, fitted to each original component AABB. Stand, base, axle/trunnion crossbar,
fuse and ammo boxes remain exact. Gallows changes: small fixed iron beam/platform joint plates;
all original timber, steps, suspension and noose geometry/rigid bindings retained. The noose
and connector span two rigid nodes; no moving corner was reassigned. Production split normals
receive subdegree per-bone tangent distinctions to prevent legacy converter deduplication.
Readonly compatibility models keep exact original BMD bytes; the official source export is
retained separately as validation/official-roundtrip.bmd and is not installed.

Raw normal-node proof checks actual BMD normal.Node against vertex.Node world rotation on
all original keys. Final production has no cross-node pairs; existing static compatibility pairs
have zero rotation difference. No converter/engine edits. Positive geometry area and winding pass.
Collapsed UV triangles: 0 before,
0 after; any are existing readonly constant-color caps,
retained in byte-identical BMDs. Production models have no collapsed UVs.

Packed source.blend includes immutable numeric REF_ORIGINAL, packed original/final artwork,
original rig/actions and excluded editable REF_HIGH_POLY. Source fingerprints, compare output,
info, SMD validation, pose matrices, actual vertex/bone mapping and raw-normal proof are in validation/.
Raw generated paintings and editable lossless PNG are under ../textures; exact prompts and the
gutter correction are recorded in ../generation-prompt.json and ../generation-refinement.json.

Matching-camera actual reimported BMD views, wireframe, reverse and reduced previews are in
review/. Hanging01 has frame0/12/24 views; Carriage01 frame0/10/20. The100-unit terrain repeat and
190-unit scale figure are staging proxies, not measured client density or a game character.
Selected exact World1 transforms are compared in ../review/placements; surroundings are omitted.
No client was operated. Client filtering, lighting, animation appearance, additive lamp preview
and surrounding-placement occlusion remain pending.
