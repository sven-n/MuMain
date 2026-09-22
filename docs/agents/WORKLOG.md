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
