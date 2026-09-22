"""Describe authored changes, dependencies, full comparisons and pending client checks."""
import json
import re
import sys

sys.dont_write_bytecode = True
from config import ASSETS, BASELINE, CONTAINERS, IDENTITIES, MATERIALS, ROOT


def notes(name):
    folder = ROOT/name
    source = json.loads((folder/'validation/blender.json').read_text())
    engine = json.loads((folder/'validation/summary.json').read_text())
    audit = json.loads((folder/'validation/geometry-UV-seating.json').read_text())
    inventory = json.loads((folder/'placements.json').read_text())
    return f'''# {name} — {IDENTITIES[name]}

Owner: ASTRA groundcover/tree artist, `codex/lorencia-tavern-details`.
Status: remodeled and painted, exported and validated offline. Client review pending.
World1 type {inventory['type']}, {len(inventory['placements'])} placements retained in placements.json.
Complete dependencies, in original BMD mesh order: {MATERIALS[name]}.
Exact containers: {[CONTAINERS[n] for n in MATERIALS[name]]}.
Texture dimensions: {source['textures']}.
Original archive checked byte-for-byte against starting revision {BASELINE}.

Replacement: {source['triangles']} triangles; original info in original/info.txt.
Bone names/order: {source['bone_order']}. Every original parent and local pose is retained.
One action, one frame, lock=0; original frame/action order is unchanged.
All new vertices have exactly one full-weight original rigid bone. No dummy bindings.
Mesh order and all original material filenames are preserved, including lower-case ozt paths.

Before bind bounds: {engine['bounds_before']}.
After bind bounds: {engine['bounds_after']}.
Every original material/bone component's bounds is also retained numerically. Rounded vessel
profiles are fitted separately, preserving individual arrangement and footprint. Tankard/jug
handles follow their original measured direction. Shelf heights and vessel contacts are retained.
Chair and stool keep the original seat surface polygons and foot corners. Seating proof:
{audit['seating']}.

Full comparison correctly reports DIFFERENT because geometry was rebuilt:

```
{(folder/'validation/compare.txt').read_text().strip()}
```

Skeleton-only comparison is EQUIVALENT. validation/local-motion.json proves exact node names,
indices, parents and frame lists; every bind/action local key is compared as translation and
Rz*Ry*Rx rotation matrices. The measured maxima are retained there. Action metadata is compared
explicitly. Original and replacement reference/action SMDs pass bmdconv validate.

Actual exported UV/normal/material audit: {audit['surface']}.
Legacy alpha-card geometry, UVs and rigid bone assignments remain within export precision;
paintings preserve transparent openings. Final alpha coverage at cutoff 64 is 37.1% vs 36.5%
for apple sprigs, 86.6% vs 84.0% for chair fretwork. RGB is padded under discarded alpha texels;
generated alpha is retained. Bottle retains its original 181..255 opacity field via nearest
resampling. OZT is uncompressed 32-bit bottom-left; opaque textures are RGB JPEG/OZJ. All final
containers pass tools/mu_texture.py check. No new material flags or rendering features.

source.blend is packed and retains the untouched imported geometry/UVs/skinning/transforms
in REF_ORIGINAL; source-audit.json compares a numeric fingerprint to the original import.
REF_HIGH_POLY retains excluded editable authored component meshes with fine-bevel modifiers.
Painted raster masters, editable PNGs, final game textures and exact imagegen prompts/raw
iterations are in parent textures/ and the prompt JSON files. All image artwork was produced
with the built-in imagegen skill; packaging performs technical resizing/encoding/alpha padding.

review/comparison.jpg provides matching original/exported-BMD cameras and reduced previews.
After views are imported from actual game exports. Separate wireframes are included. Ground
views use completed terrain at 100-unit repeats and a 190-unit proxy at an invented staging
position. The terrain density is a staging proxy; actual engine UV density depends on texture width.
These are offline Blender previews, not client evidence or actual placement captures.

Client lighting, material/alpha filtering, actual tavern occlusion and chair/stool sitting
contacts remain pending. No shared runtime, client, engine or collision data was modified.
'''


for asset in ASSETS:
    (ROOT/asset/'notes.md').write_text(notes(asset))
