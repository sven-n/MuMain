# Architecture01 — Lorencia houses, workshop, canopy and window walls

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture`.
Date: 2026-09-22. Offline production complete; client verification pending.

Five remodeled models cover 21 World1 placements. The accepted Masonry annex is a sixth readonly consumer with one placement; its 250-triangle BMD remains byte-identical. Shared paints have been previewed across all six consumers. Conservative recessed stone, shutter and timber panel details preserve every original corner, roof edge, opening and connection. Repainted linen, carved doorway, roof limestone, shutter wood, slatted awning and restrained light effects preserve recognizable material roles and the dark medieval MU palette. Frozen wood and stone atlases remain unchanged.

| Model | Placements | Before → after triangles | Bones / action keys |
| --- | ---: | ---: | ---: |
| House01 | 2 | 176 → 208 | 2 / 1 |
| House03 | 1 | 289 → 313 | 7 / 1 |
| House04 | 1 | 342 → 358 | 9 / 40 |
| Tent01 | 2 | 66 → 130 | 4 / 36 |
| HouseWall02 | 15 | 70 → 110 | 3 / 1 |
| HouseEtc02 | 1 | 250 → 250 | 3 / 1 |

All five remodeled full comparisons report DIFFERENT; all six skeleton plus full-action comparisons report EQUIVALENT. Local bind/action poses, names/order/parents and lock metadata are checked explicitly. All local translation and Euler deltas are zero at the converter's retained precision, including House04's 360 action bone samples and Tent's 144 samples. All bind bounds match. Every original corner survives within 0.000016 units. Full triangle, UV, bone and normal checks protect unmodified roof silhouettes, openings, modular ends, cloth and special effect geometry.

The official Blender exporter is used for all authored models. A documented SMD/manifest packaging step restores original protected corner records and exact bind/action data to avoid the Blender exporter's normal quantization and House04 Euler conversion drift. It does not replace the authored geometry: the raw official and final packaged BMDs compare EQUIVALENT. Both stages and their logs are retained. The final original-versus-remodeled model comparison remains DIFFERENT.

Engine effects retained: House03 and HouseWall02 mesh 4 light_02 (brightness 0.4–0.7); House04 mesh 8 tile_space01 (world-time V scroll). Awning alpha is the exact original 128² mask expanded to 512² nearest-neighbor; top-left origin is not introduced. Existing material slot order and game filenames remain unchanged. Surface paints are 512²; blue effect 256²; light effect 128². No other texture container is modified.

`dependency-ownership.json` records all consumers. `original-sha256.json` proves untouched original game files against pinned baseline f1fbb29c. `source-installation.json` records exactly 11 changed game files and 313 unchanged other World1/Object1 files; accepted HouseEtc02 is among the unchanged files. No shared runtime/client, engine, CMake, UI, terrain, main branch or remote was changed.

Exact game files changed:

- `src/bin/Data/Object1/House01.bmd`
- `src/bin/Data/Object1/tile_house01.OZJ`
- `src/bin/Data/Object1/tile_ston05.OZJ`
- `src/bin/Data/Object1/House03.bmd`
- `src/bin/Data/Object1/tile_ston07.OZT`
- `src/bin/Data/Object1/light_02.OZJ`
- `src/bin/Data/Object1/House04.bmd`
- `src/bin/Data/Object1/tile_windows01.OZJ`
- `src/bin/Data/Object1/tile_space01.OZJ`
- `src/bin/Data/Object1/Tent01.bmd`
- `src/bin/Data/Object1/HouseWall02.bmd`

Review images:

- `review/batch-review.jpg` and `review/texture-comparison.jpg`.
- Each asset's `review/comparison.png`: original versus actual final exported BMD, wireframe, reduced-scale preview.
- House04 and Tent01 `review/action-comparison.jpg`: matched poses across retained clips.
- House03, House04 and HouseWall02 `review/effect-comparison.jpg`: neutral diffuse versus labeled additive approximation.
- `review/house-pair-comparison.jpg` and `review/town-wall-comparison.jpg`: actual unchanged World1 transforms, matched camera. Placement JSONs accompany them.

All images are offline Blender renders. Additive light/blue effect shading, flat grass, 190-unit figure and camera scale are approximations; actual terrain, baked lighting, unrelated buildings and collision are omitted. No image is presented as client evidence.

Reproduction: run all commands explicitly in this worktree, with `PYTHONDONTWRITEBYTECODE=1` and `MU_BMDCONV` set to the existing converter.

1. `package_textures.py`: retained imagegen masters to editable/final images, original alpha preservation, official mu_texture wrap.
2. Blender `-b --python build_source.py`: original official import to packed source including REF_ORIGINAL and optional high-poly candidate.
3. `export_all.py`: official mu_bmd_export.py, then preserve_contract.py and supported bmdconv manifest packaging; readonly annex copied exactly.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`: full and rig/action comparisons, local pose proof, corners/UV/normals, immutable originals, dependency/alpha contracts.
5. Blender `-b --python render_exports.py`, then `render_joins.py`; bundled Pillow Python `assemble_review.py`.
6. `install_source.py`: hash guards, owned source-files-only installation; `write_notes.py` refreshes these local handoffs.

The original preparation/import and inspection scripts, exact generation prompts and unedited master images are retained. Re-running preparation refuses to overwrite preserved originals. Shared helpers are imported read-only from approved StaticBatch01 and Masonry01 scripts.

Pending: coordinator visual/integration review, independent technical review, and actual 1920×1080 client observation when stability permits. No asset is marked verified in client. User-authorized offline production has no unresolved validator rejection.
