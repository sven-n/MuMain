# GLP Milestone Catalog — SDL GPU Port

The upstream `GLP-xx` series optimized an OpenGL Core Profile renderer. MuMain
uses SDL GPU instead, so this catalog records where each landed behavior exists
without restoring `Render/Core`, `Render/RHI`, `Render/Shaders`, UBO rings, or
raw OpenGL timer queries.

Upstream performance captures remain historical evidence for that renderer.
They are not downstream SDL GPU measurements and must not be quoted as MuMain
results without a new like-for-like capture.

## `$glstats`

Use `$glstats on` and `$glstats off` in chat. The overlay reports:

- per-pass CPU time, recorded draw commands, merged draws, and vertex bytes;
- requested and submitted draw counts plus deferred command count;
- texture upload, creation, and release counts;
- CPU frame, replay, and submit time;
- the active SDL GPU driver.

The generic `FrameProfiler` contains no graphics API calls. SDL GPU does not
currently expose raw per-pass GPU timestamps through this overlay, so no GPU
milliseconds are claimed.

## Milestone mapping

| Milestone | Upstream intent | SDL GPU representation |
|---|---|---|
| GLP-01 | GL counters and GPU pass timers | Generic pass CPU timing plus renderer-owned draw, merge, byte, texture, replay, submit, and driver statistics. |
| GLP-03 | Stop rewriting sampler uniforms per bind | Pipelines own shader bindings; commands capture the resolved SDL GPU texture and sampler. No per-bind sampler uniform write exists. |
| GLP-04 | Stop redundant terrain water-flag uniforms | Terrain water motion is generated in CPU-side UVs and submitted as ordinary captured vertices; there are no per-tile GL water uniforms. |
| GLP-05 | Keep texture-unit state in one cache | Texture and sampler lookup caches live inside `MuRendererSDLGpu.cpp`; callers use renderer IDs, not active GL texture units. |
| GLP-06 | Avoid rebuilding mesh vertex formats per draw | SDL GPU vertex layouts are fixed when pipelines are created; BMD draws submit typed vertex spans. |
| GLP-08 | Probe renderer capabilities and expose the active backend | SDL selects a supported platform backend during device creation; logs, error reports, and `$glstats` expose `SDL_GetGPUDeviceDriver()`. `[Render] MaxGLVersion` is intentionally absent because no GL context is created. |
| GLP-09 | Replace repeated uniform-buffer orphaning | Per-command uniforms use SDL push data; vertices and palettes use explicit transfer/device buffers uploaded during `EndFrame()`. |
| GLP-10 | Avoid redundant state uploads | Compatible adjacent ordinary triangle commands merge only when captured pipeline, texture, sampler, MVP, fog, and contiguous vertex state match. |
| GLP-11 | Pack bones as three affine rows | `SkinningParameters` and the storage buffer use exactly 12 floats per bone; the HLSL shader reconstructs positions and normals from three `float4` rows. |
| GLP-12 | Validate the upstream fixed bone limit | Palette size is derived from 12-float rows and validated at submission. The growable storage buffer has no 200-bone UBO ceiling. |
| GLP-16 | Collapse terrain submissions safely | Deferred adjacent command merging combines compatible terrain triangles without reordering blended overlay draws. |
| GLP-19 | Batch immediate-mode draws across calls | `RenderTriangles()` records commands and merges compatible adjacent submissions before replay. State is captured by value. |
| GLP-24 | Attribute the whole frame | `Sprites`, `Particles`, `Joints`, `Overlay`, and `Other` have explicit profiler rows; reset happens after every overlay reads the frame. |
| GLP-25 | Grow streaming storage under sustained load | CPU staging plus GPU vertex and bone buffers grow on demand before replay; recorded offsets remain valid for the frame. |
| GLP-29 | Reduce sprite and quad CPU overhead | Rotated sprites use the exact 2D form of Z-only rotation. Call sites use fixed stack arrays/spans; no four-element heap scratch buffer exists. |

## Ordering constraint

Upstream GL terrain could reorder base-plus-overlay tiles because one shader draw
handled both layers with compatible depth behavior. Downstream renders the base
and blended overlay separately, and the overlay may disable depth testing.
Reordering by texture pair can therefore change pixels.

MuMain keeps submission order. Only adjacent commands with identical captured
state merge. This retains the batching intent without changing blended terrain
semantics.

## Buffer and state lifetime

Draw methods copy caller-owned spans into renderer-owned per-frame scratch
storage. `RenderCmd` snapshots the pipeline, texture, sampler, offsets, MVP,
fog, and skinning parameters needed during replay. `EndFrame()` grows device
buffers when required, uploads accumulated data, processes queued texture
updates, then replays commands in order.

This is the SDL GPU equivalent of the upstream ring-growth, dirty-state, and
batch-key safety work. It avoids stale state without importing GL handle caches
or invalidation hooks.

## Preserved upstream findings

- The original series investigated an Intel HD 530 regression. That target was
  not confirmed before the SDL GPU port.
- Upstream GL terrain bucketing reduced one capture from 1,226 draws to 48. The
  number does not transfer downstream because MuMain keeps blended layers in
  submission order.
- Upstream cross-call batching helped UI and joints but not particles whose
  blend state changed per primitive. Downstream counters retain the pass split
  needed to remeasure that behavior.
- The Z-only sprite rotation and fixed quad scratch produced a large Debug win
  upstream but no measurable Release frame win. The algebraic and heap-removal
  improvements remain; the performance claim does not.
- The upstream audit corrected three assumptions: `FrameProfiler` already
  existed, the suspected depth readback path was dead, and UBO orphaning was a
  valid intermediate fix later superseded by ring allocation. Downstream uses
  renderer-neutral CPU timing and explicit SDL GPU buffers instead.

## Verification

- `test_frame_profiler` covers nested pass attribution and counters.
- `sdl_gpu_frame_stats` covers the console command, renderer stats API, backend
  implementation, overlay consumption, and absence of raw GL queries.
- `gpu_skinning_contract_tests` and `sdl_gpu_bone_skinning_path` cover packed
  palette layout and the shader submission path.
- `terrain_texture_uv_scale` covers per-texture terrain UV generation.
