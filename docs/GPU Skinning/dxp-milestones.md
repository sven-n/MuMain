# DXP Milestone Mapping

Upstream used `DXP-xx` labels while migrating from fixed-function OpenGL to
Core Profile, then to an RHI. MuMain preserves those capabilities in its SDL GPU
renderer instead of importing the retired upstream directories.

The table describes the downstream mapping, not the presence of the original
upstream implementation. Upstream Phase A was squash-merged, so missing DXP
tags in that source history do not prove that a capability was absent. DXP-07d
is the exception: its item-preview validation markers survive in downstream
call sites.

| Milestone | Incoming intent | Downstream SDL GPU mapping |
|---|---|---|
| DXP-01 | Shader alpha testing | Renderer alpha-test state selects shader/pipeline behavior. |
| DXP-02 | GPU item specular tiers | Skinned shader supports chrome, oil, and metal coordinate modes; CPU fallback remains available. |
| DXP-03 | Cloth and cape rendering without immediate GL | Cloth remains CPU-skinned and submits renderer-neutral triangles. |
| DXP-04 | Event fog in modern geometry | Fog values travel with deferred commands and the fragment uniform. |
| DXP-05 | Terrain pass outside fixed-function GL | Terrain uses renderer-backed geometry submission. |
| DXP-06 | Shader-driven shadows | SDL GPU shadow pipeline and shader assets replace the upstream GL shader class. |
| DXP-07a | CPU orthographic matrices | Screen-space projection is produced by the renderer. |
| DXP-07b | CPU camera matrices | Camera state and `BeginScene()` provide the 3D transform. |
| DXP-07c | Picking without GL matrix reads | Picking consumes cached CPU camera/projection state. |
| DXP-07d | Item-preview camera validation | Six downstream preview paths retain CPU projection/view validation markers before renderer submission. |
| DXP-08 / 08a | Retire fixed-function runtime behavior | SDL GPU is mandatory; compatibility names do not imply a GL context. |
| DXP-09 | Primitive decomposition and pipeline hygiene | Renderer APIs submit triangles, indexed quads, strips, and lines explicitly. |
| DXP-10 | Graphics-call monopoly | `tools/check_gl_wrapper_monopoly.py` is a build guard. |
| DXP-11 | Rendering abstraction | `IMuRenderer` is the stable boundary; SDL GPU is the active backend. |
| DXP-12 | Texture management and readback | SDL GPU texture registry, queued updates, and request/consume color readback. |
| DXP-13 | Device and windowing handoff | Renderer owns the SDL GPU device, swapchain, command buffers, and resize resources. |
| DXP-14 | Shader and constant-layout parity | HLSL layouts are validated against compiled SPIR-V/MSL bindings and C++ size assertions. |
| DXP-15 | Modern 2D pipeline | `RenderQuad2D`, text engine accessors, blend state, and deferred indexed quads. |
| DXP-16 | Modern world pipelines | BMD, terrain, effects, sprites, and shadows submit through renderer pipelines. |
| DXP-17 | Clip-space projection alignment | Partially mapped: CPU projection, zero-to-one viewport depth, and fog are active; depth-pixel readback and gamma remain unaudited. |
| DXP-20 | GPU skeletal skinning | Rest vertices plus packed 3x4 palettes are processed by `skinned_textured.vert.hlsl`. |
| DXP-20-inc4 | Lazy CPU materialization | `TransformCheap()`, `EnsureCpuVertices()`, and `EnsureCpuNormals()` keep fallback data on demand. |
| DXP-20-inc5 | Chrome-mode correctness | `SkinningTextureCoordinates` preserves the distinct chrome, oil, and metal formulas. |
| DXP-21 | Cloth compute separation | Compute acceleration remains deferred; CPU cloth stays isolated from rigid BMD GPU skinning. |
| DXP-22 | Safe resource/state ownership | SDL GPU resources have renderer ownership and are released with dependent pipeline state. |
| DXP-23 | Render diagnostics | Existing frame and draw counters remain available through downstream profiler seams. |
| DXP-24 | Safe shadow bone behavior | Shadow-volume and other incompatible paths retain CPU-skinned geometry. |
| DXP-25 | Logic/render UI decoupling | Downstream UI state ownership and defaults remain unchanged. |
| DXP-26 | Efficient dynamic streaming | Growable per-frame scratch buffers and deferred uploads replace GL orphan-on-wrap. |
| DXP-27 | Animation cadence control | Existing animation task synchronization and frame-timing controls remain authoritative. |

## Corrected status notes

### DXP-07d

Upstream documented a six-increment shadow comparison across item-preview
cameras. Downstream retains one validation marker in each active path:

- `Scenes/SceneCommon.cpp`;
- `UI/NewUI/NewUI3DRenderMng.cpp`;
- `GameShop/NewUIInGameShop.cpp`;
- `UI/NewUI/Events/NewUIGoldBowmanLena.cpp`;
- `UI/NewUI/Events/NewUIRegistrationLuckyCoin.cpp`;
- `UI/Legacy/UIWindows.cpp`.

The upstream audit counted 17 occurrences across nine files before the SDL GPU
port. The downstream count is six occurrences across six files because the
validation result, not every archived diagnostic step, was retained.

### DXP-17

Upstream corrected this milestone from completed to partially completed. The
downstream mapping is also partial, for different architecture-specific
reasons:

- CPU projection and view construction feed SDL GPU's zero-to-one depth range;
- viewports explicitly use `min_depth=0` and `max_depth=1`;
- fog is captured per deferred command;
- `CameraProjection::TestDepthBuffer()` still reaches the default no-op
  `ReadPixels()` implementation because the SDL GPU backend has no depth-pixel
  readback override;
- no gamma-correction parity audit is claimed.

### DXP-21

No upstream compute implementation landed. Downstream likewise keeps dynamic
cloth simulation and cloth rendering on the CPU. That path remains separated
from rigid BMD GPU skinning, leaving compute acceleration deferred until a real
backend requirement justifies it.

## GPU skinning checkpoint

The #544 downstream port adds the concrete DXP-20 family behavior:

1. `BMD` retains rest positions, rest normals, and separate position/normal
   bone indices.
2. Eligible meshes submit `SkinnedVertex3D` spans and a versioned matrix
   palette.
3. The renderer records palette rows and a dedicated skinned draw command.
4. `EndFrame()` uploads the combined palette once and binds it as vertex
   storage buffer slot 0.
5. Unsupported state returns `false`; `BMD` materializes its CPU vertices and
   renders through the established fallback.

## Architecture rule

Future references to an upstream DXP milestone must name the downstream target
that carries its behavior. Do not recreate `Render/Core`, `Render/RHI`, or
`Render/Shaders` solely to match historical names.
