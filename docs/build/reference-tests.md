# Tests

Unit tests live under `tests/` and use
[doctest](https://github.com/doctest/doctest) (single-header, vendored at
`tests/third_party/doctest/`).

```
tests/
  CMakeLists.txt              <-- entry point + mu_add_test() helper
  main.cpp                    <-- doctest main()
  third_party/doctest/        <-- vendored single header
  text/                       <-- one subdirectory per module under test
  editor/                     <-- editor-leak guard (issue #359)
```

## Running

Enable with `-DBUILD_TESTING=ON`, build, then run CTest:

```bash
ctest --test-dir <build-dir> --output-on-failure
```

- **Native Linux build:** tests run directly.
- **MinGW / Windows binaries on a Linux host:** CTest runs each test through
  `wine` automatically (install `wine`; for 32-bit, `wine32`).
- **Native Windows:** tests run directly.

With `ENABLE_EDITOR=OFF` the suite includes `editor_leak`, which fails if any
source under `src/MuEditor/` was compiled into the non-editor client.

## Test design

- Test observable behavior through production APIs. Prefer pure units, compile
  checks, generated target properties, or small executable fixtures.
- Do not assert exact source snippets, function-body order, local names, or
  formatting. Those checks break during safe refactors without proving runtime
  behavior.
- Source inspection is limited to repository-wide negative policy guards, such
  as banning platform APIs outside their abstraction layer. Keep allowlists
  narrow and explain why compilation cannot enforce the rule.
- Do not use sleeps to coordinate asynchronous work. Use explicit signals and
  give every wait a timeout. Real-clock tests may compare against the same
  monotonic clock; microbenchmarks do not belong in the regression suite.
- Every automated test file must be registered with CTest. Delete obsolete or
  superseded tests instead of leaving inactive files under `tests/`.
- Give every `TEST_CASE` an ASCII name, without `;`, a line break or `\` (a
  comma is fine), with as many `[` as `]` and not starting with `[doctest] `.
  CTest's discovery reads the names as a CMake list, so it would drop an empty
  name and cut or join the others, and it skips lines that look like doctest's
  own output; doctest's filter reads a `\` as an escape, and on Windows the test
  binary reads its filter in the ANSI code page. Then no entry runs the case. The
  static check `test_case_names_reach_ctest_whole` finds such names before any
  build.
- A CTest entry that runs no case fails (doctest reports `test cases: 0`), and
  each test binary's `<binary>_has_one_ctest_entry_per_case` entry fails when its
  entries and its cases differ in number. So a name the discovery cut, joined or
  dropped fails in CI even where the static check cannot read it. The canary
  binaries in `tests/fixtures/ctest_guards/` check that both guards still fail. Decide an
  opt-in case at run time rather than with `doctest::skip(condition)`: the
  discovery evaluates the condition when the binary is built, so a condition
  read from the environment can differ when the tests run. Print
  `MESSAGE("MU_TEST_SKIPPED: <why>")` and return at once; CTest then reports the
  entry as skipped (a skip wins over a failure, so check nothing after it).

## Static checks and the pre-push hook

Checks that only read the repository (no compiler, dependencies or build output),
such as the policy guards above and the RML and workflow contracts, are registered
in `tests/static/CMakeLists.txt`. That directory is a standalone CMake project, so
the checks run in seconds without a build:

```bash
cmake -S tests/static -B out/static-checks
ctest --test-dir out/static-checks -C Release --output-on-failure
```

Quality Gates runs them before any native build starts. The full test tree includes
them with the `static` label, and the native CI jobs skip that label. Register a new
check there only if it stays fast and never reads build output.

Run `scripts/install-hooks.sh` once per clone to enable `.githooks/pre-push`. Before
each push it runs these checks and the changed-lines formatting check with the scripts
Quality Gates uses, on Linux, macOS and Windows (Git Bash). It finds CMake on `PATH`
or, on Windows, Visual Studio's bundled copy. It checks formatting only with the
clang-format version CI pins (`pip install clang-format==<version from ci.yml>`) and
diffs against `origin/main` (set `MU_PUSH_BASE` to change it). A missing tool skips
that check with a message; CI still enforces it. `git push --no-verify` skips the hook.

## Adding a test module

1. Create `tests/<module>/` with a `CMakeLists.txt` and one or more
   `test_*.cpp` files.
2. In `tests/<module>/CMakeLists.txt`:
   ```cmake
   mu_add_test(
       NAME test_<thing>
       SOURCES test_<thing>.cpp ${CMAKE_SOURCE_DIR}/src/source/<path>/<unit>.cpp
   )
   ```
   `mu_add_test` links against `mu_test_main` (doctest's `main()`), wires the
   `doctest` and `src/source` include paths, and registers each `TEST_CASE`
   with CTest (wrapping it in `wine` on non-Windows hosts).
3. Add `add_subdirectory(<module>)` to `tests/CMakeLists.txt`.
4. Pass the source files of the unit under test directly via `SOURCES`, keeping
   the test binary independent of `Main`: a unit must be linkable without the
   rest of the game (no `stdafx.h`, no scene globals). If a function isn't
   isolated, extract it to its own `.cpp` first (see
   `src/source/Core/Text/TextLineWrap.cpp` for the pattern).
