# NPC Dialogue Tokenizer Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stop NPC dialogue from splitting on random characters by supplying `wcstok_s()` a valid NUL-terminated delimiter string.

**Architecture:** Keep `DivideStringByPixel()` and every caller unchanged. Replace the unsafe address of a single delimiter character with a local two-element array, then protect that invariant with the repository's existing CMake source-contract test pattern. Renderer middleware and ground-item rendering remain untouched.

**Tech Stack:** C++20, CMake/CTest, existing macOS arm64 Release preset.

---

### Task 1: Add the failing delimiter contract

**Files:**
- Create: `tests/ui/test_divide_string_delimiter.cmake`
- Modify: `tests/ui/CMakeLists.txt:35-41`

- [ ] **Step 1: Create the regression contract**

```cmake
file(READ "${SOURCE_DIR}/src/source/Core/Utilities/UsefulDef.cpp" source)

if(NOT source MATCHES "const wchar_t szNewlineDelimiters\\[\\] = \\{szNewlineChar, L'\\\\0'\\};")
    message(FATAL_ERROR "DivideStringByPixel must construct a NUL-terminated delimiter array")
endif()

if(source MATCHES "wcstok_s\\([^\\n]*&szNewlineChar")
    message(FATAL_ERROR "DivideStringByPixel must not pass a single character as a delimiter string")
endif()
```

- [ ] **Step 2: Register the contract**

Append to `tests/ui/CMakeLists.txt`:

```cmake
add_test(
    NAME divide_string_delimiter_contract
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_divide_string_delimiter.cmake
)
```

- [ ] **Step 3: Reconfigure tests**

Run:

```bash
cmake --preset macos-arm64
```

Expected: configuration completes; `divide_string_delimiter_contract` appears in CTest.

- [ ] **Step 4: Verify RED**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release -R '^divide_string_delimiter_contract$' --output-on-failure
```

Expected: FAIL with `DivideStringByPixel must construct a NUL-terminated delimiter array`.

### Task 2: Terminate the tokenizer delimiter

**Files:**
- Modify: `src/source/Core/Utilities/UsefulDef.cpp:39-58`
- Test: `tests/ui/test_divide_string_delimiter.cmake`

- [ ] **Step 1: Add the minimal production fix**

In `DivideStringByPixel()`, construct one delimiter string and reuse it:

```cpp
const wchar_t szNewlineDelimiters[] = {szNewlineChar, L'\0'};

wchar_t* context = nullptr;
wchar_t* pszToken = wcstok_s(&szWorkSrc[0], szNewlineDelimiters, &context);
```

Replace the loop's second tokenizer call with:

```cpp
pszToken = wcstok_s(nullptr, szNewlineDelimiters, &context);
```

- [ ] **Step 2: Verify GREEN**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release -R '^divide_string_delimiter_contract$' --output-on-failure
```

Expected: `100% tests passed, 0 tests failed out of 1`.

### Task 3: Verify the focused change

**Files:**
- Verify: `src/source/Core/Utilities/UsefulDef.cpp`
- Verify: `tests/ui/test_divide_string_delimiter.cmake`
- Verify: `tests/ui/CMakeLists.txt`

- [ ] **Step 1: Check patch hygiene**

Run:

```bash
git diff --check
```

Expected: no output; exit code 0.

- [ ] **Step 2: Build the Release client**

Run:

```bash
cmake --build --preset macos-arm64-release --target MuClient
```

Expected: `Built target MuClient`.

- [ ] **Step 3: Run UI contracts**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release -R 'input_box_config_tests|text_search_tests|chat_command_window_focus_contract|inventory_and_system_backdrops_contract|exit_game_shutdown_contract|text_wrap_measurement_contract|divide_string_delimiter_contract' --output-on-failure
```

Expected: all selected tests pass.

- [ ] **Step 4: Run the full suite**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release --output-on-failure
```

Expected: no new failures compared with the pre-change baseline.

### Task 4: Verify in game and commit

**Files:**
- Commit: `src/source/Core/Utilities/UsefulDef.cpp`
- Commit: `tests/ui/test_divide_string_delimiter.cmake`
- Commit: `tests/ui/CMakeLists.txt`

- [ ] **Step 1: Reopen the rebuilt client**

Quit the currently running client from its UI, then run:

Run:

```bash
open out/build/macos-arm64/src/Release/Main.app
```

Expected: the rebuilt Release client launches.

- [ ] **Step 2: Verify NPC 257 visually**

At 1024x768, open the Shadow Phantom Soldier dialogue in Lorencia or Devias.

Expected:

```text
Welcome to the world of Mu.
We have been waiting for a hero like ...
```

No isolated fragments such as `av`, `waiti`, or `g`. Ground-item labels remain visible and unchanged.

- [ ] **Step 3: Commit after visual confirmation**

Run:

```bash
git add src/source/Core/Utilities/UsefulDef.cpp tests/ui/CMakeLists.txt tests/ui/test_divide_string_delimiter.cmake
git commit -m "fix(ui): terminate text split delimiter"
```

Expected: one focused implementation commit.
