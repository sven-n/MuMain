# Platform Build Checks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore simple Linux and macOS build checks and prevent successful releases from failing during fork-incompatible pull-request commenting.

**Architecture:** Keep one fixed Release/editor-OFF job per supported hosted platform. Only Windows uploads a runtime; Linux and macOS compile, test, and validate. Disable Semantic Release success comments because fork history references upstream pull requests absent from this repository.

**Tech Stack:** GitHub Actions, CMake 3.25, Python workflow contracts, Semantic Release, JSON.

---

### Task 1: Lock the corrected workflow contract

**Files:**
- Modify: `tests/test_windows_workflow_contract.py`

- [x] **Step 1: Require fixed Linux and macOS jobs**

Change the expected CI job order to:

```python
[
    "quality",
    "build-windows",
    "build-linux",
    "build-macos",
    "release",
]
```

Require both restored jobs to reject `strategy:`, `matrix.`, and
`actions/upload-artifact`. Require Linux x64 Release/editor OFF with
`out/build/linux-ci`; require macOS arm64 Release/editor OFF with
`out/build/macos-ci`. Require configure, build, test, and runtime-validation
steps for each.

- [x] **Step 2: Require all platform checks before release**

Require:

```yaml
needs: [quality, build-windows, build-linux, build-macos]
```

- [x] **Step 3: Require fork-safe release configuration**

Require the `@semantic-release/github` options to contain:

```json
"successCommentCondition": false
```

- [x] **Step 4: Require accurate documentation**

Require `README.md` and `docs/build/README.md` to state that CI validates
Linux x64 Release and macOS arm64 Release, but publishes only the Windows
runtime.

- [x] **Step 5: Verify RED**

Run:

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
```

Expected: FAIL for missing Linux/macOS jobs, old release dependencies,
success-comment configuration, and documentation.

### Task 2: Restore simplified platform build checks

**Files:**
- Modify: `.github/workflows/ci.yml`

- [x] **Step 1: Add one Linux check**

Add `build-linux` after `build-windows`. Run it only for pull requests and
main pushes:

```yaml
if: github.event_name == 'pull_request' || github.ref == 'refs/heads/main'
```

Use `ubuntu-latest`, .NET 10, the existing Linux package list, one dependency
cache, and these commands:

```bash
cmake --preset linux-x64 -B out/build/linux-ci -DENABLE_EDITOR=OFF -DMU_COPY_RUNTIME_ASSETS=OFF -DBUILD_TESTING=ON
cmake --build out/build/linux-ci --config Release
ctest --test-dir out/build/linux-ci --build-config Release --output-on-failure --no-tests=error
```

Validate `Main`, `MUnique.Client.Library.so`, `config.ini`, and `shaders/`
under `out/build/linux-ci/src/Release`. Do not upload artifacts.

- [x] **Step 2: Add one macOS check**

Add `build-macos` after `build-linux` with the same event condition. Use
`macos-latest`, .NET 10, the existing Homebrew packages, one dependency
cache, and:

```bash
cmake --preset macos-arm64 -B out/build/macos-ci -DENABLE_EDITOR=OFF -DMU_COPY_RUNTIME_ASSETS=OFF -DBUILD_TESTING=ON -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
cmake --build out/build/macos-ci --config Release
ctest --test-dir out/build/macos-ci --build-config Release --output-on-failure --no-tests=error
```

Validate `Main`, `MUnique.Client.Library.dylib`, `config.ini`, and
`shaders/` under
`out/build/macos-ci/src/Release/Main.app/Contents/MacOS`. Do not upload
artifacts.

- [x] **Step 3: Gate release on all platform checks**

Set:

```yaml
needs: [quality, build-windows, build-linux, build-macos]
```

- [x] **Step 4: Verify platform contract GREEN**

Run:

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
```

Expected: only release-configuration and documentation failures remain.

### Task 3: Prevent post-publish release failures

**Files:**
- Modify: `.releaserc.json`

- [x] **Step 1: Disable success-comment discovery**

Add this beside `assets` in the `@semantic-release/github` options:

```json
"successCommentCondition": false
```

This skips issue and pull-request discovery while preserving tag creation,
GitHub release creation, and asset upload.

- [x] **Step 2: Validate release configuration**

Run:

```bash
python3 -m json.tool .releaserc.json >/dev/null
python3 tests/test_windows_workflow_contract.py "$PWD"
```

Expected: JSON passes; only documentation failures remain.

### Task 4: Correct build and release documentation

**Files:**
- Modify: `README.md`
- Modify: `docs/build/README.md`

- [x] **Step 1: Document hosted validation**

State that Actions validate:

- Windows native x64 Release, editor OFF
- Linux x64 Release, editor OFF
- macOS arm64 Release, editor OFF

State that only Windows produces a downloadable runtime artifact.

- [x] **Step 2: Preserve local-build ownership**

State that x86, Debug, editor-ON, MinGW, and other configurations remain local
builds.

- [x] **Step 3: Verify documentation contract**

Run:

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
```

Expected: `Windows workflow contract: OK`.

### Task 5: Full verification

**Files:**
- Verify only

- [x] **Step 1: Run focused checks**

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
cmake -P tests/test_copy_runtime_assets.cmake
python3 -m json.tool .releaserc.json >/dev/null
ruby -e 'require "yaml"; ARGV.each { |f| YAML.load_file(f) }' .github/workflows/ci.yml .github/workflows/data-assets.yml
git diff --check
```

Expected: all commands exit 0.

- [x] **Step 2: Review final scope**

Run:

```bash
git status --short
git diff --stat
```

Expected: only the spec update, plan, workflow contract, CI workflow, release
configuration, and two documentation files are changed.

- [x] **Step 3: Report remote verification boundary**

Do not claim hosted platform builds or Semantic Release are fixed until the
new commit is pushed and the GitHub Actions run completes. Report that v1.1.1
was already created successfully and only the post-publish success hook failed.
