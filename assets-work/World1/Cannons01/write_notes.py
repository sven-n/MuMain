"""Build per-asset and consolidated handoff from the passing export evidence."""
import json
import re
import sys
sys.dont_write_bytecode=True
from config import ASSETS,BASELINE,CONTAINERS,FROZEN,IDENTITIES,KEYS,MATERIALS,PRODUCTION_ASSETS,ROOT,WELLS_REVISION
from PIL import Image
provenance=json.loads((ROOT/'original-provenance.json').read_text())
rows=[]
for name in ASSETS:
    folder=ROOT/name
    before=(folder/'original/info.txt').read_text();after=(folder/'validation/info-after.txt').read_text()
    triangles=[int(re.search(r'actions: \d+  triangles: (\d+)',text)[1]) for text in (before,after)]
    report=json.loads((folder/'validation/summary.json').read_text())
    geometry=json.loads((folder/'validation/geometry-UV-contacts.json').read_text())
    poses=json.loads((folder/'validation/all-key-posed-bounds.json').read_text())
    production=name in PRODUCTION_ASSETS
    full='DIFFERENT (intentional remodel)' if production else 'EQUIVALENT; actual BMD bytes unchanged'
    count=len(json.loads((folder/'original/placements.json').read_text()))
    rows.append(f'| {name} | {count} | {triangles[0]} → {triangles[1]} | {KEYS[name]} | {full} |')
    dependencies=[CONTAINERS[m] for m in MATERIALS[name]]
    dimensions={m:list(Image.open(ROOT/'textures/final'/m).size) for m in MATERIALS[name]}
    notes=f'''# {name} — {IDENTITIES[name]}

Owner `/root/groundcover`, branch `codex/lorencia-cannons`. {count} unchanged World1 placements;
exact coordinates/rotations/scales in original/placements.json. Original BMD revision
{provenance[name+'.bmd']['source_revision']}. Status: completed offline; client verification pending.

Triangles {triangles[0]} → {triangles[1]}. Full comparison: {full}. Skeleton/actions EQUIVALENT:
bone names, order, parents, action order/count, frames and lock flags unchanged. Every local key
is checked by position and Rz*Ry*Rx rotation matrix. Original action metadata {report['action_metadata']}.

Bind bounds before {report['bounds_before']}; after {report['bounds_after']}.
Every original material/bone AABB remains within .005 units. All {poses['keys']} action keys retain
posed bounds within {poses['max_bounds_delta']:.8f} units. Original origin/orientation retained.
Authored source→actual exported vertex and intended bone audit max deviation
{geometry['authored_bindings']['maximum_coordinate_deviation']:.8f} units. Retained timber,
contacts, platforms, steps and rope preserve all original UVs within .000002 and positions within .005.

Mesh/material order: {', '.join(MATERIALS[name])}.
Full dependencies: {', '.join(dependencies)}. Dimensions {dimensions}.
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
Collapsed UV triangles: {geometry['original_surface']['collapsed_uv_triangles']} before,
{geometry['surface']['collapsed_uv_triangles']} after; any are existing readonly constant-color caps,
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
'''
    (folder/'notes.md').write_text(notes)
text='''# Cannons01: swivel guns, gallows and twelve-consumer wood atlas

Owner ASTRA `/root/groundcover`, branch `codex/lorencia-cannons`; source worktree
`MuMain-lorencia-cannons`. Four new production assets, seven placements. Eight accepted models
reviewed readonly for the shared atlas, nineteen additional placements (Well01/Carriage03 zero).
All work is offline; no runtime or client activity, no client verification claim.

The three cannon variants retain recognizable timber stands and zero/one/two ammunition boxes.
Tapered round barrels now have recessed bores and round shot. Gallows stairs/platform/post and
animated noose remain; subtle iron joints support the medieval timber palette. One fixed-layout
1024² atlas repaints aged oak, dark iron and leather for all12 consumers. Imagegen revision2
replaces pale gutters with dark walnut after actual-model review exposed light blocks on cart
rims and platform edges; original UVs remain unchanged on all retained geometry.

| Model | Placements | Triangles | Action keys | Full BMD compare |
| --- | --- | --- | --- | --- |
'''+ '\n'.join(rows)+'''

Only game files changed: Object1/Cannon01.bmd, Cannon02.bmd, Cannon03.bmd, Hanging01.bmd,
horse_drawn_01.OZJ. All other World1/Object1 files are protected by hash manifest.
Baseline0bb84512; Well01 and well.OZJ/jar_01.OZJ/tub.OZJ archives are from46daddf2.
Readonly geometry stays byte-identical; official roundtrips remain evidence only. Shared texture
consumers and original placement/dependency records are retained in dependency-record.json.
No map placement, collision, terrain, lighting, alpha strip or engine file changed.

Validation includes full bmdconv compare plus skeleton/actions EQUIVALENT, all local keys,
per-material/bone and posed bounds, exact retained corner/UV proof, authored vertex-to-node proof,
actual raw-normal all-key rotations, texture wrappers and unchanged frozen dependency hashes.
Production full compare is correctly DIFFERENT. Existing collapsed constant-color cap UVs in
HouseEtc02/StoneMuWall04/StoneWall03/Well01 remain byte-identical (4/14/14/24 respectively).
No new collapsed UV or geometry triangles. All production UVs have positive area.

Each asset has immutable originals/info/dependencies, packed source.blend with REF_ORIGINAL,
editable construction copies, exported game snapshot, validation logs, matching actual-BMD
before/after/reverse/wire/readability images and per-asset notes. review/placements compares
unaltered original World1 transforms. All images are labeled offline proxies, not client captures.

Reproduction: package_textures.py; Blender build_sources.py and audit_sources.py; Python
export_validate.py, audit_exports.py, audit_bindings.py, audit_poses.py; Blender render_review.py
and render_placements.py; Python finalize.py and write_notes.py. Use explicit worktree cwd and
Blender --python-expr 'import sys; sys.dont_write_bytecode=True'. finalize.py installs only with
explicit --install into the five owned source-game files after hash guards, never shared runtime.
Scripts reuse read-only StaticBatch01 helpers. Painted raw outputs/prompt records are retained;
packaging performs only lossless source retention, size conversion and JPEG/container encoding.

Pending: coordinator/independent acceptance and actual client verification when stable. No
blocking technical rejection remains; no engine changes or filename changes were needed.
'''
(ROOT/'notes.md').write_text(text)
