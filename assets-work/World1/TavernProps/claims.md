# Tavern furniture ownership — ASTRA — 2026-09-22

Branch: `art/lorencia-tavern-props`, isolated worktree `MuMain-tavern-props`,
based on `main` at `9a8b2027`. Claim recorded before production.

## Exact game-file claim

- `src/bin/Data/Object1/Furniture03.bmd` — rectangular four-legged table, type 142, 5 placements.
- `src/bin/Data/Object1/Furniture04.bmd` — half-round pedestal table, type 143, 3 placements.
- `src/bin/Data/Object1/Furniture05.bmd` — modular rectangular tavern counter, type 144, 3 placements.
- `src/bin/Data/Object1/desk_big.OZJ` — opaque diffuse atlas, material `desk_big.jpg`, shared only by these three Object1 models.

All three consumers are included in this batch. No new game filenames.
Geometry identities checked against original Blender renders and per-mesh bounds,
not inferred from filenames. Current records and source references are saved in inventory.json.

## Ownership audit

- Main checkout: uncommitted `StaticBatch01/README.md` explicitly selects Candle01,
  TreasureChest01 and Tomb03, and defers Furniture03/04/05 as a coordinated furniture pass.
  Its candle, candle2, treasure_chest and tombstone textures are excluded here.
- `art/world1-pilot`: read its World1 and Beer01 handoffs, generation/source scripts and
  original/replacement comparison. Beer01/plate2 and all terrain files are excluded.
  The branch has no active worktree at audit time; references were read through git.
- `MuMain-ui-pilot`, branch `art/ui-pilot`: UI/notes.md claims five Interface/partCharge1
  files. All UI excluded here.
- `.claude/worktrees/inspiring-turing-0f3420`, branch `fix/audio-missing-file-crash`:
  current changes concern audio/CMake/tests; no asset claims or changes.
- Checked active worktrees, statuses and handoff notes before selecting this set.

Rechecked during production: the static batch moved to the managed worktree
`/Users/webproduktion3/.codex/worktrees/world1-static-batch/MuMain` on
`art/world1-pilot`. Its staged/modified Object1 files still cover only Candle01,
TreasureChest01, Tomb03 and their four textures in addition to the committed
Beer01/plate2 pilot. Its furniture deferral is unchanged. No overlap with this claim.

## Preserved boundaries

Only the four claimed game files may change. Deliverables stay below TavernProps,
plus a dated WORKLOG entry. No changes to placements, terrain, TerrainLight, alpha
strips, Beer01, plate2, UI, engine or CMake. No runtime installation or client launch.
Counter connection dimensions and the flat joining edge of Furniture04 are preserved.
All visual evidence will be labeled offline; client acceptance remains pending.
