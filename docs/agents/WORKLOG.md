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
