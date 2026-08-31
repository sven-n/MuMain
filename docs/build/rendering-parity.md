# Rendering parity

The client uses one SDL GPU rendering policy on every desktop platform. SDL
selects the native GPU driver; game rendering code does not select a backend or
change performance behavior by OS.

| OS | SDL GPU driver | Application policy |
|----|----------------|--------------------|
| macOS | Metal | Identical |
| Linux | Vulkan | Identical |
| Windows | D3D12 | Identical |

Native drivers, GPUs, and font rasterizers can still expose platform-specific
driver defects. Runtime parity therefore requires measurements on each target;
a successful build or a macOS run is not evidence for Linux or Windows.

## Build policy

- Debug builds enable SDL GPU validation.
- Builds with `NDEBUG` disable SDL GPU validation.
- The policy is fixed in code. There is no OS branch or runtime override.
- GPU skinning shaders and pipelines are required. An eligible submission
  failure is counted and rejected; it does not silently switch to CPU skinning.

## Packaged font roles

Release builds resolve every text role from files beside the executable:

| Role | Family/file |
|------|-------------|
| Normal | Selected family regular file |
| Bold | Selected family bold file |
| Big bold | Selected family bold file |
| Fixed | `fonts/Cousine-Regular.ttf` |

Selectable families are DejaVu Sans and Liberation Sans. Empty or unknown
configuration selects DejaVu Sans. SDL_ttf, Windows GDI, and the non-Windows
GDI shim use the same registry.

Missing or corrupt packaged roles abort Release renderer startup. Windows also
requires private GDI registration of every packaged role; partial registration
is rolled back.

Debug builds may use system fonts only when a packaged role is unavailable.
Logs mark this path as `NON-PARITY developer font fallback`. This is the sole
documented platform-dependent font exception. Release packages may not use it.

## SDL_ttf patch policy

SDL_ttf is pinned to revision
`a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b` from the 3.2.2 line. Upstream
`main` was checked at `a42434b8c96daaf7650dbd0befe480c090d1c2eb`; it did
not contain equivalent missing-glyph upload batching.

The local platform-neutral patch
`cmake/patches/sdl_ttf-3.2.2-batch-glyph-uploads.patch` batches one text
update's missing glyphs into one copy pass and command submission. It exposes
the upload count through the custom text property
`MuMain.SDL_ttf.gpu_text.uploaded_glyphs`. The client consumes that property
once and reports it as `GlyphUploads`.

## Capture procedure

Use Release builds for platform comparisons. Keep these inputs identical:

- revision;
- account and character;
- scene and visible objects;
- camera position and direction;
- resolution and window mode;
- VSync and frame-limit settings;
- editor enabled/disabled state;
- warm-up duration.

On Linux, run from the packaged runtime directory:

```bash
MU_RENDER_TIMING=1 ./Main
```

On macOS, use:

```bash
MU_RENDER_TIMING=1 ./Main.app/Contents/MacOS/Main
```

On Windows PowerShell, use:

```powershell
$env:MU_RENDER_TIMING = "1"
.\Main.exe
```

To isolate D3D12 non-indexed triangle batching, rerun with:

```powershell
$env:MU_D3D12_DISABLE_TRIANGLE_MERGING = "1"
.\Main.exe
```

Then:

1. Enable `$glstats on`.
2. Load the agreed scene and camera.
3. Warm recurring labels and visible assets until steady state.
4. Confirm validation state and bundled font paths in the log.
5. Capture 300 consecutive frames. The 60-frame diagnostics provide five
   samples.
6. Record requested/submitted draws, actual binds and uniform pushes, 2D
   merges, skinning paths, glyph uploads, replay time, submit time, and total
   frame time.

Recurring labels should report zero glyph uploads after warm-up. Failed GPU
skinning should remain zero. Submitted 2D draws and actual state applications
should be lower than requested draws in scenes with adjacent compatible work.

`$glstats` displays:

```text
Bind Pipe:<pipeline> Samp:<sampler> VU:<vertex> FU:<fragment>
2D Merge:<count> Glyph upload:<count>
Skin GPU:<submitted> CPU-ineligible:<cpu> Failed:<failed>
```

## Native verification record

Do not replace pending rows with build-only results.

| Date | Revision | OS | GPU | Driver | Resolution | Validation | Requested/submitted draws | Pipeline/sampler binds | Vertex/fragment pushes | 2D merges | GPU/CPU/failed skinning | Glyph uploads after warm-up | Replay/submit/frame ms | Result |
|------|----------|----|-----|--------|------------|------------|---------------------------|------------------------|------------------------|-----------|--------------------------|----------------------------|------------------------|--------|
| pending | pending | macOS / Metal | pending | pending | pending | disabled | pending | pending | pending | pending | pending | pending | pending | runtime evidence pending |
| pending | pending | Linux / Vulkan | pending | pending | pending | disabled | pending | pending | pending | pending | pending | pending | pending | runtime evidence pending |
| pending | pending | Windows / D3D12 | pending | pending | pending | disabled | pending | pending | pending | pending | pending | pending | pending | runtime evidence pending |

## Windows D3D12 investigation handoff (2026-08-30)

Windows is the authoritative environment for the next investigation session.
macOS currently has the desired UI scaling behavior. Linux runtime behavior has
not been tested.

### Confirmed from source

- Chat commands, including `$vsync on` and `$vsync off`, run from UI processing
  inside `RenderScene()`, after `BeginFrame()` and before `EndFrame()`.
- `MuRendererSDLGpu::SetVSyncEnabled()` immediately calls
  `SDL_SetGPUSwapchainParameters()`.
- SDL's D3D12 implementation waits for the device, destroys the current
  swapchain, and recreates it when the present mode changes. Doing this while
  the current frame owns an acquired command buffer and swapchain texture can
  invalidate active frame state. This is a credible root cause for the reported
  `$vsync off` crash, but still requires Windows runtime confirmation.
- A claimed SDL GPU window starts in VSync mode. Successful startup VSync
  currently disables software pacing with `SetTargetFps(-1)`.
- `$glstats` displays `SDL_GetGPUDeviceDriver()`. `$details` does not display the
  active driver.
- The renderer records draws into `std::vector<RenderCmd>` and replays them in
  `EndFrame()`. Each command snapshots matrices, fog, skinning, pipeline,
  texture, sampler, and fixed render state. The cost is application-side and is
  therefore affected by unoptimized Debug code and MSVC checked iterators.

### Reported runtime behavior — not yet reproduced in this workspace

- Windows D3D12 does not reliably follow the display refresh cap while VSync is
  enabled. Release can run too fast, with visible animation errors.
- `$vsync off` can crash on Windows D3D12.
- Windows Debug performance is substantially worse than the pre-SDL renderer.
- Windows UI scaling differs from the desired macOS result.
- The rendered cursor and inventory/storage hit testing are offset: item hover
  effects appear down and right of the pointer, leaving some item cells
  difficult or impossible to reach.

### Input-scaling boundaries to test

The current SDL path stores mouse events in window coordinates, converts them
through `ScreenOverlayTransform(WindowWidth, WindowHeight)`, and temporarily
recomputes `MouseX`/`MouseY` for panel-specific transforms. The window is
created with `SDL_WINDOW_HIGH_PIXEL_DENSITY`; rendering also uses swapchain
pixel dimensions. The Windows-only offset may therefore be caused by one of
these unverified mismatches:

- SDL window coordinates versus swapchain pixel coordinates under Windows DPI
  scaling;
- a panel rendered with one transform while its hover region uses another;
- stale logical dimensions after a DPI, fullscreen, or display change.

Do not patch any one boundary until diagnostics identify the first divergent
coordinate pair.

### Next Windows session

1. Record revision, Windows version, GPU, graphics-driver version, display
   resolution, refresh rate, Windows scaling percentage, window mode, and
   configured game resolution.
2. Capture the same stationary scene in Debug and Release with:

   ```powershell
   $env:SDL_GPU_DRIVER = "direct3d12"
   $env:MU_RENDER_TIMING = "1"
   .\Main.exe
   ```

   Enable `$details on` and `$glstats on`, warm the scene, then retain at least
   five 60-frame timing samples.
3. Repeat Debug with
   `MU_D3D12_DISABLE_TRIANGLE_MERGING=1`. Compare replay, submit, total-frame,
   command, requested-draw, and submitted-draw values. This tests the deferred
   command/merge hypothesis instead of assuming Debug slowness is unavoidable.
4. Reproduce inventory and storage hover offsets with
   `MU_INPUT_DIAGNOSTICS=1`. Test Windows scaling at 100% and the affected
   scaling percentage; capture pointer location, rendered item cell, logical
   mouse coordinates, window size, and pixel size.
5. Trace the first transform where rendered and hit-test coordinates diverge.
   Fix that boundary only; preserve the working macOS behavior.
6. Move present-mode changes to a safe frame boundary. Before choosing an
   uncapped mode, query support and evaluate MAILBOX before IMMEDIATE. Measure
   whether D3D12 also requires software pacing while VSync is requested.
7. Add the active SDL GPU driver to `$details`; retain the existing `$glstats`
   line.
8. Re-run Debug/Release captures, `$vsync on`/`$vsync off` transitions,
   inventory/storage hover tests, and full CTest. Record results in the native
   verification table above.
9. Test Linux/Vulkan after Windows D3D12 and input parity are closed.

## Existing CI evidence

GitHub Actions run `32982393578` proved a native Windows x64 Release,
editor-OFF build, tests, the exact 18-shader manifest, and artifact upload.
It did not launch or profile the client.

Job `98228153450` failed after publishing because it queried upstream PR 575
under the fork and received HTTP 404. That post-publish failure is unrelated to
client rendering and provides no runtime performance evidence.
