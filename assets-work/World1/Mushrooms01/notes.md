# Mushrooms01 — rounded woodland mushroom clusters

Owner: ASTRA coordinator. Branch `codex/lorencia-mushrooms`, isolated worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-mushrooms`.

Grass07 is a seven-mushroom cluster (35 placements); Grass08 is a three-mushroom cluster
(7 placements). Both share the exclusive mushroom.jpg atlas. Rebuilt curved caps and
fibrous stems retain the original red/ivory material roles, individual tilts, placement
footprints and every original per-mushroom bind bound. Counts: 126→1162 and 54→498 triangles.
The diffuse is a 512² RGB JPEG/OZJ generated through the imagegen skill. The original 32²
texture, raw generated painting, editable PNG, final JPEG/wrapper and exact prompt remain.

Only three game files change: `src/bin/Data/Object1/Grass07.bmd`, `Grass08.bmd` and
`mushroom.OZJ` in that same directory. Complete original World1 placement arrays accompany
each model. Source loader MapManager.cpp:1030–1031 loads Grass01–08 into Object1; inspection
found no special Grass07/08 mesh-index or bone controls in ZzzObject.cpp. Original rigs
remain anyway: seven/three root bones, one action with one key and lock 0, one material.

Offline validation passes: official reference/action SMD checks, skeleton/actions EQUIVALENT,
original bound equality, rigid bindings, finite nondegenerate geometry/UVs and texture loader.
Full BMD comparisons correctly report DIFFERENT. Every local reference/action pose sample is
measured; per-model notes and complete logs retain all results. Original source.blend imports
and rebuilt packed source.blend with REF_ORIGINAL are retained. No high-poly source is needed
for these explicitly authored low-poly profiles.

Matching-camera review sheets are `Grass07/review/comparison.jpg` and
`Grass08/review/comparison.jpg`; each includes wireframe and reduced readability. The 190-unit
scale bar gives a character-height reference. These images are offline Blender renders.
Client lighting, actual placed contact and gameplay camera review are pending. No runtime
folder or client process was touched.

Reproduce from this worktree or integrated checkout with bundled Pillow Python and Blender:
`prepare.py` archives immutable ac0f6dd8 originals and imports them; `package_textures.py`
packages the retained painting; Blender `build.py`; Python `export_validate.py`; Blender
`audit_sources.py` and `render_review.py`; Python `assemble_review.py`, `write_notes.py` and
`install.py`. All scripts are under this directory. Use Blender `--python-exit-code 1` and
`PYTHONDONTWRITEBYTECODE=1`. The installer permits only original/intended file hashes and
proves every other World1/Object1 file unchanged; it never installs into shared runtime Data.
