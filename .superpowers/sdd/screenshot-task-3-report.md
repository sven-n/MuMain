# Task 3 Report: Deferred Screenshot Capture

## Implementation

- Added `ScreenshotCaptureState` as a metadata-only one-request state helper.
- Replaced the synchronous scene screenshot readback with deferred
  `RequestFramePixels()` / `ConsumeFramePixels()` handling.
- Preserved JPEG quality 100, physical readback dimensions, filename and
  notification behavior, Shift selection, and `GrabScreen` modulo 10000.
- Restored non-Windows `MU_CAPTURE_FRAME` capture after `EndFrame()` and direct
  top-down RGB output to a P6 PPM.
- Added the helper source and state tests to `frame_pixel_readback_tests`; the
  recursive client source glob includes the helper in `MuClient` / `Main`.

## Verification

- RED: `cmake --build out/build/macos-arm64 --config Debug --target
  frame_pixel_readback_tests -j 8` failed during CMake generation because
  `src/source/Scenes/ScreenshotCaptureState.cpp` did not exist.
- Focused build: `cmake --build out/build/macos-arm64 --config Debug --target
  Main frame_pixel_readback_tests -j 8` completed with exit code 0.
- Focused tests:
  `out/build/macos-arm64/tests/render/Debug/frame_pixel_readback_tests`
  passed 18/18 test cases and 65/65 assertions.
- Debug Main: `cmake --build out/build/macos-arm64 --config Debug --target Main
  -j 8` completed with exit code 0. Existing missing-override/switch warnings
  and a pre-existing macOS deployment-version linker warning remained.
- Full tests: `ctest --test-dir out/build/macos-arm64 -C Debug
  --output-on-failure` passed 42/42 tests.
- Whitespace: `git diff --check` completed with exit code 0 and no output.
- Integration scan found no `ReadPixels`, `glReadPixels`, or `glPixelStorei`
  references in `SceneManager.cpp` or `Winmain.cpp`.
- Review follow-up: `cmake --build out/build/macos-arm64 --config Debug --target
  frame_pixel_readback_tests -j 8` completed; the focused test binary passed
  19/19 test cases and 71/71 assertions. `Main` also built in Debug with the
  existing missing-override warnings and macOS JPEG deployment-version linker
  warning. `git diff --check` completed with no output.

## Limitations

- Interactive Print Screen JPEG capture and the `MU_CAPTURE_FRAME` runtime PPM
  hook were not launched during this non-interactive verification pass.
  Orientation and channel conversion are covered by the focused unit tests, but
  PPM file output and JPEG persistence are not directly unit-tested.
