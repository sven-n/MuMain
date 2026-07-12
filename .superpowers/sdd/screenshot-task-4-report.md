# Screenshot Task 4 Runtime Verification Report

## Status

SDL GPU color screenshot readback is runtime-verified on macOS. No Task 1-3
defect was found, and no code was changed. Legacy depth readback remains out of
scope and needs porting.

## Builds And Tests

Commands:

```text
cmake --build out/build/macos-arm64 --config Debug --target Main -j 8
cmake --build out/build/macos-arm64 --config Release --target Main -j 8
ctest --test-dir out/build/macos-arm64 -C Debug --output-on-failure
```

Results:

- Debug `Main`: exit 0; already current (`ninja: no work to do`).
- Release `Main`: exit 0 after a full 707-step first Release build. Existing
  missing-override warnings and the macOS deployment-version warning for
  `libturbojpeg.0.dylib` remained non-fatal.
- Full Debug CTest: 42/42 passed, 0 failed, 0.19 seconds. This includes color
  format mapping, physical capture-target dimensions, RGB conversion,
  orientation, request state, and screenshot metadata coverage.

## Deterministic Capture

Launched from `out/build/macos-arm64/src/Release`:

```text
MU_CAPTURE_FRAME=120 MU_CAPTURE_PATH=/tmp/mu-frame.ppm ./Main
```

Runtime output included:

```text
[capture] wrote frame 120 (1024x768) to /tmp/mu-frame.ppm
```

The macOS close control was clicked through System Events. `Main` exited 0,
with no crash output, and `pgrep -x Main` returned no process afterward.

Artifact facts:

- `file`: Netpbm raw pixmap, `1024 x 768`.
- Header bytes: `P6\n1024 768\n255\n`.
- Length: 2,359,312 bytes, exactly 16 header bytes plus
  `1024 * 768 * 3` RGB bytes.
- Pixel range: each channel spans 0-255; 581,423 of 786,432 pixels are
  nonblack; 9,849 pixels are red-dominant and 22,450 are blue-dominant.
- Representative RGB values include orange fire `(127,79,33)`, blue scene
  geometry `(4,10,23)`, and gold lighting `(65,44,11)`.
- `sips` converted the PPM to `/tmp/mu-frame.png` at `1024x768`. Visual
  inspection confirmed an upright MU logo, readable upright disconnect dialog,
  expected top-to-bottom scene composition, orange fire, blue architecture,
  and gold lighting. This verifies orientation and red/blue channel order.

## Ordinary-Frame Timing

Launched capture-disabled from the Release executable directory:

```text
MU_RENDER_TIMING=1 ./Main
```

The final bounded run emitted one sample per 60 frames at one-second intervals
from frame 60 through frame 780. Renderer totals stayed between 15.91 and
16.94 ms; replay was 2.10-2.57 ms and submit was 0.04-0.07 ms. Frames 300,
360, 420, 480, 540, 600, 660, 720, and 780 remained in the same range after
startup. The ordinary path therefore remained near 60 Hz / 16-17 ms.

The renderer fast path checks `FrameReadbackState::IsPending()` before creating
the owned capture target or encoding a download. Without a request it retains
the ordinary `SDL_SubmitGPUCommandBuffer` path, with no screenshot allocation,
GPU copy, fence acquisition, wait, map, or RGB copy.

The native close button command returned the `MU Online` window button, the
process exited 0, and `pgrep -x Main` confirmed no remaining process. An earlier
timing observation briefly ran near 120 Hz at 7.5-9.3 ms while the window was
in a different macOS focus/display state; the final foreground run supplied the
stable 60 Hz evidence above.

## Limitations

- Print Screen JPEG was not exercised. macOS has no reliable Print Screen key
  mapping in this automation setup, so synthetic input would not be trustworthy.
- Legacy one-pixel depth sampling is unchanged: `CameraProjection.cpp` still
  calls the default no-op `IMuRenderer::ReadPixels()` and remains `needs-port`.
- `/tmp/mu-frame.ppm` and `/tmp/mu-frame.png` are local runtime artifacts and
  are not committed.

## Final Checks

- `git diff --check`: exit 0, no output.
- `git diff --cached --check`: exit 0, no output.
- The staged diff contains only this report and the six-line screenshot
  readback ledger hunk. Existing unrelated ledger edits remain unstaged.
- Final `pgrep -x Main`: exit 1, confirming no `Main` process remains.
