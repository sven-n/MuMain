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
