# SDL GPU Screenshot Readback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore JPEG and diagnostic frame capture on the SDL GPU renderer without adding GPU copies, allocations, or fences to ordinary frames.

**Architecture:** A renderer-neutral `FramePixelReadback` unit owns RGB results, channel conversion, row orientation, and the one-request state machine. The SDL GPU backend creates a download transfer buffer only for a pending request, encodes the copy after rendering, submits with a fence, and completes the owned result. Scene code requests and consumes screenshots without seeing SDL objects.

**Tech Stack:** C++20, CMake/Ninja, doctest, SDL3 GPU, libturbojpeg, macOS Metal backend.

## Global Constraints

- Main's current folder structure wins over source-branch paths.
- Ordinary frames must continue using `SDL_SubmitGPUCommandBuffer` with no readback allocation, copy, fence, or wait.
- Only one pending or completed frame capture is allowed at a time.
- Public renderer data is tightly packed, top-down RGB with physical-pixel dimensions.
- Support `R8G8B8A8_UNORM`, `B8G8R8A8_UNORM`, and both sRGB variants.
- Unknown texture formats fail cleanly and never write corrupted image data.
- Depth-buffer sampling is out of scope for this plan.
- All GPU resources must be released before device shutdown, including failure paths.

---

### Task 1: Renderer-Neutral Pixel Conversion And Request State

**Files:**
- Create: `src/source/Render/Renderer/FramePixelReadback.h`
- Create: `src/source/Render/Renderer/FramePixelReadback.cpp`
- Create: `tests/render/CMakeLists.txt`
- Create: `tests/render/test_frame_pixel_readback.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `mu::FramePixels { std::uint32_t width; std::uint32_t height; std::vector<std::uint8_t> rgb; }`
- Produces: `mu::PixelChannelOrder::{Rgba,Bgra}`
- Produces: `bool mu::ConvertToTopDownRgb(std::span<const std::uint8_t>, std::uint32_t, std::uint32_t, std::uint32_t, PixelChannelOrder, bool, FramePixels&)`
- Produces: `mu::FrameReadbackState::{Request,IsPending,Complete,Fail,Consume,Reset}`

- [ ] **Step 1: Add failing conversion and state tests**

Create tests covering RGBA, BGRA, padded row pitch, bottom-up row reversal, unsupported buffer size, duplicate requests, completion, consumption, and failure reset. The core assertions are:

```cpp
TEST_CASE("RGBA readback converts to tightly packed top-down RGB")
{
    const std::array<std::uint8_t, 8> source{255, 0, 0, 7, 0, 255, 0, 9};
    mu::FramePixels output;
    CHECK(mu::ConvertToTopDownRgb(source, 2, 1, 8, mu::PixelChannelOrder::Rgba, false, output));
    CHECK(output.rgb == std::vector<std::uint8_t>{255, 0, 0, 0, 255, 0});
}

TEST_CASE("only one frame readback can be outstanding")
{
    mu::FrameReadbackState state;
    CHECK(state.Request());
    CHECK_FALSE(state.Request());
    CHECK(state.IsPending());
}
```

- [ ] **Step 2: Run the tests and verify RED**

Run:

```bash
cmake --build out/build/macos-arm64 --config Debug --target frame_pixel_readback_tests -j 8
```

Expected: compilation fails because `FramePixelReadback.h` and its interfaces do not exist.

- [ ] **Step 3: Implement conversion and state ownership**

Implement `ConvertToTopDownRgb` with checked byte-count arithmetic, `rowPitch >= width * 4`, explicit RGBA/BGRA channel selection, and optional row reversal. Implement the state machine with states `Idle`, `Pending`, and `Completed`; `Complete` moves a non-empty `FramePixels`, `Fail` returns to `Idle`, and `Consume` moves the result then returns to `Idle`.

- [ ] **Step 4: Run the focused tests and verify GREEN**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Debug -R "frame readback|pixel readback" --output-on-failure
```

Expected: all conversion and state tests pass.

- [ ] **Step 5: Commit the pure readback unit**

```bash
git add tests/CMakeLists.txt tests/render/CMakeLists.txt tests/render/test_frame_pixel_readback.cpp \
  src/source/Render/Renderer/FramePixelReadback.h src/source/Render/Renderer/FramePixelReadback.cpp
git commit -m "feat(render): add frame pixel readback state"
```

---

### Task 2: SDL GPU One-Shot Download

**Files:**
- Modify: `src/source/Render/Renderer/MuRenderer.h`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Create: `src/source/Render/Renderer/SdlGpuPixelFormat.h`
- Create: `src/source/Render/Renderer/SdlGpuPixelFormat.cpp`
- Modify: `tests/render/CMakeLists.txt`
- Test: `tests/render/test_frame_pixel_readback.cpp`

**Interfaces:**
- Consumes: `mu::FramePixels` and `mu::FrameReadbackState` from Task 1.
- Produces: `virtual bool IMuRenderer::RequestFramePixels()`.
- Produces: `virtual bool IMuRenderer::ConsumeFramePixels(FramePixels& pixels)`.
- Produces: `std::optional<PixelChannelOrder> mu::GetSdlGpuPixelChannelOrder(SDL_GPUTextureFormat format)`.

- [ ] **Step 1: Add failing renderer-contract tests**

Add compile-time checks using a minimal `IMuRenderer` test implementation and verify the default implementation rejects requests and has no completed image:

```cpp
mu::FramePixels pixels;
CHECK_FALSE(renderer.RequestFramePixels());
CHECK_FALSE(renderer.ConsumeFramePixels(pixels));
```

Add format-mapping tests for all four supported SDL formats through
`GetSdlGpuPixelChannelOrder`, plus one unsupported depth format. Compile
`SdlGpuPixelFormat.cpp` directly into the focused test target and link SDL3.

- [ ] **Step 2: Run focused tests and verify RED**

Run the `frame_pixel_readback_tests` target. Expected: compile failure because the renderer methods and format helper are absent.

- [ ] **Step 3: Add the renderer API and SDL request state**

Include `FramePixelReadback.h` from `MuRenderer.h` and add default virtual methods that return `false`. Implement the SDL format switch in `SdlGpuPixelFormat.cpp`. In `MuRendererSDLGpu`, override both renderer methods and back them with one static `FrameReadbackState`. Reject requests while a request or completed result exists.

- [ ] **Step 4: Encode the download only for pending requests**

After `SDL_EndGPURenderPass`, when `FrameReadbackState::IsPending()` is true:

1. Resolve the swapchain format to RGBA or BGRA.
2. Align row pitch to 256 bytes and create one `SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD` buffer.
3. Begin a copy pass and call `SDL_DownloadFromGPUTexture` for the full swapchain region with the aligned `pixels_per_row`.
4. End the copy pass.
5. Submit with `SDL_SubmitGPUCommandBufferAndAcquireFence` instead of the ordinary submit.
6. Wait for that fence, map the transfer buffer, call `ConvertToTopDownRgb`, then unmap and release the transfer buffer and fence.
7. Call `Complete` on success or `Fail` on every error path.

Keep the existing `SDL_SubmitGPUCommandBuffer` branch byte-for-byte equivalent for frames without a pending request.

- [ ] **Step 5: Add shutdown cleanup**

Call `FrameReadbackState::Reset()` before releasing the GPU device. Ensure locally owned fence and transfer handles are released in the same `EndFrame` invocation, including failed wait/map/conversion paths.

- [ ] **Step 6: Build and run focused tests**

```bash
cmake --build out/build/macos-arm64 --config Debug --target Main frame_pixel_readback_tests -j 8
ctest --test-dir out/build/macos-arm64 -C Debug -R "frame readback|pixel readback" --output-on-failure
```

Expected: build succeeds and focused tests pass.

- [ ] **Step 7: Commit the SDL GPU backend**

```bash
git add src/source/Render/Renderer/MuRenderer.h \
  src/source/Render/Renderer/MuRendererSDLGpu.cpp \
  src/source/Render/Renderer/SdlGpuPixelFormat.h src/source/Render/Renderer/SdlGpuPixelFormat.cpp \
  tests/render/CMakeLists.txt tests/render/test_frame_pixel_readback.cpp
git commit -m "feat(render): download requested SDL GPU frames"
```

---

### Task 3: Deferred JPEG Capture And Diagnostic Hook

**Files:**
- Create: `src/source/Scenes/ScreenshotCaptureState.h`
- Create: `src/source/Scenes/ScreenshotCaptureState.cpp`
- Modify: `src/source/Scenes/SceneManager.cpp`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Test: `tests/render/test_frame_pixel_readback.cpp`

**Interfaces:**
- Consumes: `IMuRenderer::RequestFramePixels()` and `ConsumeFramePixels(FramePixels&)`.
- Produces: `ScreenshotCaptureState::{Begin,HasPending,FileName,Message,IncludesMessage,Clear}`.
- Produces: deferred screenshot save flow with the existing `WriteJpeg(..., 100)` behavior.
- Produces: functional `MU_CAPTURE_FRAME` / `MU_CAPTURE_PATH` PPM verification hook on non-Windows builds.

- [ ] **Step 1: Add a failing screenshot-state test**

Add `ScreenshotCaptureState` tests proving that filename/message metadata remains pending until `Clear()`, `IncludesMessage()` preserves the Shift behavior, and a second `Begin()` is rejected. Compile `ScreenshotCaptureState.cpp` directly into the focused test target; keep file I/O outside this helper.

- [ ] **Step 2: Run the test and verify RED**

Run `frame_pixel_readback_tests`. Expected: compile failure because `ScreenshotRequestState` is absent.

- [ ] **Step 3: Migrate `SceneManager` from `ReadPixels`**

Replace `CaptureScreenshot()` with two focused functions:

- `RequestScreenshot()` generates and stores the filename/message, optionally adds the timestamp message before rendering, and calls `RequestFramePixels()`.
- `ConsumeScreenshot()` calls `ConsumeFramePixels()`, writes the JPEG using returned physical dimensions, adds the deferred no-timestamp message after capture, increments `GrabScreen`, and clears request metadata.

Call `ConsumeScreenshot()` before processing a new Print Screen edge. Remove the screenshot call to `ReadPixels`; leave the legacy method only for the separate depth-readback gap.

- [ ] **Step 4: Revive the diagnostic capture hook**

Rewrite `MaybeCaptureFrame()` to request once when its target frame is reached and consume on a later call. Write top-down RGB directly without the old OpenGL row reversal. Call it after `EndFrame()` so `MU_CAPTURE_FRAME=<N>` creates a deterministic PPM for runtime verification.

- [ ] **Step 5: Build and run all tests**

```bash
cmake --build out/build/macos-arm64 --config Debug --target Main -j 8
ctest --test-dir out/build/macos-arm64 -C Debug --output-on-failure
git diff --check
```

Expected: `Main` builds, every configured test passes, and the diff check is empty.

- [ ] **Step 6: Commit scene integration**

```bash
git add src/source/Scenes/SceneManager.cpp src/source/App/Platform/Windows/Winmain.cpp \
  src/source/Scenes/ScreenshotCaptureState.h src/source/Scenes/ScreenshotCaptureState.cpp \
  tests/render/CMakeLists.txt tests/render/test_frame_pixel_readback.cpp
git commit -m "fix(render): restore SDL GPU screenshots"
```

---

### Task 4: macOS Runtime And Performance Verification

**Files:**
- Modify: `docs/porting/cross-platform-sdl-ledger.md`

**Interfaces:**
- Consumes: completed Tasks 1-3.
- Produces: runtime evidence and updated readback ledger status.

- [ ] **Step 1: Build Debug and Release**

```bash
cmake --build out/build/macos-arm64 --config Debug --target Main -j 8
cmake --build out/build/macos-arm64 --config Release --target Main -j 8
```

Expected: both configurations build successfully.

- [ ] **Step 2: Capture a deterministic macOS frame**

From `out/build/macos-arm64/src/Release` run:

```bash
MU_CAPTURE_FRAME=120 MU_CAPTURE_PATH=/tmp/mu-frame.ppm ./Main
```

Expected: `/tmp/mu-frame.ppm` is created after frame 120 with a valid `P6` header, physical window dimensions, top-down orientation, and correct red/blue channels.

- [ ] **Step 3: Verify ordinary-frame performance and clean shutdown**

Run Release with `MU_RENDER_TIMING=1`, inspect multiple diagnostics after frame 300, close through the macOS window control, and verify exit status 0. Expected: ordinary frames remain near the 60 Hz budget and contain no readback allocation or fence diagnostics.

- [ ] **Step 4: Update the migration ledger**

Record that reconnect capture and color screenshot readback are ported, including the focused tests, PPM path, Release timing, and clean-close evidence. Keep depth sampling explicitly `needs-port`.

- [ ] **Step 5: Run final verification**

```bash
ctest --test-dir out/build/macos-arm64 -C Debug --output-on-failure
git diff --check
git status --short
```

Expected: all tests pass, diff check is empty, and only intentional migration files remain modified/untracked.

- [ ] **Step 6: Commit verification evidence**

```bash
git add docs/porting/cross-platform-sdl-ledger.md
git commit -m "docs(render): record SDL GPU readback verification"
```
