# Core Profile Migration Mapped to SDL GPU

Upstream retired fixed-function OpenGL by moving to OpenGL Core Profile, UBOs,
an `IR::` immediate renderer, and an RHI. MuMain reaches the same product goal
through SDL GPU. This document maps the incoming concepts to the active
downstream architecture.

## Fixed-function replacement

| Legacy or upstream concept | Downstream equivalent |
|---|---|
| `glBegin` / `glEnd` geometry | `IMuRenderer::RenderQuad2D`, `RenderTriangles`, `RenderQuadStrip`, and `RenderLines` |
| Driver matrix stacks | CPU camera/projection math plus renderer-owned MVP state |
| `GlobalUBO`, `SceneUBO` | Per-command vertex/fog uniform structs pushed through SDL GPU |
| `BoneUBO` | Growable per-frame SDL GPU storage buffer containing packed 3x4 rows |
| `ImmediateRenderer` / `RHI` | `IMuRenderer` contract plus the deferred command queue in `MuRendererSDLGpu.cpp` |
| GLSL Core Profile shaders | HLSL sources compiled to SPIR-V and MSL; DXIL remains the repository's current stub strategy |
| GL texture objects | Renderer texture registry, SDL GPU textures, samplers, and queued updates |
| Synchronous readback | Request/consume frame-pixel API backed by an asynchronous SDL GPU transfer |

The historical names `BeginBitmap`, `BeginOpengl`, and
`ZzzOpenglUtil` remain for call-site compatibility. They do not require a
normal runtime OpenGL context.

## Matrices and passes

- `CameraProjection.cpp` and the camera state build projection and view data on
  the CPU. Picking therefore does not depend on driver matrix readback.
- `IMuRenderer::BeginScene()` establishes the 3D viewport and projection.
- `Begin2DPass()` marks screen-space rendering. The SDL GPU backend selects
  depth behavior through pipelines rather than mutating a global GL state.
- Every deferred draw stores the MVP and fog values it needs. Later state
  changes cannot retroactively alter an already recorded command.

## Primitive conversion

SDL GPU pipelines consume triangles or lines. Compatibility helpers convert
legacy shapes before submission:

- A quad becomes triangles `[v0, v1, v2]` and `[v0, v2, v3]`.
- A quad strip becomes repeated pairs `[v0, v1, v2]`, `[v1, v3, v2]`.
- Triangle and line callers submit already expanded vertices.

There is no downstream `IR::Begin()` state machine. Callers construct a
bounded span and submit it through `IMuRenderer` or an existing rendering
wrapper.

## Deferred frame flow

1. `BeginFrame()` acquires SDL GPU frame resources.
2. Draw calls append vertex bytes to growable CPU scratch storage and record a
   `RenderCmd` containing pipeline, texture, sampler, offsets, and uniforms.
3. Compatible adjacent triangle commands may merge.
4. `EndFrame()` uploads vertex, index, bone, and texture-update data.
5. Commands replay in order into the active render pass.

This preserves legacy ordering while avoiding per-call GPU uploads.

## Textures and readback

- `GlobalBitmap.cpp` owns game texture creation through the renderer's SDL GPU
  device.
- Dynamic changes use queued texture updates; draw submission resolves game
  bitmap IDs through the renderer registry.
- Screenshots and diagnostic captures call `RequestFramePixels()` and later
  `ConsumeFramePixels()`. Ordinary frames do not allocate readback resources.

## Enforcement

`tools/check_gl_wrapper_monopoly.py` runs as a build dependency. It permits
raw GL compatibility implementation only inside the render layer and rejects
new raw graphics calls in gameplay, UI, scene, and world code.

Use the renderer-neutral contract for new code. Extend a compatibility wrapper
only when an existing legacy call family must remain source-compatible.
