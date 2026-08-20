# GPU Skinning and SDL GPU Rendering

This directory documents the GPU-skinning and modern-rendering capabilities
ported from the upstream OpenGL Core Profile work into MuMain's SDL GPU
architecture.

The upstream concepts remain useful, but their original `Render/Core`,
`Render/RHI`, `Render/Shaders`, `BoneUBO`, and `ImmediateRenderer`
implementations are not part of this project. Their behavior is represented by
`IMuRenderer`, `MuRendererSDLGpu.cpp`, the legacy compatibility wrappers,
and HLSL shaders compiled for the active SDL GPU backend.

## Documents

1. [DXP milestone mapping](dxp-milestones.md) maps upstream milestone names to
   their downstream SDL GPU equivalents.
2. [GPU skinning architecture](gpu-skinning-architecture.md) describes
   rest-pose vertices, packed 3x4 bone palettes, shader inputs, palette
   deduplication, and CPU fallbacks.
3. [Core Profile migration mapping](core-profile-migration.md) explains how
   fixed-function and Core Profile concepts map to the renderer-neutral API.
4. [Immediate renderer mapping](immediate-renderer-architecture.md) documents
   the deferred SDL GPU command path replacing upstream `IR::`.
5. [Gotchas and invariants](gotchas-and-patterns.md) records safety,
   compatibility, and performance constraints.

## Architecture highlights

- BMD meshes can submit rest-pose vertices plus separate position and normal
  bone indices through `IMuRenderer::RenderSkinnedTriangles()`.
- Bone matrices use packed row-major 3x4 affine rows: 12 floats per bone.
- The SDL GPU backend records deferred draw commands, uploads per-frame vertex
  and bone data once, then replays commands with dedicated skinned pipelines.
- Palette pointer and version checks avoid duplicate palette copies while
  preventing stale reuse when a memory address is recycled.
- Mesh, chrome, oil, and metal texture-coordinate modes are generated in the
  skinned vertex shader.
- CPU skinning remains the fallback for unsupported draws. Cloth, shadow-volume,
  shadow-map, and vertex-wave paths retain their required CPU behavior.
- Legacy GL-shaped helpers remain compatibility APIs only. The build guard
  rejects raw graphics calls outside the rendering layer.

## Source map

| Concern | Files |
|---|---|
| Renderer contract | [`MuRenderer.h`](../../src/source/Render/Renderer/MuRenderer.h) |
| SDL GPU backend | [`MuRendererSDLGpu.cpp`](../../src/source/Render/Renderer/MuRendererSDLGpu.cpp) |
| BMD submission and CPU fallback | [`ZzzBMD.cpp`](../../src/source/Render/Models/ZzzBMD.cpp), [`ZzzBMD.h`](../../src/source/Render/Models/ZzzBMD.h) |
| Skinning shader | [`skinned_textured.vert.hlsl`](../../src/shaders/skinned_textured.vert.hlsl) |
| Compatibility wrappers | [`ZzzOpenglUtil.cpp`](../../src/source/Render/Textures/ZzzOpenglUtil.cpp), `Render/Renderer/GLCompatShim.cpp` |
| Guard | [`check_gl_wrapper_monopoly.py`](../../tools/check_gl_wrapper_monopoly.py) |
| Regression coverage | [`tests/render`](../../tests/render) |
