# Lorencia rebuild consolidated handoff — 2026-09-22

**102/106 in-scope static assets accepted offline**, representing the actual World1 inventory; the complete static inventory has 2,786 placements. Four completed pilot models are preserved, and later batches cover the remaining models. The 17 completed terrain paintings remain unchanged. Client acceptance is pending for this entire pass.

Integration branch: **art/lorencia-rebuild**. Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rebuild`. Reviewed baseline: `ac0f6dd8` (contains `2e2ed427`, `b232470c`, `8d22a912` and completed static pilot). Art/review revision when this handoff was generated: `95386cd82270de9b125d31c289d4de6285b59756`. The final documentation commit may follow it. No push or merge to main.

## Accepted batches and owners

Preserved pilot: Beer01 (`8d22a912`, 784 triangles and 512² plate2 atlas), Candle01 (`c7a9765a`), TreasureChest01 (`993304be`) and Tomb03 (`100071aa`). Existing tavern work was inspected, independently reviewed and integrated from its original branch. Production workers used separate worktrees; coordinator integrated accepted commits sequentially.

The model column lists newly covered models once. Batch notes also list all readonly shared-texture consumers and their pinned original revisions. The [asset board](asset-board.md) records exact ownership, worktrees, BMD paths, complete texture dependencies and World1 placements for every candidate.

| Batch | Owner | Newly covered models | Source commits | Integration commits |
|---|---|---|---|---|
| [TavernProps](../TavernProps/notes.md) | prior ASTRA tavern artist | Furniture03, Furniture04, Furniture05 | `fe69aa12`, `45c82ea0` | `9a10643c`, `527e82d8` |
| [Groundcover01](../Groundcover01/README.md) | groundcover subagent | Grass01, Grass02, Grass05, Grass06 | `85cef70e`, `06091da4` | `ea56fc66`, `d67ae0c7` |
| [Fences01](../Fences01/notes.md) | fences subagent | Fence01, Fence02, Fence03, Fence04 | `150fcfd0` | `185fd5a8` |
| [Scrub01](../Scrub01/notes.md) | reviewer / scrub subagent | Tree09, Tree10, Grass03, Grass04 | `3ea6d48b` | `465136d4` |
| [Trees01](../Trees01/README.md) | groundcover / trees subagent | Tree01, Tree02, Tree11, Tree06 | `0b6864f1` | `ac16ffeb` |
| [TimberProps01](../TimberProps01/notes.md) | fences / timber props subagent | House02, TreasureDrum01, StreetLight01 | `0b271edc` | `9a7eedf7` |
| [Rocks01](../Rocks01/notes.md) | reviewer / rocks subagent | Stone01, Stone02, Stone03, Stone04, Stone05, Waterspout01 | `8d3cbcb7` | `db27198a` |
| [Flowering01](../Flowering01/README.md) | groundcover / flowering subagent | Tree08, Tree12, Tree13 | `70cce08a` | `2d827ed5` |
| [Mushrooms01](../Mushrooms01/notes.md) | ASTRA coordinator; independent review groundcover agent | Grass07, Grass08 | `37a3cbcf` | `f8928697` |
| [BareTrees01](../BareTrees01/notes.md) | groundcover / bare trees subagent | Tree03, Tree04, Tree05, Tree07 | `c9705c23` | `8b853434` |
| [Ironwork01](../Ironwork01/notes.md) | reviewer / ironwork subagent | SteelWall01, SteelWall02, SteelWall03, SteelDoor01, HouseEtc03 | `8788f85c`, `a031bc72` | `f5c11181`, `37844e87` |
| [Masonry01](../Masonry01/notes.md) | fences / masonry subagent | HouseEtc01, StoneMuWall01, StoneMuWall02, StoneMuWall03, StoneMuWall04, HouseEtc02 | `5180fd6d` | `f1fbb29c` |
| [StoneWalls01](../StoneWalls01/notes.md) | reviewer / stone walls subagent | StoneWall01, StoneWall02, StoneWall03, StoneWall05, StoneWall06 | `0fc5cb36` | `149780be` |
| [FireProps01](../FireProps01/notes.md) | ASTRA coordinator; independent review reviewer agent | FireLight01, FireLight02, Bonfire01, DoungeonGate01 | `967dc888`, `ece578a3` | `d3c18b11`, `97f21562` |
| [TavernDetails01](../TavernDetails01/notes.md) | groundcover / tavern details subagent | Beer02, Beer03, Furniture01, Furniture02, Furniture06, Furniture07 | `dd01d2eb` | `d051ee31` |
| [Architecture01](../Architecture01/notes.md) | fences / architecture subagent | House01, House03, House04, Tent01, HouseWall02 | `6fef840f` | `8094616d` |
| [SignsBanners01](../SignsBanners01/notes.md) | reviewer / signs and banners | Sign01, Sign02, Curtain01, StoneWall04 | `50a9de6e` | `cd7ed604` |
| [Ship01](../Ship01/notes.md) | coordinator / ship | Ship01 | `e687807a`, `29487fcd` | `0e2b794d`, `16da3679` |
| [Architecture02](../Architecture02/notes.md) | fences / architecture timber | House05, HouseWall01, HouseWall04, HouseWall05, HouseWall06, Stair01 | `0c8cee9d` | `6b025039` |
| [Statues01](../Statues01/notes.md) | reviewer / monuments | StoneStatue01, StoneStatue02, StoneStatue03, SteelStatue01, Tomb01, Tomb02 | `d5a59c62` | `2065d4e5` |
| [CartHay01](../CartHay01/notes.md) | groundcover / carts and hay | Carriage01, Carriage02, Carriage03, Carriage04, Straw01, Straw02 | `93ca193e` | `8e6f9ef0` |
| [NormalBindings01](../NormalBindings01/notes.md) | coordinator / normal bindings | Binding-preservation correction; no additional models | `f6998b30`, `3c504580` | `68b72139`, `b30174c2` |
| [Wells01](../Wells01/notes.md) | coordinator / wells | Well01, Well02, Well03, Well04 | `46daddf2`, `0599ac0a` | `8e24f6fb`, `08d2c469` |
| [Architecture03](../Architecture03/notes.md) | fences / architecture final | HouseWall03, Bridge01, BridgeStone01 | `e8fef936`, `29cffdb4` | `3ce1f68b`, `95386cd8` |

## Exact game changes and protected data

**178 game files differ from the reviewed pilot: 94 BMDs and 84 texture containers**, all under `src/bin/Data/Object1/`. See the [complete path list](changed-game-files.txt) and [path/export/SHA-256 manifest](changed-game-files.md). No game files were renamed or added outside the authorized directory.

All other **146** files in the 324-file World1/Object1 baseline remain byte-identical, including all World1 terrain paintings, placement, height, walk data, baked lighting, TerrainLight and alpha strips. Beer01/plate2 and the other pilot props remain unchanged. The bridge shadow strip is also unchanged. Nine excluded fauna/hidden-marker models remain untouched: Bird01, Butterfly01, Fish01, Light01, Light02, Light03, MerchantAnimal01, MerchantAnimal02, PoseBox01.

The [dependency map](dependency-map.json) resolves all 115 BMDs and 105 original texture containers, including exact JPG/OZJ versus TGA/OZT distinctions. Shared-material changes were reviewed on every actual consumer. Latest combined checks report no missing textures, no ownership overlap, preserved mesh-material order and bone/action metadata, matching source/export hashes and exact untouched original archives.

## Evidence and practical limits

- [Combined integration validation](combined-validation.json): PASS: offline integration; no client claim; all 115 models resolve.
- [Authored vertex audit](authored-vertex-audit.json): PASS for 68 remodeled models. Every authored and final-reimport vertex is matched in both directions only within its intended named bone; measured errors are retained.
- [Raw normal-node audit](normal-binding-audit.json): PASS for 97 current changed/retained BMDs, across bind and every animation key. See [readable report](normal-binding-audit.txt).
- [Final combined gallery](final-review.md): six labeled sheets of all 106 actual integrated BMD/material combinations. Each `final-inspection/<model>/provenance.json` records exact game-file hashes; the gallery is an offline diffuse preview, not engine shading or client evidence.
- Per-asset sources, `REF_ORIGINAL`, untouched originals, full converter comparisons, before/after bind bounds, triangle counts, texture dimensions, UV/alpha checks, rig/action proofs, matching-camera renders, wireframes and reduced-scale previews remain in the batch directories linked above. Image-generation prompts, generated masters, editable paintings and reproduction scripts are retained.
- Original inventory validation/imports and all 105 original containers remain under `baseline-validation/`, `inspection/` and `texture-baseline/`. `inventory-sheet-01.jpg` through `inventory-sheet-06.jpg` show the original integration baseline.

Remodeled geometry correctly reports **DIFFERENT** in full-model comparisons; exact skeleton/actions are checked independently. Full-model EQUIVALENT alone does not prove UV or raw normal-node equivalence. The converter merges some shared normals across bones, so raw BMD checks were added. Grass02, Tree12 and Tree13 keep **exact original BMD bytes** with their new paintings; this preserves original behavior, including legacy Tree12/13 normal sharing. No engine fix is claimed. Material-only assets intentionally retain geometry.

Modular connections, openings, footprints and original anchor vertices are preserved. Bone-local animation, hierarchy, mesh-slot effects and original placements are retained. Batch-specific harmless floating-point export differences and original collapsed UV faces are explicitly documented instead of being hidden by an equivalence claim. Source packaging uses the official Blender importer/exporter and supported converter SMD/manifest workflow.

## Remaining work

Production still pending: Cannon01, Cannon02, Cannon03, Hanging01. No unresolved validator rejection remains in accepted batches. Excluded models are out of scope, not unfinished static production.

**Actually verified in client: none.** The user authorized offline production after repeated client crashes. This task did not launch the client or install into shared runtime Data. In a stable serial client session, verify loading, lighting/filtering/alpha, animated poses, mesh-index additive/scroll effects, collision/interaction alignment, modular seams and gameplay readability, then capture genuine matching 1920×1080 before/after views. Blender additive/scroll/fade previews remain approximations.

No engine/CMake, UI, character, monster, equipment, other-map, placement/collision or terrain-data edits. Other task checkouts and their uncommitted work were preserved. Shared handoffs and the board were edited only by the coordinator.

## Reproduce

Run from the integration worktree explicitly. Use the bundled Python with Pillow for texture checks and image assembly, and Blender 5.2.2 for Blender scripts. `validate_integration.py`, `audit_normal_bindings.py`, Blender `audit_authored_vertices.py`, Blender `inspect_final.py`, `assemble_final.py`, `update_board.py` and `write_handoff.py` are the coordinator entry points. The cached gallery rerenders only models whose BMD or exact texture-container hashes changed. Per-batch scripts retain the production reproduction commands and pinned original revisions.
