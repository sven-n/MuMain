// MuRenderer.cpp: OpenGL immediate-mode implementation of IMuRenderer.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
//
// MuRendererGL wraps the existing glBegin/glEnd + glVertex3f/glTexCoord2f
// patterns from ZzzOpenglUtil.cpp behind the stable IMuRenderer interface.
//
// OpenGL calls appear ONLY in this file — never in MuRenderer.h.
// stdafx.h provides inline no-op stubs for all gl* calls on non-Windows,
// so this file compiles on macOS/Linux without #ifdef _WIN32 guards.
#include "stdafx.h"
#include "MuRenderer.h"
#include "ErrorReport.h"

namespace mu
{

// ---------------------------------------------------------------------------
// MuRendererGL: Concrete OpenGL backend implementing IMuRenderer.
// ---------------------------------------------------------------------------
class MuRendererGL : public IMuRenderer
{
public:
    MuRendererGL() = default;
    ~MuRendererGL() override = default;

    // -----------------------------------------------------------------------
    // RenderQuad2D: Render a screen-space textured quad.
    // Mirrors the RenderBitmap* pattern in ZzzOpenglUtil.cpp:
    //   glBegin(GL_QUADS) → glTexCoord2f + glVertex3f per vertex → glEnd()
    // Requires exactly 4 vertices; logs an error and returns on bad input.
    // -----------------------------------------------------------------------
    void RenderQuad2D(std::span<const Vertex2D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty())
        {
            g_ErrorReport.Write(L"RENDER: MuRenderer::RenderQuad2D -- vertex buffer empty");
            return;
        }

        // NOTE for the OpenGL immediate-mode backend:
        // Texture binding is managed by the caller (via BindTexture() or DisableTexture()
        // in ZzzOpenglUtil.cpp) before calling RenderQuad2D. The textureId parameter is
        // reserved for the future SDL_gpu backend (story 4.3.1) where the backend manages
        // its own pipeline state. On this backend, textureId is informational only.
        (void)textureId;

        glBegin(GL_QUADS);
        for (const Vertex2D& v : vertices)
        {
            // Unpack ABGR: A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
            const auto a = static_cast<float>((v.color >> 24) & 0xFFu) / 255.0f;
            const auto b = static_cast<float>((v.color >> 16) & 0xFFu) / 255.0f;
            const auto g = static_cast<float>((v.color >> 8) & 0xFFu) / 255.0f;
            const auto r = static_cast<float>((v.color) & 0xFFu) / 255.0f;
            glColor4f(r, g, b, a);
            glTexCoord2f(v.u, v.v);
            glVertex3f(v.x, v.y, 0.0f);
        }
        glEnd();
    }

    // -----------------------------------------------------------------------
    // RenderTriangles: Render world-space triangles.
    // Mirrors the glDrawArrays path in ZzzBMD.cpp:
    //   glBegin(GL_TRIANGLES) → loop over vertex span → glEnd()
    // -----------------------------------------------------------------------
    void RenderTriangles(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty())
        {
            g_ErrorReport.Write(L"RENDER: MuRenderer::RenderTriangles -- vertex buffer empty");
            return;
        }

        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureId));
        glBegin(GL_TRIANGLES);
        for (const Vertex3D& v : vertices)
        {
            glTexCoord2f(v.u, v.v);
            glNormal3f(v.nx, v.ny, v.nz);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    // -----------------------------------------------------------------------
    // RenderQuadStrip: Render a triangle strip from world-space vertices.
    // Mirrors the GL_QUAD_STRIP trail paths in ZzzEffectJoint.cpp:
    //   glBegin(GL_QUAD_STRIP) → loop → glEnd()
    // -----------------------------------------------------------------------
    void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty())
        {
            g_ErrorReport.Write(L"RENDER: MuRenderer::RenderQuadStrip -- vertex buffer empty");
            return;
        }

        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(textureId));
        glBegin(GL_QUAD_STRIP);
        for (const Vertex3D& v : vertices)
        {
            glTexCoord2f(v.u, v.v);
            glNormal3f(v.nx, v.ny, v.nz);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    // -----------------------------------------------------------------------
    // SetBlendMode: Translate BlendMode enum to GL blend factor pairs.
    // Factor table from docs/architecture-rendering.md § Blend Modes and
    // ZzzOpenglUtil.cpp Enable*Blend functions.
    // -----------------------------------------------------------------------
    void SetBlendMode(BlendMode mode) override
    {
        glEnable(GL_BLEND);
        switch (mode)
        {
        case BlendMode::Alpha:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::Additive:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendMode::Subtract:
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
            break;
        case BlendMode::InverseColor:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            break;
        case BlendMode::Mixed:
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::LightMap:
            glBlendFunc(GL_ZERO, GL_SRC_COLOR);
            break;
        default:
            g_ErrorReport.Write(L"RENDER: MuRenderer::SetBlendMode -- unknown blend mode %d", static_cast<int>(mode));
            break;
        }
    }

    // -----------------------------------------------------------------------
    // SetDepthTest: Enable or disable depth testing.
    // Uses GL_LEQUAL as the depth function (matches existing ZzzOpenglUtil.cpp
    // depth test setup patterns).
    // -----------------------------------------------------------------------
    void SetDepthTest(bool enabled) override
    {
        if (enabled)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }

    // -----------------------------------------------------------------------
    // SetFog: Configure hardware fog parameters.
    // Mirrors the fog setup in ZzzOpenglUtil.cpp and GMBattleCastle.cpp:
    //   glFogi(GL_FOG_MODE, ...) + glFogf(GL_FOG_DENSITY/START/END, ...)
    //   + glFogfv(GL_FOG_COLOR, ...)
    // FogParams::mode is stored as an int holding GL_LINEAR/GL_EXP/GL_EXP2.
    // -----------------------------------------------------------------------
    void SetFog(const FogParams& params) override
    {
        if (params.mode != GL_LINEAR && params.mode != GL_EXP && params.mode != GL_EXP2)
        {
            g_ErrorReport.Write(L"RENDER: MuRenderer::SetFog -- unsupported fog mode %d", params.mode);
            return;
        }

        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, params.mode);
        glFogf(GL_FOG_DENSITY, params.density);
        glFogf(GL_FOG_START, params.start);
        glFogf(GL_FOG_END, params.end);
        glFogfv(GL_FOG_COLOR, params.color);
    }
};

// ---------------------------------------------------------------------------
// GetRenderer: Return the singleton MuRendererGL instance.
// Callers use this to obtain the active rendering backend.
// ---------------------------------------------------------------------------
IMuRenderer& GetRenderer()
{
    static MuRendererGL s_instance;
    return s_instance;
}

} // namespace mu
