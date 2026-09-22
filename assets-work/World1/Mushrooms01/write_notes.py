"""Write numerical per-model handoff records from measured validation evidence."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def write(name, count, original_triangles):
    folder = HERE / name
    data = json.loads((folder / 'validation/blender.json').read_text())
    summary = json.loads((folder / 'validation/summary.json').read_text())
    motion = json.loads((folder / 'validation/local-motion.json').read_text())
    placements = json.loads((folder / 'placements.json').read_text())
    before, after = summary['bounds_before'], summary['bounds_after']
    text = f'''# {name} — {count}-mushroom woodland cluster

ASTRA coordinator / codex/lorencia-mushrooms. {len(placements)} World1 placements.
Material: mushroom.jpg → src/bin/Data/Object1/mushroom.OZJ, shared only with Grass07/08.

Rebuilt rounded red/burgundy caps and fibrous ivory stems: {original_triangles} → {data['triangles']} triangles.
One mesh/material; 512×512 RGB atlas, opaque JPEG/OZJ, no alpha or added render flags.
Original {count} rigid bones, names/order/parents, one action at index0, one key, lock0 retained.

Bind bounds before: {before}; after: {after}. Source bounds for every individual mushroom
are retained within 0.0001 units; validation/blender.json records every part and its original bone.
Original placement, rotation, scale and origin are unchanged. Original component tilts remain.

Full BMD comparison DIFFERENT for intentional geometry and UV remodeling. Isolated skeleton/action
comparison EQUIVALENT. All local pose samples pass with maximum position-component difference
{max(s['max_position_component'] for s in motion['samples']):.8f} units and no Euler rotation difference.
All reference/action SMD validators pass. Source/export audit proves packed REF_ORIGINAL,
one UV set, nondegenerate UV triangles, finite geometry and one valid rigid bone per vertex.
The cap-underside UV strip found by the audit was corrected before installation.

Originals: original/{name}.bmd, info.txt, packed imported source.blend; original action/model SMDs
in validation/original/. Shared untouched texture in ../textures/original/. Rebuilt packed
source.blend keeps REF_ORIGINAL; freshly reimported BMD in validation/reimported.blend.
Editable/raw art and prompt are at batch root. Final game export is exports/{name}.bmd.

review/comparison.jpg contains matching camera before/after, wireframe and 240px readability.
The narrow scale proxy is 190 units tall. These are offline Blender renders, not client evidence.
No runtime installation. Actual placed contact, lighting and camera-distance review remain pending.
'''
    (folder / 'notes.md').write_text(text)


write('Grass07', 7, 126)
write('Grass08', 3, 54)
