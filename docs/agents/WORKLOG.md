# Work log

Chronological record of what was done on this fork, by whom (human or AI agent), with the
outcome and the next step. **Every session appends an entry at the end.** Keep entries factual
and short; link to docs instead of repeating them. Dates are ISO (YYYY-MM-DD).

Entry template:

```
## YYYY-MM-DD - <short title> (<agent or person>)
**Goal:** ...
**Done:** ...
**Verified:** ... (commands run, results)
**Open / next:** ...
```

---

## 2026-09-22 - macOS environment, first build, local OpenMU server (Claude Fable 5.1)
**Goal:** Prepare the user's Apple Silicon Mac to build and run this client against OpenMU, as
the base for a custom client with new assets.

**Done:**
- Installed cmake, ninja, pkgconf, glslang, spirv-cross (Homebrew), .NET 10 SDK (`~/.dotnet`,
  PATH block in `~/.zshrc`), Blender 5.2.2 plus the Blender Source Tools add-on.
- Initialized the SDL and imgui submodules; added `upstream` (sven-n/MuMain).
- Configured and built the player client in `out/build/macos-arm64` (preset `macos-arm64`,
  editor OFF, Release) with `OPENSSL_ROOT_DIR` from Homebrew and a libc++ include workaround;
  wrote [`docs/build/macos/console.md`](../build/macos/console.md) and linked it.
- Cloned OpenMU next to this repo (`../OpenMU`) and started it with a localhost-only compose
  file (`deploy/all-in-one/docker-compose.mumain-local.yml`): admin panel `127.0.0.1:8090`,
  connect port 44406 for this client.

**Verified:** 209/209 unit tests pass; the client starts under Metal, loads the Native AOT
network library, connects to the local server and receives the server list (login scene).
Later the same day the owner played a full session on the Mac: server selection, login as
`test0`, character selection, two minutes in the main scene, clean shutdown (no crash report).

**Open / next:** editor build (`ENABLE_EDITOR=ON`) fails on macOS: three Map Editor files use
the Win32 file dialog. The repository data has no `Data/Sound` or `Data/Music` and misses
most of `Data/Object74`. The machine's Command Line Tools carry a stale libc++ header folder
(see the macOS guide); the permanent fix needs `sudo`.

## 2026-09-22 - Asset tooling groundwork: bmdconv, texture tool, Blender scripts (Claude Fable 5.1)
**Goal:** Lay the groundwork for regenerating all graphics with Blender-driven AI.

**Done:**
- `tools/bmdconv/`: command-line converter between BMD and Valve SMD using the engine's own
  parser and writer (`info`, `validate`, `bmd2smd`, `bmd2smd-dir`, `smd2bmd`, `compare`).
  Built by default (`MU_BUILD_ASSET_TOOLS`), tests in `tests/tools/`.
- Engine fixes found on the way: `FixupSMD` grouped triangles wrongly when they shared the
  first texture (sentinel bug); `BMD::Save2` wrote through a fixed 1 MB buffer and a 64-char
  path buffer (overflowed on `Player.bmd`); root-motion arrays were allocated for unlocked
  actions; `TIME_MAX` raised from 100 to 256 frames because a shipped NPC has 105.
- `tools/mu_texture.py`: wrap/unwrap/check for `.OZJ`/`.OZT`/`.OZB`.
- `tools/blender/mu_bmd_import.py` and `mu_bmd_export.py`: BMD to `.blend` and back through
  Blender Source Tools, preserving action order and lock flags.
- Docs: [`docs/asset-pipeline.md`](../asset-pipeline.md),
  [`ASSET_REGENERATION_PLAN.md`](ASSET_REGENERATION_PLAN.md), [`HANDOFF.md`](HANDOFF.md), this log.

**Verified:** `Sword01`, `Monster01` and `Player.bmd` (284 actions) round-trip BMD -> SMD ->
BMD as `EQUIVALENT` in `bmdconv compare`; texture wrap/unwrap is byte-identical on shipped
files; tool tests 5/5 (211 assertions); full suite 214/214. Blender round trip
(`mu_bmd_import.py` -> Blender 5.2.2 + Source Tools 3.4.3 -> `mu_bmd_export.py`) on
`Monster01.bmd`: `EQUIVALENT`, 0 unmatched triangles, max corner deviation 0.0007 units, max
bone deviation 0.0002 units over all 7 actions, bone order and lock flags restored.

**Open / next:** first real asset pass per the plan (Phase 1 textures, World1). Port the three
Map Editor file dialogs to SDL so the editor builds on macOS. Add sound/music data.

## 2026-09-22 - Lorencia tavern furniture batch (ASTRA / Codex)
**Goal:** Rebuild three additional Lorencia tavern props in an isolated worktree, respecting
parallel asset ownership and the original engine contract.

**Done:** Created `art/lorencia-tavern-props` from `main` at `9a8b2027` in the sibling
`MuMain-tavern-props` worktree. Inspected active worktrees/handoffs, all 115 Object1 models,
geometry and World1 placements; claimed Furniture03/04/05 and their exclusively shared
`desk_big.OZJ` before production. Rebuilt the four-legged table, half-round pedestal table
and modular counter with coherent carved oak and restrained iron. Delivered immutable
originals, packed Blender sources with `REF_ORIGINAL` and editable high-poly references,
layered texture sources/prompts, BMD/OZJ exports, offline comparisons and validation reports.
Installed exactly those four files into this worktree's source Data. Batch handoff:
[`assets-work/World1/TavernProps/notes.md`](../../assets-work/World1/TavernProps/notes.md).

**Verified:** 680/324/412 triangles (all below 1,500), one original mesh/bone/action per model,
one frame and lock=0. `bmdconv validate` and every `mu_texture.py check` pass; rig-only
`compare` is EQUIVALENT for all three, with zero local bone translation/rotation deviation.
Bounds and modular joining corners match at SMD precision; full-model DIFFERENT results
are intentional remodeled geometry. Blender source audits preserve original geometry,
UVs/skin/transforms and packed images. Re-imported exports were reviewed from matching
cameras, reduced scale, reverse views, wireframes and original repeated placements.
Final exports have no zero-area UV faces or winding/normal disagreements. Installed hashes
match exports; 320 non-claimed Object1/World1 files remain byte-identical. No engine/CMake,
UI, Beer01/plate2, terrain/placement or shared-runtime changes; no client launch.

**Open / next:** User-authorized offline continuation leaves client acceptance pending.
In a stable coordinated client session, check load logs, runtime lighting/filtering/culling,
table silhouettes and the paired-half-table/counter seams at recorded placements; capture
matched 1920×1080 before/after views. All supplied previews are labeled offline Blender.
