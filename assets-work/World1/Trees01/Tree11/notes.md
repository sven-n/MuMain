# Tree11 — tiered conifer

Owner: ASTRA groundcover/tree worker, `codex/lorencia-trees`.
Status: exported and validated offline. Client verification pending.
World1 type 10, 165 placements; exact records in `placements.json`.
Game file: `src/bin/Data/Object1/Tree11.bmd`.
Complete texture dependencies: {'tree_06.tga': ['src/bin/Data/Object1/tree_06.OZT'], 'tree_03.jpg': ['src/bin/Data/Object1/tree_03.OZJ']}.
Original mesh/material order: ['tree_06.tga', 'tree_03.jpg'].

Geometry retained: 169 triangles, 2 meshes, 10 bones.
Surface artwork rebuilt within the original geometry, origins, rigid bindings, footprint,
UV semantics and sway. No subdivision or altered placements/collision was used.
Texture dimensions: {'tree_06.tga': [512, 512], 'tree_03.jpg': [128, 64]}.
Bind bounds before: [[-178.11, -170.89, -0.05], [195.18, 187.6, 560.52]].
Bind bounds after: [[-178.11, -170.89, -0.05], [195.18, 187.6, 560.52]].

Full converter comparison:

```
meshes 2 vs 2, bones 10 vs 10, actions 1 vs 1, triangles 169 vs 169
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0001  differing bone names: 0
EQUIVALENT
```

One original action, 31 keys, lock=0. Names, order, parents, dummy status, action order
and frame indices retained. Every bind/action local key is compared as translation and a
rotation matrix in `validation/local-motion.json`; maximums and tolerances are recorded.
The 0.001-unit local-position tolerance covers Float32 hierarchy round trips and is 50 times
stricter than converter default tolerance. Rotation-matrix tolerance is 0.00001. Numerical
roundtrip deviations are reported honestly; this is semantic equivalence, not binary identity.
Independent skeleton-only BMD compare is EQUIVALENT. Reference and action SMD validation pass.

`validation/uv-material.json` verifies every original corner's material, original rigid bone
and UV, including intentional out-of-range bark repeats. Original material order is asserted
from the exported BMD info. Blender checks finite coordinates, one UV set, nondegenerate
triangles and exactly one valid non-dummy bone per vertex. Original unused dummy bones remain.

Tree01/02 engine collision box and velocity=0.4/scale are unmodified. Original root and trunk
contacts remain. Tree11's shared tree_03.OZJ is frozen and byte-identical in game and deliverable.
Tree06 retains its pale bare snag identity. No filename, alpha convention or render flag changes.

Untouched originals, converter info/SMD and packed original source are in `original/`.
Packed `source.blend` retains original geometry in REF_ORIGINAL and cloned export geometry.
No high-poly mesh is needed for this unchanged low-cost tree/card structure. Painted masters,
editable PNG/JPG/TGA, final wrappers, alpha/fringe audit, tiling previews and imagegen prompts
are in parent `textures/` and `prompts.md`. Final containers pass mu_texture checks.

`review/comparison.jpg` contains matched original/reimported-export views, terrain scale and
240px readability. Animated models additionally include `review/action-comparison.jpg` at
frames0,15,30; all31 keys are audited numerically. Wireframe is retained. Review renders use
completed pilot TileGrass01 at100 units per repeat and a190-unit figure proxy at an invented
origin staging position. These are offline previews, not actual client/placement evidence.

Client alpha sorting, vertex lighting, actual foliage silhouette/contacts and motion under
engine timing remain pending. No shared runtime or client process was touched.
