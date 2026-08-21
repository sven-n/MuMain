# Upstream merge: PR #325 + PR #335 follow-ups

Tracking the partial port of two upstream PRs onto
`cross-platform-sdl-migration-merged`, started 2026-04-28. Companion to
`upstream-merge-features.md` and `pr-329-review-followups.md`.

> **Resume hint for a future session:** read this whole doc, then `git log
> --oneline 33529d81..HEAD` to confirm the five session commits below are
> still on top. The next concrete task is the camera-globals migration in
> §3 (PR #335 phase 2c).

## 1. Branch state at session start

- Branch: `cross-platform-sdl-migration-merged`
- HEAD before session: `33529d81 docs: refresh PR-329 follow-ups doc`
- Merge-base with `origin/main`: `c134d086` (PR #308 merge, 2026-04-25)
- Divergence at start: 56 commits on main, 430 on branch

Two upstream PRs accumulated since merge-base:

| PR | Subject | Merge commit | Status after this session |
|---|---|---|---|
| **#325** | packet-enums (sven-n) | `a8d8611f` | **Fully ported** ✅ |
| **#335** | 3d-camera-rework (Mosch0512) | `f1ffa170` | **Wiring complete + boot crash fixed** ✅ — needs runtime smoke-test (§4.1). NewUIOptionWindow remaining commits (§4.2) still gated on Win32/wzAudio/ApplyResolution prerequisites. |

## 2. Commits added this session (oldest → newest)

```
d1bd1b09  build(dotnet): port packet-enum codegen + 0.9.9 bump from main (PR #325)
e2880fd2  refactor(packets): adopt typed packet enums at call-sites (PR #325)
ebd0704e  refactor(camera): consolidate camera code under src/source/Camera/
7e9dbdba  feat(camera): add modular camera architecture from main (PR #335)
c453fac6  feat(ui,profiler): add NewUIComboBox + FrameProfiler from main (PR #335)
ddba58d3  docs: add resumption notes for partial PR #325 + #335 port (this doc)
197ff67d  fix(build): make Phase 2b camera framework compile on the SDL3 branch
fa71393a  docs: record Phase 2b build-fix shims and the eighth session commit
68120dd0  feat(camera): port LoginScene waypoint offset corrections (PR #335)
d4787dd8  refactor(camera): drop dead legacy CameraState wrapper + 3D-anaglyph globals (PR #335)
b14bf725  refactor(camera): migrate eight small users from legacy globals to g_Camera (PR #335)
56a72c76  refactor(camera): migrate ten more single-ref users to g_Camera (PR #335)
80c808bf  refactor(camera): migrate ZzzObject + MainScene + LoginScene to g_Camera (PR #335)
0bd4bbde  refactor(camera): migrate ZzzLodTerrain to g_Camera (PR #335)
c3a02c0f  refactor(camera): finish g_Camera migration — remove legacy globals (PR #335)
c07a7813  docs: update PR #335 follow-ups — camera-globals migration done
ccde5046  chore(upstream): port .gitignore + edge-of-map terrain fix from main (PR #335)
00f48b68  fix(camera): port two upstream camera bug fixes (PR #335)
231e2f14  docs: record §4.6 trivial cleanups landed + §4.7 already-absorbed inventory
955b71ca  feat(editor): port DevEditor camera tooling from main (PR #335 §4.4)
8069156a  feat(overlay): port per-pass timing + build info to $details (PR #335 §4.3)
fe692ea2  docs: record §4.3 + §4.4 landed; §4.2 NewUIOptionWindow flagged as needing hand-merge
a3259704  fix(ui): port volume + render slider math bugs from PR #335
6ba472c8  docs: §4.2 partially landed — slider bugfixes ported, rest gated on prerequisites
c2779c21  feat(camera): route MoveMainCamera through CameraManager (PR #335 §4.1)
0adfe226  docs: §4.1 OrbitalCamera scene-wiring landed — needs runtime smoke-test
67e39123  fix(camera): kill gluPerspective infinite recursion + localize the GL stubs
```

Phase 1 (`d1bd1b09` + `e2880fd2`) was build-and-run-verified. Everything
from `ebd0704e` through `c3a02c0f` builds clean on macOS-arm64 (Main + all
four test executables link green; only the pre-existing
ld macos-12.0-vs-Homebrew-26.0 warnings remain). Runtime behaviour through
the new `g_Camera` path is **not yet verified** — see §6 for the testable
checkpoint.

### 2.1 Branch-side adjustments to PR #335 imports

These adjustments make upstream's flat-layout assumptions work on the SDL
branch's reorganised tree. None of them are aspirational stubs that need
replacement before declaring the port done — they are simply the correct
shape on this branch. Keeping them here so a future re-merge of `Camera/*`
from main knows what to redo.

**`src/source/Main/stdafx.h`** — adds REFERENCE_WIDTH / REFERENCE_HEIGHT
constants (mirroring `origin/main:src/source/stdafx.h:149`), referenced by
`Camera/CameraProjection.cpp`. Real values, not stubs — keep these.

> Earlier (`197ff67d`) this also held a block of GL function shims —
> `gluPerspective` / `glViewport` forwarders plus no-op
> `glReadPixels` / `glGetFloatv` / immediate-mode draw stubs. The
> `gluPerspective` forwarder caused an infinite-recursion crash (it
> exact-matched the float overload that `gluPerspective2` calls
> internally, while the file-local `gluPerspective(double,…)` got
> shadowed). `67e39123` removed the entire block from stdafx.h and
> moved the genuinely-needed stubs to TU-locals — `Camera/CameraProjection.cpp`
> calls the wrapped names (`gluPerspective2` / `glViewport2`) directly
> and stubs its dead `TestDepthBuffer`/`GetOpenGLMatrix` bodies inline;
> `Camera/DefaultCamera.cpp::HandleEditorMode` computes a Z-rotation
> matrix directly instead of going through the GL matrix stack;
> `UI/Framework/NewUIComboBox.cpp` carries an anonymous-namespace
> set of no-op draw stubs (the widget itself is unreferenced until
> the still-pending §4.2 wiring lands).

**`src/source/Camera/*`** — local include-path patches for SDL-branch
header layout (`_types.h` → `mu_base_types.h`, `_define.h` → `mu_define.h`,
`GameConfig/GameConfig.h` → `GameConfig.h`, `gluPerspective` →
`gluPerspective2`, `glViewport` → `glViewport2`). Keep these patches;
they are not aspirational shims, they are the correct names on this
branch.

**`src/source/Data/GameConfig.{h,cpp}` + `GameConfigConstants.h`** — added
the `Get/SetZoom` API + `[Camera]/Zoom = 1100` ini section/key/default.
This is permanent — `OrbitalCamera` persists its radius via this.

**`src/CMakeLists.txt`** — added
`-Wno-error=return-type-c-linkage` to the Clang relaxation list so
`extern "C" CameraManager& CameraManager_Instance()` doesn't trip
`-Werror`. Aligns with main's tolerance — keep.

## 3. PR #335 camera-globals migration — DONE ✅

**Status: completed in commits `68120dd0` … `c3a02c0f`.** All twelve legacy
camera globals are gone from the codebase, replaced with fields on the new
`g_Camera` instance (`CameraState`, defined in
`src/source/Camera/CameraState.cpp`, declared in `Camera/CameraState.h`
which is now pulled in by `RenderFX/ZzzOpenglUtil.h`). The two sub-sections
below are kept for historical context (the "before" snapshot of the work);
§3.5 documents the actual sequence we landed.

> If you're resuming and just want the next thing to do, skip to §4.

### 3.1 Globals to migrate (~210 references total)

| Old global (branch) | New (`g_Camera.X`) | Where defined on branch |
|---|---|---|
| `CameraDistance` | `g_Camera.Distance` | `Scenes/SceneCore.cpp:106` |
| `CameraDistanceTarget` | `g_Camera.DistanceTarget` | `Scenes/SceneCore.cpp:105` |
| `Camera3DFov` | *removed* | `Scenes/SceneCore.cpp:107` |
| `Camera3DRoll` | *removed* | `Scenes/SceneCore.cpp:108` |
| `g_CameraState` | *removed* (new `g_Camera` replaces it) | `Scenes/SceneCore.cpp:111` |
| `CameraPosition` | `g_Camera.Position` | extern in `RenderFX/ZzzOpenglUtil.h` |
| `CameraAngle` | `g_Camera.Angle` | extern in same file |
| `CameraMatrix` | `g_Camera.Matrix` | extern in same file |
| `CameraFOV` | `g_Camera.FOV` | extern in same file |
| `CameraViewNear` | `g_Camera.ViewNear` | extern in same file |
| `CameraViewFar` | `g_Camera.ViewFar` | extern in same file |
| `CameraTopViewEnable` | `g_Camera.TopViewEnable` | extern in same file |
| `PerspectiveX` | `g_Camera.PerspectiveX` | extern in same file |
| `PerspectiveY` | `g_Camera.PerspectiveY` | extern in same file |
| `g_fCameraCustomDistance` | `g_Camera.CustomDistance` | extern in same file |

Order matters when scripting substitutions — do `CameraDistanceTarget` *before*
`CameraDistance`, `CameraTopViewEnable` *before* `CameraViewFar/Near`, etc.
(longer-prefix-first).

### 3.2 Files to migrate (26)

```
src/source/Gameplay/Characters/ZzzObject.cpp
src/source/Gameplay/Items/PersonalShopTitleImp.cpp
src/source/GameShop/NewUIInGameShop.cpp
src/source/Platform/PlatformGlobalStubs.cpp        ← branch-only stub file
src/source/RenderFX/SideHair.cpp
src/source/RenderFX/ZzzEffectFireLeave.cpp
src/source/RenderFX/ZzzEffectPoint.cpp
src/source/RenderFX/ZzzOpenglUtil.cpp
src/source/RenderFX/ZzzOpenglUtil.h
src/source/Scenes/CharacterScene.cpp
src/source/Scenes/LoginScene.cpp
src/source/Scenes/MainScene.cpp
src/source/Scenes/SceneCommon.cpp
src/source/Scenes/SceneCore.cpp                    ← drop legacy global defs here
src/source/Scenes/SceneManager.cpp
src/source/UI/Events/NewUIGoldBowmanLena.cpp
src/source/UI/Framework/NewUI3DRenderMng.cpp
src/source/UI/Legacy/CharMakeWin.cpp
src/source/UI/Legacy/UIWindows.cpp
src/source/UI/Legacy/ZzzInterface.cpp
src/source/UI/Windows/Commerce/NewUIRegistrationLuckyCoin.cpp
src/source/World/Maps/GM_PK_Field.cpp
src/source/World/Maps/GM3rdChangeUp.cpp
src/source/World/Maps/GMBattleCastle.cpp
src/source/World/Maps/GMDoppelGanger2.cpp
src/source/World/ZzzLodTerrain.cpp
```

Plus the four files in `src/source/Camera/` whose main version is the
target state of the migration (currently still on branch's pre-PR-335
content):

```
src/source/Camera/CameraMove.cpp     ← take main's version (adds ApplyLoginSceneOffset)
src/source/Camera/CameraMove.h       ← take main's version (adds LoginSceneCameraDefaults namespace + extern offsets)
src/source/Camera/CameraUtility.cpp  ← take main's version (heavy gut — most logic moved into CameraManager)
src/source/Camera/CameraUtility.h    ← take main's version (drops legacy CameraState struct, includes new Camera/CameraState.h)
```

### 3.3 Path mapping (branch ↔ main)

The SDL reorg moved many files. Use this when scripting per-file 3-way merges:

```
src/source/Camera/CameraMove.cpp                            ↔ src/source/CameraMove.cpp
src/source/Camera/CameraMove.h                              ↔ src/source/CameraMove.h
src/source/Camera/CameraUtility.cpp                         ↔ src/source/Camera/CameraUtility.cpp   (same)
src/source/Camera/CameraUtility.h                           ↔ src/source/Camera/CameraUtility.h     (same)
src/source/RenderFX/ZzzOpenglUtil.cpp                       ↔ src/source/ZzzOpenglUtil.cpp
src/source/RenderFX/ZzzOpenglUtil.h                         ↔ src/source/ZzzOpenglUtil.h
src/source/RenderFX/SideHair.cpp                            ↔ src/source/SideHair.cpp
src/source/RenderFX/ZzzEffectPoint.cpp                      ↔ src/source/ZzzEffectPoint.cpp
src/source/RenderFX/ZzzEffectFireLeave.cpp                  ↔ src/source/ZzzEffectFireLeave.cpp
src/source/Scenes/MainScene.cpp                             ↔ src/source/Scenes/MainScene.cpp        (same)
src/source/Scenes/LoginScene.cpp                            ↔ src/source/Scenes/LoginScene.cpp       (same)
src/source/Scenes/CharacterScene.cpp                        ↔ src/source/Scenes/CharacterScene.cpp   (same)
src/source/Scenes/LoadingScene.cpp                          ↔ src/source/Scenes/LoadingScene.cpp     (same)
src/source/Scenes/SceneCommon.cpp                           ↔ src/source/Scenes/SceneCommon.cpp      (same)
src/source/Scenes/SceneCore.cpp                             ↔ src/source/Scenes/SceneCore.cpp        (same)
src/source/Scenes/SceneManager.cpp                          ↔ src/source/Scenes/SceneManager.cpp     (same)
src/source/Scenes/WebzenScene.cpp                           ↔ src/source/Scenes/WebzenScene.cpp      (same)
src/source/Gameplay/Characters/ZzzObject.cpp                ↔ src/source/ZzzObject.cpp
src/source/Gameplay/Items/PersonalShopTitleImp.cpp          ↔ src/source/PersonalShopTitleImp.cpp
src/source/GameShop/NewUIInGameShop.cpp                     ↔ src/source/GameShop/NewUIInGameShop.cpp (same)
src/source/World/ZzzLodTerrain.cpp                          ↔ src/source/ZzzLodTerrain.cpp
src/source/World/ZzzLodTerrain.h                            ↔ src/source/ZzzLodTerrain.h
src/source/World/Maps/GM_PK_Field.cpp                       ↔ src/source/GM_PK_Field.cpp
src/source/World/Maps/GM3rdChangeUp.cpp                     ↔ src/source/GM3rdChangeUp.cpp
src/source/World/Maps/GMBattleCastle.cpp                    ↔ src/source/GMBattleCastle.cpp
src/source/World/Maps/GMDoppelGanger2.cpp                   ↔ src/source/GMDoppelGanger2.cpp
src/source/UI/Framework/NewUI3DRenderMng.cpp                ↔ src/source/NewUI3DRenderMng.cpp
src/source/UI/Framework/NewUI3DRenderMng.h                  ↔ src/source/NewUI3DRenderMng.h
src/source/UI/Legacy/CharMakeWin.cpp                        ↔ src/source/CharMakeWin.cpp
src/source/UI/Legacy/UIWindows.cpp                          ↔ src/source/UIWindows.cpp
src/source/UI/Legacy/ZzzInterface.cpp                       ↔ src/source/ZzzInterface.cpp
src/source/UI/Events/NewUIGoldBowmanLena.cpp                ↔ src/source/NewUIGoldBowmanLena.cpp
src/source/UI/Windows/Commerce/NewUIRegistrationLuckyCoin.cpp ↔ src/source/NewUIRegistrationLuckyCoin.cpp
```

### 3.4 Gotchas surfaced during this session

**A. Local-variable shadowing in `CreateFrustrum2D`**
`src/source/World/ZzzLodTerrain.cpp:1969` declares
`float Width = 0.0f, CameraViewFar = 0.0f, CameraViewNear = 0.0f, CameraViewTarget = 0.0f;`
— these are *local* variables that shadow the globals. A naive
`s/CameraViewFar/g_Camera.ViewFar/g` would corrupt the local declaration
(`float g_Camera.ViewFar = 0.0f` is invalid). Either rename the locals first
(main does this in its version of the file) or hand-merge that hunk.

**B. `extern` re-declarations in scenes**
Several scenes have lines like `extern float CameraPosition[3];` near the top
of the .cpp. Those have to be *deleted* (not rewritten) when migrating, since
`g_Camera.Position` is provided through `Camera/CameraState.h` (transitively
included via `ZzzOpenglUtil.h`).

**C. Branch's char16_t / `mu::log::Get(...)` substitutions**
PR #325 phase 1b painstakingly preserved branch-side substitutions:
- `const wchar_t*` → `const char16_t*` (cross-platform string type)
- `g_ConsoleDebug->Write(MCD_SEND, L"...")` → `mu::log::Get("ui")->debug("...")`
- `symLoad(` → `mu::platform::GetSymbol(` (in regenerated `Dotnet/PacketBindings_*.h`)

When taking main's version of any file in §3.2, re-apply these substitutions
afterward via `sed` so we don't regress phase 1b. The Dotnet codegen targets
specifically need the `symLoad` and `wchar_t*` substitutions.

**D. ours-vs-theirs reformatting collisions**
`UI/Legacy/ZzzInterface.cpp` has 3,788 lines of branch-side reformatting +
log-refactor diff vs merge-base. Main's PR-#335 changes to that file are
small but land on lines the branch already touched. 3-way merge produces 13
conflict regions there that are 95% reformatting noise; the mitigation that
worked in phase 1b was: reset the file to branch HEAD, extract main's
*semantic* diff with `git diff -w c134d086..origin/main -- <file>`, and
apply only those substitutions surgically. Same trick applies to
`ZzzLodTerrain.cpp` (18 conflict regions), `ZzzOpenglUtil.cpp` (9), and
`UIWindows.cpp` (7).

**E. `_define.h` was consolidated into `Core/mu_define.h`**
Two of the 22 new Camera files (`Frustum.cpp`, `FrustumRenderer.cpp`) include
`"_define.h"` which doesn't exist on the branch — it was merged into
`Core/mu_define.h` during the SDL reorg. Phase 2b already rewrote those
includes; if any future file from main pulls in `_define.h`, do the same
rewrite.

**F. `g_Camera` visibility through include chains**
The new `g_Camera` is declared in `Camera/CameraState.h`. Files that need it
must reach that header transitively. On main, the canonical path is via
`RenderFX/ZzzOpenglUtil.h` which now `#include "Camera/CameraState.h"`. The
GM* maps and effect files don't directly include ZzzOpenglUtil.h on the
branch — verify each file's include chain when migrating, and add an
explicit `#include "Camera/CameraState.h"` if needed.

### 3.5 What actually landed (commit-by-commit)

The plan mostly held; we sequenced bottom-up (small users first) instead
of top-down (foundation first) so the build stayed green at every step,
which made each commit reviewable in isolation. Build was verified after
every commit with `cmake --build out/build/macos-arm64`.

1. **`68120dd0` LoginScene waypoint offsets** — additive port of main's
   `CameraMove.{cpp,h}` changes (LoginSceneCameraDefaults namespace,
   `g_LoginScene{Offset,Angle}*` runtime globals, `ApplyLoginSceneOffset`
   helper, `extern "C"` instance accessor). One conflict resolved: kept
   branch's two-line formatting of the `CameraVector2 toTarget{...}`
   declaration while inserting main's three new offset calls before it.
2. **`d4787dd8` Drop dead 3D-anaglyph globals + legacy `struct CameraState`** —
   removes `Camera3DFov`, `Camera3DRoll`, `g_CameraState`, and the legacy
   `struct CameraState` from `Camera/CameraUtility.h` (no live callers
   anywhere). `CameraDistance`/`Target` kept here pending §3.5 step 6.
3. **`b14bf725` Migrate eight small users** — 1-2 references each in
   `PersonalShopTitleImp`, `NewUIInGameShop`, `NewUIGoldBowmanLena`,
   `NewUIRegistrationLuckyCoin`, `CharMakeWin`, `SideHair`,
   `ZzzEffectPoint`, `ZzzEffectFireLeave`. The single architectural
   move in this commit was adding `#include "Camera/CameraState.h"` to
   `RenderFX/ZzzOpenglUtil.h` so every TU that already pulls in
   `ZzzOpenglUtil.h` gets `g_Camera` transitively (matches main's layout).
4. **`56a72c76` Migrate ten more single-ref users** — `ZzzInterface.cpp`,
   four `GM*.cpp` (the `CameraPosition[1] + 400.f` cull check),
   `NewUI3DRenderMng`, `UIWindows`, `SceneCommon`, `SceneManager`,
   plus dropping a stale local `extern float CameraViewFar` in
   `CharacterScene.cpp`.
5. **`80c808bf` ZzzObject + MainScene + LoginScene** — 6/8/14 refs each.
   Stale local externs (`extern float CameraAngle[3];` at the top of
   MainScene + three more in LoginScene) deleted along with the migration.
6. **`0bd4bbde` ZzzLodTerrain** — 26 refs across the heaviest user. The
   local-shadow gotcha in `CreateFrustrum2D` (locals named
   `CameraViewFar` / `Near` / `Target` shadow the globals inside a
   178-line block) was handled by renaming those locals to
   `localFar` / `localNear` / `localTarget` first, then bulk-substituting
   only the genuine global references. The two local
   `extern float CameraDistance{,Target};` lines at the top of the
   file dropped at the same time.
7. **`c3a02c0f` Final teardown** — the storage-owning translation units.
   `RenderFX/ZzzOpenglUtil.cpp` (60 substitutions + remove the eleven
   legacy storage definitions), `RenderFX/ZzzOpenglUtil.h` (drop ten
   externs), `Camera/CameraUtility.cpp` (the file we forgot earlier — 59
   substitutions + drop its top-of-file externs), `CameraUtility.h` (drop
   the last two externs), `Scenes/SceneCore.cpp` (drop the two
   `CameraDistance{,Target}` storage definitions),
   `Platform/PlatformGlobalStubs.cpp` (drop the non-Win32
   `g_fCameraCustomDistance` stub). Plus a CMake fix:
   `Camera/CameraState.cpp` moved from MUGame to MURenderFX so the test
   binaries (which link MURenderFX without MUGame) can resolve `g_Camera`.

After step 7, `grep -rE '\b(CameraPosition|CameraAngle|CameraMatrix|...|g_fCameraCustomDistance)\b'` across `src/source` returns only matches inside `g_Camera.X` field accesses or `state.X` parameter
accesses inside `Camera/CameraProjection.cpp`.

**Branch-specific name fixes** (landed in `67e39123`).
`Camera/CameraProjection.cpp` calls `gluPerspective2` / `glViewport2`
directly (the branch's MuRenderer-backed wrappers) instead of upstream's
unwrapped names. Its dead `TestDepthBuffer`/`GetOpenGLMatrix` bodies are
TU-local stubs. `Camera/DefaultCamera.cpp::HandleEditorMode` builds a
direct Z-rotation matrix instead of going through `glPushMatrix /
glLoadIdentity / glRotatef / GetOpenGLMatrix / glPopMatrix`.
`UI/Framework/NewUIComboBox.cpp` carries TU-local no-op draw stubs (the
widget is unreferenced — added by `c453fac6` as a prerequisite for the
still-pending §4.2 resolution-selector wiring).

These are deliberate, not aspirational stubs. Future re-merges of
`Camera/*` from main will need the same name substitutions; everything
else stays byte-identical.

## 4. PR #335 — what's still pending

The big-rock migration (§3) is done. What remains is the polish stack
and the actual *wiring* of the new architecture. None of these block
each other — order is up to the next session.

### 4.1 OrbitalCamera scene-wiring — DONE ✅ (needs runtime smoke-test)

Landed in `c2779c21`. `Camera/CameraUtility.cpp` shrank from 417 lines
to 28 — `MoveMainCamera()` now just handles the F9 mode toggle and
delegates to `CameraManager::Instance().Update()`. The legacy in-file
static helpers (`CalculateCameraViewFar`, `AdjustHeroHeight`,
`CalculateCameraPosition`, `SetCameraAngle`,
`UpdateCustomCameraDistance`, `UpdateCameraDistance`, `SetCameraFOV`,
`HandleEditorMode`) all duplicated logic that already lives in
`Camera/DefaultCamera.cpp` (and `Camera/OrbitalCamera.cpp` for orbital
mode) — kept as dead code only because no scene was actually wired
through CameraManager. With `MoveMainCamera()` now delegating, the
five existing callers (MainScene, CharacterScene, LoginScene, plus the
LoginScene re-entry path) flow through `DefaultCamera::Update()` /
`OrbitalCamera::Update()` automatically.

`03caf434` and `2b806f8d` (drop unused `ICamera*` from RenderObjects /
RenderTerrain) were already absorbed via the SDL reorg / Phase 2b —
the branch's signatures match main's post-cleanup form.

**Smoke-test before declaring this fully shipped:**
1. Login scene tour: camera should sweep through waypoints with
   `LoginSceneCameraDefaults::OFFSET_*` corrections applied
   (`68120dd0`).
2. Character scene: camera should sit at the canonical character-pick
   position (Pos 9758/18913/675, Angle -84.5/0/-75).
3. Main scene: hero-follow camera at default zoom level should look
   right; mouse wheel zooms 0–5.
4. Press F9: camera cycles Default → Orbital. Orbital should orbit on
   right-mouse-drag and zoom on wheel.
5. Per-map ViewFar: visit BattleCastle, Home6thChar, PKField,
   Doppelganger2 — these used to have hardcoded ViewFar overrides
   (3000 / 3220 / 3700 / 3700) that `889eb9f0` deliberately removed.
   The unified zoom-level scaling drives them now — verify they look
   acceptable. If any feels wrong, the right fix is per-scene config
   (`Camera/CameraConfig.h`) rather than reverting to per-map specials.

After `67e39123`, the TU-local stubs replace the global stdafx.h block.
`CameraProjection::TestDepthBuffer` still reports "unoccluded" and
`CameraProjection::GetOpenGLMatrix` returns zero — both methods have
no live callers on this branch (Default/Orbital cameras' main update
loops never call them). When DevEditor exercises them, they should
be ported to `MuRenderer::ReadPixels` / `MatrixStack::Top()`
respectively. `FrustumRenderer` is `#ifdef _EDITOR`-gated so its
no-op draw stubs are dead in the default `ENABLE_EDITOR=OFF` build.


### 4.2 NewUIOptionWindow merge — partially landed; rest needs hand-merge

After per-commit review, only the two slider math bugfixes port cleanly
without dragging in main's flat-layout infrastructure. Those landed in
`a3259704`:

- ✅ `316ef1fa` — drop the `+ 1` offset so the volume slider can reach 0.
- ✅ `ffc3e580` — round-to-nearest on volume + render-level so both
  ends are reachable without pixel-perfect cursor placement.

The remaining §4.2 commits all depend on infrastructure the branch
doesn't have yet — porting any one of them requires implementing the
prerequisite first:

- ❌ `9d7d6b32` "Process resolution combo box before checkboxes/sliders"
  — references `m_ResolutionCombo` (a NewUIComboBox member) that
  doesn't exist on the branch yet. Needs the combo-box integration
  (e8b15d35 prerequisite) first.
- ❌ `9f17a79c` "Drop redundant font/system reinit in ApplyResolution"
  — references an `ApplyResolution()` method that doesn't exist on
  the branch.
- ❌ `a9b5b4dd` "Use consistent windowed-mode style across all entry
  points" — Win32 SDK code (`SetWindowLongPtr`, `WS_OVERLAPPEDWINDOW`,
  etc.). The SDL3 branch routes window-style decisions through
  SDLWindow.cpp; would need an SDL3-flavoured equivalent.
- ❌ `6732c1e9` "Query desktop bit depth for exclusive fullscreen" —
  Win32 `EnumDisplaySettings`. Same as above: branch goes through
  SDL3.
- ❌ `0c5fb402` "Harden option window and config" — multi-file rework
  (NewUIChatInputBox.h, NewUIOptionWindow.cpp +236, NewUISystem.cpp,
  UIControls.{cpp,h}, Winmain.cpp +65). Touches Winmain.cpp (gone on
  branch — Main/MuMain.cpp now).
- ❌ `e8b15d35` "Improve runtime resolution UX" — adds the
  NewUIComboBox-driven resolution selector. Wires NewUIComboBox.{cpp,h}
  (already in `c453fac6`) into the option window. Needs UIMng.cpp
  hooks plus an ApplyResolution method.
- ❌ `e94adadc` "Stabilize window handling and improve options/audio
  UX" — adds music volume slider, splits sound/music volume,
  initializes audio always. Branch has its own
  m_iBGMVolumeLevel / m_iSFXVolumeLevel split (Story 5.4.1) that
  conflicts with main's m_iVolumeLevel / m_iMusicLevel split, and
  uses MuAudio (SDL3) instead of `wzAudio.h`. Needs careful merging
  of the two audio architectures.

The remaining commits are best ported as one focused PR by someone
who can pair them with the branch's existing SDL3 window/audio
plumbing.

### 4.3 $details overlay + per-pass timing — DONE ✅

Landed in `8069156a`. `MainScene.cpp` wraps the major render passes
with `FRAME_PROFILE(...)` scopes; `SceneManager.cpp::RenderDebugInfo`
gains a "Frame ms T:.. O:.. C:.. I:.. E:.." line driven by
`FrameProfiler::AccumulatorMs` plus a "Build: ..." build-info line.
Skipped the `7e5f0d5a` / `a0b7204c` / `efb5a283` add-then-remove
chain since those netted out — we never added the noisy stats so we
don't need to remove them or fix the crash they introduced.

### 4.4 DevEditor port (editor-only build) — DONE (port-wise) ✅, build-blocked ❌

Landed in `955b71ca` — DevEditor source files (DevEditorUI.{cpp,h} +
edits to MuEditorCore.{cpp,h}, MuInputBlockerCore.cpp, MuEditorUI.{cpp,h})
are byte-identical to origin/main and link in cleanly when the gate
is open.

**However, `ENABLE_EDITOR=ON` does not build successfully on this
branch right now.** The editor build path goes through MUData's
loaders (`Data/CommonDataSaver.{cpp,h,inl}`,
`Data/Items/ItemDataLoader.cpp`) which still reference Win32-only
APIs: `WIN32_FIND_DATAW`, `OutputDebugStringW`, `_snwprintf_s`,
`FindClose`, `CompareFileTime`, `FILE_ATTRIBUTE_DIRECTORY`. That's a
pre-existing SDL3-migration gap unrelated to PR #335; the DevEditor
port itself adds no new platform dependencies. The editor build
will start working once those Data/* loaders are ported to
cross-platform filesystem APIs.

### 4.5 Per-map camera-overrides cleanup

Main's `889eb9f0` "Remove per-gameplay-map camera overrides" deletes
the per-map `CameraViewFar = ...` lines from a handful of GM* files
(`GM3rdChangeUp`, `GMBattleCastle`, `GMCrywolf1st`, `GMDoppelGanger2`,
`GMEmpireGuardian1`, `GMHellas`, `GMSwampOfQuiet`, `GM_Kanturu_3rd`,
`GM_Raklion`) so the unified camera config in `OrbitalCamera`'s default
profile drives everything. This is gameplay-visible — verify each map
still looks right before/after.

### 4.6 Trivial cleanups — DONE ✅

Landed in `ccde5046` and `00f48b68`:

- ✅ `.gitignore` additions (`docs/CODING_RULES.md`, `docs/reviews/`).
- ✅ Edge-of-map terrain rendering (`455f8034` — clamp to
  `TERRAIN_SIZE - tileWidth` instead of `TERRAIN_SIZE_MASK - tileWidth`).
- ✅ ScreenCenterYFlip bug fix (`b98d4518` — was `WindowWidth -
  ScreenCenterY`, must be `WindowHeight - ScreenCenterY`).
- ✅ Character aim tracks active camera yaw (`421e9d65` — replace the
  hardcoded ±45 with `g_Camera.Angle[2]` in MoveHero's two screen-to-
  world angle conversions).
- ❌ **Skipped: Macro.txt deletion.** The branch still has six
  SaveMacro / OpenMacro call sites in NewUICustomMessageBox /
  ZzzInterface / CSMapServer / ZzzOpenData that read and write that
  file at runtime, so deleting the data file would break the
  macro-save UX on first launch.

### 4.7 What's NOT pending — already absorbed by Phase 2b

The `7e9dbdba` Phase-2b drop-in took `src/source/Camera/*` byte-identical
from `origin/main`, which captures any PR #335 commit that only touches
files in that directory. By inspection:

```
ancestor of a8d8611f AND only touches Camera/* + CullingConstants.h:
  ae18846e  Restore item cull radius to legacy 400
  d9a95d59  Match culling near plane to projection (20, was 500)
  3456d902  Reverse SetCustom2DHull output to CW order
  6e680004  Increase MAX_HULL_VERTICES to 16
  bf28224b  Use named MAX_HULL_VERTICES for FrustrumX/Y capacity
  92329c0d  Document why OrbitalCamera mount-offset baseline is fixed
  ebbb5357  Make OrbitalCamera use its own MainScene config coherently
  889eb9f0  Remove per-gameplay-map camera overrides
  c9424fa7  Use exact gluPerspective depth formula in TestDepthBuffer
  497cf506  Merge two anonymous namespace blocks in OrbitalCamera.cpp
  b7d37f52  Name magic numbers in DefaultCamera position math
  de08a2af  Apply zoom scale on activation
  054f547f  Smooth hull-bisector expansion at very flat corners
  4e1e6aac  Track aspect ratio in FrustumCache to invalidate on resize
  f79031e2  Make Frustum 2D hull capacity consistent
  f9ca71a7  Remove F8 in-camera free-fly mode (Camera/-only)
  8861041f  Extend side view at wider aspects (CameraConfig.h portion only —
            SceneManager.cpp portion is part of §4.3)
```

These are mentioned for completeness so a future session knows not to
re-do them.

## 5. Reusable scripts from this session

The 3-way merge driver used for phase 1a / 2c-attempt is worth keeping
around. Re-create it as needed:

```bash
# Per-file 3-way merge driver. Pass branch_path<TAB>main_path tuples on stdin.
T=/tmp/3way; mkdir -p "$T"
while IFS=$'\t' read -r branch_path main_path; do
  safe=$(echo "${branch_path}" | tr '/' '_')
  git show "c134d086:${main_path}"     > "$T/${safe}.base"   2>/dev/null \
    || git show "c134d086:${branch_path}" > "$T/${safe}.base" 2>/dev/null
  git show "HEAD:${branch_path}"       > "$T/${safe}.ours"   2>/dev/null
  git show "origin/main:${main_path}"  > "$T/${safe}.theirs" 2>/dev/null
  cp "$T/${safe}.ours" "$T/${safe}.merged"
  if git merge-file -L ours -L base -L theirs \
       "$T/${safe}.merged" "$T/${safe}.base" "$T/${safe}.theirs"; then
    cp "$T/${safe}.merged" "$branch_path"; echo "OK       $branch_path"
  else
    cp "$T/${safe}.merged" "$branch_path"; echo "CONFLICT $branch_path"
  fi
done
```

Use with the path map in §3.3 (one tuple per line) to drive a fresh batch
3-way merge for the camera migration files.

## 6. Open questions surfaced during this session

- **Macro.txt deletion** (PR #335) — should the branch follow? Need to
  check whether the SDL3 work added any consumer of that data file.
- **`Camera3DFov` / `Camera3DRoll`** — main drops these entirely. Confirm
  no branch-side feature still depends on stereoscopic / anaglyph rendering
  before deleting from `Scenes/SceneCore.cpp`. (Branch has 1 reference each
  per the §3.1 table — likely the definition itself only.)
- **`Utilities/` vs `Core/` for `FrameProfiler.h`** — chose `Utilities/` to
  match main. If branch standardizes everything header-only into `Core/`
  later, this can be moved.
- **Network nuget 0.9.8 → 0.9.9 + `net9.0` → `net10.0`** — adopted from
  main in `d1bd1b09`. Verify the nuget package restore still resolves on
  CI; if not, downgrade in the .csproj is a one-line change.
