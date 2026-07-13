# Cross-Platform SDL Port Ledger Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Audit and finish porting every relevant cross-platform SDL, macOS, audio, input, rendering, texture, model, and lifecycle bug fix from `cross-platform-sdl-migration` onto `main`'s restructured tree, with `main`'s folder layout taking precedence.

**Architecture:** Treat the old branch as a patch source and the current branch as the integration target. Each source commit gets mapped from old paths to current paths, assigned one ledger status, and verified by semantic code inspection plus build/runtime checks where the change affects behavior.

**Tech Stack:** C++/CMake/Ninja, SDL3, SDL GPU renderer, SDL audio stack, OpenGL compatibility shim, macOS arm64 debug preset.

## Global Constraints

- `main`'s folder restructure wins over old branch paths.
- The old branch's bug-fix intent wins over accidental omissions in this replay branch.
- Avoid new `#ifdef _WIN32` or platform-specific implementation forks unless the platform API boundary requires one.
- Prefer single-source cross-platform code paths for SDL, audio, rendering, input, and shutdown.
- Do not delete the Windows OpenGL compatibility branch until its cost and remaining call sites are documented.
- Every relevant source-branch commit must end with one ledger status: `ported`, `already-present`, `superseded`, `not-applicable`, or `needs-port`.
- Every `needs-port` entry must include the current-file target path and the verification required after porting.
- Each implementation stage must build with `cmake --build --preset macos-arm64-debug`.
- Runtime verification must cover loading splash, login/title scene, textured 2D UI, 3D models, item/effect glow, click/input behavior, and clean game close on macOS.

## Source And Target

- Source branch: `cross-platform-sdl-migration`
- Target branch: `replay/cross-platform-sdl-migration`
- Shared base observed: `d98b7bf26228cabd1fbb711ae43f0dd02ad77c4a`
- Initial source commits touching `src/source`: `269`
- Ledger file: `docs/porting/cross-platform-sdl-ledger.md`

## Stage 1: Build The Commit Ledger

- [ ] Generate the source commit list from the shared base:

  ```bash
  base=$(git merge-base main cross-platform-sdl-migration 2>/dev/null || git merge-base origin/main cross-platform-sdl-migration)
  git log --reverse --format='%H%x09%h%x09%s' "$base..cross-platform-sdl-migration" -- src/source
  ```

- [ ] Create `docs/porting/cross-platform-sdl-ledger.md` with one row per relevant commit.
- [ ] Exclude only commits that are strictly documentation, generated workflow bookkeeping, or branch metadata; keep any commit that changed runtime code, build files, platform abstractions, tests, or data loading behavior.
- [ ] For each included commit, record:
  - source hash
  - source subject
  - subsystem
  - old touched paths
  - current target paths
  - status
  - evidence command or file reference
  - verification required
- [ ] Use semantic inspection, not path-only diff matching, because `main` restructured folders.

## Stage 2: Renderer Core And Frame Lifecycle

- [ ] Audit and port renderer state, frame lifecycle, and compatibility shim commits.
- [ ] Required current files:
  - `src/source/Render/Renderer/MuRenderer.h`
  - `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
  - `src/source/Render/Renderer/GLCompatShim.cpp`
  - `src/source/Scenes/LoadingScene.cpp`
  - `src/source/Scenes/WebzenScene.cpp`
  - `src/source/UI/Legacy/UIMng.cpp`
- [ ] Confirm or port these known source fixes:
  - `0c3c55b6 fix(renderer): deferred draw commands eliminate 1-frame vertex delay causing streaks`
  - `0e104e93 fix(renderer): distinguish disabled texture from stale binding in ResolveTextureId`
  - `2b7f2612 fix(renderer): skip stencil-dependent draws`
  - `d9f516ed fix(renderer): skip draw calls when color writes disabled`
  - `8a967f11` vertex buffer and 2D depth behavior changes
  - `5f9e37a7` 3D/2D depth compare behavior changes
  - loading/title scene frame ownership and swap removal fixes
- [ ] Verify loading screen renders as image plus loading bar without white full-screen clears or stale frame contents.

## Stage 3: Textures, Bitmap Uploads, And 2D UI

- [ ] Audit and port texture upload, logical texture ID, bitmap registry, and 2D quad commits.
- [ ] Required current files:
  - `src/source/Render/Sprites/GlobalBitmap.cpp`
  - `src/source/Render/Sprites/GlobalBitmap.h`
  - `src/source/Render/Textures/ZzzOpenglUtil.cpp`
  - `src/source/UI/Legacy/UIControls.cpp`
  - `src/source/Engine/Object/ZzzInventory.cpp`
- [ ] Confirm or port direct SDL texture upload fixes for JPEG/TGA and RGB-to-RGBA conversion.
- [ ] Confirm dynamic bitmap updates bind logical bitmap IDs, not stale backend texture handles.
- [ ] Confirm `RenderSprite` and `RenderSpriteUV` pass actual texture IDs and clip sprites behind the camera.
- [ ] Verify UI textures, logo, text, loading bar, inventory textures, and click targets render at stable positions.

## Stage 4: Models, Effects, Glow, And Immediate-Mode Replacements

- [ ] Audit and port model/effect rendering commits.
- [ ] Required current files:
  - `src/source/Render/Models/ZzzBMD.cpp`
  - `src/source/Render/Effects/ZzzEffectJoint.cpp`
  - `src/source/Render/Effects/ZzzEffectBlurSpark.cpp`
  - `src/source/Render/Effects/ZzzEffectParticle.cpp`
  - `src/source/Render/Effects/ZzzEffectSprite.cpp`
  - `src/source/Render/Textures/ZzzOpenglUtil.cpp`
- [ ] Confirm or port these known source fixes:
  - `4dd90930 fix(render): port model glow state handling`
  - `c6337e6d fix(render): restore GL-style projection handedness/depth mapping`
  - `e4fd48b1 fix(render): restore glow trail depth and texture behavior`
  - `5dd1609f fix(renderer): pass explicit texture ID for effect joint quads`
  - `de7dfd5a fix(renderer): clip sprites behind camera`
  - `684ce989 fix(renderer): pass actual texture ID in RenderSprite/RenderSpriteUV`
- [ ] Verify 3D models no longer streak, glow effects match old branch intent, and additive/alpha/depth state resets do not leak between effects and UI.

## Stage 5: SDL Window, Input, Main, And Click Behavior

- [ ] Audit and port SDL window/event loop/input/main commits.
- [ ] Required current files:
  - `src/source/Main.cpp`
  - `src/source/Platform/*`
  - `src/source/Input/*`
  - `src/source/Scenes/SceneManager.cpp`
  - any current file replacing old WinMain or Windows message handling
- [ ] Confirm one cross-platform `main` entry point remains sufficient.
- [ ] Confirm mouse stuck-state, text input, focus/display, cursor, and click-position fixes are ported.
- [ ] Remove or document any remaining Windows-specific conditional behavior.
- [ ] Verify click behavior on macOS in the loading/title/login flow.

## Stage 6: macOS, Audio, SDL Libraries, And Shutdown

- [ ] Audit and port macOS compatibility, SDL library, audio, and shutdown commits.
- [ ] Required current files:
  - audio backend/source files under `src/source`
  - CMake dependency declarations
  - platform/window lifecycle files
  - shutdown/destructor paths reached by `./ctl run`
- [ ] Confirm SDL audio and Vorbis/miniaudio decisions are represented in current build files.
- [ ] Confirm close-game segmentation fault is fixed at root cause, not hidden by ordering changes.
- [ ] Verify closing the macOS app does not report a segmentation fault.

## Stage 7: Verification, Cleanup, And Merge Readiness

- [ ] Build:

  ```bash
  cmake --build --preset macos-arm64-debug
  ```

- [ ] Run:

  ```bash
  ./ctl run
  ```

- [ ] Capture manual visual checkpoints:
  - loading splash image and loading bar
  - login/title scene textures
  - 3D model geometry
  - item glow/effect glow
  - text overlays only when intentionally enabled
  - click/input behavior
  - clean app close
- [ ] Remove temporary render diagnostics that are not intended to stay.
- [ ] Update ledger statuses after every stage.
- [ ] Commit each stage separately after it builds and the ledger records evidence.
- [ ] Only call the branch merge-friendly after the ledger has no `needs-port` rows and the runtime checklist passes.
