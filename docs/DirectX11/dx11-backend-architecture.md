# DirectX 11 Backend Architecture

This document outlines the architecture of the DirectX 11 rendering backend implemented in the MU Online client (`Render/RHI/RHI_D3D11.cpp` and related shader modules).

## 1. The RHI (Render Hardware Interface)

To support multiple backends (OpenGL and D3D11) side-by-side during the porting and stabilization phases, the `RHI` namespace was introduced to dispatch generic rendering commands to the active backend.

```cpp
namespace RHI
{
    // E.g., RHI::Init forwards to either RHI_D3D11_Impl::Init or RHI_GL_Impl::Init
    // based on g_RenderBackend.
    bool Init(void* nativeWindowHandle, int width, int height);
    void BeginFrame();
    void EndFrame();
    // ...
}
```

The dispatching happens at runtime without virtual function overhead by maintaining a shim layer (`RHI.cpp`) that branches purely on the `g_RenderBackend` variable read at startup.

## 2. Boot Sequence and Swapchain

The D3D11 backend controls its own swapchain and device, fully independent of SDL's OpenGL context.

When `g_RenderBackend == RenderBackend::D3D11`:
1. `SDL_CreateWindow` is called **without** the `SDL_WINDOW_OPENGL` flag.
2. The HWND is extracted from the SDL Window via `SDL_PROP_WINDOW_WIN32_HWND_POINTER`.
3. `RHI::Init` is called, which allocates the `ID3D11Device`, `ID3D11DeviceContext`, and `IDXGISwapChain`.
4. `PlatformSwapBuffers` hooks into `RHI::EndFrame()` which directly calls `m_pSwapChain->Present()`.

### Swapchain Rebuilding
When the window is resized, `RHI::OnResize` is triggered. In D3D11, this explicitly flushes the pipeline, releases the render target views, and invokes `m_pSwapChain->ResizeBuffers()` before rebuilding the targets.

## 3. Shader Implementations (HLSL)

All major shaders used in the OpenGL pipeline have been hand-ported to HLSL and embedded within the C++ binaries as string literals, compiled at runtime via `D3DCompile` (`d3dcompiler.lib`).

Shaders implemented include:
- `PassthroughShader` (UI, Sprites, 2D overlays)
- `TerrainShader` (Ground meshes, water, blending)
- `PlanarShadowShader` (Shadow pass)
- `BMDMeshShader` (Static meshes, Character/Monster meshes, Chrome reflective passes)

### `BMDMeshShader` and Vertex Layouts
Instead of utilizing OpenGL's split Vertex Array Objects (geometry vs color), D3D11 utilizes interleaved layout structures (e.g. `13-float stride` for pos, uv, color, normal, bone index). 
`ZzzBMD.cpp` provides a fallback CPU staging array (`m_D3D11DynStaging`) to expand legacy 9-float GL vertex formats into the 13-float D3D11 format, seamlessly updating an `RHI::BufferUsage::Dynamic` buffer.

## 4. Constant Buffers (Cbuffers)

The D3D11 backend leverages Constant Buffers instead of Uniform Buffer Objects (UBOs). These strictly follow the byte-for-byte layout defined in GLSL `std140` to ensure variables like `u_Bones` and `u_MVP` are written seamlessly regardless of backend.

In code, this is statically asserted:
```cpp
static_assert(sizeof(BMDFlagsCB) == 176, "BMDFlagsCB must match the HLSL BMDFlags cbuffer layout byte-for-byte");
```

### Bone Palette Layout (post-GLP-11)
The bone palette is **not** an array of 4×4 matrices. GLP-11 compacted it to **3 packed `float4` affine rows per bone** — the constant `(0,0,0,1)` fourth row is never shipped to the GPU:

```hlsl
cbuffer BoneMatrices : register(b2)
{
    float4 u_Bones[600]; // 3 rows per bone, 200 bones
};
```

Row *i* of bone *b* is `u_Bones[b*3 + i]`. Both backends reconstruct position and normal via **dot products**, never a matrix cast — `r0`/`r1`/`r2` are *rows*, so `(float3x3)`/`mat3(...)` would silently yield the transpose of the intended rotation:

```hlsl
float4 r0 = u_Bones[input.a_BoneIndex * 3 + 0];
float4 r1 = u_Bones[input.a_BoneIndex * 3 + 1];
float4 r2 = u_Bones[input.a_BoneIndex * 3 + 2];
float4 p  = float4(input.a_Pos, 1.0);
float3 bonePos = float3(dot(r0, p), dot(r1, p), dot(r2, p));
```

`BMDMeshShader` (position + normal) and `PlanarShadowShader` (position only) both read this same slot-2 cbuffer, fed by the backend-agnostic `BoneUBO` (`float m_BoneRows[GPU_MAX_BONES * 12]`). Changing this layout is a change to **both** HLSL shaders and the GLSL twins simultaneously — see [Gotchas §5.1](gotchas-and-patterns.md).

## 5. Blend & Pipeline States

Instead of raw `glEnable` / `glDisable` toggles, D3D11 bundles blending, culling, and depth write settings into an `RHI::BlendMode` atomic state object. The `ZzzOpenglUtil.cpp` functions (like `EnableAlphaBlend`, `EnableCullFace`) detect D3D11 and forward these updates to `RHI::SetBlendMode()`, caching states to minimize context swaps.

All **eight** blend combos are implemented in `RHI_D3D11.cpp`'s `kBlendCombos` table and must each have a corresponding D3D11 branch in their `ZzzOpenglUtil.cpp` wrapper:

| `RHI::BlendMode` | Wrapper | GL `glBlendFunc` equivalent |
|---|---|---|
| `Opaque` | `DisableAlphaBlend()` | (blend disabled) |
| `LightMap` | `EnableLightMap()` | `ZERO, SRC_COLOR` |
| `AlphaTest` | `EnableAlphaTest()` | `SRC_ALPHA, INV_SRC_ALPHA` |
| `Additive` | `EnableAlphaBlend()` | `ONE, ONE` |
| `Minus` | `EnableAlphaBlendMinus()` | `ZERO, INV_SRC_COLOR` |
| `Blend2` | `EnableAlphaBlend2()` | `INV_SRC_COLOR, ONE` |
| `Blend3` | `EnableAlphaBlend3()` | `SRC_ALPHA, INV_SRC_ALPHA` |
| `Blend4` | `EnableAlphaBlend4()` | `ONE, INV_SRC_COLOR` |

Two constraints apply to every one of these branches — both were violated in shipped code and are documented in full at [Gotchas §4.1 and §4.3](gotchas-and-patterns.md):
1. **The branch must exist.** A missing D3D11 branch falls through to a `glBlendFunc` no-op and leaves the GPU on the previous blend state (silent, not a crash).
2. **It must dirty-check, then `IR::Flush()`, then set state.** `RHI::SetBlendMode()` is an immediate context bind, while `IR::` defers its draws — an unflushed batch would be submitted under the new blend mode.

D3D11's alpha-channel blend slots (`SrcBlendAlpha`/`DestBlendAlpha`) reject the `*_COLOR` factor family outright, so `ToAlphaBlendFactor()` maps each RGB-space factor to its alpha-space equivalent. This is mathematically identical to GL, where a single `glBlendFunc` factor applies component-wise to all four channels.
