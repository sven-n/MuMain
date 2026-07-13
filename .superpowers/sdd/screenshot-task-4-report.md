# Screenshot Task 4 Runtime Verification Report

## Status

Task 4 screenshot color readback was verified from an exact committed-source
snapshot at `aec17830`, rather than from the dirty project working tree. The
source snapshot was created with `git archive aec17830` under
`/private/tmp/mu-screenshot-aec17830`; pinned SDL `d9d55367` and imgui
`21d3299e` submodule contents were copied into it. No dirty project source was
overlaid and no Git worktree was used.

CMake configure succeeded. The exact snapshot completed a full Debug build in
720 steps and a full Release build in 721 steps. Its committed Debug CTest run
passed 41/41 tests in 0.18 seconds. The current dirty repository has additional
uncommitted test registration, so its test count must not be conflated with the
snapshot result.

## Deterministic Capture

The exact Release runtime command was:

```text
env MU_CAPTURE_FRAME=120 MU_CAPTURE_PATH=/tmp/mu-frame-aec17830.ppm ./Main
```

It explicitly logged:

```text
[capture] wrote frame 120 (1024x768) to /tmp/mu-frame-aec17830.ppm
```

The resulting PPM was 2,359,312 bytes and a valid `P6` `1024x768` image. It was
converted to PNG with `sips` and visually inspected: the MU logo and
disconnected dialog were upright and readable; the RGB image had plausible
blue, orange, and gold colors; and it was nonblank and nonuniform. This
verifies the committed snapshot's color capture, dimensions, orientation, and
channel ordering.

## Runtime Cadence And Shutdown

The exact clean runtime retained frame-numbered cadence:

```text
frame=300 at 20:17:07.929
frame=600 at 20:17:12.929
```

A second capture-disabled run reported every 300 frames at exact or near
five-second intervals from frame 300 through frame 24000. This supports a
stable 60 Hz cadence. Internal CPU timing was not measured.

Both exact runs returned exit 0, and `pgrep -x Main` returned no process after
cleanup. A native System Events close attempt raced with process exit and
reported the process absent; the evidence establishes clean shutdown, but does
not establish that the attempted click caused it.

## Limitations

- Print Screen runtime coverage remains incomplete. `Main` was focused and
  macOS key code 105 was sent, but no `Screen*.jpg` appeared. Static review and
  unit coverage exist, but the user-facing JPEG path was not runtime-exercised.
- Legacy one-pixel depth sampling remains `needs-port`: `CameraProjection.cpp`
  still uses the default no-op `IMuRenderer::ReadPixels()` path.
- The whole SDL migration remains incomplete.
- `/tmp/mu-frame-aec17830.ppm` and its PNG conversion are local runtime
  artifacts and are not committed.

## Final Checks

- `git diff --check` was run after the documentation correction.
- Only this report and the Task 4 screenshot-evidence lines in the porting
  ledger are intended for staging and commit.
