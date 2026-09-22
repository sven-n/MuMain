# Flowering01 — hollow log and accent trees

Owner: ASTRA groundcover/tree worker, branch `codex/lorencia-flowering`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-flowering`.
Status: exported and validated offline; coordinator review and client acceptance pending.

| Model | Actual identity | World1 placements | Retained triangles | Bones / action keys |
|---|---|---:|---:|---:|
| Tree08 | Hollow fallen trunk | 8 | 176 | 1 / 1 |
| Tree12 | Pink-lilac flowering tree | 1 | 393 | 9 / 31 |
| Tree13 | Ochre autumn tree | 4 | 393 | 10 / 31 |

Thirteen placements share this complete bark dependency group. This is a surface artwork
rebuild with original economical geometry retained. The open hollow trunk ends, bark role,
crown silhouettes, origins, footprints, rigid bindings and full sway remain. No geometry
subdivision or placement change compensates for the paintings.

Exactly six game files change under `src/bin/Data/Object1/`:

- `Tree08.bmd`, `Tree12.bmd`, `Tree13.bmd`.
- `tree_01.OZJ`: dark gray shared bark, 128×64 → 512×256.
- `tree_04.OZT`: dusty pink/lilac flowering branches, 64² → 512².
- `tree_05.OZT`: ochre/russet leaves, 64² → 512².

The shared bark has exactly these three consumers. Tree12 keeps bark at mesh 0 and flower
alpha at mesh 1; Tree13 keeps ochre alpha at mesh 0 and bark at mesh 1. Tree08 retains its single
bark mesh. No texture names, original mesh ordering or render flags change. Actual model
bones have no dummy prefix in this batch; every original name, index and parent is retained.

All three full BMD comparisons report **EQUIVALENT**, with zero unmatched triangles. Maximum
geometry corner distance is 0.0004 units for Tree08 and 0.0000 at printed precision for the
two trees. Bind bounds agree exactly at converter precision. Skeleton-only comparisons are
EQUIVALENT. All 31 frames of both tree actions are checked as local translations and rotation
matrices, with largest local translation difference 0.000107 units and no rotation-matrix
difference. Every action remains at its original index and key count, lock=0.

Exported UVs match exactly, including out-of-range bark repeats. Each original corner keeps
its rigid bone and material. The official SMD validator and texture loader checks pass.
Blender checks one UV set, finite positions/UVs, nondegenerate triangles and one valid rigid
bone per vertex. Detailed numerical reports and complete comparison output are retained.

Final visible-alpha coverage is 48.9% for flowers versus original 50.2%, and 53.6% for ochre
versus original 51.8%, measured at alpha >= 64. Initial sparse, bright and overly dense paintings
were corrected with imagegen; those iterations remain in `textures/raw` with all prompts.
Faint colored pixels below alpha 64 are padded with nearby visible RGB to prevent filtering
fringes, while preserving the generated alpha channel. Final alpha uses 32-bit uncompressed
bottom-left TGA/OZT. Bark is RGB JPEG/OZJ. No PBR or normal map is required.

Each model folder contains `notes.md`, exact `placements.json`, untouched BMD/textures/info/
SMD and packed original sources, packed `source.blend` with `REF_ORIGINAL`, exported game
files, validation logs, a fresh export reimport, matching comparison renders, wireframe and
240px readability. Animated models include frames 0,15,30 in `review/action-comparison.jpg`;
all 31 keys are validated numerically. No high-poly mesh is needed for this unchanged geometry.

Review the per-asset `review/comparison.jpg` sheets and
`review/actual-accent-pair-before.png` / `review/actual-accent-pair-after.png`. The pair uses
the actual adjacent World1 tree transforms at tiles (116,108.5) and (114.5,108.5), including
pink scale 0.6 and ochre scale 1. A shared coordinate offset normalizes the scene. Its ground,
lighting and 190-unit figure are inspection proxies; actual terrain/neighboring buildings are
omitted. These are **offline Blender renders, not client screenshots**.

Raw/editable paintings and final PNG/JPG/TGA/wrappers are in `textures/`; prompts are in
`prompts.md`. Tiling/alpha reports accompany final textures. Original hashes and source-file
installation hashes are recorded at batch root. No previous accepted asset, shared handoff,
coordination board, engine/CMake source, other worktree or runtime Data folder was changed.

Reproduce using bundled Python with Pillow/NumPy and Blender with Source Tools. From this
worktree/integrated checkout run Python `package_textures.py`, Blender `build_sources.py`,
Python `export_validate.py`, Blender `render_review.py`, Blender `render_placement_pair.py`,
then Python `finalize.py`, all under `assets-work/World1/Flowering01/`. `config.py` records
baseline and tool paths. Finalization installs only the six owned source-game files.

Pending: actual client loading, alpha sorting/filtering, vertex lighting, material colors,
sway under engine timing and gameplay readability. No runtime installation or client session
was performed by this worker.
