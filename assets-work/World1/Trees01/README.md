# Trees01 — Lorencia oak, conifer and pale snag

Owner: ASTRA tree worker. Branch `codex/lorencia-trees`, isolated worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-trees`.
Status: exported and validated offline; coordinator review and client acceptance pending.

| Model | Identity | World1 placements | Triangles retained | Bones / action keys |
|---|---|---:|---:|---:|
| Tree01 | Spreading oak-like tree | 80 | 412 | 16 / 31 |
| Tree02 | Large spreading oak-like tree | 23 | 770 | 24 / 31 |
| Tree11 | Tiered conifer | 165 | 169 | 10 / 31 |
| Tree06 | Tall bare pale birch snag | 52 | 89 | 1 / 1 |

320 placements. This batch rebuilds surface artwork and retains the original economical
trunk/crown geometry and all sway. Trunk footprints, root contacts, branch hierarchy,
original rigid bindings, original material order and intentional repeated UVs remain.
Tree01/02's engine collision box and velocity=0.4/scale are untouched.

Exact owned game changes under `src/bin/Data/Object1/`:

- `Tree01.bmd`, `Tree02.bmd`, `Tree11.bmd`, `Tree06.bmd`.
- `tree.OZJ` — oak bark, 128² → 512².
- `Tree_a.OZT` — oak foliage, 128² → 512².
- `tree_06.OZT` — conifer boughs, 128² → 512².
- `tree_02.OZJ` — pale snag bark, 128×64 → 512×256.

Tree11's other dependency `tree_03.OZJ` is **frozen**, because it is shared with other models.
It remains byte-identical in source game files and is copied unchanged into the complete
Tree11 export package. Each asset's exact dependencies and placements are in `notes.md` and
`placements.json`. No other tree group, terrain data or runtime folder was edited.

All four full BMD comparisons report EQUIVALENT. Maximum geometry corner deviation is
0.0048 units for Tree02, 0.0000 at printed precision for the others; these are reported
conversion differences, not deliberate remodeling. All before/after bind bounds agree at
converter precision. Mesh count/order, material names, bone names/order/parents, dummy names,
action order, key counts and lock=0 are unchanged. Skeleton-only comparisons are EQUIVALENT.

Every local action key is independently checked as translation and rotation matrices,
including all 31 frames of each animated tree. The largest local position-component change
is 0.000183 units; largest rotation-matrix change is 0.000000332. Explicit tolerances are
0.001 units and 0.00001, accommodating normal Float32 hierarchy conversion. Exported UVs
match exactly, including out-of-range bark repeats; every corner retains its rigid bone.
Reference/action SMD validation and final texture container checks pass.

Both alpha paintings preserve approximate coverage: oak 54.1% versus original 51.3%; conifer
58.2% versus original 58.3%, measured at alpha >= 64. Generated alpha is retained, and RGB
under alpha-zero texels is extended from adjacent visible colors to prevent filtering
fringes. Final alpha uses 32-bit uncompressed bottom-left TGA/OZT. Bark is RGB JPEG/OZJ.
No new render flags, shaders, filenames or PBR dependencies were introduced.

Built-in imagegen prompts and raw editable paintings are in `prompts.md` and `textures/raw`.
Final editable PNG/JPG/TGA and game wrappers are in `textures/final`, alongside coverage
reports and 3×3 bark tiling previews. Each asset contains untouched originals, converter
info/SMD, a packed original Blender source, packed `source.blend` with `REF_ORIGINAL`, final
game exports, all logs, a freshly reimported export and complete validation records. There
is no high-poly source because this pass deliberately retains original low-cost geometry.

Review `Tree*/review/comparison.jpg` and `Tree*/review/action-comparison.jpg`. Wireframe and
240px readability images are included. The action sheets show frames 0,15,30 through matched
cameras; every frame is checked numerically. Terrain previews use completed pilot TileGrass01
at 100 units per repeat with a 190-unit figure proxy. These are offline Blender renders at an
invented staging origin, not client screenshots or actual map placement captures.

Reproduce from this worktree or the integrated checkout, using bundled Python with Pillow/
NumPy and Blender with Source Tools: Python `package_textures.py`, Blender `build_sources.py`,
Python `export_validate.py`, Blender `render_review.py`, then Python `finalize.py`. Run these
paths under `assets-work/World1/Trees01/`. `config.py` fixes the original baseline and engine
tool paths. Finalization installs only the eight owned files into source Data and proves the
frozen texture unchanged; it never installs runtime files. `import_originals.py` reconstructs
packed originals from the archived untouched BMD/texture files.

Pending: actual client load, alpha sorting/filtering, vertex lighting, full sway under engine
timing, gameplay-scale silhouette and contacts. No client process was operated in this batch.
