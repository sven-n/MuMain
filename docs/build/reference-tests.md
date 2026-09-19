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
