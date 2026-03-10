// MuRenderer.h: Rendering abstraction interface for the MU Online game client.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
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
// ---------------------------------------------------------------------------
struct Vertex2D
{
    float x, y;          // screen position
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

    // Render a triangle strip from a sequence of world-space vertices.
    virtual void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;

    // Set the active alpha-blending equation.
    virtual void SetBlendMode(BlendMode mode) = 0;

    // Disable alpha blending entirely (glDisable(GL_BLEND) path).
    // Distinct from SetBlendMode — "no blending" is a render state, not a blend mode.
    virtual void DisableBlend() = 0;

    // Enable or disable depth testing (glDepthFunc defaults to GL_LEQUAL).
    virtual void SetDepthTest(bool enabled) = 0;

    // Configure hardware fog for the current scene.
    virtual void SetFog(const FogParams& params) = 0;

    // Per-frame lifecycle: acquire command buffer / render pass (SDL_gpu backend).
    // No-op in the OpenGL backend. Called from Winmain.cpp game loop.
    virtual void BeginFrame() {}
    virtual void EndFrame() {}
};

// ---------------------------------------------------------------------------
// GetRenderer: Singleton accessor — returns the active rendering backend.
// Initially returns MuRendererGL; will return MuRendererSDLGPU after 4.3.1.
// ---------------------------------------------------------------------------
[[nodiscard]] IMuRenderer& GetRenderer();

} // namespace mu
