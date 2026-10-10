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

## RmlUi multisampling

RmlUi UI layers default to one sample (`RMLUI_SDL_GPU_NUM_MSAA_SAMPLES=1`)
on every SDL GPU driver. This disables UI MSAA and avoids the multisampled
layer/resolve path implicated in reported NVIDIA/Vulkan device loss shortly
after startup. The reported failure occurs on the first UI frames, with Windows
event 153 from `nvlddmkm` and subsequent texture-upload failures ending in
`LoadBitmap Failed`. The same build with one sample runs successfully.

This is a compatibility mitigation. The underlying driver, SDL, or RmlUi
backend fault remains unisolated; it is independent of the swapchain-format
issue. UI layers, clipping, and filters remain enabled, but UI edges no longer
receive multisample anti-aliasing.

For diagnostic builds, opt into MSAA through CMake, for example:

```powershell
cmake --preset windows-x64 -DRMLUI_SDL_GPU_NUM_MSAA_SAMPLES=2
cmake --build --preset windows-x64-release
```

Supported requests are `1`, `2`, `4`, and `8`. For multisampled requests the
backend chooses a supported sample count for both color and stencil, preferring
more samples when the exact count is unavailable. Restore `1` and rebuild to
return to the default behavior. Record the sample-count setting alongside the
inputs below when comparing native runs.

## Packaged font roles

Release builds resolve every text role from files beside the executable:

| Role | Family/file |
|------|-------------|
| Normal | Selected family regular file |
| Bold | Selected family bold file |
| Big bold | Selected family bold file |
| Fixed | `fonts/Cousine-Regular.ttf` |
| Missing glyph fallback | `fonts/NanumGothic-Regular.ttf` |

Selectable families are DejaVu Sans and Liberation Sans. Empty or unknown
configuration selects DejaVu Sans. SDL_ttf, Windows GDI, and the non-Windows
GDI shim use the same registry.

SDL_ttf attaches Nanum Gothic to every role at startup. This preserves the
selected Latin family while rendering Hangul labels instead of missing-glyph
boxes; bold roles use SDL_ttf's synthetic bold style for the fallback face.

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
