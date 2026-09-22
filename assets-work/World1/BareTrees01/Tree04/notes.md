# Tree04 — crooked dead tree

Owner: ASTRA groundcover/tree worker, branch `codex/lorencia-bare-trees`.
Status: offline validated; client verification pending. Production export installed into this worktree source Data only.
Original model provenance: ac0f6dd8 (legacy original); archive also checked against starting integration HEAD 2d827ed5225979087e9b68fbcf306879b48acd29.
World1 type 3; 15 placements recorded in placements.json.
Full texture dependencies: {'tree_03.jpg': ['src/bin/Data/Object1/tree_03.OZJ']}.
Material/mesh order: ['tree_03.jpg'].

Geometry retained deliberately: 273 triangles, 1 bones,
1 material meshes. Original dead-branch/root silhouettes remain recognizable.
The narrow branches, stump seating platform, contact edges, origin, orientation, footprint and
UV projection already suit their roles. No arbitrary subdivision or extra twig geometry was added.
The shared charcoal bark is rebuilt as a 512x256 diffuse painting. Frozen textures retain their
original dimensions: {'tree_03.jpg': [512, 256]}. There are no new rendering features or filenames.

Bind bounds before: [[-103.02, -278.2, -5.63], [298.21, 251.98, 494.72]].
Bind bounds after: [[-103.02, -278.2, -5.63], [298.21, 251.98, 494.72]].

Full BMD comparison (geometry retained, equivalent within Float32 export precision):

```
meshes 1 vs 1, bones 1 vs 1, actions 1 vs 1, triangles 273 vs 273
unmatched triangles: 0  max corner distance: 0.0000  max bone distance: 0.0000  differing bone names: 0
EQUIVALENT
```

Exact names, bone order, parents, mesh order, action order, key count (1), lock=0,
and rigid bindings are preserved. Skeleton-only compare is EQUIVALENT. Every bind/action local
pose is compared numerically in validation/local-motion.json. Rotations use Rz*Ry*Rx matrices
because equivalent Euler branches may differ by pi. Translation tolerance 0.001 engine units
and matrix tolerance 0.00001; actual maxima reported. All original animation keys are checked,
not just the selected visual frames. SMDs pass converter validation.

validation/uv-material.json checks every corner's original UV, bone and material. Finite UVs,
one UV set, nondegenerate faces and one full-weight real bone per vertex pass. Original bark
repeat UVs outside 0..1 are preserved. Original hierarchy and any unused bones are preserved.

Tree07 uses CreateOperate at src/source/Engine/Object/ZzzObject.cpp:4667; its top platform,
cut-face material, footprint, bounds and bone remain unchanged for seating. tree_04.OZJ is a
frozen dependency shared with BridgeStone01. Tree11's accepted BMD and tree_06.OZT foliage
are frozen; SHA-256 proof is in the batch frozen-files.json and protected-check.json.
Only the shared bark changes visually for that compatibility consumer.

Untouched original BMD, info, source.blend and converter extraction are archived. Packed
source.blend contains original geometry in REF_ORIGINAL and a separate export copy using
the painted bark. This is a texture-art rebuild; there is no high-poly remodeling claim.
Editable/raw PNG painting, final PNG/JPG/OZJ, prompt, tiling preview and wrapper checks are
in ../textures and ../prompts.md. Alpha is not edited in this batch; the accepted conifer
alpha container remains byte-identical. Bark remains opaque RGB.

review/comparison.jpg compares original against officially exported/reimported geometry
with matching cameras and lighting. Tree11 additionally has all 31-key proof and selected
frames 0, 15, 30 in action-comparison.jpg. Its matching renders use the unchanged accepted game BMD with final bark.
The separately validated roundtrip is compatibility evidence only, not an installed replacement. Wireframe and 240px readability previews are included. Terrain
views use completed TileGrass01 with 100-unit repeats and a 190-unit scale proxy at an
invented origin staging position. These are offline Blender previews, not client evidence.

Client lighting, actual world placement occlusion, stump sitting contact and conifer
sway/material appearance remain pending. No runtime or client process was touched.
