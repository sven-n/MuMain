# Task 3 Report: Deferred Screenshot Capture

## Implementation

- Added `ScreenshotCaptureState` as a metadata-only one-request state helper.
- Replaced the synchronous scene screenshot readback with deferred
  `RequestFramePixels()` / `ConsumeFramePixels()` handling.
- Preserved JPEG quality 100, physical readback dimensions, filename, and
  `GrabScreen` modulo 10000.
- Deferred the success notification until `WriteJpeg` succeeds. Renderer,
  pixel-validation, and JPEG failures do not report success.
- Removed the obsolete Shift/message policy from `ScreenshotCaptureState`; the
  helper now stores only metadata needed by every pending capture.
- Kept the existing counter policy: a consumed, valid frame advances
  `GrabScreen` after the JPEG write attempt, even when that attempt fails.
- Restored non-Windows `MU_CAPTURE_FRAME` capture with a request before
  `BeginFrame()` for the numbered frame and result consumption after its
  `EndFrame()`, with direct top-down RGB output to a P6 PPM.
- Added a pure diagnostic frame schedule with focused tests for one-based frame
  numbering, pre-frame request timing, post-frame consumption, and request-once
  behavior.
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
- Important-finding RED: the focused build failed with the expected
  `ScreenshotCaptureState::Begin` arity errors after tests removed the dead
  message-policy argument.
- Important-finding GREEN: the focused test binary passed 18/18 test cases and
  65/65 assertions. Debug `Main` built successfully with the existing warnings
  noted above, and `git diff --check` completed with no output.
- Final-review Important issue 1 RED: the focused target failed during CMake
  generation because `DiagnosticFrameCaptureSchedule.cpp` did not yet exist.
- Final-review Important issue 1 GREEN: the focused binary passed 20/20 test
  cases and 75/75 assertions; the two registered diagnostic scheduling tests
  passed through CTest, Debug `Main` linked successfully, and full Debug CTest
  passed 44/44 tests.

## Limitations

- Interactive Print Screen JPEG capture and the `MU_CAPTURE_FRAME` runtime PPM
  hook were not launched during this non-interactive verification pass.
  Orientation and channel conversion are covered by the focused unit tests, but
  PPM file output and JPEG persistence are not directly unit-tested.
- The prior `MU_CAPTURE_FRAME=120` runtime artifact was produced by the old
  after-`EndFrame()` request hook and therefore captured frame 121. That runtime
  evidence is superseded and must be regenerated with the corrected lifecycle.
