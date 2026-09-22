# Architecture02 — Watermill, modular walls, roof caps and ladder

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-timber`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture-timber`.
Date: 2026-09-22. Offline production complete. Client verification pending.

Six new models cover 27 placements. Three accepted compatibility models cover another 17 placements and retain byte-identical BMDs: House04/HouseWall02 from Architecture01 integration 8094616d; HouseEtc02 from Masonry integration f1fbb29c. Original snapshots use current accepted textures at baseline 8094616d.

The two shared filenames are misleading: tile_wood01 is green-weathered coursed stone ashlar, tile_wood03 is green roof shingles. Both are repainted at 512×512 with original layout/material roles retained and previewed across all nine consumers. Static masonry panel edge recesses, roof interior trim and shallow ladder tread wear preserve original silhouettes, openings, joining planes and contacts. Frozen timber, water and all other materials remain unchanged.

| Model | World1 placements | Before → after triangles | Bones / action keys |
| --- | ---: | ---: | ---: |
| House05 | 1 | 276 → 324 | 10 / 30 |
| HouseWall01 | 3 | 36 → 52 | 1 / 1 |
| HouseWall04 | 8 | 56 → 88 | 1 / 1 |
| HouseWall05 | 8 | 30 → 54 | 1 / 1 |
| HouseWall06 | 4 | 28 → 52 | 1 / 1 |
| Stair01 | 3 | 78 → 182 | 1 / 1 |
| House04 | 1 | 358 → 358 | 9 / 40 |
| HouseWall02 | 15 | 110 → 110 | 3 / 1 |
| HouseEtc02 | 1 | 250 → 250 | 3 / 1 |

All six remodeled full comparisons are DIFFERENT; the three compatibility models are EQUIVALENT and byte-identical. Every skeleton plus actual action comparison is EQUIVALENT. Full local bind/action translations and Euler rotations, bone names/order/parents and action metadata are retained. Every hierarchical action matrix and every posed bound matches exactly in the explicit SMD matrix audit, including 300 mill action-bone samples and 360 readonly workshop samples. Every original vertex survives within 0.000016 units; protected triangles, normals and UVs pass. No meshes or material slots are added. A separate packed-source versus actual final-BMD audit matches every new panel triangle by material, bone index, position and UV; every new corner retains its intended bone. Only fixed ki12 owns new mill geometry; the five other rebuilt models have one bone each. A direct raw BMD audit confirms no vertex/normal bone mismatch on any owned mesh, including the animated mill; readonly normal bindings are unchanged. The read-only parser from the parallel CartHay worker is retained with its hash.

The official Blender exporter produces each authored mesh. Supported SMD/manifest packaging then restores immutable protected corner records, original panel-boundary coordinates and original bind/action data to avoid Blender roundtrip quantization. New interior geometry and bevel normals remain authored. Raw official and final packaged models compare EQUIVALENT; raw exports and all packing sources/logs remain available. This does not turn the intentional original-versus-remodeled geometry DIFFERENT result into a full-equivalence claim.

House05's complete wheel and ston02 water mesh 2 remain original; its 30-key action and world-time V scroll are preserved. HouseWall05/06 keep HeroTile 4 fade behavior. All frozen container hashes match, including accepted paints and original ston02. Both owned textures have complete consumer ownership recorded in `dependency-ownership.json`.

Exact game files changed (eight total):

- `src/bin/Data/Object1/House05.bmd`
- `src/bin/Data/Object1/tile_wood01.OZJ`
- `src/bin/Data/Object1/HouseWall01.bmd`
- `src/bin/Data/Object1/HouseWall04.bmd`
- `src/bin/Data/Object1/HouseWall05.bmd`
- `src/bin/Data/Object1/tile_wood03.OZJ`
- `src/bin/Data/Object1/HouseWall06.bmd`
- `src/bin/Data/Object1/Stair01.bmd`

`source-installation.json` confirms eight isolated-worktree game changes and 316 other World1/Object1 files unchanged. No shared runtime, client, engine, CMake, UI, terrain, main branch or remote changes.

Review images: `review/batch-review.jpg`, `review/texture-comparison.jpg`; all nine per-model `review/comparison.png` sheets; mill/workshop action sheets; indexed-effect neutral/additive sheets. Actual-transform assemblies are `review/town-roof-comparison.jpg`, `town-interior-comparison.jpg` and `west-corners-comparison.jpg`. Companion JSONs retain the exact unchanged World1 placement records. The interior view illustrates the roof target alpha 0 by hiding roof-cap render objects.

All evidence is offline Blender. Grass, 190-unit figure, camera, diffuse lighting, additive shading, V-scroll phases and roof omission are approximations, never client evidence. Actual terrain/collision/baked lighting and unowned buildings are omitted.

Reproduction from this assigned worktree, always explicit cwd, `PYTHONDONTWRITEBYTECODE=1` and the existing converter in `MU_BMDCONV`:

1. `package_textures.py` packages retained built-in imagegen masters with official mu_texture wrapping; copies every frozen dependency unchanged.
2. Blender `-b --python build_source.py` creates packed sources using reviewed read-only helpers from Masonry01/StaticBatch01.
3. `export_all.py` runs official export and preserve_contract supported converter packaging; readonly BMDs are copied exactly.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`, `validate_raw_bindings.py`, then Blender `validate_matrices.py` and `validate_authored_bindings.py` check the full engine/material/UV/bone/action/anchor/posed-bound contracts.
5. Blender `render_exports.py`, then `render_joins.py`; bundled Pillow Python `assemble_review.py` produces labeled review sheets.
6. `install_source.py` uses ownership and hash guards for isolated source installation only; `write_notes.py` refreshes local handoffs.

Immutable preservation/import and inspection scripts, original files, packed sources, editable master/final paintings and exact built-in imagegen prompts are retained. Preparation refuses to overwrite original archives. The coordinator alone edits shared handoffs and worklog.

Pending: coordinator and independent acceptance; actual client observation at 1920×1080 when stability permits. No asset is marked verified in client. No unresolved validator rejection remains.
