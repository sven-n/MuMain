# Grass06 — narrow low broad-leaf cluster

Status: exported and validated offline. Client verification pending; no client was operated.
Owner: ASTRA groundcover worker, branch `codex/lorencia-groundcover`.

World1 model type 25; 82 actual placements.
The exact placement records, tile coordinates, scales and rotations are in `placements.json`.
Game BMD: `src/bin/Data/Object1/Grass06.bmd`.
Complete texture dependency: `src/bin/Data/Object1/tree_09.OZT` (material `tree_09.tga`).

The low-cost original vegetation cards, footprint, pivot and orientation are retained. This
is a texture art rebuild, with no additional triangles. Original reference meshes remain in
`REF_ORIGINAL` inside packed `source.blend`; untouched BMD, converter info, SMD and packed
original source remain in `original/`. A high-poly mesh would add no value for these cutouts.

Triangles: 112 before and after. One mesh. Texture: 256×512.
Bind bounds before: [[-161.84, -51.36, 0.11], [102.59, 58.3, 60.74]].
Bind bounds after: [[-161.84, -51.36, 0.11], [102.59, 58.3, 60.74]].

Full BMD comparison: EQUIVALENT; all original triangles match. Full output:

```
meshes 1 vs 1, bones 4 vs 4, actions 1 vs 1, triangles 112 vs 112
unmatched triangles: 0  max corner distance: 0.0001  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

Bone names, original indices, parents, action order, one-frame action and lock=0 are retained.
`validation/local-motion.json` compares every local position and rotation matrix. Grass05/06
can encode the same rotation using a different Euler branch after Blender; equivalence is
proved as rotation matrices, not by assuming identical Euler components. The skeleton-only
BMD comparison is independently EQUIVALENT. `validation/uv-material.json` checks every
original triangle corner's rigid binding, UV and material against the exported BMD.

All exported SMD references/actions pass bmdconv validation. Final 32-bit uncompressed
bottom-left TGA/OZT passes mu_texture check. Alpha artwork is retained, including irregular
cutouts; RGB beneath fully transparent texels is padded from adjacent visible leaf colors.
Raw paintings, rejected density study, final editable PNG/TGA and prompts live in the parent
batch `textures/` and `prompts.md`. The two paired models share only their assigned texture.

Review images are in `review/comparison.jpg`, with individual before/after, wireframe and
240px readability images. After images come from a fresh re-import of the exported BMD.
Terrain previews use the completed pilot TileGrass01 at 100 units per repeat and a 190-unit
figure proxy. These are matched offline previews at the origin, not actual client captures.

Engine inspection: MapManager.cpp loads Grass01 through the Grass model range;
_enum.h assigns MODEL_GRASS01=20. Lorencia switches in ZzzObject.cpp contain no special
mesh-index handling for types 20/21/24/25. Single material and mesh order remain unchanged.

Reproduction: run the parent batch `package_textures.py`, Blender `build_sources.py`,
`export_validate.py`, Blender `render_review.py`, then `finalize.py`, with commands executed
from the assigned worktree. No runtime installation is performed by this batch.
