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
| Missing glyph fallbacks | `fonts/NotoSansTC-*.otf`, `fonts/NanumGothic-Regular.ttf`, `fonts/DejaVuSans*.ttf` |

Selectable families are DejaVu Sans, Liberation Sans and Noto Sans TC. Empty
or unknown configuration selects DejaVu Sans. SDL_ttf, Windows GDI, and the
non-Windows GDI shim use the same registry.

SDL_ttf attaches three fallbacks to every role at startup, tried in this order:

1. Noto Sans TC: Traditional Chinese (`zh-TW`), kana and CJK punctuation.
   Bold roles use its bold file.
2. Nanum Gothic: Hangul. Bold roles use SDL_ttf's synthetic bold style.
3. DejaVu Sans: Latin, Greek and Cyrillic letters the selected family or
   Cousine lack.

This keeps the selected family for the text it covers and draws the rest
instead of missing-glyph boxes. A test checks that every character of every
translation has a glyph with each selectable family. Japanese (`ja`) is the
known exception: some Japanese kanji forms need Noto Sans JP, and Simplified
Chinese would need Noto Sans SC.

SDL_ttf places every glyph at the ascent of the font it comes from. Each
fallback is therefore opened with the ascent of the role font it backs, so its
glyphs share that font's baseline. The layout was tuned for DejaVu Sans
(ascent 0.93 em); a selected family with a taller ascent (Noto Sans TC,
1.16 em) is opened with its ascent limited to that, so its text does not sit
lower in every text box. The font files stay unchanged; the ascent is rewritten
while SDL_ttf reads the file.

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
