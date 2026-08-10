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

The D3D11 backend leverages Constant Buffers instead of Uniform Buffer Objects (UBOs). These strictly follow the byte-for-byte layout defined in GLSL `std140` to ensure variables like `u_Bones[200]` and `u_MVP` are written seamlessly regardless of backend.

In code, this is statically asserted:
```cpp
static_assert(sizeof(BMDFlagsCB) == 176, "BMDFlagsCB must match the HLSL BMDFlags cbuffer layout byte-for-byte");
```

## 5. Blend & Pipeline States

Instead of raw `glEnable` / `glDisable` toggles, D3D11 bundles blending, culling, and depth write settings into an `RHI::BlendMode` atomic state object. The `ZzzOpenglUtil.cpp` functions (like `EnableAlphaBlend`, `EnableCullFace`) detect D3D11 and forward these updates to `RHI::SetBlendMode()`, caching states to minimize context swaps.
