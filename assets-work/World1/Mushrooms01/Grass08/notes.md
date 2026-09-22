# Grass08 — 3-mushroom woodland cluster

ASTRA coordinator / codex/lorencia-mushrooms. 7 World1 placements.
Material: mushroom.jpg → src/bin/Data/Object1/mushroom.OZJ, shared only with Grass07/08.

Rebuilt rounded red/burgundy caps and fibrous ivory stems: 54 → 498 triangles.
One mesh/material; 512×512 RGB atlas, opaque JPEG/OZJ, no alpha or added render flags.
Original 3 rigid bones, names/order/parents, one action at index0, one key, lock0 retained.

Bind bounds before: [[-47.68, -8.23, -0.4], [26.67, 18.41, 22.88]]; after: [[-47.68, -8.23, -0.4], [26.67, 18.41, 22.88]]. Source bounds for every individual mushroom
are retained within 0.0001 units; validation/blender.json records every part and its original bone.
Original placement, rotation, scale and origin are unchanged. Original component tilts remain.

Full BMD comparison DIFFERENT for intentional geometry and UV remodeling. Isolated skeleton/action
comparison EQUIVALENT. All local pose samples pass with maximum position-component difference
0.00000200 units and no Euler rotation difference.
All reference/action SMD validators pass. Source/export audit proves packed REF_ORIGINAL,
one UV set, nondegenerate UV triangles, finite geometry and one valid rigid bone per vertex.
The cap-underside UV strip found by the audit was corrected before installation.

Originals: original/Grass08.bmd, info.txt, packed imported source.blend; original action/model SMDs
in validation/original/. Shared untouched texture in ../textures/original/. Rebuilt packed
source.blend keeps REF_ORIGINAL; freshly reimported BMD in validation/reimported.blend.
Editable/raw art and prompt are at batch root. Final game export is exports/Grass08.bmd.

review/comparison.jpg contains matching camera before/after, wireframe and 240px readability.
The narrow scale proxy is 190 units tall. These are offline Blender renders, not client evidence.
No runtime installation. Actual placed contact, lighting and camera-distance review remain pending.
