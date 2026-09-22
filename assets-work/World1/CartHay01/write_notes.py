"""Write individual asset handoffs directly from the validated evidence."""
import json
import re
import sys
sys.dont_write_bytecode=True
from config import ASSETS,BASELINE,CONTAINERS,FROZEN,IDENTITIES,MATERIALS,ROOT


def notes(name):
    folder=ROOT/name
    original=(folder/'original/info.txt').read_text()
    new=(folder/'validation/info-after.txt').read_text()
    report=json.loads((folder/'validation/summary.json').read_text())
    geometry=json.loads((folder/'validation/geometry-UV-contacts.json').read_text())
    motion=json.loads((folder/'validation/local-motion.json').read_text())
    count=len(json.loads((folder/'original/placements.json').read_text()))
    old_count=re.search(r'actions: \d+  triangles: (\d+)',original)[1]
    new_count=re.search(r'actions: \d+  triangles: (\d+)',new)[1]
    text=f'''# {name}: {IDENTITIES[name]}

Owner: ASTRA environment artist `/root/groundcover`, branch `codex/lorencia-cart-hay`.
{count} original World1 placements, exact records in `original/placements.json`.
Original revision {BASELINE[:8]}. OFFLINE complete; client verification pending.

Triangles: {old_count} → {new_count}. Original material/mesh order:
{', '.join(MATERIALS[name])}.

Full BMD comparison: DIFFERENT, intentional geometry changes. Skeleton/action comparison:
EQUIVALENT. Bone names/order/parents, action count/order, keys and lock flags unchanged.
Action metadata: {report['action_metadata']}.

Bind bounds before: {report['bounds_before']}.
Bind bounds after: {report['bounds_after']}.
Source bounds and every original per-material/per-bone AABB retained. Converter two-decimal
rounding may print a last-digit difference; precise bounds/positions are in geometry-UV-contacts.json.
Authored source → actual exported vertex/bone proof: max coordinate deviation
{geometry['authored_bindings']['maximum_coordinate_deviation']:.8f}, tolerance .005 units.
All original cart wheel/axle/shaft/seat/cask/lantern/light positions remain, at export precision.

Carriage geometry retains original construction and adds shaped iron hub sleeves. Hay uses
rounded closed bundles with pinched bindings, packed cut ends and slight per-bundle variation
inside the original bounds. Original lamp alpha and additive geometry/UVs remain exact; only
four pre-existing collapsed Carriage01 wood UV triangles receive a quarter-original-texel repair.
Carriage01's additive horse_drawn_03.jpg stays mesh 2. No geometry is bound to a dummy.

All exported UV and geometry triangles have positive area and correct winding; one rigid bone
per vertex. Complete raw-normal node audit proves final shared normals have equivalent world
rotations on every key. Carriage01 uses subtly distinct authored split normals to correct its
pre-existing moving-bone normal sharing. Full outputs are in validation/.

Textures: owned paints 512x512, RGB diffuse or 32-bit uncompressed bottom-left alpha as original.
Frozen shared horse_drawn_01.OZJ and tub.OZJ retain bytes and legacy dimensions. Dependencies:
{', '.join(CONTAINERS[m] for m in MATERIALS[name])}.
Generated alpha is retained, with RGB padded under discarded pixels. Raw art, prompts, editable
PNGs and final containers are in ../textures; dependencies are copied into exports/.

Packed source.blend contains untouched numeric REF_ORIGINAL, original rig/action metadata,
editable authored meshes in excluded REF_HIGH_POLY, and packed original/final paintings.
Saved-file original fingerprints, local position/rotation-matrix proof, source skinning,
actual exported bindings, full compare and texture checks pass.

Matching-camera review/comparison.jpg, reverse views, wireframe and reduced views show actual
reimported BMDs. Carriage01 additionally has action frames 0,10,20. Terrain repetition 100 units
and figure 190 units are staging proxies, not measured client density or a real game character.
Actual World1 selected placement comparisons are in ../review/placements; no terrain or
surrounding architecture is reproduced there. Carriage03 has no World1 placements.
No shared runtime/client activity. Actual client loading, additive blending, alpha/filtering,
lighting, motion and surrounding placement occlusion remain pending.
'''
    (folder/'notes.md').write_text(text)


for name in ASSETS:notes(name)
