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

## 2026-09-22 - World1 pilot, offline art and validation (ASTRA / Codex)
**Goal:** Establish the Lorencia terrain benchmark and exercise one static prop through the
Blender/BMD pipeline on `art/world1-pilot`, branched from main at `9a8b2027`.

**Done:** Inventoried and unwrapped all 32 World1 texture containers. Repainted all 17 base
filenames explicitly listed in the brief (14 loaded terrain slots plus three legacy variants)
at 512×512. Rebuilt Beer01 as the tavern still life it actually contains, after user
confirmation: bottle, mug, bowls, grapes and vine. Exported 784 triangles with an exclusive
512×512 plate2 atlas, retaining all five bones and the original one-frame action. Preserved
original geometry, packed Blender sources, higher-resolution source geometry, raw paintings,
scripts, comparison renders and validation reports under
[`assets-work/World1/`](../../assets-work/World1/notes.md). Installed the 19 exported files in
source and runtime Data folders. No engine/CMake files or filenames changed.

**Verified:** All 18 replacement texture containers pass mu_texture checks; decoded terrain
JPEG edge mismatch averages at most 0.424/255 (maximum single-channel delta 5/255). Reviewed
3×3 repeats offline. Reference and action SMDs pass bmdconv validation. Full-model compare
reports DIFFERENT for the intended geometry change (216 → 784 triangles; 5 → 1 meshes),
with zero bone-motion deviation. Isolated original/replacement skeletons and actual actions
compare EQUIVALENT. Bind sizes: 91.20×58.57×61.80 → 91.19×58.48×61.80 units. All 35 protected
World1 files, including TerrainLight and alpha strips, match their baseline hashes. All 19
installed file hashes match the exports in both source and runtime.

**Open / next:** **Not verified in client.** The unchanged client repeatedly crashed in
server selection/Lorencia, including Metal command-buffer assertions, before the three
1920×1080 baseline captures were completed. The user explicitly authorized offline
continuation and deferred client verification. Resolve stability separately, obtain baseline
town/grass/rock screenshots, compare replacements at matching views and inspect Beer01 near
Lorencia `(127.4,128.4)`. The delivered Blender comparison is labeled offline. The visual
benchmark and end-to-end proof remain pending client acceptance.


## 2026-09-22 - Three Lorencia static props, offline continuation (ASTRA / Codex)
**Goal:** Continue `art/world1-pilot` with three visually identified props, preserving engine
contracts and completed terrain/Beer01. Client stability explicitly outside scope.

**Done:** Inventoried all 115 Object1 BMDs, imported/rendered eight candidates and decoded
World1 placements read-only. Selected Candle01 (three-candle stand, 6 placements),
TreasureChest01 (arched timber chest, 3 placements), and Tomb03 (upright grave marker,
5 placements). Rebuilt them at 1,018 / 1,174 / 200 triangles from 116 / 66 / 30. Repainted
exclusive candle, treasure_chest and tombstone atlases at 512×512, plus candle2 at 128×128.
Retained all original game filenames and mesh slots. Preserved originals, packed Blender
sources with REF_ORIGINAL and excluded higher-resolution sources, paintings/prompts,
exports, raw validation data and labeled offline before/after, wireframe, action and scale
reviews under [`assets-work/World1/`](../../assets-work/World1/notes.md).

**Verified:** Reference/action SMDs and all four textures pass engine/loader validation.
All three isolated skeleton/action comparisons EQUIVALENT; names, order, parents, action
order, lock=0 and 7/7/1 frame counts unchanged. Local translations/rotations checked over
all keys (largest component differences 0.000015 units / 0.0000003072 radians modulo 2π).
Candle's six original flame triangles and UVs separately verified; original BlendMesh=1
material order retained. Intentional full-model comparisons DIFFERENT. Bind sizes remain
38.67×22.47×87.06, 114.57×64.32×84.04 and 89.76×19.64×117.25 units; chest keyhole adds only
0.0036 units of front projection. Packed sources reopened, final BMDs re-imported for review.
Seven replacements installed with matching hashes in the art source and existing runtime;
317 other World1/Object1 files remain unchanged in each, including all World1 terrain,
TerrainLight/alpha strips and Beer01. No engine/CMake edits.

**Workspace coordination:** Another process switched the shared primary checkout to main
during validation. The installation guard refused the changed baseline before writing.
Continued on the existing art/world1-pilot branch in the isolated worktree
`/Users/webproduktion3/.codex/worktrees/world1-static-batch/MuMain`; no commits or replacement
files written to main. The existing runtime is still the primary checkout's macOS app.

**Open / next:** **Not verified in client.** User-authorized offline continuation remains.
Check real client loading/logs, candle additive blending/flicker, lighting, gameplay zoom,
object contacts/orientation and matched screenshots once stability is addressed separately.
Useful review tiles: Candle01 `(126.58,128.25)`, chest `(185.17,140.06)`, Tomb03 `(130.50,215.00)`.
The delivered renders are Blender evidence, not client screenshots or visual acceptance.

## 2026-09-22 - Publish the World1 pilot for review (ASTRA / Codex)
**Goal:** Commit, push and open a PR for the completed art branch.

**Done:** Confirmed the asset work was already committed and the art worktree clean. Pushed
`art/world1-pilot` to origin and opened [PR #5](https://github.com/vaskodagamo/MuMain/pull/5)
against `main`, covering the complete branch: 17 terrain repaints and Beer01, Candle01,
TreasureChest01 and Tomb03, including sources, paintings, previews and validation evidence.
Recorded this publication in a separate documentation commit.

**Verified:** Refreshed origin/main, reviewed the branch scope (26 replaced game files,
World1/Object1 plus asset-work/handoff files), and passed `git diff --check`. The PR records
previously completed offline validation and explicitly labels all preview images as Blender
renders. No asset changes or new engine build were made during publication.

**Open / next:** PR review and the previously deferred client loading, lighting/blending,
placement and screenshot checks. Client verification is still pending; no merge requested.
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

## 2026-09-22 - Lorencia rebuild coordination and first integrations (ASTRA / Codex)
**Goal:** Rebuild the placed static environment in independent worktrees while preserving
completed terrain/Beer01 and all gameplay-bound data.

**Done:** Created `art/lorencia-rebuild` in the sibling `MuMain-lorencia-rebuild` worktree
from reviewed World1 pilot `ac0f6dd8`. Inspected all task/worktree ownership and existing
handoffs, mapped 115 BMDs/105 texture dependencies with exact World1 placements, and imported
and visually identified all 106 in-scope assets. Added coordinator-only
[`asset-board.md`](../../assets-work/World1/coordination/asset-board.md), immutable baseline
inspection scenes, six review sheets, dependency map and integration ledger. Independent
review accepted prior Furniture03/04/05; integrated their original and production commits.
Parallel subagents completed Grass01/02/05/06 surface paintings and Fence01–04 remodels;
coordinator reviewed actual export images and integrated each validated batch sequentially.
Trees, tall scrub, barrel/crate/lantern and the next shared rock group are assigned separately.

**Verified:** Tavern joins and original rig/actions independently pass; groundcover full-model
comparisons are EQUIVALENT; fences intentionally DIFFERENT for geometry with EQUIVALENT
rig/actions. Coordinator rechecked installed groundcover BMDs. Independent cross-review
accepted fence visuals and actual-placement joining sheets. Combined model scan resolves all
textures, installed hashes match exports and untouched original archives match pilot history.
Fifteen game files differ from the integration baseline; all other 309 World1/Object1 files,
including all terrain/light/placement/alpha data and completed pilot assets, are unchanged.
No runtime installation, client launch, engine/CMake/UI edit, push or main merge.

**Open / next:** Production continues through the board. These are offline acceptances only;
no asset was observed in client by this task. All client loading, shading/filtering, motion,
placement and matched 1920×1080 capture checks remain pending. Consolidated handoff:
[`coordination/notes.md`](../../assets-work/World1/coordination/notes.md).
