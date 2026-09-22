# Masonry01 — Lorencia modular stone and dragon reliefs

Owner: ASTRA fences/timber/masonry worker. Branch `codex/lorencia-masonry`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-masonry`.
Date: 2026-09-22. Offline production complete. Client verification pending.

Six connected models cover 137 placements. Coordinated 512×512 stone paintings preserve c_wall04's tan limestone and narrow carved lower scroll band, c_wall05's central monster face, c_wall06's paired-frill dragon face, and tile_ston01's large weathered ashlar. Shallow dressed-stone panel recesses add edge depth without moving any original corner. This is a conservative architecture pass preserving the exact original silhouettes and connection geometry.

| Model | World1 placements | Before → after triangles | Bones / action |
| --- | ---: | ---: | ---: |
| HouseEtc01 | 52 | 34 → 50 | 1 / 1 key |
| StoneMuWall01 | 4 | 318 → 398 | 1 / 1 key |
| StoneMuWall02 | 32 | 44 → 76 | 1 / 1 key |
| StoneMuWall03 | 45 | 52 → 84 | 1 / 1 key |
| StoneMuWall04 | 3 | 285 → 317 | 3 / 1 key |
| HouseEtc02 | 1 | 226 → 250 | 3 / 1 key |

All full model comparisons report DIFFERENT. All isolated skeleton plus actual action comparisons report EQUIVALENT; all local poses are explicitly checked. All bind bounds match. Material slot counts/order and exact game filenames are unchanged. Every original vertex survives export within 0.000001 units. The portcullis, gate inner planes, hut doorway, all carved face geometry and frozen-material scenery retain their original triangles, UVs and bindings. Original normal directions differ by at most 0.081719 degrees after official roundtrip normalization/quantization; no byte-equivalence claim is made for normals.

All four repainted containers are exclusive to the six owned models, independently checked in `dependency-ownership.json`. Frozen materials are tile_wood02, tile_ston04, tile_wood03, tile_house01, horse_drawn_01, tile_ston06, tile_02 and bridge_01. Their OZJ bytes are unchanged. `source-installation.json` confirms exactly ten isolated-worktree game changes and all other World1/Object1 files unchanged. No runtime, engine, UI, terrain, main-branch or remote changes were made.

Actual game files changed:

- `src/bin/Data/Object1/HouseEtc01.bmd`
- `src/bin/Data/Object1/StoneMuWall01.bmd`
- `src/bin/Data/Object1/StoneMuWall02.bmd`
- `src/bin/Data/Object1/StoneMuWall03.bmd`
- `src/bin/Data/Object1/StoneMuWall04.bmd`
- `src/bin/Data/Object1/HouseEtc02.bmd`
- `src/bin/Data/Object1/c_wall04.OZJ`
- `src/bin/Data/Object1/c_wall05.OZJ`
- `src/bin/Data/Object1/c_wall06.OZJ`
- `src/bin/Data/Object1/tile_ston01.OZJ`

Review images: `review/batch-review.jpg`; per-model `review/comparison.png`; actual unchanged placement assemblies `review/south-gate-comparison.jpg` and `review/siege-wall-comparison.jpg`. The latter uses the west-side wall and siege fixture near its gate. These are Blender images of official re-imported exported BMDs. Flat grass/figure studies, camera and diffuse lighting are offline approximations, never client evidence. Complete retained placement records accompany the assemblies.

Reproduction from this worktree (set MU_BMDCONV to the existing converter and use the bundled Pillow Python for image scripts):

1. `package_textures.py`: retained built-in imagegen master PNGs → 512 RGB editable PNG/JPEG + official OZJ wrap; frozen containers copied unchanged.
2. Blender `-b --python build_source.py`: original official-import files → packed low-poly source plus REF_ORIGINAL/high-poly candidate.
3. `export_all.py`: official mu_bmd_export.py only, with logs and generated SMD/actions.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`: engine validation, full/skeleton/action comparison, exact anchors and protected triangle proof, original SHA verification and full dependency audit.
5. Blender `-b --python render_exports.py`, then `render_joins.py`; bundled Pillow Python `assemble_review.py`.
6. `install_source.py`: owned-file and shared-texture hash guards, isolated source install only. `write_notes.py` refreshes these local handoffs.

Every command must use this assigned worktree as cwd. The preserved original baseline commit is pinned in `baseline-commit.txt`; preservation scripts refuse to overwrite originals. Python bytecode writes are disabled. No shared documentation is edited by this worker.

Pending checks: coordinator visual/integration review and real client observation at 1920×1080 once stability permits. No asset is marked verified in client. The user authorized continued offline production.
