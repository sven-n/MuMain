# Main Replay Staged Integration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reapply `cross-platform-sdl-migration-merged` onto current `origin/main` in merge-friendly stages without preserving the branch's noisy commit history.

**Architecture:** Build a fresh branch from `origin/main`, keep current `main`'s directory layout as canonical, and port branch behavior into that layout. Split integration into reviewable stages so rename conflicts, refactor conflicts, dependency additions, feature behavior, and tests are not resolved all at once.

**Tech Stack:** Git, CMake, C++, SDL3, SDL_ttf, miniaudio, shader tooling, .NET packet binding generation, repository CI.

## Global Constraints

- Do not rewrite or delete the existing source branch `cross-platform-sdl-migration-merged`.
- Do not touch unrelated untracked files: `.claude/` and `docs/ground-item-label-sdl3-cache-story-draft.md`.
- Use `origin/main` or `fork/main` at `071a404d` or newer as the replay base, not stale local `main` at `a8d8611f`.
- Use current `main`'s source layout as canonical when both sides renamed the same legacy path differently.
- Enable `rerere` before resolving conflicts so repeated conflict resolutions are remembered.
- Keep generated/cache files out unless they are intentionally required by the build.
- Prefer small commits grouped by integration stage over one huge squash commit.

---

### Task 1: Create the Clean Replay Branch

**Files:**
- Modify: none

**Interfaces:**
- Consumes: current branch `cross-platform-sdl-migration-merged`
- Produces: branch `replay/cross-platform-sdl-migration` based on current `origin/main`

- [ ] **Step 1: Confirm current state**

Run:
```bash
git status --short --branch
git rev-parse --short HEAD
git rev-parse --short origin/main
```

Expected:
```text
Current branch is cross-platform-sdl-migration-merged.
origin/main is 071a404d or newer.
Only known untracked files are .claude/ and docs/ground-item-label-sdl3-cache-story-draft.md.
```

- [ ] **Step 2: Refresh remotes**

Run:
```bash
git fetch --all --prune
```

Expected: fetch completes without errors.

- [ ] **Step 3: Create replay branch**

Run:
```bash
git switch -c replay/cross-platform-sdl-migration origin/main
git config rerere.enabled true
```

Expected:
```text
Switched to a new branch 'replay/cross-platform-sdl-migration'
```

- [ ] **Step 4: Commit branch setup checkpoint**

No commit is needed. Record the base instead:
```bash
git rev-parse --short HEAD
```

Expected: same short SHA as `origin/main`.

---

### Task 2: Stage A - Layout And Rename Baseline

**Files:**
- Modify: paths under `src/source/**`
- Modify: `src/CMakeLists.txt`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: branch final tree from `cross-platform-sdl-migration-merged`
- Produces: source tree that keeps `main`'s canonical directories while preparing space for branch behavior

- [ ] **Step 1: Simulate the full merge and list rename conflicts**

Run:
```bash
git merge-tree origin/main cross-platform-sdl-migration-merged | rg '^CONFLICT \(rename|^CONFLICT \(file location|^CONFLICT \(rename/delete' > /tmp/mu-rename-conflicts.txt
wc -l /tmp/mu-rename-conflicts.txt
```

Expected: conflict count is large, previously around `548` rename/file-location related conflicts.

- [ ] **Step 2: Apply squash merge without committing**

Run:
```bash
git merge -s ort -X find-renames=40% --squash --no-commit cross-platform-sdl-migration-merged
```

Expected: merge stops with conflicts.

- [ ] **Step 3: Resolve directory policy**

Use these target mappings when both sides moved the same old file:
```text
Branch path prefix              Canonical target prefix from main
src/source/Gameplay/            src/source/GameLogic/
src/source/World/               src/source/World/MapInfra/ or src/source/World/GameMaps/
src/source/RenderFX/            src/source/Render/
src/source/Core/                src/source/Core/Utilities/, Core/Globals/, Core/Input/, Core/Math/, or Core/Time/
src/source/Data/                src/source/Data/DataHandler/, Data/GameData/, or Data/Translation/
src/source/Platform/            src/source/App/Platform/
src/source/Resources/Windows/   src/source/App/Platform/Windows/
```

Expected: new branch behavior is moved into current `main`'s layout, not the older replay branch layout.

- [ ] **Step 4: Stage only resolved rename/layout files**

Run:
```bash
git status --short
git add src/source src/CMakeLists.txt CMakeLists.txt
git diff --cached --name-status
```

Expected: staged paths use canonical `main` directories and do not leave duplicate branch-only directories for the same subsystem.

- [ ] **Step 5: Commit layout baseline**

Run:
```bash
git commit -m "chore: align SDL migration with main source layout"
```

Expected: commit succeeds and remaining unstaged files are non-layout integration work.

---

### Task 3: Stage B - Mechanical Refactors

**Files:**
- Modify: C++ include paths under `src/source/**`
- Modify: namespace or type usage affected by moved headers
- Modify: `src/CMakeLists.txt`

**Interfaces:**
- Consumes: canonical directory layout from Task 2
- Produces: compiling source references before adding new dependency behavior

- [ ] **Step 1: Find stale branch-layout includes**

Run:
```bash
rg '#include ".*(Gameplay|RenderFX|Platform|Resources|Core/[A-Za-z_]+\.h|Data/[A-Za-z_]+\.h)' src/source
```

Expected: each result is either intentionally still valid or needs conversion to current `main` layout.

- [ ] **Step 2: Update mechanical include paths**

Examples:
```cpp
#include "GameLogic/Items/ItemManager.h"
#include "Core/Utilities/UsefulDef.h"
#include "Core/Globals/_define.h"
#include "Render/Textures/ZzzTexture.h"
```

Expected: no stale includes reference branch-only layout names when a canonical `main` path exists.

- [ ] **Step 3: Verify no duplicate subsystem files remain**

Run:
```bash
find src/source -type d \( -path '*/Gameplay' -o -path '*/RenderFX' -o -path '*/Resources/Windows' \) -print
```

Expected: no branch-only directories remain unless explicitly justified in the commit message.

- [ ] **Step 4: Commit mechanical refactors**

Run:
```bash
git add src/source src/CMakeLists.txt CMakeLists.txt
git commit -m "refactor: update includes for main layout"
```

Expected: commit succeeds.

---

### Task 4: Stage C - New Libraries And Build Foundation

**Files:**
- Modify: `vcpkg.json`
- Modify: `CMakeLists.txt`
- Modify: `src/CMakeLists.txt`
- Create/modify: `cmake/**`
- Create/modify: `src/dependencies/miniaudio/**`
- Create/modify: `src/ThirdParty/stb/**`
- Create/modify: `.github/workflows/**`

**Interfaces:**
- Consumes: mechanically valid source layout from Task 3
- Produces: dependency and build configuration for SDL3, SDL_ttf, miniaudio, shader tools, and native CI

- [ ] **Step 1: Stage dependency manifest changes**

Run:
```bash
git diff -- vcpkg.json CMakeLists.txt src/CMakeLists.txt cmake .github/workflows
```

Expected: changes are limited to dependency discovery, toolchain setup, CI, and build graph wiring.

- [ ] **Step 2: Stage third-party source additions**

Run:
```bash
git add vcpkg.json CMakeLists.txt src/CMakeLists.txt cmake .github/workflows src/dependencies/miniaudio src/ThirdParty/stb
git diff --cached --stat
```

Expected: miniaudio and stb additions are isolated in this commit, not mixed with gameplay/UI behavior.

- [ ] **Step 3: Commit dependency foundation**

Run:
```bash
git commit -m "build: add SDL migration dependency foundation"
```

Expected: commit succeeds.

---

### Task 5: Stage D - Platform, Input, Audio, And Render Behavior

**Files:**
- Modify: `src/source/App/**`
- Modify: `src/source/Core/Input/**`
- Modify: `src/source/Audio/**`
- Modify: `src/source/Render/**`
- Modify: `src/shaders/**`

**Interfaces:**
- Consumes: dependency/build foundation from Task 4
- Produces: portable runtime behavior for input, audio, rendering, shaders, and OS/platform compatibility

- [ ] **Step 1: Port platform/input changes**

Run:
```bash
git add src/source/App src/source/Core/Input src/source/Input
git commit -m "feat(platform): port SDL input and platform runtime changes"
```

Expected: commit includes platform/input behavior only.

- [ ] **Step 2: Port audio changes**

Run:
```bash
git add src/source/Audio tests/audio
git commit -m "feat(audio): port miniaudio backend and volume fixes"
```

Expected: commit includes audio backend and related tests only.

- [ ] **Step 3: Port render and shader changes**

Run:
```bash
git add src/source/Render src/shaders tests/render
git commit -m "feat(render): port SDL GPU rendering and shader pipeline"
```

Expected: commit includes render/shader behavior and related tests only.

---

### Task 6: Stage E - Data, Network, Camera, UI, And Gameplay Ports

**Files:**
- Modify: `ClientLibrary/**`
- Modify: `src/source/Dotnet/**`
- Modify: `src/source/Data/**`
- Modify: `src/source/Camera/**`
- Modify: `src/source/UI/**`
- Modify: `src/source/GameLogic/**`
- Modify: `src/source/Engine/**`
- Modify: `src/source/World/**`

**Interfaces:**
- Consumes: runtime foundation from Task 5
- Produces: feature-level behavior from the branch replayed into current `main`

- [ ] **Step 1: Port packet enum and .NET binding changes**

Run:
```bash
git add ClientLibrary src/source/Dotnet tests/network
git commit -m "feat(network): port typed packet enum bindings"
```

Expected: commit excludes unrelated UI/render/gameplay files.

- [ ] **Step 2: Port data and config changes**

Run:
```bash
git add src/source/Data src/bin/config.ini src/bin/config.ini.template tests/data
git commit -m "feat(data): port portable data and config handling"
```

Expected: commit includes data/config behavior only.

- [ ] **Step 3: Port camera changes**

Run:
```bash
git add src/source/Camera tests/scenes
git commit -m "feat(camera): port modular camera migration"
```

Expected: commit includes camera architecture and camera-specific fixes.

- [ ] **Step 4: Port UI and gameplay fixes**

Run:
```bash
git add src/source/UI src/source/GameLogic src/source/Engine src/source/World tests/gameplay tests/world
git commit -m "feat(ui): port SDL migration UI and gameplay fixes"
```

Expected: commit includes user-visible gameplay/UI fixes after lower-level runtime support exists.

---

### Task 7: Stage F - Tests, Docs, And Cleanup

**Files:**
- Modify: `tests/**`
- Modify: `docs/**`
- Modify: `README.md`
- Modify: `.gitignore`
- Modify: `.clang-format`
- Modify: `.clang-tidy`
- Modify: `.cppcheck`

**Interfaces:**
- Consumes: fully ported behavior from Task 6
- Produces: reviewable final branch with tests/docs and no build-cache noise

- [ ] **Step 1: Remove generated cache files from the index**

Run:
```bash
git rm --cached --ignore-unmatch Testing/Temporary/CTestCostData.txt tests/build/CMakeCache.txt tests/build/_build/CMakeCache.txt tests/build/Testing/Temporary/CTestCostData.txt
```

Expected: generated CMake/CTest cache files are not part of the final branch.

- [ ] **Step 2: Stage tests and docs**

Run:
```bash
git add tests docs README.md .gitignore .clang-format .clang-tidy .cppcheck
git commit -m "test: add SDL migration coverage and docs"
```

Expected: commit contains tests/docs/style config only.

- [ ] **Step 3: Verify no conflict markers remain**

Run:
```bash
rg '<<<<<<<|=======|>>>>>>>' .
```

Expected: no results.

---

### Task 8: Final Verification

**Files:**
- Modify: none unless verification exposes defects

**Interfaces:**
- Consumes: staged replay branch from Tasks 1-7
- Produces: verified integration candidate

- [ ] **Step 1: Configure**

Run the repo's supported preset for the current machine, for example:
```bash
cmake --preset macos-arm64
```

Expected: configure completes.

- [ ] **Step 2: Build**

Run:
```bash
cmake --build --preset macos-arm64
```

Expected: build completes.

- [ ] **Step 3: Test**

Run:
```bash
ctest --preset macos-arm64 --output-on-failure
```

Expected: tests pass or failures are triaged into follow-up commits.

- [ ] **Step 4: Review final diff**

Run:
```bash
git log --oneline origin/main..HEAD
git diff --stat origin/main..HEAD
git status --short
```

Expected: commits are grouped by stage, diff contains no generated cache files, and working tree is clean except intentionally untracked local files.
