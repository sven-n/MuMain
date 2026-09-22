# Architecture03 — Bridges and final shared architectural materials

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-final`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture-final`.
Date: 2026-09-22. Offline production complete; client verification pending.

Three authored assets cover 34 placements: HouseWall03 timber lintel/threshold, Bridge01 carved stone parapet, and BridgeStone01 twin-log railing. Seventeen accepted BMD consumers remain byte-identical to the current baseline recorded in baseline-commit.txt, which contains Architecture02 integration 6b025039. They are archived with complete dependencies and fresh packed compatibility sources. Six shared diffuse paintings close the remaining architectural texture families; the seventh owned family, bridge_shadow01, stays byte-exact because its alpha gradient already serves the correct role.

The stone bridge retains its double arches and both original projecting beast heads. The timber bridge retains its log silhouettes, original end-grain faces and diagonal braces. Only safe interior faces are refined, preserving all original corner positions, modular end planes, doorway opening and contact surfaces. Painting identities and original atlas regions are retained; bridge_01 is 1024×512 and the five square paintings are 512×512 RGB. The 2×32 shadow strip and every other frozen material container remain unchanged.

| Model | Placements | Before → after triangles | Bones / action keys | Full comparison |
| --- | ---: | ---: | ---: | --- |
| HouseWall03 | 4 | 20 → 36 | 1 / 1 | DIFFERENT |
| Bridge01 | 16 | 196 → 276 | 2 / 1 | DIFFERENT |
| BridgeStone01 | 14 | 74 → 106 | 2 / 1 | DIFFERENT |
| Fence01 | 180 | 116 → 116 | 1 / 1 | EQUIVALENT |
| FireLight02 | 18 | 34 → 34 | 2 / 1 | EQUIVALENT |
| House01 | 2 | 208 → 208 | 2 / 1 | EQUIVALENT |
| House03 | 1 | 313 → 313 | 7 / 1 | EQUIVALENT |
| House04 | 1 | 358 → 358 | 9 / 40 | EQUIVALENT |
| House05 | 1 | 324 → 324 | 10 / 30 | EQUIVALENT |
| HouseEtc02 | 1 | 250 → 250 | 3 / 1 | EQUIVALENT |
| HouseWall01 | 3 | 52 → 52 | 1 / 1 | EQUIVALENT |
| HouseWall02 | 15 | 110 → 110 | 3 / 1 | EQUIVALENT |
| HouseWall04 | 8 | 88 → 88 | 1 / 1 | EQUIVALENT |
| HouseWall05 | 8 | 54 → 54 | 1 / 1 | EQUIVALENT |
| HouseWall06 | 4 | 52 → 52 | 1 / 1 | EQUIVALENT |
| StoneMuWall01 | 4 | 398 → 398 | 1 / 1 | EQUIVALENT |
| StoneWall01 | 8 | 414 → 414 | 1 / 1 | EQUIVALENT |
| StoneWall02 | 6 | 414 → 414 | 1 / 1 | EQUIVALENT |
| Tent01 | 2 | 130 → 130 | 4 / 36 | EQUIVALENT |
| Tree07 | 36 | 90 → 90 | 1 / 1 | EQUIVALENT |

All 20 converter validations, texture wrapper checks, full rig/action comparisons, local-key and hierarchical matrix/posed-bound checks pass. Every original corner and each protected triangle's UV/normal data passes. All authored new triangle corners retain intended bone indices, positions and UVs in the actual final BMD; the raw vertex/normal-node audit passes. All 17 readonly BMDs are unchanged. No model exceeds 1500 triangles. Complete consumer coverage is in dependency-ownership.json; frozen hashes and every texture dimension are retained per model.

The official importer/exporter and mu_texture are used. Supported converter manifest packaging restores exact original rig/actions and protected corner records after export; authored new panel interiors remain. Raw official versus final packaged output compares EQUIVALENT, while original-versus-remodeled geometry correctly reports DIFFERENT. Every original BMD and container also matches the pinned Git revision independently.

Exact game files changed:

- `src/bin/Data/Object1/HouseWall03.bmd`
- `src/bin/Data/Object1/tile_wood02.OZJ`
- `src/bin/Data/Object1/Bridge01.bmd`
- `src/bin/Data/Object1/bridge_01.OZJ`
- `src/bin/Data/Object1/BridgeStone01.bmd`
- `src/bin/Data/Object1/tree_04.OZJ`
- `src/bin/Data/Object1/tile_ston06.OZJ`
- `src/bin/Data/Object1/tile_ston04.OZJ`
- `src/bin/Data/Object1/tile_02.OZJ`

Source installation is confined to this isolated worktree: 9 changed game files, 315 other World1/Object1 files unchanged. bridge_shadow01 is byte-identical. No runtime/client, engine, CMake, UI, terrain, main branch or remote changes.

Review images: `review/batch-review.jpg`, `review/compatibility-1.jpg`, `review/compatibility-2.jpg`, `review/texture-comparison.jpg`, and all 20 individual `review/comparison.png` sheets. House04, House05 and Tent01 include three-pose action sheets; House03/04/05 and HouseWall02 include neutral/additive effect sheets. Actual unchanged placement assemblies are `stone-bridge-comparison.jpg`, `log-bridge-comparison.jpg`, `west-door-comparison.jpg` and `town-interior-comparison.jpg`. Companion JSONs retain all placement records. The interior preview omits roof caps to illustrate the existing HeroTile4 fade target.

All review evidence is offline Blender, never client evidence. Grass/figure/camera/light/additive/scroll/fade approximations are labeled. Actual terrain, collision, baked lighting and assets outside this dependency group are omitted. Bridge fire emitters are not simulated. Real client validation at 1920×1080 remains pending due known client instability; no asset is marked verified in client.

Reproduction, run with explicit assigned-worktree cwd and PYTHONDONTWRITEBYTECODE=1; MU_BMDCONV points to the existing converter:

1. `package_textures.py` wraps retained imagegen masters and copies frozen dependencies.
2. Blender `-b --python build_source.py` creates packed sources with REF_ORIGINAL and editable high-poly candidates.
3. `export_all.py` runs the official exporter, protected-corner/rig packaging and exact readonly BMD copying.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`, `validate_raw_bindings.py`; Blender `validate_matrices.py` and `validate_authored_bindings.py` validate the package.
5. Blender `render_exports.py` and `render_joins.py`; Pillow Python `assemble_review.py` composes labeled evidence.
6. `install_source.py` checks ownership/hashes before isolated source installation; `write_notes.py` refreshes local notes.

Preparation/import/inspection scripts, exact prompts, unchanged generated master artwork, editable PNGs, source blends, exported game files and complete validation output remain in this deliverable. The coordinator alone updates shared handoffs and worklog. No unresolved validator rejection remains.
