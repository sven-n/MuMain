# Groundcover01 — Lorencia vegetation art

Owner: ASTRA groundcover production worker. Branch: `codex/lorencia-groundcover`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-groundcover`.
Status: ready for coordinator review; offline validated, client verification pending.

| Asset | Identity | World1 placements | Triangles | Texture |
|---|---|---:|---:|---|
| Grass01 | Wide low ivy/bramble leaf mat | 175 | 120 retained | tree_08, 512×512 |
| Grass02 | Spread low ivy/bramble leaf mat | 184 | 120 retained | tree_08, 512×512 |
| Grass05 | Four bent broad-leaf ground clumps | 317 | 112 retained | tree_09, 256×512 |
| Grass06 | Narrow low broad-leaf cluster | 82 | 112 retained | tree_09, 256×512 |

758 placements benefit from two coordinated alpha paintings. Each pair owns its exclusive
texture dependency; no other filenames or World1 files changed. Geometry remains economical
and retains its original footprint, origin, orientation, rigid bindings, UVs and mesh order.
This batch rebuilds the surface artwork rather than adding unnecessary polygons to cards.

All four official Blender exports report **EQUIVALENT** in full bmdconv comparison, with zero
unmatched triangles and maximum corner deviation 0.0002 units. Skeleton-only comparisons also
report EQUIVALENT. Per-bone local position and rotation-matrix checks, action metadata, exported
UV/material correspondence, bounds, SMD validator and TGA/OZT checks pass. Alternate Euler
encodings in Grass05/06 represent the original unchanged orientations.

Ivy visible-alpha coverage is 39.5%, versus 33.9% in the 32×32 original. Broad-leaf coverage is
51.5%, versus 57.2% in the 32×64 original. Coverage uses alpha >= 64 for this audit. The initial
overly dense ivy concept was rejected and is retained for provenance. Fully transparent RGB
is filled from the nearest visible colors to avoid dark filtering fringes; alpha is unchanged.

Each asset folder contains `notes.md`, exact `placements.json`, untouched originals, packed
`source.blend` with `REF_ORIGINAL`, exported game files, validation logs, a fresh reimported
Blender file and matched review images. `textures/raw` retains built-in imagegen paintings;
`textures/final` contains editable PNG/TGA and encoded game files. See `prompts.md` for every
generation/edit prompt. No PBR or normal maps are required. No high-poly source is needed for
unchanged alpha cards.

Review `Grass*/review/comparison.jpg`; individual matched views, wireframes and 240px
readability images sit beside them. Terrain previews use completed TileGrass01 at 100 world
units per repeat and a 190-unit figure proxy. They are offline inspection renders at the origin,
not actual client screenshots, actual object placements or a measured client camera.

Run scripts from this assigned worktree, in order:

1. Python `package_textures.py` packages the retained generated sources.
2. Blender `build_sources.py` builds packed production files around original references.
3. Python `export_validate.py` uses the official exporter and bmdconv to prove equivalence.
4. Blender `render_review.py` renders original and freshly reimported exports.
5. Python `finalize.py` writes notes/review sheets and installs only the six owned game files
   into this worktree's source `src/bin/Data/Object1` folder.

Use the bundled Python interpreter documented by the coordinator (Pillow and NumPy), Blender
5.2, and the existing built bmdconv path recorded in the scripts. `import_originals.py` recreates
the immutable packed original projects. Final source-file hashes are recorded in
`installed-source-files.json`; untouched original hashes are in `original-hashes.json`.

No shared runtime Data folder, client process, terrain file, engine/CMake source, shared
coordination document or another worker's worktree was modified. Client appearance, cutout
sorting, filtering and vegetation against the actual map lighting remain pending.
