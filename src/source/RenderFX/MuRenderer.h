// MuRenderer.h: Rendering abstraction interface for the MU Online game client.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
// Story 4.4.1 — Flow Code: VS1-RENDER-TEXTURE-MIGRATE (GetDevice accessor added)
//
// IMuRenderer defines the stable rendering API surface that game code calls.
// MuRendererGL (in MuRenderer.cpp) implements it using OpenGL immediate mode.
// A future MuRendererSDLGPU will replace only MuRenderer.cpp (story 4.3.1).
//
// DESIGN CONTRACT:
//   - No OpenGL types (GLenum, GLuint, GLint, etc.) in this header.
//   - All parameters expressed in project-defined enums and plain C++ types.
//   - stdafx.h OpenGL stubs allow MuRenderer.cpp to compile on macOS/Linux.
//   - No #ifdef _WIN32 in game logic — cross-platform via stubs.
#pragma once

#include <cstdint>
#include <span>

// Story 4.4.1: Forward declaration of SDL_GPUDevice so IMuRenderer::GetDevice()
// can be declared without pulling SDL3 headers into every TU that includes MuRenderer.h.
// The returned pointer is opaque — callers cast to SDL_GPUDevice* after including SDL3 headers.
#ifdef MU_ENABLE_SDL3
struct SDL_GPUDevice;
#endif

namespace mu
{

// ---------------------------------------------------------------------------
// BlendMode: Rendering blend equation presets.
// Maps to GL blend factor pairs documented in docs/architecture-rendering.md.
// ---------------------------------------------------------------------------
enum class BlendMode : std::uint8_t
{
    Alpha,        // GL_SRC_ALPHA,          GL_ONE_MINUS_SRC_ALPHA
    Additive,     // GL_SRC_ALPHA,          GL_ONE
    Subtract,     // GL_ZERO,               GL_ONE_MINUS_SRC_COLOR
    InverseColor, // GL_ONE_MINUS_DST_COLOR, GL_ZERO
    Mixed,        // GL_ONE,                GL_ONE_MINUS_SRC_ALPHA
    LightMap,     // GL_ZERO,               GL_SRC_COLOR
    Glow,         // GL_ONE,                GL_ONE             (Story 4.2.5 — EnableAlphaBlend)
    Luminance,    // GL_ONE_MINUS_SRC_COLOR, GL_ONE            (Story 4.2.5 — EnableAlphaBlend2)
};

// ---------------------------------------------------------------------------
// FogParams: Parameters for hardware fog setup.
// mode is one of GL_LINEAR / GL_EXP / GL_EXP2 (stored as plain int so that
// MuRenderer.h does not expose GL types; MuRenderer.cpp translates to GLenum).
// ---------------------------------------------------------------------------
struct FogParams
{
    int mode; // GL_LINEAR=0x2601, GL_EXP=0x0800, GL_EXP2=0x0801
    float start;
    float end;
    float density;
    float color[4]; // RGBA (same layout as GL_FOG_COLOR float[4])
};

// ---------------------------------------------------------------------------
// Vertex2D: Screen-space quad vertex (texture-mapped 2D).
// (x, y) are final screen pixels (post-conversion). Callers convert from
// their source coordinate space (e.g., 640x480 logical) before constructing.
// ---------------------------------------------------------------------------
struct Vertex2D
{
    float x, y;          // screen position (final pixels)
    float u, v;          // texture coordinates
    std::uint32_t color; // packed ABGR (matches GL vertex colour layout)
};

// ---------------------------------------------------------------------------
// Vertex3D: World-space vertex with normals (models, effects).
// ---------------------------------------------------------------------------
struct Vertex3D
{
    float x, y, z;       // world position
    float nx, ny, nz;    // surface normal
    float u, v;          // texture coordinates
    std::uint32_t color; // packed ABGR
};

// ---------------------------------------------------------------------------
// IMuRenderer: Pure abstract rendering interface.
// Game code obtains the active backend via GetRenderer() (see below).
// ---------------------------------------------------------------------------
class IMuRenderer
{
public:
    virtual ~IMuRenderer() = default;

    // Render a screen-space textured quad (4 vertices, GL_QUADS).
    virtual void RenderQuad2D(std::span<const Vertex2D> vertices, std::uint32_t textureId) = 0;

    // Render world-space triangles (vertex count must be divisible by 3).
    virtual void RenderTriangles(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Render a quad strip from world-space vertices (requires >= 4 vertices, even count ideal).
    virtual void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Set the active alpha-blending equation.
    virtual void SetBlendMode(BlendMode mode) = 0;

    // Disable alpha blending entirely (glDisable(GL_BLEND) path).
    // Distinct from SetBlendMode — "no blending" is a render state, not a blend mode.
    virtual void DisableBlend() = 0;

    // Enable or disable depth testing (glDepthFunc defaults to GL_LEQUAL).
    virtual void SetDepthTest(bool enabled) = 0;

    // GL state forwarding — SDL_gpu pipeline handles these per-draw.
    // Default implementations are no-ops; OpenGL backend overrides them.
    virtual void SetDepthMask(bool /*enabled*/) {}
    virtual void SetCullFace(bool /*enabled*/) {}
    virtual void SetAlphaTest(bool /*enabled*/) {}
    virtual void SetTexture2D(bool /*enabled*/) {}
    virtual void SetFogEnabled(bool /*enabled*/) {}

    // Bind texture by game bitmap index. SDL_gpu resolves this to SDL_GPUTexture*
    // via the texture registry; OpenGL calls glBindTexture directly.
    virtual void BindTexture(int /*texId*/) {}

    // Configure hardware fog for the current scene.
    virtual void SetFog(const FogParams& params) = 0;

    // Story 7-9-2 (AC-1): 3D scene projection setup — replaces BeginOpengl()/EndOpengl().
    // BeginScene sets viewport and projection (perspective + modelview).
    // EndScene restores matrix state.
    virtual void BeginScene(int x, int y, int w, int h) = 0;
    virtual void EndScene() = 0;

    // Story 7-9-2 (AC-2): 2D orthographic pass — replaces BeginBitmap()/EndBitmap().
    // Begin2DPass sets up orthographic projection for screen-space rendering.
    // End2DPass restores the previous projection state.
    // NOTE: OpenGL backend disables depth test in Begin2DPass and re-enables in End2DPass
    // as a side effect. SDL_gpu backend handles depth per-pipeline; these are no-ops there.
    virtual void Begin2DPass() = 0;
    virtual void End2DPass() = 0;

    // Story 7-9-2 (AC-7): Clear the color and depth buffers.
    // OpenGL backend: glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT).
    // SDL_gpu backend: no-op (SDL_gpu clears in BeginFrame).
    virtual void ClearScreen() = 0;

    // Story 7-9-2 (AC-7): Clear depth buffer only (mid-frame depth reset).
    // Used by UI 3D render panels to clear depth before rendering 3D items on top.
    // OpenGL backend: glClear(GL_DEPTH_BUFFER_BIT).
    // SDL_gpu backend: no-op (depth handled by render pass management).
    virtual void ClearDepthBuffer() {}

    // Story 7-9-2 (AC-5): Render line primitives (GL_LINES replacement).
    // Used by debug visualizations (collision, skeleton, waypoint gizmos).
    // Vertex count should be even (pairs); odd count logs a warning, last vertex ignored.
    virtual void RenderLines(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Story 7-9-2 (AC-6): Query whether a frame is currently active.
    // OpenGL backend: always false (immediate mode, no frame lifecycle).
    // SDL_gpu backend: true when s_renderPass != nullptr.
    // Used by RenderTitleSceneUI() to self-manage BeginFrame/EndFrame.
    [[nodiscard]] virtual bool IsFrameActive() const
    {
        return false;
    }

    // Per-frame lifecycle: acquire command buffer / render pass (SDL_gpu backend).
    // No-op in the OpenGL backend. Called from MuMain.cpp game loop.
    virtual void BeginFrame() {}
    virtual void EndFrame() {}

    // Story 4.4.1 — Texture System Migration: SDL_gpu device accessor.
    // Returns the SDL_GPUDevice* used by the active backend, or nullptr if not available.
    // Default implementation returns nullptr (OpenGL backend has no SDL_GPUDevice).
    // MuRendererSDLGpu overrides to return s_device.
    // Used by GlobalBitmap.cpp to upload textures via SDL_gpu without a layering violation.
    // [[nodiscard]]: callers must check for nullptr before using the device.
#ifdef MU_ENABLE_SDL3
    [[nodiscard]] virtual void* GetDevice()
    {
        return nullptr;
    }
#endif

    // -----------------------------------------------------------------------
    // Story 7-9-6: GL state migration — replaces raw OpenGL calls.
    // Default implementations are no-ops; SDL_gpu backend overrides them.
    // -----------------------------------------------------------------------

    // AC-3: Clear color — replaces glClearColor.
    // SDL_gpu backend stores RGBA and applies in BeginFrame render pass.
    virtual void SetClearColor(float /*r*/, float /*g*/, float /*b*/, float /*a*/) {}

    // AC-4: Matrix stack — replaces glMatrixMode/glPushMatrix/glPopMatrix/etc.
    // SDL_gpu backend maintains internal MatrixStack and uploads to GPU uniform buffer.
    virtual void SetMatrixMode(int /*mode*/) {}
    virtual void PushMatrix() {}
    virtual void PopMatrix() {}
    virtual void LoadIdentity() {}
    virtual void Translate(float /*x*/, float /*y*/, float /*z*/) {}
    virtual void Rotate(float /*angle*/, float /*x*/, float /*y*/, float /*z*/) {}
    virtual void Scale(float /*x*/, float /*y*/, float /*z*/) {}
    virtual void MultMatrix(const float* /*m*/) {}
    virtual void LoadMatrix(const float* /*m*/) {}
    virtual void GetMatrix(int /*mode*/, float* /*m*/) {}

    // AC-6: Depth/stencil/state — replaces glDepthFunc/glAlphaFunc/glStencilFunc/etc.
    virtual void SetStencilTest(bool /*enabled*/) {}
    virtual void SetDepthFunc(int /*func*/) {}
    virtual void SetAlphaFunc(int /*func*/, float /*ref*/) {}
    virtual void SetStencilFunc(int /*func*/, int /*ref*/, unsigned int /*mask*/) {}
    virtual void SetStencilOp(int /*sfail*/, int /*dpfail*/, int /*dppass*/) {}
    virtual void SetColorMask(bool /*r*/, bool /*g*/, bool /*b*/, bool /*a*/) {}

    // Additional state — replaces glPolygonMode/glFrontFace.
    virtual void SetPolygonMode(int /*face*/, int /*mode*/) {}
    virtual void SetFrontFace(int /*mode*/) {}
    virtual void SetMultisample(bool /*enabled*/) {}

    // Texture environment — replaces glTexEnvi/glTexEnvf (fixed-function, no-op in SDL GPU).
    virtual void SetTexEnv(int /*target*/, int /*pname*/, int /*param*/) {}
    virtual void SetTexParameter(int /*target*/, int /*pname*/, int /*param*/) {}

    // AC-6: Viewport/scissor — replaces glViewport/glScissor.
    virtual void SetViewport(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
    virtual void SetScissor(int /*x*/, int /*y*/, int /*w*/, int /*h*/) {}
    virtual void SetScissorEnabled(bool /*enabled*/) {}

    // AC-7: Screenshot — replaces glReadPixels.
    // SDL_gpu backend: SDL_GPUDownloadFromGPUTexture or SDL_RenderReadPixels.
    virtual void ReadPixels(int /*x*/, int /*y*/, int /*w*/, int /*h*/, void* /*data*/) {}

    // [Story 7-6-7: AC-3] GPU backend driver name for error reporting.
    // Returns "unknown" by default; SDL GPU backend overrides with SDL_GetGPUDeviceDriver().
    [[nodiscard]] virtual const char* GetGPUDriverName() const
    {
        return "unknown";
    }
};

// ---------------------------------------------------------------------------
// GetRenderer: Singleton accessor — returns the active rendering backend.
// Initially returns MuRendererGL; will return MuRendererSDLGPU after 4.3.1.
// ---------------------------------------------------------------------------
[[nodiscard]] IMuRenderer& GetRenderer();

} // namespace mu
