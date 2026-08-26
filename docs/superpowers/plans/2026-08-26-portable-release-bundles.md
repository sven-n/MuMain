# Portable Release Bundles Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make packaged clients load the network library beside the executable and publish clearly named Release data/no-data bundles.

**Architecture:** Remove the build-time absolute library directory from CMake. Derive the library path from the running executable before loading it, using the existing Win32-compatible executable-path shim with a macOS implementation. Keep asset ownership unchanged: no-data users provide `Data/` and `fonts/`.

**Tech Stack:** C++20, CMake, SDL3, GitHub Actions, Python workflow contracts, doctest/CTest.

---

### Task 1: Resolve the network library beside the executable

**Files:**
- Modify: `tests/network/test_connection_library_load.cpp`
- Modify: `src/source/Core/Platform/WinApiShims.h`
- Modify: `src/source/Dotnet/Connection.cpp`
- Modify: `src/source/Dotnet/Connection.h`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Write the failing executable-relative path test**

Add the POSIX-only includes, external path declaration, and test:

```cpp
#ifndef _WIN32
#include <filesystem>
#include <string>

#include "Core/Platform/WinApiShims.h"

extern const std::string g_dotnetLibPath;
#endif

#ifndef _WIN32
TEST_CASE("Connection library path follows the executable [network][dotnet]")
{
    constexpr DWORD executablePathCapacity = 4096;
    wchar_t executablePath[executablePathCapacity] = {};
    REQUIRE(GetModuleFileNameW(nullptr, executablePath, executablePathCapacity) != 0);

    const auto libraryName = "MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT);
    const auto expectedPath = std::filesystem::path(mu_narrow_path(executablePath)).parent_path() / libraryName;
    CHECK(std::filesystem::path(g_dotnetLibPath) == expectedPath);
}
#endif
```

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```bash
cmake --build out/build/commit-verify-editor-on --config Debug --target test_connection_library_load
ctest --test-dir out/build/commit-verify-editor-on -C Debug -R "Connection library path follows" --output-on-failure
```

Expected: FAIL. On macOS the current `/proc/self/exe` shim cannot resolve the executable; on Linux the current `MU_DOTNET_LIB_DIR` path differs from the test executable directory.

- [ ] **Step 3: Make executable-path lookup portable on macOS**

In `WinApiShims.h`, include `<mach-o/dyld.h>` only on Apple platforms. Replace the unconditional `readlink` assignment inside `GetModuleFileNameW` with:

```cpp
char path[4096] = {};
#ifdef __APPLE__
std::uint32_t pathSize = sizeof(path);
if (_NSGetExecutablePath(path, &pathSize) != 0)
{
    lpFilename[0] = L'\0';
    return 0;
}
const ssize_t n = static_cast<ssize_t>(std::strlen(path));
#else
const ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
#endif
```

Keep the existing UTF-8-to-wide conversion and failure handling after this block.

- [ ] **Step 4: Replace the build-time directory with runtime resolution**

Delete this CMake block:

```cmake
if(UNIX)
  target_compile_definitions(MuClient PUBLIC
    "MU_DOTNET_LIB_DIR=\"$<TARGET_FILE_DIR:Main>\""
  )
endif()
```

Replace the conditional `g_dotnetLibPath` initialization in `Connection.cpp` with:

```cpp
namespace
{
std::string ManagedLibraryPath()
{
    const std::filesystem::path libraryName =
        "MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT);
#ifdef _WIN32
    return libraryName.string();
#else
    constexpr DWORD executablePathCapacity = 4096;
    wchar_t executablePath[executablePathCapacity] = {};
    if (GetModuleFileNameW(nullptr, executablePath, executablePathCapacity) == 0)
    {
        return libraryName.string();
    }

    return (std::filesystem::path(mu_narrow_path(executablePath)).parent_path() / libraryName).string();
#endif
}
} // namespace

const std::string g_dotnetLibPath = ManagedLibraryPath();
```

Update `Connection.h` comments: the path is executable-relative at runtime; no `MU_DOTNET_LIB_DIR` definition remains.

- [ ] **Step 5: Run the focused test and verify GREEN**

Run the two Step 2 commands again.

Expected: the new test passes; the existing missing-library test also passes.

- [ ] **Step 6: Commit the loader fix**

```bash
git add tests/network/test_connection_library_load.cpp src/source/Core/Platform/WinApiShims.h src/source/Dotnet/Connection.cpp src/source/Dotnet/Connection.h src/CMakeLists.txt
git commit -m "fix(runtime): load network library beside executable"
```

### Task 2: Publish Release bundles from every workflow

**Files:**
- Modify: `tests/test_windows_workflow_contract.py`
- Modify: `.github/workflows/ci.yml`
- Modify: `.github/workflows/linux-build.yml`

- [ ] **Step 1: Change workflow expectations to Release**

In the workflow contract:

- expect main CI Linux artifact names `mu-client-linux-native-x64-release-editor-off` and `mu-client-linux-native-x64-release-editor-on`;
- replace Linux `--config Debug`, `--build-config Debug`, and `/src/Debug/` expectations with `Release`;
- replace macOS Debug build, test, validation, and upload paths with `Release`;
- expect macOS artifact names `main-macos-arm64-release-${{ github.ref_name }}` and `main-macos-arm64-release-no-data-${{ github.ref_name }}`;
- expect standalone Linux artifact names `mu-client-linux-x64-release-editor-${{ matrix.editor }}` and `mu-client-linux-x64-release-editor-${{ matrix.editor }}-no-data`;
- assert standalone Linux configure still contains `-DCMAKE_BUILD_TYPE=Release`.

- [ ] **Step 2: Run the workflow contract and verify RED**

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
```

Expected: FAIL messages identify the current Debug paths and old artifact names.

- [ ] **Step 3: Switch main CI Linux and macOS to Release**

In `.github/workflows/ci.yml`:

- change macOS build/test/validation/upload paths from `Debug` to `Release`;
- rename macOS artifacts to include `release`;
- change Linux matrix artifact names from `debug` to `release`;
- change Linux build/test/validation/upload paths from `Debug` to `Release`.

Do not add a second build configuration. Release becomes the single packaged and tested configuration for these jobs.

- [ ] **Step 4: Make standalone Linux artifact names explicit**

In `.github/workflows/linux-build.yml`, retain `-DCMAKE_BUILD_TYPE=Release`; rename both artifacts:

```yaml
name: mu-client-linux-x64-release-editor-${{ matrix.editor }}
```

```yaml
name: mu-client-linux-x64-release-editor-${{ matrix.editor }}-no-data
```

- [ ] **Step 5: Run the workflow contract and verify GREEN**

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
```

Expected: `Windows workflow contract: OK`.

### Task 3: Document user-owned assets

**Files:**
- Modify: `docs/build/README.md`

- [ ] **Step 1: Correct bundle usage wording**

Replace “two runnable bundles” with “two runtime bundles.” Document:

```markdown
- The existing artifact name is the data bundle: the complete runtime directory.
- The `-no-data` artifact is the normal redistributable runtime. It excludes
  only `Data/` and `fonts/`; users must supply compatible copies before running
  the client. The executable, shaders, configuration, network library when
  supported, and platform runtime libraries remain included.
```

Add one sentence that CI artifact names include `release` for packaged builds.

- [ ] **Step 2: Commit workflow and documentation changes**

```bash
git add .github/workflows/ci.yml .github/workflows/linux-build.yml tests/test_windows_workflow_contract.py docs/build/README.md docs/superpowers/plans/2026-08-26-portable-release-bundles.md
git commit -m "fix(ci): publish explicit Release bundles"
```

### Task 4: Full verification

**Files:**
- Verify only; modify failing files only when evidence identifies a regression caused by these changes.

- [ ] **Step 1: Run contract and formatting checks**

```bash
python3 tests/test_windows_workflow_contract.py "$PWD"
python3 -m json.tool .releaserc.json >/dev/null
git diff --check HEAD~2
```

Expected: contract OK; JSON command exits 0; diff check exits 0.

- [ ] **Step 2: Run the complete configured suite**

```bash
ctest --test-dir out/build/commit-verify-editor-on -C Debug --output-on-failure
```

Expected: 100% tests passed.

- [ ] **Step 3: Verify repository state and commits**

```bash
git status --short
git log -3 --oneline
```

Expected: clean worktree; design, runtime-fix, and CI-fix commits visible.

- [ ] **Step 4: Hosted proof after push**

After the user pushes, download one standalone Linux no-data artifact. Verify:

```bash
file Main
python3 - <<'PY'
from pathlib import Path

binary = Path("Main").read_bytes()
assert b"/home/runner/work/" not in binary
assert b".debug_info" not in binary
PY
```

Expected: Release ELF; no runner path; no debug information marker.
