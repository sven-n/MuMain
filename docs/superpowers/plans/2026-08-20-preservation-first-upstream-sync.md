# Preservation-First Upstream Sync Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Integrate upstream `fork/main` through `c0d74c4e` into downstream `main`, preserve downstream defaults, retain every incoming capability, and leave a validated ancestry-preserving integration branch ready for publication.

**Architecture:** Start from the pinned downstream `main`, work in an isolated Git worktree, then merge eleven chronological upstream mainline checkpoints. Resolve each conflict by keeping downstream structure and porting the upstream behavioral delta. Track every upstream non-merge commit in a preservation ledger; publish only after ancestry, build, test, and quality gates pass.

**Tech Stack:** Git merge/rerere/worktrees, CMake presets, C++20, Catch2/CTest, worktree-local quality gates, Markdown audit ledger.

---

### Task 1: Freeze Scope And Create Rollback Boundary

**Files:**
- Read: `../docs/upstream-sync.md`
- Read: `docs/superpowers/specs/2026-08-20-preservation-first-upstream-sync-design.md`
- Modify: none

- [x] **Step 1: Confirm the planning branch is clean**

Run:

```bash
git status --short --branch
git rev-parse main
git rev-parse origin/main
```

Expected: clean worktree; local and remote-tracking `main` both resolve to `db2d3c7a94b6be43240ac37f6809104ddd078ea2`.

- [x] **Step 2: Refresh remote-tracking references**

Run:

```bash
git fetch --prune fork
git fetch --prune origin
```

Expected: both fetches exit 0. The integration scope remains pinned to `c0d74c4ed5edfad527b71899337f72d39bfc49f3` even if `fork/main` advances.

- [x] **Step 3: Verify pinned objects and ancestry**

Run:

```bash
git cat-file -e db2d3c7a94b6be43240ac37f6809104ddd078ea2^{commit}
git cat-file -e c0d74c4ed5edfad527b71899337f72d39bfc49f3^{commit}
git merge-base db2d3c7a94b6be43240ac37f6809104ddd078ea2 c0d74c4ed5edfad527b71899337f72d39bfc49f3
```

Expected merge base: `685936c10a8ee50982d0e1ca8d9430bf88186dd1`.

- [x] **Step 4: Create the rollback branch**

Run:

```bash
git branch rollback/pre-upstream-merge-2026-08-20 db2d3c7a94b6be43240ac37f6809104ddd078ea2
```

Expected: branch created. If it already exists, verify it resolves to the pinned downstream SHA before proceeding.

### Task 2: Create Isolated Integration Worktree And Baseline

**Files:**
- Create worktree: `.worktrees/integration-upstream-2026-08-20/`
- Create branch: `integration/upstream-2026-08-20`
- Create: `docs/porting/upstream-sync-2026-08-20-ledger.md`

- [x] **Step 1: Verify worktree isolation path is ignored**

Run:

```bash
git check-ignore -q .worktrees
```

Expected: exit 0. If it fails, stop; do not create a project-local worktree until `.worktrees/` is ignored.

- [x] **Step 2: Create the integration worktree from pinned downstream main**

Run:

```bash
git worktree add .worktrees/integration-upstream-2026-08-20 -b integration/upstream-2026-08-20 db2d3c7a94b6be43240ac37f6809104ddd078ea2
```

Expected: worktree created on `integration/upstream-2026-08-20`.

- [x] **Step 3: Carry the approved design and plan into integration**

Run from the new worktree:

```bash
git cherry-pick main..feature/preservation-first-upstream-sync-design
```

Expected: design and implementation-plan commits applied in chronological order.

- [x] **Step 4: Verify reusable conflict resolution**

Run:

```bash
git config rerere.enabled true
git config rerere.autoupdate true
git config --get rerere.enabled
git config --get rerere.autoupdate
```

Expected: both values print `true`.

- [x] **Step 5: Run the pre-merge baseline**

Run:

```bash
cmake --preset macos-arm64
cmake --build --preset macos-arm64-debug
cmake -S . -B build-test-upstream-sync -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
python3 scripts/check-win32-guards.py
make format-check
make lint
make tidy-gate
```

Observed: both builds passed and CTest passed 85/85. The Win32 guard check has three pre-existing violations (`UI/Windows/CBTMessageBox.h`, `UI/Windows/CBTMessageBox.cpp`, and `Core/Input/Input.cpp`); whole-tree format and cppcheck gates have pre-existing debt; tidy was not run after those failures. The workspace-root `ctl` command is intentionally excluded because it validates the primary checkout, not this worktree.

For every code checkpoint, builds and tests must remain green. Run quality tools directly from this worktree against changed C/C++ paths only. The Win32 guard output may contain only the three baseline paths above. No new changed-path clang-format, cppcheck, or clang-tidy finding is accepted.

Run the changed-path gates with:

```bash
CHANGED_CPP=$(git diff --name-only --diff-filter=ACMR rollback/pre-upstream-merge-2026-08-20 -- src/source \
  | rg '\.(cpp|h|hpp)$' | rg -v '/(ThirdParty|Dotnet)/' || true)
CHANGED_IMPL=$(printf '%s\n' "$CHANGED_CPP" | rg '\.cpp$' || true)

if [ -n "$CHANGED_CPP" ]; then
  git clang-format --diff rollback/pre-upstream-merge-2026-08-20 -- $CHANGED_CPP
  printf '%s\n' "$CHANGED_CPP" | xargs cppcheck \
    --enable=warning,performance,portability --std=c++20 --language=c++ \
    --suppress=missingInclude --suppress=unmatchedSuppression \
    --suppress=unusedFunction --suppress='*:*/ThirdParty/*' \
    --suppress=useInitializationList --suppress=passedByValue \
    --suppress=postfixOperator --suppress=returnByReference --inline-suppr
fi

if [ -n "$CHANGED_IMPL" ]; then
  printf '%s\n' "$CHANGED_IMPL" | xargs clang-tidy -p out/build/macos-arm64 \
    --checks='-*,bugprone-sizeof-expression,bugprone-suspicious-memset-usage,bugprone-misplaced-pointer-arithmetic-in-alloc,-clang-diagnostic-*' \
    --extra-arg=-Wno-everything
fi

WIN32_NEW=$(python3 scripts/check-win32-guards.py 2>&1 | rg '^VIOLATION' \
  | rg -v '^VIOLATION  (UI/Windows/CBTMessageBox\.(h|cpp)|Core/Input/Input\.cpp):' || true)
test -z "$WIN32_NEW" || { printf '%s\n' "$WIN32_NEW"; false; }
```

`git clang-format` must report no formatting patch. Compare cppcheck and clang-tidy IDs/messages with the same paths at `rollback/pre-upstream-merge-2026-08-20`; every current-only finding must be fixed. New files have no baseline and therefore must be clean. Whole-file legacy findings remain out of scope unless the incoming hunk caused them.

- [x] **Step 6: Create the preservation ledger**

Create `docs/porting/upstream-sync-2026-08-20-ledger.md` with one row for each commit returned by:

```bash
git log --reverse --no-merges --format='%H%x09%s' 685936c10a8ee50982d0e1ca8d9430bf88186dd1..c0d74c4ed5edfad527b71899337f72d39bfc49f3
```

Columns: upstream commit, intent, upstream paths, downstream targets, resolution, verification, checkpoint. Initial resolution and verification cells use `pending`; replace every `pending` before final acceptance.

- [x] **Step 7: Commit the ledger baseline**

Run:

```bash
git add docs/porting/upstream-sync-2026-08-20-ledger.md
git commit -m "docs: add upstream sync preservation ledger"
```

Expected: one focused documentation commit.

### Task 3: Merge PR #543 Rendering Fixes

**Files:**
- Modify from upstream delta: paths reported by `git diff --name-status 685936c1..5cf17732`
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`
- Test: `tests/render/test_matrix_math_7_9_7.cpp`
- Verify by build/hunk audit: Blood Castle quest-item rendering path, which has no isolated upstream test seam

- [ ] **Step 1: Inspect the incoming delta**

Run:

```bash
git log --reverse --no-merges --oneline 685936c1..5cf17732
git diff --name-status 685936c1..5cf17732
```

Expected: commit `ba2a3fa5` and its two-file rendering delta.

- [ ] **Step 2: Merge the checkpoint without committing**

Run:

```bash
git merge --no-ff --no-commit 5cf17732
git diff --name-only --diff-filter=U
```

Expected: merge pauses for inspection; direct simulation exposed two content conflicts.

- [ ] **Step 3: Port each conflicted upstream hunk**

For every conflicted path, inspect:

```bash
git diff --cc -- <path>
git show :1:<path>
git show :2:<path>
git show :3:<path>
```

Keep downstream structure and defaults. Apply the upstream matrix-initialization and Blood Castle rendering behavior. Stage each resolved path with `git add <path>`.

- [ ] **Step 4: Update ledger and validate**

Replace `pending` for `ba2a3fa5` with the actual downstream targets, resolution, focused verification, and checkpoint `5cf17732`.

Run:

```bash
git diff --name-only --diff-filter=U
git diff --check
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
```

Expected: no unresolved paths; all commands exit 0.

- [ ] **Step 5: Complete the merge**

Run:

```bash
git add docs/porting/upstream-sync-2026-08-20-ledger.md
git commit --no-edit
```

Expected: checkpoint merge committed.

### Task 4: Merge PR #544 GPU Skinning And Core Profile Port

**Files:**
- Modify: `src/CMakeLists.txt`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Modify: conflicting paths under `src/source/Camera/`, `src/source/Core/`, `src/source/Engine/`, `src/source/Render/`, `src/source/Scenes/`, `src/source/UI/`, and `src/source/World/`
- Modify: renderer shaders and documentation imported by `5cf17732..1a678b30`
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`

- [ ] **Step 1: Inspect all four incoming commits and changed paths**

Run:

```bash
git log --reverse --no-merges --oneline 5cf17732..1a678b30
git diff --name-status 5cf17732..1a678b30
```

Expected commits: `0702144c`, `b1d26158`, `3a47e2c4`, `31dcbe7a`.

- [ ] **Step 2: Merge checkpoint #544**

Run:

```bash
git merge --no-ff --no-commit 1a678b30
git diff --name-only --diff-filter=U
```

Expected: the largest conflict set; current direct exposure is 49 paths.

- [ ] **Step 3: Resolve foundation before call sites**

Resolve and stage in this order:

```text
src/CMakeLists.txt
src/source/App/Platform/Windows/Winmain.cpp
src/source/Render/Core/**
src/source/Render/RHI/**
src/source/Render/Shaders/**
src/source/Render/Models/**
src/source/Engine/**
src/source/Render/Effects/**
src/source/Render/Sprites/**
src/source/Render/Terrain/**
src/source/Render/Textures/**
src/source/Camera/**
src/source/Scenes/**
src/source/UI/**
src/source/World/**
```

For each conflict, compare index stages `:1`, `:2`, and `:3`. Keep downstream platform abstractions, source layout, resource ownership, and product defaults. Port GPU skinning, Core Profile rendering, shader interfaces, and portability fixes without restoring retired downstream architecture.

- [ ] **Step 4: Preserve or add focused regression coverage before each manual behavior port**

Use existing tests when they exercise the touched seam. If no test exists, add one smallest test under the matching `tests/` module, run it before the port, confirm the expected failure, then apply the port and rerun it.

- [ ] **Step 5: Update ledger and run major checkpoint gates**

Resolve ledger rows for `0702144c`, `b1d26158`, `3a47e2c4`, and `31dcbe7a`.

Run:

```bash
git diff --name-only --diff-filter=U
git diff --check
git grep -n -E '^(<<<<<<< |>>>>>>> )' -- 'src/source/*'
cmake --preset macos-arm64
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
# Run the Task 2 worktree-local changed-path quality gates.
```

Expected: no conflicts or markers; every gate exits 0.

- [ ] **Step 6: Complete checkpoint #544**

Run:

```bash
git add docs/porting/upstream-sync-2026-08-20-ledger.md
git commit --no-edit
```

Expected: GPU/Core Profile checkpoint committed.

### Task 5: Merge Documentation Checkpoints #545 And #546

**Files:**
- Modify: renderer documentation changed by `1a678b30..b330c33c`
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`

- [ ] **Step 1: Merge and audit PR #545**

Run:

```bash
git merge --no-ff --no-commit 9fdcdb08
git diff --name-only --diff-filter=U
git diff --check
```

Adapt documentation paths and terminology to downstream architecture. Resolve ledger rows `74fdb8ae` and `7d71b550`, then run:

```bash
git add docs docs/porting/upstream-sync-2026-08-20-ledger.md
git commit --no-edit
```

- [ ] **Step 2: Merge and audit PR #546**

Run:

```bash
git merge --no-ff --no-commit b330c33c
git diff --name-only --diff-filter=U
git diff --check
```

Resolve ledger rows `1b8b2a6a`, `5dca2010`, `bbed8e82`, and `00e1bbc3`, then run:

```bash
git add docs docs/porting/upstream-sync-2026-08-20-ledger.md
git commit --no-edit
```

Expected: both documentation checkpoints committed; no code gate required because neither checkpoint changes production code.

### Task 6: Merge Focused Render And Frame-Pacing Fixes

**Files:**
- Modify: paths changed by `b330c33c..5cfebed3`
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`
- Test: focused renderer, frame limiter, and terrain tests where seams exist

- [ ] **Step 1: Merge PR #551**

Run:

```bash
git merge --no-ff --no-commit 39107a81
```

Port the GL debug callback performance behavior. Resolve ledger rows `af7b27b3` and `9480bed0`; run build and CTest gates; commit with `git commit --no-edit`.

- [ ] **Step 2: Merge PR #555**

Run:

```bash
git merge --no-ff --no-commit 9a6eb943
```

Port shader `blendMeshAlpha` behavior. Resolve ledger row `61a61707`; run build and CTest gates; commit with `git commit --no-edit`.

- [ ] **Step 3: Merge PR #553**

Run:

```bash
git merge --no-ff --no-commit 8b5ae75e
```

Preserve downstream frame-pacing defaults while importing fallback limiting and explicit VSync-off handling. Resolve ledger rows `2c2ef7ea`, `67bbd930`, and `d9e20753`; run build and CTest gates; commit with `git commit --no-edit`.

- [ ] **Step 4: Merge PR #557**

Run:

```bash
git merge --no-ff --no-commit 5cfebed3
```

Port per-texture terrain UV scale. Resolve ledger row `c6603879`; run build and CTest gates; commit with `git commit --no-edit`.

For every step, first require:

```bash
git diff --name-only --diff-filter=U
git diff --check
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
```

Expected: four separate ancestry checkpoints, zero unresolved paths, all gates exit 0.

### Task 7: Merge PR #560 GL Performance Series

**Files:**
- Modify: 42 paths changed by `5cfebed3..60d4efed`
- Modify: Render RHI, shaders, models, sprites, terrain, textures, profiling, CPU usage, and documentation paths
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`

- [ ] **Step 1: Inventory the complete PR #560 series**

Run:

```bash
git log --reverse --no-merges --oneline 5cfebed3..60d4efed
git diff --name-status 5cfebed3..60d4efed
```

Expected: 27 non-merge commits from `896afef6` through `2f7e1bfa`.

- [ ] **Step 2: Merge checkpoint #560**

Run:

```bash
git merge --no-ff --no-commit 60d4efed
git diff --name-only --diff-filter=U
```

Expected direct exposure: eight conflicts.

- [ ] **Step 3: Port in dependency order**

Resolve RHI and buffer interfaces first; then UBO state, shaders, model draws, terrain batching, ImmediateRenderer batching, profiling/counters, CPU usage, and docs. Preserve downstream APIs and defaults. Keep every `$glstats`, FrameProfiler, GL capability, RHI indexed-draw, ring-allocation, and batching capability represented in the ledger.

- [ ] **Step 4: Validate and commit #560**

Run:

```bash
git diff --name-only --diff-filter=U
git diff --check
git grep -n -E '^(<<<<<<< |>>>>>>> )' -- 'src/source/*'
cmake --preset macos-arm64
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
# Run the Task 2 worktree-local changed-path quality gates.
```

Replace all PR #560 ledger `pending` values, stage them, then run `git commit --no-edit`.

Expected: all gates exit 0; PR #560 checkpoint committed.

### Task 8: Merge PR #540 Chat Commands

**Files:**
- Modify: 30 paths changed by `60d4efed..56fb3636`
- Modify: `src/source/GameLogic/Commands/**`, related network/config/UI paths
- Modify: `tests/text/test_text_line_wrap.cpp`
- Modify: chat-command documentation
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`

- [ ] **Step 1: Inventory chat behavior and tests**

Run:

```bash
git log --reverse --no-merges --oneline 60d4efed..56fb3636
git diff --name-status 60d4efed..56fb3636
```

Expected: 16 commits covering network receipt, UI, parameters, key binding, history, favorites, templates, pagination, and docs.

- [ ] **Step 2: Preserve/adapt the upstream text test before manual UI ports**

Restore `tests/text/test_text_line_wrap.cpp` in the downstream test layout. Run its registered CTest target before completing manual production ports; confirm failure if the required wrapping behavior is absent, then port the behavior and confirm the test passes.

- [ ] **Step 3: Merge checkpoint #540**

Run:

```bash
git merge --no-ff --no-commit 56fb3636
git diff --name-only --diff-filter=U
```

Keep downstream networking, input, config, and UI ownership. Port all upstream command capabilities into those seams. Preserve downstream defaults when a key binding or UI behavior conflicts.

- [ ] **Step 4: Validate and commit #540**

Run:

```bash
git diff --name-only --diff-filter=U
git diff --check
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
# Run the Task 2 worktree-local changed-path quality gates.
```

Resolve all 16 ledger rows; commit with `git commit --no-edit`.

Expected: chat command receipt, display, execution, parameters, persistence, history, pagination, and docs retained; all gates exit 0.

### Task 9: Merge PR #566 Profiling Attribution

**Files:**
- Modify: 10 paths changed by `56fb3636..c0d74c4e`
- Modify: ImmediateRenderer/profiling paths
- Modify: `docs/porting/upstream-sync-2026-08-20-ledger.md`

- [ ] **Step 1: Merge final checkpoint**

Run:

```bash
git merge --no-ff --no-commit c0d74c4e
git diff --name-only --diff-filter=U
```

Port `$glstats` batch-break attribution into downstream profiling interfaces. Resolve ledger row `38f51abf`.

- [ ] **Step 2: Validate and complete final checkpoint**

Run:

```bash
git diff --name-only --diff-filter=U
git diff --check
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
git add docs/porting/upstream-sync-2026-08-20-ledger.md
git commit --no-edit
```

Expected: final upstream checkpoint committed; build and tests pass.

### Task 10: Audit Completeness And Prepare Publication

**Files:**
- Verify: `docs/porting/upstream-sync-2026-08-20-ledger.md`
- Modify only if verification exposes a missing port or inaccurate ledger entry

- [ ] **Step 1: Prove no upstream commit remains outside integration**

Run:

```bash
git merge-base --is-ancestor c0d74c4ed5edfad527b71899337f72d39bfc49f3 HEAD
git rev-list --left-right --count c0d74c4ed5edfad527b71899337f72d39bfc49f3...HEAD
```

Expected: ancestry command exits 0; left count is `0`.

- [ ] **Step 2: Prove ledger completion**

Run:

```bash
rg -n '\bpending\b|\bdropped\b|not applicable' docs/porting/upstream-sync-2026-08-20-ledger.md
```

Expected: no matches.

- [ ] **Step 3: Run final full validation**

Run:

```bash
git diff --check rollback/pre-upstream-merge-2026-08-20..HEAD
git grep -n -E '^(<<<<<<< |>>>>>>> )' -- 'src/source/*'
cmake --preset macos-arm64
cmake --build --preset macos-arm64-debug
cmake --build build-test-upstream-sync -j8
ctest --test-dir build-test-upstream-sync --output-on-failure
# Run the Task 2 worktree-local changed-path quality gates.
git status --short --branch
```

Expected: all commands exit 0; worktree clean.

- [ ] **Step 4: Prepare publication without pushing**

Run in the primary checkout:

```bash
git rev-parse integration/upstream-2026-08-20
git rev-parse main
git log --oneline --decorate main..integration/upstream-2026-08-20
```

Expected: integration is a descendant of `main`. Stop before switching `main` or pushing `origin/main`; publication can trigger CI and semantic release and requires explicit approval.
