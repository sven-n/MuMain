# DXP Milestone Mapping

Upstream used `DXP-xx` labels while migrating from fixed-function OpenGL to
Core Profile, then to an RHI. MuMain preserves those capabilities in its SDL GPU
renderer instead of importing the retired upstream directories.

The status below describes the downstream mapping, not the presence of the
original upstream implementation.

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
| DXP-08 / 08a | Retire fixed-function runtime behavior | Normal builds use SDL GPU; compatibility names do not imply a GL context. |
| DXP-09 | Primitive decomposition and pipeline hygiene | Renderer APIs submit triangles, indexed quads, strips, and lines explicitly. |
| DXP-10 | Graphics-call monopoly | `tools/check_gl_wrapper_monopoly.py` is a build guard. |
| DXP-11 | Rendering abstraction | `IMuRenderer` is the stable boundary; SDL GPU is the active backend. |
| DXP-12 | Texture management and readback | SDL GPU texture registry, queued updates, and request/consume frame readback. |
| DXP-13 | Device and windowing handoff | Renderer owns the SDL GPU device, swapchain, command buffers, and resize resources. |
| DXP-14 | Shader and constant-layout parity | HLSL layouts are validated against compiled SPIR-V/MSL bindings and C++ size assertions. |
| DXP-15 | Modern 2D pipeline | `RenderQuad2D`, text engine accessors, blend state, and deferred indexed quads. |
| DXP-16 | Modern world pipelines | BMD, terrain, effects, sprites, and shadows submit through renderer pipelines. |
| DXP-17 | Clip-space projection alignment | Shared CPU projection math feeds SDL GPU's zero-to-one depth convention. |
| DXP-20 | GPU skeletal skinning | Rest vertices plus packed 3x4 palettes are processed by `skinned_textured.vert.hlsl`. |
| DXP-20-inc4 | Lazy CPU materialization | `TransformCheap()`, `EnsureCpuVertices()`, and `EnsureCpuNormals()` keep fallback data on demand. |
| DXP-20-inc5 | Chrome-mode correctness | `SkinningTextureCoordinates` preserves the distinct chrome, oil, and metal formulas. |
| DXP-21 | Cloth compute separation | Cloth simulation and CPU skinning remain isolated from rigid BMD GPU skinning. |
| DXP-22 | Safe resource/state ownership | SDL GPU resources have renderer ownership and are released with dependent pipeline state. |
| DXP-23 | Render diagnostics | Existing frame and draw counters remain available through downstream profiler seams. |
| DXP-24 | Safe shadow bone behavior | Shadow-volume and other incompatible paths retain CPU-skinned geometry. |
| DXP-25 | Logic/render UI decoupling | Downstream UI state ownership and defaults remain unchanged. |
| DXP-26 | Efficient dynamic streaming | Growable per-frame scratch buffers and deferred uploads replace GL orphan-on-wrap. |
| DXP-27 | Animation cadence control | Existing animation task synchronization and frame-timing controls remain authoritative. |

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
