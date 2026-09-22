# Lorencia rebuild consolidated handoff

Integration branch: `art/lorencia-rebuild`. Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rebuild`. This branch is based on `ac0f6dd8`, preserving all completed World1 pilot assets, with no main merge or push.

## Current accepted work

The 17 terrain paintings, Beer01, Candle01, TreasureChest01 and Tomb03 are preserved from the pilot. Furniture03/04/05 and their exclusive shared desk_big atlas were independently reviewed and integrated with `9a10643c`, `527e82d8`, and review `9787b981`. Their source branch/worktree remains unchanged. Groundcover01 (`ea56fc66`, `d67ae0c7`), Fences01 (`185fd5a8`) Scrub01 (`465136d4`) Trees01 (`ac16ffeb`) TimberProps01 (`9a7eedf7`) Rocks01 (`db27198a`) Flowering01 (`2d827ed5`) and independently reviewed Mushrooms01 (`f8928697`) are also accepted offline; Ironwork01 (`37844e87`, independent earlier-batch review `f5c11181`) adds five iron structures; BareTrees01 (`8b853434`) adds four models and a checked conifer bark revision. Masonry01 (`f1fbb29c`) adds six modular stone structures. StoneWalls01 (`149780be`) adds five defensive wall and gate models. Independently reviewed FireProps01 (`d3c18b11`) adds four fire props/entrance models. TavernDetails01 (`d051ee31`) adds six tavern furniture and tableware models; Architecture01 (`8094616d`) adds five buildings/canopy/window structures with HouseEtc02 checked as an unchanged shared-material consumer. SignsBanners01 (`cd7ed604`) adds four notice/banner models; independently reviewed Ship01 (`0e2b794d`, review`16da3679`) adds the preserved animated boat with seven new material paintings. The combined set now contains 77 accepted static models and 136 changed game files beyond the reviewed pilot. The combined validator and ledger cover their exact paths. Offline review accepts visual consistency with the aged timber/metal palette of Beer01; exact bounds, action poses and modular connections pass.

[Asset board](asset-board.md) tracks every candidate. [Dependency map](dependency-map.json) contains all 115 model reports, 105 resolved texture dependencies and exact placements. [Integration ledger](integration-ledger.json) records exact game files and their reviewed export paths. [Combined validation](combined-validation.json) checks the current integrated set, original filenames, complete texture resolution, ownership and protected hashes.

## Inventory evidence

All 106 in-scope baseline BMDs and their actual action files pass `bmdconv validate` (animation mode used for actions). `baseline-validation.json` retains results; `baseline-validation/<model>/` retains exact baseline BMD bytes and extracted model/action SMDs. No original engine-contract rejection remains.

All 106 in-scope models were imported through the official importer and visually inspected, including previously completed pilot assets. The six `inventory-sheet-XX.jpg` images show this integration baseline; they are not client screenshots. `inspection/<model>/` retains packed baseline imports, geometry reports and labeled render filenames. Nine fauna/creature/hidden-marker models remain excluded and unchanged. Precise semantic identities are in `identities.json`; production workers refine details after inspecting bone/mesh data.

All 105 baseline texture containers and decoded artwork are preserved in `texture-baseline/`, with three `texture-sheet-XX.jpg` contact sheets. Baseline inspection reproduction uses these archived textures and BMDs, never subsequently installed replacements. The baseline inventory builder refuses to overwrite an existing dependency map.

## Production and boundaries

CartHay01, Architecture02 and Statues01 are isolated subagent batches; Wells01 is in offline coordinator production. See the board for exact owners and claims. Shared architecture textures stay frozen where geometry ownership is split. No worker may change shared documents, runtime Data or operate the client. Coordinator alone integrates validated commits sequentially.

**Client verification: none in this task.** Prior asset handoffs record client instability and user authorization for offline continuation. No engine or CMake changes, runtime writes, UI work, terrain/lighting/placement/walk-map changes or concurrent client sessions have occurred in this task. All new client visual acceptance remains pending.

## Reproduce combined verification

From this integration worktree:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 assets-work/World1/coordination/validate_integration.py
```

The script rejects any changed game file absent from the accepted integration ledger, unresolved texture, changed protected file, out-of-batch shared texture consumer, or mismatched source/export hash. Per-asset authoritative BMD validation and skeleton/action comparisons remain in each delivered batch.
