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
## 2026-09-22 - Right HUD material benchmark, offline pilot (ASTRA / Codex)
**Goal:** Establish a dark medieval UI art benchmark while preserving the existing
asset layout, filenames, dimensions, state alignment and engine behavior.

**Done:** Created `art/ui-pilot` from `main` (`9a8b2027`) in the separate
`../MuMain-ui-pilot` worktree. Inventoried 760 Interface images, retained all
untouched payloads and produced 26 labeled contact sheets. Traced active HUD
loads, slices, UVs, scaling, state remapping and alpha behavior read-only. Used
the imagegen skill/built-in tool for five paintings; assembled native-size
OpenRaster sources, PNG masters, JPEG payloads and OZJ exports. Repainted
`Interface/partCharge1/newui_menu03.OZJ` (exposed trim and empty green well) and
`newui_menu_Bt01.OZJ` through `newui_menu_Bt04.OZJ` (Character, Inventory, Friends,
Menu). Installed only those validated source Data files in this worktree.
Prompts, scripts, inventory, exact mappings and review notes are under
[`assets-work/UI/`](../../assets-work/UI/notes.md).

**Verified:** `assemble.py`, `preview.py`, `validate.py --install`; every exported
file checked with `tools/mu_texture.py check`. Exit 0, no rejections; the five
non-power-of-two warnings exactly match the shipped originals (256×51 panel,
30×164 buttons). No resize or atlas change. Protected PNG master pixels are
identical; JPEG maximum per-channel error is 4/255. All 760 original payload
hashes match. Reviewed all four control states at native size and in enlarged
crops, light/dark opacity, and 1920×1080 offline before/after mockups using the
actual HUD geometry. Source Data/export bytes match. No engine, CMake, World1,
Object1 or shared runtime changes; no client stability work.

**Open / next:** Client verification remains pending under the owner's offline
authorization: load errors, actual hover/selected/alert behavior, dynamic text
and skill/gauge overlays, HiDPI, resizing and gameplay readability. The mockups
are explicitly labeled offline reconstructions. Cash-shop and remaining HUD
art are unchanged dependencies, outside this five-file pilot.
## 2026-09-22 - macOS client crashes: miniaudio use-after-free on missing audio files (Claude Fable 5.1)
**Goal:** Find and fix the recurring crashes of the macOS client (ten crash reports on this day:
IOGPU assertion and blit-encoder assertion in `EndFrame()`, `objc_release` of `0x1` on the Metal
completion queue, CFPrefs walking `0x1` from `IMKClient`, NSXPC and AudioComponent crashes).

**Done:**
- Read the ten `.ips` reports: every crash site is an Apple framework object holding a pointer
  that is `0x1` or `0x9` (a `0` or `0x8` incremented by one), on different threads and scenes,
  as early as 12 s after launch. That is heap corruption in the client, not a renderer bug. The
  SDL GPU code in `EndFrame()` and the buffer growth helpers are sound: SDL releases buffers
  and textures deferred, by reference count, once the command buffers that use them complete.
- Built the client with `-fsanitize=address` in a second build directory
  (`out/build/macos-arm64-asan`, config `RelWithDebInfo` with `-O1 -g` so asserts stay on;
  add `-fsanitize-recover=address` and run with `ASAN_OPTIONS=halt_on_error=0` to collect
  every finding in one run). Findings, in the order they appeared:
  1. miniaudio 0.11.25 `ma_resource_manager_data_buffer_node_acquire()` reads the node after
     freeing it when a sound file cannot be opened (every `LoadSound` at startup, no `Data/Sound`).
  2. miniaudio's data-stream load job increments `pDataStream->executionPointer` *after*
     signalling the waiting caller. When the file cannot be opened, the caller frees the stream
     on wake-up, so the job thread writes `+1` into freed memory. The login scene and Lorencia's
     safe zone call `PlayMp3()` every frame and the same-track guard never engages on failure,
     so with no `Data/Music` this ran ~75 times per second (38 000 log lines per session). This
     is the mechanism behind the `0x1` / `0x9` pointers.
  3. `BMD::CreateBoundingBox()` indexes the global `BoundingMin/Max` tables with the vertex bone
     index; `Data/Skill/CW_Bow_Skill.bmd` carries `-8888` on an unreferenced vertex and normal,
     so every launch read and wrote far outside those tables.
  4. `ReceiveOption()` reads the 4-byte `QWERLevel` field one byte past the 32-byte option
     packet OpenMU sends (the client struct is 34 bytes). Read only; left as a follow-up.
- Fixes: `cmake/patches/miniaudio-0.11.25-resource-manager-use-after-free.patch` (applied by
  the existing `ApplyGitPatch.cmake` step; that script now stops git's repository discovery at
  the dependency directory, because a tarball dependency inside the build tree was silently
  skipped before), `MiniAudioBackend` remembers a track that failed to open and skips it until
  a different or enforced request (one log line per track instead of one per frame),
  `BMD::Open2()` clamps out-of-range bone indices to bone 0 and reports the model.
- Docs: macOS guide (missing music behaviour), `HANDOFF.md` (state, symptom table, the stale
  libc++ folder is gone), unit test for the failed-track guard.

**Verified:**
- `ctest` 215/215 (Release), including the new audio test.
- Sanitizer build, before the fixes: report within 2 s of launch (finding 1); after the
  miniaudio patch: finding 3; after all fixes, 200 s run in which the owner logged in and
  played in the main scene: only finding 4, clean exit.
- Release build with `MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1`: 4 min 25 s alive (login,
  character select, main scene), no validation error, no crash report, clean shutdown on
  SIGTERM. Before the fixes the same build died within 16 s to 3 min of the main scene.
- Do not launch the client from a sandboxed tool shell (window server, audio and GPU access);
  the Bash tool needs its sandbox disabled for the run, and `MTL_DEBUG_LAYER_WARNING_MODE=nslog`
  writes gigabytes per minute (sampler descriptor dumps), so keep warnings off.

**Open / next:**
- Follow-up chip: size-check the option packet in `ReceiveOption()` before reading `QWERLevel`.
- miniaudio's other resource-manager jobs (`load_data_buffer_node`, `load_data_buffer`,
  `free_data_buffer_node`) touch their object after signalling as well; the client never
  exercises them (sound effects decode synchronously). Report upstream together with the patch.
- SDL 3.4.8 `METAL_INTERNAL_AcquireSwapchainTexture()` does not check `nextDrawable` for nil;
  upstream main is the same. Revisit only if a render-pass crash appears without heap corruption.
- `[UI] EnableAnimationTaskPool=1` (worker threads for character animation) was not tested.

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

## 2026-09-22 - Publish the tavern furniture batch (ASTRA / Codex)
**Goal:** Commit, push and create a PR for the completed tavern furniture work.

**Done:** Pushed `art/lorencia-tavern-props` and opened
[PR #6](https://github.com/vaskodagamo/MuMain/pull/6) against `main`. Merged the latest
main (`300911ed`) first, preserving every work-log entry when resolving the sole conflict.
Updated the installer to protect other artists' committed Data against HEAD after a main
merge while retaining original-backup and claimed-export hash checks. The PR includes
an offline preview, exact four-file game scope, validation evidence and pending client checks.

**Verified:** Installer preflight passes with all four exports matching recorded hashes and
320 protected files matching committed HEAD. The PR game diff contains only Furniture03,
Furniture04, Furniture05 and desk_big.OZJ; `git diff origin/main...HEAD --check` passes.
No assets were regenerated, runtime files written, client session launched or engine build
performed during publication. Prior offline validation remains applicable.

**Open / next:** PR review and previously deferred client acceptance. The PR is not merged.

## 2026-09-22 - Modern UI revision after visual feedback (ASTRA / Codex)
**Goal:** Make the five-file right-HUD pilot visibly cleaner and more readable after
the first pass failed the user's visual expectations.

**Done:** Created isolated `MuMain-ui-modern` / `art/ui-modern-pilot` from main
`7a88d829`; the earlier pilot PR #4 was already merged. Generated five new painted
sources with the built-in imagegen tool and repainted complete button faces with
dark metal, bold pale symbols and a gold selected-state underline. The user selected
the clean, restrained dark-fantasy direction. Repainted the emerald skill well and
XP trough while retaining the AG/mana backing required by unchanged opaque gauge
fills. Kept filenames, dimensions, atlas boundaries and all interaction geometry.
Retained first-pass payloads/prompts for comparison, updated editable sources and
reproduction scripts, and produced eight offline previews including actual 1080p
control sizes and both existing HUD layout modes. Installed only the same five
validated OZJs into this worktree's source Data. Details:
[`assets-work/UI/notes.md`](../../assets-work/UI/notes.md).

**Verified:** `mu_texture.py check` exits 0 for every export with only the same five
pre-existing NPOT warnings. Verified 760 original payload hashes, unchanged opaque
RGB dimensions, wrappers, protected panel pixels, state ordering and JPEG error
at most 4/255 per channel. Reassembly reproduced all ten master/export hashes;
all five editable ORA composites match their masters. Inspected native, all-state,
1080p and light/dark previews. No engine, CMake, World1, Object1 or shared-runtime
changes; no client launch or engine build.

**Open / next:** Client loading, GPU filtering, localized text, skill/counter
overlays, input/alert states, gameplay readability and HiDPI checks remain pending.
The rest of the HUD is outside this five-file benchmark. All previews are labeled
offline mockups, not client screenshots.

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

## 2026-09-22 - Lorencia static inventory completed offline (ASTRA / Codex)
**Goal:** Complete the coherent dark-medieval Lorencia static art pass through the actual
World1/Object1 inventory, with independent worktrees, exclusive texture ownership and
sequential reviewed integration.

**Done:** All 106 in-scope static models are accepted offline on `art/lorencia-rebuild` in
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rebuild`. This includes
four preserved pilot models and 102 accepted integrations (including prior tavern work),
covering furniture/tableware, buildings and modular walls, gates/fences, vegetation/rocks,
lamps/fire props, notices/banners, cart/hay sets, monuments/graves, boat, wells/pottery,
bridges, cannons and gallows. The three worker agents (groundcover, fences, reviewer) and
coordinator used isolated branches/worktrees with explicit BMD/container ownership; shared
materials were reviewed across all consumers. Final production integration `973ab58d`
completes Cannons01; every source/integration/review commit and exact asset owner is recorded
in the consolidated ledger and board. All production sources, REF_ORIGINAL, originals,
editable paintings, imagegen masters/prompts, reproduction scripts and offline evidence
are retained under assets-work/World1. Six final combined sheets show all 106 actual
integrated model/material combinations with per-model SHA-256 provenance.

**Verified:** Combined validation passes for all 115 inventory BMDs and complete texture
resolution. Exactly 183 source-game files differ from reviewed pilot ac0f6dd8: 98 Object1
BMDs and 85 Object1 texture containers; all 141 other baseline World1/Object1 files are
byte-identical. Original archives and source/export hashes match. Rig names/order/parents,
actions, mesh-material ordering, original anchors and modular openings are preserved.
Remodeled full compares correctly report DIFFERENT; rig/action equivalence is proved
separately. The coordinator's bidirectional named-bone vertex audit passes all 72 remodeled
models; raw normal-node checks pass 101 changed/retained BMDs across bind and all keys.
Grass02/Tree12/Tree13 retain exact original BMD bytes with new paintings after converter
normal-sharing review; pre-existing Tree12/13 behavior is preserved, not declared repaired.
Independent review accepted the batch packages and cross-batch completeness; two final
cannon visual issues (bright atlas gutters and dark muzzle rims) were corrected before
acceptance. Protected completed terrain/Beer01/other pilot files remain unchanged.
No engine/CMake, UI, characters, monsters, equipment, other maps, terrain placement/height/
walk/lighting/alpha data, shared runtime, push or main-merge changes were performed.

**Open / next:** No production asset remains blocked or unfinished within the 106-model
static scope. Nine fauna/hidden-marker models are excluded and unchanged. **Client verified:
none in this task.** Existing client instability and the user's explicit offline authorization
leave loading, shading/filtering/alpha, engine effects, motion, collision/interaction and
1920×1080 before/after captures pending. Do these serially in a stable client session; all
current images are labeled offline Blender evidence. Consolidated handoff, complete owners/
commits, exact changed paths and review gallery:
[coordination/notes.md](../../assets-work/World1/coordination/notes.md).
