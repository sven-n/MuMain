# Immediate Renderer Capability in the SDL GPU Backend

Upstream documented an `IR::` subsystem backed by OpenGL buffers and an RHI.
MuMain does not contain that subsystem. Its capability is provided by the
renderer-neutral submission API and the deferred command queue in
`MuRendererSDLGpu.cpp`.

The filename remains unchanged so upstream links and milestone references stay
valid.

## Submission API

| Geometry | Renderer entry point |
|---|---|
| Screen-space quads and UI | `RenderQuad2D()` |
| World triangles and expanded fans | `RenderTriangles()` |
| Quad strips | `RenderQuadStrip()` |
| Debug and helper lines | `RenderLines()` |
| Rest-pose skeletal triangles | `RenderSkinnedTriangles()` |

Legacy helpers such as `RenderBitmap`, `RenderColor`, and older GL-shaped
wrappers build vertices and call these methods. New code should call
`IMuRenderer` directly when the abstraction fits.

## Primitive conversion

SDL GPU draws triangles and lines. Compatibility code expands unsupported
legacy topologies before replay:

- quad: `[v0, v1, v2]`, `[v0, v2, v3]`;
- quad strip segment: `[v0, v1, v2]`, `[v1, v3, v2]`;
- triangle fan: callers or wrappers expand around `v0`;
- lines: submitted as vertex pairs.

No global `IR::Begin()`/attribute/`IR::End()` state machine is required.

## Deferred command architecture

Draw methods do not immediately issue SDL GPU commands. They:

1. validate frame state, topology, texture, and pipeline availability;
2. copy vertices into growable per-frame CPU scratch storage;
3. select a graphics pipeline from captured blend, cull, depth-test, and
   depth-write state;
4. record a `RenderCmd` with texture, sampler, offsets, counts, MVP, and fog;
5. merge an adjacent ordinary triangle command only when all compatible state
   matches.

During `EndFrame()`, the renderer uploads the accumulated buffers, applies
queued texture updates, then replays commands in order.

The command queue never sorts by texture. This is deliberate: blended terrain
overlays and effects rely on submission order. Only adjacent commands with
identical captured state and contiguous vertex storage can merge.

## Data paths

- `Vertex2D` contains screen position, UV, and packed ABGR color: 20 bytes.
- `Vertex3D` contains position, normal, UV, and packed ABGR color: 36 bytes.
- `SkinnedVertex3D` adds separate position and normal bone indices: 44 bytes.
- Quad indices and strip indices use dedicated per-frame storage.
- Bone palettes use a separate growable storage buffer.

This replaces upstream GL ring-buffer orphaning with explicit SDL GPU transfer
and device buffers.

CPU scratch storage grows with the frame's submitted data. Vertex and bone GPU
buffers double until they can hold the pending upload, then the renderer copies
the frame once before replay. There is no fixed upstream IR ring-wrap ceiling.

The upstream audit corrected its dynamic type from `VertexStream` to
`IRVertex`, with a 36-byte float-color layout, and placed program bind caching
in `IR::Begin()` rather than `IR::End()`. Downstream has no `IRVertex` or
program-ID cache to correct: draw methods select an SDL GPU pipeline, store it
in `RenderCmd`, and replay binds that pipeline through SDL GPU.

## State ownership

Each command snapshots all state required for replay. Pipeline selection encodes
blend, culling, depth testing, and depth writes. Texture IDs resolve through the
renderer registry to an SDL GPU texture and sampler.

This avoids two common deferred-rendering bugs:

- reading mutable renderer state after a later draw changed it;
- retaining a caller-owned span after the caller's storage expired.

## Statistics

`$glstats on` enables generic pass counters and backend `RendererStats`. The
overlay shows recorded and submitted draws, adjacent merges, command and vertex
volume, texture operations, CPU replay/submit time, and the active SDL GPU
driver. `FrameProfiler` contains no raw GPU query API and reports no per-pass GPU
milliseconds.

## Upstream milestone mapping

| Upstream milestone | Downstream equivalent |
|---|---|
| DXP-03 | Cloth keeps CPU simulation and submits renderer-neutral geometry. |
| DXP-09 | Explicit topology conversion in wrappers and renderer methods. |
| DXP-11 | `IMuRenderer` replaces the upstream RHI boundary. |
| DXP-15 | 2D quads, text engine integration, texture registry, and SDL GPU pipelines. |
| DXP-22 | Renderer-owned resource lifecycle replaces GL bind-cache invalidation. |
| DXP-26 | Growable per-frame staging plus one deferred upload/replay phase. |

## Primary call sites

| Category | Typical files |
|---|---|
| 2D UI and sprites | `Render/Textures/ZzzOpenglUtil.cpp`, `Render/Sprites/GlobalBitmap.cpp`, UI modules |
| Effects | `Render/Effects/*.cpp` |
| Cloth | `Engine/Physics/PhysicsManager.cpp` |
| BMD models | `Render/Models/ZzzBMD.cpp` |
| Terrain and water | `Render/Terrain/*.cpp` |
| Renderer implementation | `Render/Renderer/MuRendererSDLGpu.cpp` |
