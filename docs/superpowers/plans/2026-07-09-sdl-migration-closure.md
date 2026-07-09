# SDL Migration Closure Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the remaining SDL3/SDL_gpu migration gaps on top of main while preserving main's folder restructure and keeping one multiplatform code path.

**Architecture:** Keep main's current app layout for now: `App/Platform/Windows/Winmain.cpp` remains the shared desktop bootstrap until a separate app-layout refactor is explicitly approved. Runtime rendering must use SDL_gpu through `IMuRenderer`; legacy GL-shaped calls are treated as compatibility API and must not require a real OpenGL context or OpenGL libraries in normal client builds.

**Tech Stack:** C++20, CMake, SDL3, SDL_gpu, SDL3_ttf, SDL_mixer, GLM, miniaudio, macOS/Linux/Windows desktop targets.

## Global Constraints

- Main's folder restructure wins; do not recreate the old branch's `src/source/Main` or `src/source/RenderFX` layout.
- Normal runtime must be multiplatform; avoid adding `_WIN32` branches in game, render, UI, audio, and gameplay logic.
- `_WIN32` is acceptable only at platform boundary files, Windows resource files, Windows message hooks, and narrowly scoped compatibility shims.
- Do not keep separate Windows OpenGL and SDL_gpu renderer implementations for normal runtime; that creates divergent behavior and duplicates every rendering fix.
- Keep commits staged by concern: audit/docs, shutdown crash, glow rendering, GL compatibility, texture parity, Windows build cleanup, runtime verification.
- Build verification must include `cmake --build --preset macos-arm64-debug` before each commit that touches renderer or app lifecycle.
- Runtime verification is required before declaring migration closed; compile success is not enough.

---

## File Structure

- `docs/superpowers/plans/2026-07-09-sdl-migration-closure.md`: this executable migration plan.
- `src/source/App/Platform/Windows/Winmain.cpp`: current shared desktop bootstrap and SDL window/renderer lifecycle; do not rename in this plan.
- `src/source/App/stdafx.h`: legacy global include point; must stop depending on real OpenGL headers in normal runtime.
- `src/source/Render/Renderer/MuRenderer.h`: renderer abstraction and SDL_gpu lifecycle declarations.
- `src/source/Render/Renderer/MuRendererSDLGpu.cpp`: active SDL_gpu backend.
- `src/source/Render/Renderer/GLCompatShim.h`: GL-shaped compatibility API declarations/macros.
- `src/source/Render/Renderer/GLCompatShim.cpp`: compatibility routing from legacy `gl*`/`glu*` calls into `IMuRenderer`.
- `src/source/Render/Textures/ZzzOpenglUtil.{h,cpp}`: legacy render utility API; must become SDL_gpu-compatible despite historic names.
- `src/source/Render/Sprites/GlobalBitmap.cpp`: texture upload and dynamic texture registration risk area.
- `src/source/Render/Effects/ZzzEffectJoint.cpp`: item glow/trail rendering risk area.
- `src/source/Render/Effects/ZzzEffectBlurSpark.cpp`, `src/source/Render/Effects/ZzzEffectParticle.cpp`, `src/source/Render/Effects/ZzzEffectMagicSkill.cpp`: effect rendering parity risk areas.
- `src/CMakeLists.txt`, `CMakeLists.txt`: renderer dependency selection, shader assets, SDL_ttf/GLM, and platform link cleanup.
- `docs/sdl-migration-runtime-checklist.md`: create during Task 1 as the manual runtime verification checklist.

---

### Task 1: Baseline SDL Renderer Audit

**Files:**
- Modify: `docs/sdl-migration-runtime-checklist.md`
- Inspect: `src/CMakeLists.txt`
- Inspect: `src/source/App/Platform/Windows/Winmain.cpp`
- Inspect: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Inspect: `src/source/Render/Renderer/GLCompatShim.cpp`

**Interfaces:**
- Consumes: current branch state at `384dabd9 fix(render): activate SDL_gpu compatibility backend`
- Produces: a checklist of confirmed and open SDL migration gaps

- [ ] **Step 1: Record current renderer activation facts**

Run:

```bash
rg -n "MuRendererSDLGpu|MuRenderer\\.cpp|InitSDLGpuRenderer|BeginFrame\\(|EndFrame\\(|SDL_WINDOW_OPENGL|SDL_GL_CreateContext|OpenGL::|opengl32|glu32" src/CMakeLists.txt src/source/App/Platform/Windows/Winmain.cpp src/source/Render/Renderer
```

Expected:
- `MuRendererSDLGpu.cpp` is included in `MuClient`.
- `MuRenderer.cpp` is excluded from `MuClient`.
- `Winmain.cpp` calls `InitSDLGpuRenderer`, `BeginFrame`, and `EndFrame`.
- No `SDL_WINDOW_OPENGL` or `SDL_GL_CreateContext` in active app lifecycle.
- Windows CMake may still show `opengl32`/`glu32`; record as Task 6 scope.

- [ ] **Step 2: Create runtime checklist document**

Create `docs/sdl-migration-runtime-checklist.md` with:

```markdown
# SDL Migration Runtime Checklist

## Build Matrix

- [ ] macOS arm64 debug: `cmake --build --preset macos-arm64-debug`
- [ ] Linux debug preset or documented local Linux command
- [ ] Windows MSVC debug preset
- [ ] Windows MinGW preset, if still supported

## Runtime Scenes

- [ ] Launch app and exit app without macOS crash reporter.
- [ ] Login scene renders background, logo, text, and buttons.
- [ ] Character select renders characters, wings, glow, text, and click targets.
- [ ] Main world renders terrain, sky, water, entities, particles, and UI.
- [ ] Item glow renders correctly for inventory, world drops, shop, and character equipment.
- [ ] Ground item labels render and update without severe FPS collapse.
- [ ] Text input, IME candidate positioning, backspace/delete, paste, and focus transitions work.
- [ ] Music and sound effects start, stop, and do not crash on shutdown.
- [ ] Windowed/fullscreen transitions keep viewport and clicks aligned.

## Known Gaps

- Item glow not yet fixed.
- macOS app exit currently reports a segmentation fault.
- Windows OpenGL link path still exists in CMake.
- Real OpenGL headers are still included before the compatibility shim.
- Some GL-shaped compatibility calls are no-op and need classification.
```

- [ ] **Step 3: Commit**

```bash
git add docs/sdl-migration-runtime-checklist.md docs/superpowers/plans/2026-07-09-sdl-migration-closure.md
git commit -m "docs: add SDL migration closure plan"
```

---

### Task 2: Fix macOS Shutdown Segmentation Fault

**Files:**
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Modify if needed: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Modify if needed: `src/source/Core/Platform/Audio/MiniAudioBackend.cpp`

**Interfaces:**
- Consumes: `mu::ShutdownSDLGpuRenderer()`, `DestroySound()`, SDL window lifetime in `Winmain.cpp`
- Produces: deterministic shutdown order with no macOS crash reporter on quit

- [ ] **Step 1: Capture shutdown stack**

Run the built app from a terminal so macOS crash output and app logs are visible:

```bash
./out/build/macos-arm64/src/Debug/Main
```

Quit from the normal in-game exit path. If macOS reports a crash, collect:
- Crashed thread function names.
- Last app log lines.
- Whether crash occurs before or after `SDL_gpu -- Shutdown complete`.

- [ ] **Step 2: Add temporary shutdown trace points**

In `src/source/App/Platform/Windows/Winmain.cpp`, add short `g_ErrorReport.Write(...)` lines around:

```cpp
DestroySound();
g_MuEditorCore.Shutdown();
KillGLWindow();
DestroyWindow();
cpuUsageRecorder.join();
```

Expected order to verify:

```text
DestroySound begin/end
Editor shutdown begin/end
Renderer/window shutdown begin/end
DestroyWindow begin/end
CPU recorder join begin/end
```

- [ ] **Step 3: Fix owner order**

If crash is in SDL_gpu resources, ensure:
- No draw/text/texture work runs after `Destroy = true`.
- `mu::ShutdownSDLGpuRenderer()` runs before `SDL_DestroyWindow(g_sdlWindow)`.
- SDL_ttf text engine/fonts are released before `SDL_ReleaseGPUDevice`.
- Dynamic texture registries are cleared before GPU device release.

If crash is in audio, ensure:
- `DestroySound()` stops and joins/tears down miniaudio before SDL quit.
- Sound callbacks cannot touch freed game/audio data after teardown starts.

- [ ] **Step 4: Verify**

Run:

```bash
cmake --build --preset macos-arm64-debug
./out/build/macos-arm64/src/Debug/Main
```

Expected:
- Build passes.
- App can be launched and closed three times.
- No macOS crash reporter appears.

- [ ] **Step 5: Remove temporary noisy trace points and commit**

Keep only useful low-noise lifecycle logging.

```bash
git add src/source/App/Platform/Windows/Winmain.cpp src/source/Render/Renderer/MuRendererSDLGpu.cpp src/source/Core/Platform/Audio/MiniAudioBackend.cpp
git commit -m "fix(runtime): stabilize SDL shutdown order"
```

---

### Task 3: Fix Item Glow Rendering

**Files:**
- Inspect/modify: `src/source/Render/Effects/ZzzEffectJoint.cpp`
- Inspect/modify: `src/source/Render/Effects/ZzzEffectBlurSpark.cpp`
- Inspect/modify: `src/source/Render/Effects/ZzzEffectParticle.cpp`
- Inspect/modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Inspect/modify: `src/source/Render/Renderer/GLCompatShim.cpp`
- Inspect/modify: `src/source/Render/Textures/ZzzOpenglUtil.cpp`

**Interfaces:**
- Consumes: `mu::BlendMode`, `GLCompatShim` color/blend/depth/matrix routing
- Produces: item glow parity across inventory, character select, world items, and effects

- [ ] **Step 1: Compare old branch glow changes**

Run:

```bash
git diff origin/main..cross-platform-sdl-migration-merged -- src/source/RenderFX/ZzzEffectJoint.cpp src/source/RenderFX/ZzzOpenglUtil.cpp src/source/RenderFX/MuRendererSDLGpu.cpp | rg -n "glow|Glow|BITMAP_FLARE|Blend|Depth|DepthMask|Alpha|Render|Light|Color" -C 4
```

Expected:
- Identify every glow/depth/blend/color change that was made in the old branch.
- Confirm whether each has a corresponding main-layout change.

- [ ] **Step 2: Instrument blend/depth for glow paths**

In `MuRendererSDLGpu.cpp`, add temporary debug counters for:
- `BlendMode::Glow`
- depth-test off
- depth-write off
- depth-read-only pipeline
- texture ID used by item glow

Log once every 300 frames using existing renderer logger.

- [ ] **Step 3: Fix missing routing**

If item glow still does not render, check these first:
- `GLCompatShim::ApplyBlend(GL_ONE, GL_ONE)` routes to `BlendMode::Glow`.
- `glDepthMask(false)` routes to `SetDepthMask(false)`.
- `glDisable(GL_DEPTH_TEST)` routes to `SetDepthTest(false)`.
- `glColor3f`/`glColor4f` updates per-vertex color before `glBegin`.
- `glBindTexture` uses an SDL_gpu registered texture ID.

- [ ] **Step 4: Runtime verify**

Use `docs/sdl-migration-runtime-checklist.md` and mark:
- Character select item glow.
- Equipped weapon glow.
- Inventory item glow.
- World/drop glow.
- Skill/effect glow.

- [ ] **Step 5: Commit**

```bash
git add src/source/Render/Effects/ZzzEffectJoint.cpp src/source/Render/Effects/ZzzEffectBlurSpark.cpp src/source/Render/Effects/ZzzEffectParticle.cpp src/source/Render/Renderer/MuRendererSDLGpu.cpp src/source/Render/Renderer/GLCompatShim.cpp src/source/Render/Textures/ZzzOpenglUtil.cpp docs/sdl-migration-runtime-checklist.md
git commit -m "fix(render): restore SDL item glow parity"
```

---

### Task 4: Replace Real GL Headers With Local Compatibility Headers

**Files:**
- Create: `src/source/Core/Platform/compat-headers/GL/gl.h`
- Create: `src/source/Core/Platform/compat-headers/GL/glu.h`
- Modify: `src/source/App/stdafx.h`
- Modify: `src/CMakeLists.txt`
- Inspect/modify: `src/source/Render/Terrain/ZzzLodTerrain.cpp`

**Interfaces:**
- Consumes: `GLCompatShim.h`
- Produces: code compiles without real OpenGL/GLEW headers for normal runtime

- [ ] **Step 1: Add local GL header stubs**

`src/source/Core/Platform/compat-headers/GL/gl.h`:

```cpp
#pragma once
#include "Render/Renderer/GLCompatShim.h"
```

`src/source/Core/Platform/compat-headers/GL/glu.h`:

```cpp
#pragma once
#include "Render/Renderer/GLCompatShim.h"
```

- [ ] **Step 2: Add include directory before dependency includes**

In `src/CMakeLists.txt`, add before `${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include`:

```cmake
target_include_directories(MuClient PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/source/Core/Platform/compat-headers"
)
```

Use the repo's existing target include block location; do not create duplicate include ordering.

- [ ] **Step 3: Remove real GL includes from `App/stdafx.h`**

Replace:

```cpp
#include <gl/glew.h>
#include <gl/GL.h>
#include "Render/Renderer/GLCompatShim.h"
```

with:

```cpp
#include "Render/Renderer/GLCompatShim.h"
```

- [ ] **Step 4: Verify no real GL headers are required**

Run:

```bash
rg -n "<gl/|<GL/|<OpenGL/" src/source
cmake --build --preset macos-arm64-debug
```

Expected:
- Any remaining real GL includes are either removed or justified in comments as editor-only.
- Build passes.

- [ ] **Step 5: Commit**

```bash
git add src/source/Core/Platform/compat-headers/GL/gl.h src/source/Core/Platform/compat-headers/GL/glu.h src/source/App/stdafx.h src/CMakeLists.txt src/source/Render/Terrain/ZzzLodTerrain.cpp
git commit -m "refactor(render): replace OpenGL headers with SDL compatibility headers"
```

---

### Task 5: Texture Upload and Dynamic Texture Parity

**Files:**
- Inspect/modify: `src/source/Render/Sprites/GlobalBitmap.cpp`
- Inspect/modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Inspect/modify: `src/source/Render/Renderer/GLCompatShim.cpp`
- Inspect/modify: `src/source/UI/Legacy/UIControls.cpp`
- Inspect/modify: `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`
- Inspect/modify: `src/source/GameLogic/Items/ZzzInventory.cpp`

**Interfaces:**
- Consumes: `RegisterTexture`, `UnregisterTexture`, `QueueTextureUpdate`, `GetRenderer().GetDevice()`
- Produces: every dynamic texture has an SDL_gpu texture and registry entry

- [ ] **Step 1: List all texture mutation sites**

Run:

```bash
rg -n "glGenTextures|glBindTexture|glTexImage2D|glTexSubImage2D|glDeleteTextures|RegisterTexture|UnregisterTexture|QueueTextureUpdate" src/source
```

Expected:
- Every texture mutation site is categorized as static asset, text/dynamic UI, item label/cache, screenshot/readback, or debug-only.

- [ ] **Step 2: Add debug guard for unregistered texture updates**

In `MuRendererSDLGpu.cpp`, when `QueueTextureUpdate` or draw submission receives an unknown texture ID, log:

```text
SDL_gpu -- unknown textureId <id> from <operation>
```

Rate-limit to avoid log spam.

- [ ] **Step 3: Fix dynamic texture registration**

For each dynamic texture path:
- Generate a stable game texture ID.
- Create/register an SDL_gpu texture before first upload.
- Queue updates only after registration.
- Unregister before freeing CPU-side bitmap state.

- [ ] **Step 4: Runtime verify**

Mark in `docs/sdl-migration-runtime-checklist.md`:
- Fonts/text.
- Tooltips.
- Ground labels.
- Inventory icons.
- Shop/item preview.
- Minimap or generated UI textures, if present.

- [ ] **Step 5: Commit**

```bash
git add src/source/Render/Sprites/GlobalBitmap.cpp src/source/Render/Renderer/MuRendererSDLGpu.cpp src/source/Render/Renderer/GLCompatShim.cpp src/source/UI/Legacy/UIControls.cpp src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp src/source/GameLogic/Items/ZzzInventory.cpp docs/sdl-migration-runtime-checklist.md
git commit -m "fix(render): complete SDL texture upload parity"
```

---

### Task 6: Remove Windows OpenGL Runtime Branch

**Files:**
- Modify: `src/CMakeLists.txt`
- Inspect/modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Inspect/modify: `src/source/App/stdafx.h`
- Inspect/modify: editor CMake block if `ENABLE_EDITOR=ON`

**Interfaces:**
- Consumes: SDL_gpu runtime path and GL compatibility headers from Tasks 2-5
- Produces: Windows normal runtime uses the same SDL_gpu path as macOS/Linux

- [ ] **Step 1: Quantify cost of keeping Windows OpenGL**

Document in `docs/sdl-migration-runtime-checklist.md`:

```markdown
## Windows OpenGL Branch Decision

Keeping a Windows OpenGL runtime branch is rejected for normal client builds because it doubles the rendering behavior matrix:

- Every blend/depth/glow fix must be implemented twice.
- Texture upload bugs can differ between Windows and macOS/Linux.
- Click/viewport math can diverge by backend.
- Shutdown/device lifetime bugs become backend-specific.
- Test coverage must multiply by renderer backend and platform.

Windows should use SDL_gpu for normal runtime. Any OpenGL usage must be editor-only or removed.
```

- [ ] **Step 2: Remove normal runtime OpenGL links**

In `src/CMakeLists.txt`, remove `opengl32` and `glu32` from non-editor normal runtime link blocks after confirming no unresolved symbols remain.

Keep editor-specific OpenGL links only inside `if(ENABLE_EDITOR)` if the editor still uses ImGui OpenGL2.

- [ ] **Step 3: Build Windows**

Run on Windows host:

```powershell
cmake --build --preset windows-x64
cmake --build --preset windows-x86
```

Expected:
- Normal client builds link without `opengl32`/`glu32`, unless a third-party/editor-only target still requires them.

- [ ] **Step 4: Commit**

```bash
git add src/CMakeLists.txt docs/sdl-migration-runtime-checklist.md
git commit -m "build(windows): remove OpenGL runtime renderer links"
```

---

### Task 7: Enforce Multiplatform Source Boundaries

**Files:**
- Modify: `docs/sdl-migration-runtime-checklist.md`
- Inspect/modify: all files returned by the scan below

**Interfaces:**
- Consumes: project-wide rule that `_WIN32` is discouraged outside platform boundaries
- Produces: documented ifdef policy and reduced platform branches

- [ ] **Step 1: Scan platform conditionals**

Run:

```bash
rg -n "#\\s*(if|ifdef|ifndef).*(_WIN32|WIN32|__APPLE__|__linux__|_MSC_VER|__MINGW)" src/source
```

Categorize each hit:
- Allowed platform boundary.
- Allowed compatibility shim.
- Candidate for platform abstraction.
- Candidate for deletion.

- [ ] **Step 2: Document allowed boundaries**

Append to `docs/sdl-migration-runtime-checklist.md`:

```markdown
## Platform Conditional Policy

Allowed:
- `src/source/App/Platform/**`
- `src/source/Core/Platform/**`
- Windows resource files and native message hook code
- Small compatibility headers that expose a platform-neutral interface

Discouraged:
- Render logic
- Gameplay logic
- UI logic
- Audio logic outside `Core/Platform/Audio`
- Network/game protocol logic

Rule: prefer one source path calling a platform-neutral function over `_WIN32` branches in feature code.
```

- [ ] **Step 3: Remove low-risk conditionals**

For every conditional that only selects includes or no-op behavior, move it behind a compatibility header/function rather than branching in feature code.

- [ ] **Step 4: Commit**

```bash
git add docs/sdl-migration-runtime-checklist.md src/source
git commit -m "refactor(platform): document and reduce platform conditionals"
```

---

### Task 8: Runtime Verification and Closeout

**Files:**
- Modify: `docs/sdl-migration-runtime-checklist.md`
- Modify if needed: code files touched by fixes discovered during verification

**Interfaces:**
- Consumes: completed Tasks 1-7
- Produces: evidence-backed closeout that SDL migration is merge-ready

- [ ] **Step 1: Build all available local targets**

Run locally on macOS:

```bash
cmake --build --preset macos-arm64-debug
```

Run on Windows/Linux hosts or CI:

```bash
cmake --build --preset windows-x64
cmake --build --preset windows-x86
```

Use the project’s Linux preset or documented Linux command if available.

- [ ] **Step 2: Execute runtime checklist**

Run the app and mark every checkbox in `docs/sdl-migration-runtime-checklist.md`.

Do not mark migration closed with unchecked items in:
- shutdown crash
- item glow
- texture upload
- text input
- audio
- main scene rendering
- Windows SDL_gpu build

- [ ] **Step 3: Add closeout note**

Append:

```markdown
## Closeout Evidence

- macOS build command and result:
- Linux build command and result:
- Windows build command and result:
- Runtime scenes verified:
- Remaining accepted risks:
```

- [ ] **Step 4: Commit**

```bash
git add docs/sdl-migration-runtime-checklist.md
git commit -m "docs: record SDL migration verification evidence"
```

---

## Self-Review

- Spec coverage: item glow, macOS shutdown crash, Windows OpenGL branch cost, multiplatform source discipline, and `_WIN32` discouragement are each covered by explicit tasks.
- Placeholder scan: no task contains open-ended "TBD" instructions; each task has paths, commands, and expected outcomes.
- Type consistency: renderer references use existing names `mu::GetRenderer`, `InitSDLGpuRenderer`, `ShutdownSDLGpuRenderer`, `QueueTextureUpdate`, `RegisterTexture`, and `UnregisterTexture`.
