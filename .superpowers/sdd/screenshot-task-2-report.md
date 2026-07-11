# Task 2: SDL GPU One-Shot Download Report

## Status

Complete. Task 2 is implemented and committed. The pre-existing renderer migration work remains in the working tree and was not included in the Task 2 commit.

## Commit

- `e56b34c7 feat(render): download requested SDL GPU frames`
- Commit scope: 6 files, 247 insertions.
- `git diff HEAD^ HEAD --check`: exit 0, no output.

## Files

- Modified `src/source/Render/Renderer/MuRenderer.h`
- Modified `src/source/Render/Renderer/MuRendererSDLGpu.cpp`
- Created `src/source/Render/Renderer/SdlGpuPixelFormat.h`
- Created `src/source/Render/Renderer/SdlGpuPixelFormat.cpp`
- Modified `tests/render/CMakeLists.txt`
- Modified `tests/render/test_frame_pixel_readback.cpp`

No other file was included in the commit. `MuRenderer.h` and `MuRendererSDLGpu.cpp` still have their unrelated pre-existing migration changes as unstaged working-tree changes after the commit.

## RED

Command:

```text
cmake --build out/build/macos-arm64 --config Debug --target frame_pixel_readback_tests -j 8
```

Result: exit 1 during CMake regeneration, before compilation. Exact relevant diagnostics:

```text
Cannot find source file:
  .../src/source/Render/Renderer/SdlGpuPixelFormat.cpp
No SOURCES given to target: frame_pixel_readback_tests
CMake Generate step failed.
```

The focused target named the required direct source before the production helper existed. This verified the missing Task 2 production contract, although CMake rejected the absent direct source before the compiler could report the missing methods/header.

## GREEN And Build Results

Initial focused GREEN build:

```text
cmake --build out/build/macos-arm64 --config Debug --target frame_pixel_readback_tests -j 8
```

Result: exit 0. CMake detected `SdlGpuPixelFormat.cpp`, regenerated, compiled the helper, conversion unit, and test, then linked `frame_pixel_readback_tests`.

Required combined build after implementation:

```text
cmake --build out/build/macos-arm64 --config Debug --target Main frame_pixel_readback_tests -j 8
```

Result: exit 0. `MuRendererSDLGpu.cpp` compiled, `MuClient` and `Main` linked, and the focused target built. Existing project warnings were emitted; the linker also warned that the installed `libturbojpeg.0.dylib` targets a newer macOS version. No Task 2 error occurred.

Fresh post-commit combined build:

```text
cmake --build out/build/macos-arm64 --config Debug --target Main frame_pixel_readback_tests -j 8
```

Result: exit 0, `ninja: no work to do.`

Focused test command, run after GREEN and again after the commit:

```text
ctest --test-dir out/build/macos-arm64 -C Debug -R "frame readback|pixel readback" --output-on-failure
```

Final result: 13/13 passed, 0 failed, total time 0.07 seconds. This includes the default renderer rejection contract, all four supported SDL GPU color formats, unsupported depth format rejection, pixel conversion, row padding/orientation, and request-state cases.

## Fast-Path Evidence

The committed EndFrame integration is:

```cpp
if (s_cmdBuf && s_frameReadbackState.IsPending() && SubmitFramePixelDownload(s_cmdBuf))
{
    s_cmdBuf = nullptr;
}

if (s_cmdBuf)
{
    SDL_SubmitGPUCommandBuffer(s_cmdBuf);
    s_cmdBuf = nullptr;
}
```

For an ordinary frame, `IsPending()` is false and the original `SDL_SubmitGPUCommandBuffer` block executes unchanged. `EncodeFramePixelDownload`, transfer-buffer creation, `SDL_DownloadFromGPUTexture`, fence acquisition/wait, mapping, and RGB conversion are only called through the pending branch. Therefore the no-request path performs no readback allocation, copy, fence, wait, map, or pixel copy.

## Self-Review

- API defaults return false and preserve compatibility with minimal/non-SDL renderers.
- One static `FrameReadbackState` rejects requests while pending or completed and resets during shutdown before device release.
- Format mapping is explicit for RGBA8/BGRA8 UNORM and sRGB variants; unsupported formats fail the request.
- Row pitch is aligned to exactly 256 bytes. Transfer byte-count arithmetic is checked before multiplication and before narrowing to `Uint32`.
- The full physical swapchain region is downloaded with aligned `pixels_per_row` and converted to tightly packed RGB.
- A successful encoded download submits only through `SDL_SubmitGPUCommandBufferAndAcquireFence`; an encode failure clears the request and preserves ordinary frame submission.
- Transfer and fence handles are local to the EndFrame call. Allocation, copy-pass, fence, wait, map, and conversion failures call `Fail`; every created transfer/fence is released, and mapped memory is unmapped before release.
- Staged diff inspection showed exactly the six Task 2 files. Unrelated texture capture, timing, command merging, and lookup-cache migration hunks were excluded from the commit.

## Concerns

- No blocking Task 2 concern.
- Actual Metal/Vulkan/D3D12 screenshot orientation and channel output require the runtime capture verification planned for Task 4; Task 2 verification is compile-time/unit-level plus the full Debug link.
- The repository retains extensive unrelated uncommitted migration work, including unstaged changes in both renderer files; it was preserved intentionally.

## Independent Review Fixes

The follow-up fixes replace direct swapchain download with a portable owned color target. SDL's bundled API documentation states that acquired swapchain textures are write-only, while its validation requires a blit source to have `SDL_GPU_TEXTUREUSAGE_SAMPLER` and a blit destination to have `SDL_GPU_TEXTUREUSAGE_COLOR_TARGET`.

On a requested capture frame, `EndFrame` now:

1. Creates a physical-drawable-sized texture in the swapchain format with `SAMPLER | COLOR_TARGET` usage.
2. Renders the frame into that owned texture using the existing swapchain-compatible pipelines.
3. Blits the complete owned texture into the write-only swapchain texture for presentation.
4. Downloads only from the owned texture, submits with a fence, waits, converts to tightly packed top-down RGB, and releases the fence, transfer buffer, and owned texture.

Frames without a pending request still render directly to the swapchain and follow the ordinary submit branch. They do not create a capture texture or transfer buffer, encode a blit or download, acquire a fence, wait, map, or convert pixels.

`BeginFrame` now fails the pending request when the device/window, command buffer, vertex-transfer mapping, swapchain acquisition, or swapchain texture is unavailable. `EndFrame` fails it when no frame is active or the requested capture render pass cannot begin. The shared failure helper also releases any owned capture texture, so a failed frame cannot leave the one-request state occupied. Shutdown releases the capture texture before destroying the GPU device and then resets the request state.

`GetSdlGpuFrameCaptureTextureInfo` centralizes the portable texture metadata and cleanly rejects zero dimensions and unsupported formats. The helper preserves all four supported RGBA/BGRA UNORM and sRGB formats and the physical drawable dimensions.

### Follow-up RED/GREEN

RED command:

```text
cmake --build out/build/macos-arm64 --config Debug --target frame_pixel_readback_tests -j 8
```

Result: exit 1 with four compiler errors reporting that `mu::GetSdlGpuFrameCaptureTextureInfo` did not exist. This was the expected failure for the new readable-target contract.

After adding the helper, the same target built with exit 0 and the focused tests passed 15/15. After integrating the renderer path, this combined command built and linked both targets with exit 0:

```text
cmake --build out/build/macos-arm64 --config Debug --target Main frame_pixel_readback_tests -j 8
```

The only emitted diagnostic was the existing linker warning that the installed `libturbojpeg.0.dylib` was built for a newer macOS version.

Final focused test command:

```text
ctest --test-dir out/build/macos-arm64 -C Debug -R "frame readback|pixel readback" --output-on-failure
```

Result: exit 0, 15/15 passed, 0 failed, total test time 0.09 seconds.

Final staged whitespace verification:

```text
git diff --cached --check
```

Result: exit 0 with no output. The staged renderer diff excludes the unrelated working-tree texture capture, timing diagnostics, command merging, and texture/sampler cache changes.
