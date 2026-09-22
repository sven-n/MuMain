# Cannons01: swivel guns, gallows and twelve-consumer wood atlas

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
| Cannon01 | 1 | 181 → 707 | 1 | DIFFERENT (intentional remodel) |
| Cannon02 | 2 | 155 → 681 | 1 | DIFFERENT (intentional remodel) |
| Cannon03 | 3 | 75 → 367 | 1 | DIFFERENT (intentional remodel) |
| Hanging01 | 1 | 162 → 338 | 25 | DIFFERENT (intentional remodel) |
| Carriage01 | 4 | 472 → 472 | 21 | EQUIVALENT; actual BMD bytes unchanged |
| Carriage02 | 7 | 450 → 450 | 1 | EQUIVALENT; actual BMD bytes unchanged |
| Carriage03 | 0 | 538 → 538 | 1 | EQUIVALENT; actual BMD bytes unchanged |
| Carriage04 | 2 | 882 → 882 | 1 | EQUIVALENT; actual BMD bytes unchanged |
| HouseEtc02 | 1 | 250 → 250 | 1 | EQUIVALENT; actual BMD bytes unchanged |
| StoneMuWall04 | 3 | 317 → 317 | 1 | EQUIVALENT; actual BMD bytes unchanged |
| StoneWall03 | 2 | 333 → 333 | 1 | EQUIVALENT; actual BMD bytes unchanged |
| Well01 | 0 | 915 → 915 | 1 | EQUIVALENT; actual BMD bytes unchanged |

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
