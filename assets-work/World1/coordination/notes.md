# Lorencia rebuild consolidated handoff

Integration branch: `art/lorencia-rebuild`. Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rebuild`. This branch is based on `ac0f6dd8`, preserving all completed World1 pilot assets, with no main merge or push.

## Current accepted work

The 17 terrain paintings, Beer01, Candle01, TreasureChest01 and Tomb03 are preserved from the pilot. Furniture03/04/05 and their exclusive shared desk_big atlas were independently reviewed and integrated with `9a10643c`, `527e82d8`, and review `9787b981`. Their source branch/worktree remains unchanged. Offline review accepts visual consistency with the aged timber/metal palette of Beer01; exact bounds, action poses and modular connections pass.

[Asset board](asset-board.md) tracks every candidate. [Dependency map](dependency-map.json) contains all 115 model reports, 105 resolved texture dependencies and exact placements. [Integration ledger](integration-ledger.json) records exact game files and their reviewed export paths. [Combined validation](combined-validation.json) checks the current integrated set, original filenames, complete texture resolution, ownership and protected hashes.

## Inventory evidence

All 106 in-scope models were imported through the official importer and visually inspected, including previously completed pilot assets. The six `inventory-sheet-XX.jpg` images show this integration baseline; they are not client screenshots. `inspection/<model>/` retains packed baseline imports, geometry reports and labeled render filenames. Nine fauna/creature/hidden-marker models remain excluded and unchanged. Precise semantic identities are in `identities.json`; production workers refine details after inspecting bone/mesh data.

## Production and boundaries

Groundcover01, Fences01 and Scrub01 are active isolated subagent batches; see the board for exact owners/claims. Shared architecture textures stay frozen where geometry ownership is split. No worker may change shared documents, runtime Data or operate the client. Coordinator alone integrates validated commits sequentially.

**Client verification: none in this task.** Prior asset handoffs record client instability and user authorization for offline continuation. No engine or CMake changes, runtime writes, UI work, terrain/lighting/placement/walk-map changes or concurrent client sessions have occurred in this task. All new client visual acceptance remains pending.

## Reproduce combined verification

From this integration worktree:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 assets-work/World1/coordination/validate_integration.py
```

The script rejects any changed game file absent from the accepted integration ledger, unresolved texture, changed protected file, out-of-batch shared texture consumer, or mismatched source/export hash. Per-asset authoritative BMD validation and skeleton/action comparisons remain in each delivered batch.
