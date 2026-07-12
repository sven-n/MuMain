# Screenshot Task 4 Runtime Verification Report

## Status

Runtime capture and timing were produced from the integrated working tree at
`HEAD cd5f9d5e` plus pre-existing uncommitted renderer/timing migration
changes. They do not validate a clean commit-only snapshot. In that integrated
run, SDL GPU color screenshot readback was integration-verified on macOS; no
Task 1-3 defect was observed, and no code was changed. Legacy depth readback
remains out of scope and needs porting.

The committed Task 1-4 code structure is separately review/test verified. The
runtime evidence below is limited to the integrated worktree and does not make
performance a property of the screenshot commits alone.

## Builds And Tests

Commands:

```text
cmake --build out/build/macos-arm64 --config Debug --target Main -j 8
cmake --build out/build/macos-arm64 --config Release --target Main -j 8
ctest --test-dir out/build/macos-arm64 -C Debug --output-on-failure
```

Results:

- The commands and results below came from the integrated worktree described
  above, not a clean `cd5f9d5e` checkout.
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

Integrated-worktree runtime output included:

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
  nonblack. The red-/blue-dominance counts and their analysis command were not
  retained, so this report makes no dominance-count claim.
- The retained PPM facts can be reproduced from the local artifact with:

  ```text
  python3 -c 'from pathlib import Path; d = Path("/tmp/mu-frame.ppm").read_bytes(); p = d[d.index(b"\n255\n") + 5:]; rgb = zip(p[0::3], p[1::3], p[2::3]); n = sum(bool(r or g or b) for r, g, b in rgb); print({"rgb_bytes": len(p), "channel_min": min(p), "channel_max": max(p), "nonblack": n, "nonuniform": min(p) != max(p)})'
  ```
- Representative RGB values include orange fire `(127,79,33)`, blue scene
  geometry `(4,10,23)`, and gold lighting `(65,44,11)`.
- `sips` converted the PPM to `/tmp/mu-frame.png` at `1024x768`. Visual
  inspection confirmed an upright MU logo, readable upright disconnect dialog,
  expected top-to-bottom scene composition, orange fire, blue architecture,
  and gold lighting. This verifies orientation and red/blue channel order.

## Ordinary-Frame Timing

Launched capture-disabled from the Release executable directory in the same
integrated worktree:

```text
MU_RENDER_TIMING=1 ./Main
```

Raw frame-numbered timing samples were not retained. Accordingly, this report
makes no auditable extrema or per-frame timing claim. The observation only
demonstrates that no performance regression was observed in the tested,
capture-disabled integrated worktree; it does not measure exact `cd5f9d5e` or
attribute performance solely to the screenshot commits.

The renderer fast path checks `FrameReadbackState::IsPending()` before creating
the owned capture target or encoding a download. Without a request it retains
the ordinary `SDL_SubmitGPUCommandBuffer` path, with no screenshot allocation,
GPU copy, fence acquisition, wait, map, or RGB copy.

The native close button command returned the `MU Online` window button, the
process exited 0, and `pgrep -x Main` confirmed no remaining process.

## Limitations

- Print Screen JPEG was not exercised. macOS has no reliable Print Screen key
  mapping in this automation setup, so synthetic input would not be trustworthy.
- Legacy one-pixel depth sampling is unchanged: `CameraProjection.cpp` still
  calls the default no-op `IMuRenderer::ReadPixels()` and remains `needs-port`.
- `/tmp/mu-frame.ppm` and `/tmp/mu-frame.png` are local runtime artifacts and
  are not committed.
- A clean commit-only runtime build was not performed because this repository
  contains the larger uncommitted renderer/timing migration integration under
  audit.

## Final Checks

- `git diff --check`: exit 0, no output.
- `git diff --cached --check`: exit 0, no output.
- The staged Task 4 diff contained only this report and the six-line screenshot
  readback ledger hunk; unrelated ledger edits remained unstaged.
- Final `pgrep -x Main`: exit 1, confirming no `Main` process remains.
