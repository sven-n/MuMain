// MuRenderer.cpp: OpenGL immediate-mode implementation of IMuRenderer.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
//
// MuRendererGL wraps the existing glBegin/glEnd + glVertex3f/glTexCoord2f
// patterns from ZzzOpenglUtil.cpp behind the stable IMuRenderer interface.
//
// OpenGL calls appear ONLY in this file — never in MuRenderer.h.
// stdafx.h provides inline no-op stubs for all gl* calls on non-Windows,
// so this file compiles on macOS/Linux without #ifdef _WIN32 guards.
//
// Story 4.3.1: This entire file is guarded under MU_USE_OPENGL_BACKEND.
// When MU_USE_OPENGL_BACKEND is OFF (the default), MuRendererSDLGpu.cpp
// provides GetRenderer() instead. The file still compiles as an empty
// translation unit when the flag is OFF.
#include "stdafx.h"
#include "MuRenderer.h"
#include "ErrorReport.h"

#ifdef MU_USE_OPENGL_BACKEND

// ZzzOpenglUtil.h include removed (story 7-9-2 review Finding 2):
// the OpenGL backend uses direct GL calls, avoiding a circular dependency
// where ZzzOpenglUtil wraps IMuRenderer but the backend called back into it.

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
    //
    // NOTE: textureId is NOT used on this backend. Texture state is managed
    // by the caller via BindTexture() / DisableTexture() in ZzzOpenglUtil.cpp
    // before calling RenderTriangles. The textureId parameter is reserved for
    // the future SDL_gpu backend (story 4.3.1) where the backend owns its own
    // pipeline state. Using textureId here would re-bind the wrong GL object
    // (BITMAP slot index ≠ GL texture object name) and corrupt chrome/metal
    // render paths where the caller binds BITMAP_CHROME before calling us.
    // -----------------------------------------------------------------------
    void RenderTriangles(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty())
        {
            g_ErrorReport.Write(L"RENDER: MuRenderer::RenderTriangles -- vertex buffer empty");
            return;
        }

        // textureId is informational on this backend — caller manages GL texture state.
        (void)textureId;

        glBegin(GL_TRIANGLES);
        for (const Vertex3D& v : vertices)
        {
            // Unpack ABGR: A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
            const auto a = static_cast<float>((v.color >> 24) & 0xFFu) / 255.0f;
            const auto b = static_cast<float>((v.color >> 16) & 0xFFu) / 255.0f;
            const auto g = static_cast<float>((v.color >> 8) & 0xFFu) / 255.0f;
            const auto r = static_cast<float>((v.color) & 0xFFu) / 255.0f;
            glColor4f(r, g, b, a);
            glTexCoord2f(v.u, v.v);
            glNormal3f(v.nx, v.ny, v.nz);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    // -----------------------------------------------------------------------
    // RenderQuadStrip: Render a quad strip from world-space vertices.
    // Mirrors the GL_QUAD_STRIP trail paths in ZzzEffectJoint.cpp:
    //   glBegin(GL_QUAD_STRIP) → loop → glEnd()
    //
    // NOTE for the OpenGL immediate-mode backend:
    // Texture binding is managed by the caller (via BindTexture() in
    // ZzzOpenglUtil.cpp) before calling RenderQuadStrip. The textureId
    // parameter is reserved for the future SDL_gpu backend (story 4.3.1).
    // On this backend, textureId=0 is the sentinel for "caller manages texture".
    // -----------------------------------------------------------------------
    void RenderQuadStrip(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty())
        {
            g_ErrorReport.Write(L"RENDER: MuRenderer::RenderQuadStrip -- vertex buffer empty");
            return;
        }

        // textureId is informational on this backend — caller manages GL texture state.
        (void)textureId;

        glBegin(GL_QUAD_STRIP);
        for (const Vertex3D& v : vertices)
        {
            // Unpack ABGR: A=bits31-24, B=bits23-16, G=bits15-8, R=bits7-0
            const auto a = static_cast<float>((v.color >> 24) & 0xFFu) / 255.0f;
            const auto b = static_cast<float>((v.color >> 16) & 0xFFu) / 255.0f;
            const auto g = static_cast<float>((v.color >> 8) & 0xFFu) / 255.0f;
            const auto r = static_cast<float>((v.color) & 0xFFu) / 255.0f;
            glColor4f(r, g, b, a);
            glTexCoord2f(v.u, v.v);
            glNormal3f(v.nx, v.ny, v.nz);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-1): BeginScene — 3D projection setup.
    // Mirrors the current BeginOpengl() body in ZzzOpenglUtil.cpp.
    // Sets viewport, perspective projection, modelview from camera state.
    // -----------------------------------------------------------------------
    void BeginScene(int x, int y, int w, int h) override
    {
        x = x * WindowWidth / 640;
        y = y * WindowHeight / 480;
        w = w * WindowWidth / 640;
        h = h * WindowHeight / 480;

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glViewport2(x, y, w, h);

        gluPerspective2(CameraFOV, static_cast<float>(w) / static_cast<float>(h), CameraViewNear, CameraViewFar * 1.4f);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glRotatef(CameraAngle[1], 0.f, 1.f, 0.f);
        if (!CameraTopViewEnable)
        {
            glRotatef(CameraAngle[0], 1.f, 0.f, 0.f);
        }
        glRotatef(CameraAngle[2], 0.f, 0.f, 1.f);
        glTranslatef(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);

        glDisable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthMask(true);
        glDepthFunc(GL_LEQUAL);
        glAlphaFunc(GL_GREATER, 0.25f);
        if (FogEnable)
        {
            glEnable(GL_FOG);
            glFogi(GL_FOG_MODE, GL_LINEAR);
            glFogf(GL_FOG_DENSITY, FogDensity);
            glFogfv(GL_FOG_COLOR, FogColor);
        }
        else
        {
            glDisable(GL_FOG);
        }

        GetOpenGLMatrix(CameraMatrix);
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-1): EndScene — restore matrix state after 3D pass.
    // Mirrors the current EndOpengl() body in ZzzOpenglUtil.cpp.
    // -----------------------------------------------------------------------
    void EndScene() override
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-2): Begin2DPass — orthographic projection setup.
    // Mirrors the current BeginBitmap() body in ZzzOpenglUtil.cpp.
    // -----------------------------------------------------------------------
    void Begin2DPass() override
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        glViewport(0, 0, WindowWidth, WindowHeight);
        gluPerspective(CameraFOV, static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight), CameraViewNear,
                       CameraViewFar);

        glLoadIdentity();
        gluOrtho2D(0, WindowWidth, 0, WindowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-2): End2DPass — restore matrix state after 2D pass.
    // Mirrors the current EndBitmap() body in ZzzOpenglUtil.cpp.
    // -----------------------------------------------------------------------
    void End2DPass() override
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-7): ClearScreen — clear color and depth buffers.
    // -----------------------------------------------------------------------
    void ClearScreen() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Story 7-9-2 (AC-7): ClearDepthBuffer — depth-only clear for UI 3D panels.
    void ClearDepthBuffer() override
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    // -----------------------------------------------------------------------
    // Story 7-9-2 (AC-5): RenderLines — line primitive rendering.
    // Replaces GL_LINES blocks in debug visualization code.
    // -----------------------------------------------------------------------
    void RenderLines(std::span<const Vertex3D> vertices, std::uint32_t textureId) override
    {
        if (vertices.empty())
        {
            return;
        }

        (void)textureId;

        glBegin(GL_LINES);
        for (const Vertex3D& v : vertices)
        {
            const auto a = static_cast<float>((v.color >> 24) & 0xFFu) / 255.0f;
            const auto b = static_cast<float>((v.color >> 16) & 0xFFu) / 255.0f;
            const auto g = static_cast<float>((v.color >> 8) & 0xFFu) / 255.0f;
            const auto r = static_cast<float>((v.color) & 0xFFu) / 255.0f;
            glColor4f(r, g, b, a);
            glTexCoord2f(v.u, v.v);
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
        case BlendMode::Glow:
            // GL_ONE, GL_ONE — bright additive compositing (EnableAlphaBlend in ZzzOpenglUtil).
            // Story 4.2.5: added to cover the EnableAlphaBlend() helper mapping.
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        case BlendMode::Luminance:
            // GL_ONE_MINUS_SRC_COLOR, GL_ONE — inverse-based luminance blend (EnableAlphaBlend2).
            // Story 4.2.5: added to cover the EnableAlphaBlend2() helper mapping.
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE);
            break;
        default:
            g_ErrorReport.Write(L"RENDER: MuRenderer::SetBlendMode -- unknown blend mode %d", static_cast<int>(mode));
            break;
        }
    }

    // -----------------------------------------------------------------------
    // DisableBlend: Disable alpha blending entirely (glDisable(GL_BLEND)).
    // Called by the DisableAlphaBlend() wrapper in ZzzOpenglUtil.cpp (Story 4.2.5).
    // Distinct from SetBlendMode — "no blending" is a render state, not a mode.
    // -----------------------------------------------------------------------
    void DisableBlend() override
    {
        glDisable(GL_BLEND);
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

#endif // MU_USE_OPENGL_BACKEND
