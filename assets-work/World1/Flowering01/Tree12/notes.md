# Tree12 — pink-lilac flowering tree

Owner: ASTRA groundcover/tree worker, `codex/lorencia-flowering`.
Status: exported and validated offline. Client verification pending.
World1 type 11, 1 placements; exact records in `placements.json`.
Game file: `src/bin/Data/Object1/Tree12.bmd`.
Complete texture dependencies: {'tree_01.jpg': ['src/bin/Data/Object1/tree_01.OZJ'], 'tree_04.tga': ['src/bin/Data/Object1/tree_04.OZT']}.
Original mesh/material order: ['tree_01.jpg', 'tree_04.tga'].

Geometry retained: 393 triangles, 2 meshes, 9 bones.
Surface artwork rebuilt within the original geometry, origins, rigid bindings, footprint,
UV semantics and sway. No subdivision or altered placements/collision was used.
Texture dimensions: {'tree_01.jpg': [512, 256], 'tree_04.tga': [512, 512]}.
Bind bounds before: [[-187.71, -196.27, 0.19], [236.19, 229.76, 490.08]].
Bind bounds after: [[-187.71, -196.27, 0.19], [236.19, 229.76, 490.08]].

Full converter comparison:

```
meshes 2 vs 2, bones 9 vs 9, actions 1 vs 1, triangles 393 vs 393
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
triangles and exactly one valid non-dummy bone per vertex. No additional or dummy bones are introduced.

Original hollow-log openings and crown silhouettes remain. No special mesh-index control
was found for these model types. Tree12 retains bark then flower order; Tree13 retains ochre
alpha then bark order. Bark ownership includes all three consumers. No texture is renamed.
Original local keys/bones retain their actual names; there are no dummy-prefixed bones in this group.

Untouched originals, converter info/SMD and packed original source are in `original/`.
Packed `source.blend` retains original geometry in REF_ORIGINAL and cloned export geometry.
No high-poly mesh is needed for this unchanged low-cost tree/card structure. Painted masters,
editable PNG/JPG/TGA, final wrappers, alpha/fringe audit, tiling previews and imagegen prompts
are in parent `textures/` and `prompts.md`. Final containers pass mu_texture checks.

`review/comparison.jpg` contains matched original/reimported-export views, terrain scale and
240px readability. Animated models additionally include `review/action-comparison.jpg` at
frames 0,15,30; all 31 keys are audited numerically. Wireframe is retained. Review renders use
completed pilot TileGrass01 at 100 units per repeat and a 190-unit figure proxy at an invented
origin staging position. These are offline previews, not actual client/placement evidence.

Client alpha sorting, vertex lighting, actual foliage silhouette/contacts and motion under
engine timing remain pending. No shared runtime or client process was touched.
