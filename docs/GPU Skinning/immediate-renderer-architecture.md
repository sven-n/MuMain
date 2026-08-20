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

SDL GPU draws triangles and lines. Compatibility code expands unsupported legacy
topologies before replay:

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

## Data paths

- `Vertex2D` contains screen position, UV, and packed ABGR color.
- `Vertex3D` contains position, normal, UV, and packed ABGR color.
- `SkinnedVertex3D` adds separate position and normal bone indices.
- Quad indices and strip indices use dedicated per-frame storage.
- Bone palettes use a separate growable storage buffer.

This replaces upstream GL ring-buffer orphaning with explicit SDL GPU transfer
and device buffers.

## State ownership

Each command snapshots all state required for replay. Pipeline selection encodes
blend, culling, depth testing, and depth writes. Texture IDs resolve through the
renderer registry to an SDL GPU texture and sampler.

This avoids two common deferred-rendering bugs:

- reading mutable renderer state after a later draw changed it;
- retaining a caller-owned span after the caller's storage expired.

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
