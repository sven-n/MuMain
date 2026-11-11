# Cross-Platform Rendering Parity

## Status

Approved direction: fix every identified rendering issue in shared application
code. Platform-specific behavior is allowed only when no portable SDL API or
bundled asset can provide equivalent behavior. Every remaining exception must
be documented with its detection and verification method.

## Goal

Make the SDL GPU client use the same rendering policy on macOS, Linux, and
Windows while removing backend-sensitive per-draw work, Release validation
overhead, avoidable text work, silent GPU-skinning fallback, and host-dependent
font selection.

## Evidence

The renderer currently binds the complete graphics state for every recorded
draw. D3D12 pipeline binding marks sampler, storage, and uniform resources
dirty. Vulkan pipeline binding requests new resource and uniform descriptor
sets. Metal retains unchanged sampler and texture bindings. This creates
deterministic extra CPU and descriptor work on Windows and Linux.

The application also:

- requests SDL GPU debug mode unconditionally, including Release builds;
- records every screen-space quad and text sequence as an independent draw;
- measures each text string before SDL_ttf lays out the same string again;
- changes one shared `TTF_Text`, forcing recurring labels to rebuild engine
  draw data;
- allows the optional skinned shader or pipeline to fail, then silently runs
  the existing CPU materialization and skinning path;
- chooses the first arbitrary file in `Data/Font`, otherwise uses different
  system fonts per OS, then opens that same file for normal, bold, big, and
  fixed-width variants;
- inherits SDL_ttf's one-command-submission-per-new-glyph upload behavior.

GitHub Actions run `32982393578` checked out the same revision as this design,
`13e47a35`. Its native Windows x64 Release, editor-off job configured with
MSVC, built successfully, passed its automated tests, validated the exact
18-file shader manifest, and uploaded the runtime artifact. It did not launch
the client or collect D3D12 renderer, font, skinning, or timing diagnostics, so
it proves build/package health only.

The user-provided job `98228153450` is the run's Semantic Release job. It
published version `1.1.1`, then failed while querying
`yesid-bocanegra/MuMain/pulls/575/commits`; PR 575 belongs to the upstream
repository, so GitHub returned 404. That failure is unrelated to renderer
performance and is outside this design.

## Design Principles

- Rendering behavior lives above Metal, Vulkan, and D3D12.
- No performance fix branches on `_WIN32`, `__linux__`, or `__APPLE__`.
- Each change is isolated and measured before the next optimization.
- Existing draw order, blend order, viewport, scissor, depth, and texture
  invalidation behavior remain unchanged.
- Vendored backend changes are avoided. A third-party patch is acceptable only
  for behavior that cannot be corrected through the public SDL API.
- Release builds disable validation uniformly. Debug builds enable it uniformly.
- Runtime failure replaces silent fallback when required cross-platform GPU
  resources are unavailable.

## Shared Replay-State Cache

Extract the repeated draw-state replay block into focused helpers in the SDL
GPU renderer. Keep one render-pass-local state cache containing:

- current graphics pipeline;
- current fragment texture and sampler;
- current vertex-uniform layout and bytes;
- current fragment-uniform bytes;
- current vertex storage buffer;
- current index buffer, offset, and element size;
- current viewport and scissor.

Bind or push a value only when it differs from the cached value. Vertex-buffer
offsets normally change per draw and remain bound as required.

A real pipeline change re-applies the current scissor. An unchanged pipeline
does not. Explicit viewport and scissor commands update both SDL and the cache.
The editor overlay invalidates the entire cache because its backend can mutate
the render pass externally; viewport and scissor are then restored.

This removes redundant application calls without depending on undocumented
backend caching. D3D12 and Vulkan no longer receive forced descriptor
invalidation for consecutive draws using one pipeline. Metal also avoids
redundant pipeline and state commands.

## Release Validation Policy

Select SDL GPU debug mode from the normal build configuration:

- Debug: enabled;
- Release and other `NDEBUG` configurations: disabled.

Use one shared constant passed to `SDL_CreateGPUDevice`. Do not set backend
environment variables or native validation options in application code. SDL
continues selecting its native validation implementation when the shared flag
is enabled.

The startup log records whether GPU validation is enabled. This makes packaged
build behavior auditable without platform-specific code.

## Shared 2D Batching

Generalize existing adjacent geometry merging to accept the vertex stride and
command family. Maintain separate previous-command indices for:

- 3D triangles;
- 3D quads;
- 2D text triangles;
- 2D screen quads.

Merge only adjacent commands with contiguous vertex storage and identical
pipeline, texture, sampler, uniforms, blend, depth, cull, viewport, and scissor
state. Preserve the static quad-index capacity. State or non-draw commands
remain hard batch boundaries.

This reduces draws before any backend sees them and therefore benefits Metal,
Vulkan, and D3D12 identically.

## Text Layout and Draw-Data Reuse

Configure the SDL_ttf text object before measuring. Obtain dimensions from the
configured `TTF_Text`, then reuse the same updated object for GPU draw data.
This removes the current independent `TTF_GetStringSize` layout pass.

Retain prepared `TTF_Text` objects for recurring `(font, UTF-8 string)` pairs.
The cache owns each object so SDL_ttf glyph references and atlas UVs remain
valid. Position and color stay outside the key because the renderer already
applies them while copying draw vertices. Bound retained entries with a small
fixed ceiling and deterministic whole-cache eviction; dynamic text can rebuild
without allowing unbounded memory growth.

The cache is renderer-independent application code. Font shutdown clears it
before fonts and the text engine are destroyed.

### SDL_ttf glyph uploads

First inspect the pinned SDL_ttf revision for an upstream batch-upload fix. If
available, update to that exact release or commit and pin it. Otherwise apply a
documented FetchContent patch that records all missing-glyph copies in one GPU
copy pass and submits once per text update, not once per glyph. The patch must
use only public SDL GPU APIs and remain identical on every platform.

If SDL API alignment or lifetime requirements prevent a safe local batch, keep
the expanded warmup and retained text cache, document the upstream limitation,
link the exact SDL_ttf source revision, and expose a counter for post-warmup
glyph uploads. This is the only anticipated third-party exception.

## Required GPU Skinning

Treat `skinned_textured.vert` and every skinned pipeline variant selected by
runtime state as required renderer resources. Shader loading or pipeline
creation failure aborts renderer initialization with the exact driver, shader,
pipeline set, blend, depth, cull, and SDL error already available in the
pipeline diagnostics.

CPU skinning remains valid only when a mesh or render mode is not eligible for
the GPU path. A renderer-resource failure must not silently convert an eligible
mesh into CPU work. Add counters separating ineligible CPU skinning from
renderer submission failure so `$glstats` can prove which path is active.

## Deterministic Bundled Fonts

Replace arbitrary directory iteration and OS font preference with explicit
bundled font roles:

- normal regular face;
- bold face;
- large bold face;
- fixed-width face.

Reuse the repository's bundled-font registry and executable-relative asset
resolution. The selected user family supplies its regular and bold files.
Choose one explicitly bundled monospace file for the fixed role. All release
packages must stage these files.

System-font fallback is retained only for developer builds missing packaged
assets. It logs a warning naming the missing bundled role and chosen system
file. Because system font inventories differ by OS, this fallback is explicitly
documented as non-parity mode and must not be used by release bundles.

## Diagnostics

Extend existing renderer statistics without platform branches:

- actual pipeline binds;
- sampler binds;
- vertex and fragment uniform pushes;
- 2D merges;
- eligible GPU-skinning submissions;
- ineligible CPU-skinning draws;
- failed GPU-skinning submissions;
- post-warmup glyph uploads when observable.

`MU_RENDER_TIMING=1` and `$glstats on` remain the collection mechanisms. The
same scene and resolution must be captured on each OS.

## Error Handling

- Invalid or missing required GPU resources stop initialization.
- Cache allocation failure falls back to uncached text for that call; rendering
  continues without data loss.
- Text cache eviction destroys `TTF_Text` objects before font or engine teardown.
- Batch merging rejects incompatible command types, noncontiguous geometry,
  capacity overflow, or intervening state commands.
- External editor rendering invalidates replay state before game rendering
  resumes.
- Missing bundled fonts are prominent warnings and mark the build as
  non-parity; a system font remains a developer-only availability fallback.

## Testing

Use red-green-refactor for each isolated change:

1. Replay cache tests prove identical state is skipped, changed state binds,
   pipeline changes restore scissor, and editor invalidation forces rebinds.
2. Build-policy tests prove Debug requests validation and Release does not.
3. Topology tests prove compatible 2D commands merge, incompatible commands do
   not, and quad capacity is preserved.
4. Text tests prove one configured layout supplies measurement and draw data,
   recurring strings reuse retained `TTF_Text`, and eviction releases entries.
5. SDL_ttf dependency tests prove its revision is pinned and any local patch is
   applied exactly once.
6. Pipeline tests prove skinned shaders and pipelines are required and startup
   fails with actionable diagnostics when absent.
7. Font tests prove every role resolves to an explicit bundled file and release
   packaging contains those files.
8. Run focused tests, the editor-off and editor-on macOS builds, full CTest, and
   shader validation locally.
9. Run the same Release scene on macOS, Linux, and Windows with
   `MU_RENDER_TIMING=1` and `$glstats on`. Compare submitted draws, actual binds,
   uniform pushes, skinning paths, replay time, submit time, frame time, and
   loaded font files.

Local macOS verification cannot prove native D3D12 or Vulkan runtime behavior.
Windows and Linux runtime evidence remains required before claiming complete
cross-platform parity.

## Documentation

Add a build-facing renderer parity document containing:

- the shared policies above;
- required bundled font roles;
- Debug versus Release validation behavior;
- exact diagnostics and capture procedure;
- remaining third-party or native-backend exceptions;
- a macOS/Linux/Windows verification table with date, revision, GPU, driver,
  resolution, counters, timings, and result.

No platform exception is considered accepted unless this table names it and
shows why a shared SDL/application solution was unavailable.

## Excluded

Login, missing game assets, gameplay correctness, MSAA, renderer replacement,
and speculative backend-specific tuning are separate concerns.
