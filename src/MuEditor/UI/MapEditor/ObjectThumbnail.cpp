#include "stdafx.h"

#ifdef _EDITOR

#include "ObjectThumbnail.h"

#include "Render/Models/ZzzBMD.h"   // BMD / Models[] / BoneTransform / RENDER_TEXTURE / OBB_t

#include <cmath>

// Global bone scale the model Transform/Animation multiply by; the game sets it
// per object (Calc_RenderObject). Must be 1 for an un-scaled preview.
extern float BoneScale;

namespace
{
    // The FBO entry points are OpenGL extensions. The engine never used them, so
    // glew isn't linked - load them directly from the GL driver instead. The GL
    // enum tokens still come from glew.h (included via stdafx).
    typedef void (APIENTRY* PFN_GenFB)(GLsizei, GLuint*);
    typedef void (APIENTRY* PFN_BindFB)(GLenum, GLuint);
    typedef void (APIENTRY* PFN_FBTex2D)(GLenum, GLenum, GLenum, GLuint, GLint);
    typedef void (APIENTRY* PFN_GenRB)(GLsizei, GLuint*);
    typedef void (APIENTRY* PFN_BindRB)(GLenum, GLuint);
    typedef void (APIENTRY* PFN_RBStorage)(GLenum, GLenum, GLsizei, GLsizei);
    typedef void (APIENTRY* PFN_FBRB)(GLenum, GLenum, GLenum, GLuint);

    PFN_GenFB     s_GenFramebuffers = nullptr;
    PFN_BindFB    s_BindFramebuffer = nullptr;
    PFN_FBTex2D   s_FramebufferTexture2D = nullptr;
    PFN_GenRB     s_GenRenderbuffers = nullptr;
    PFN_BindRB    s_BindRenderbuffer = nullptr;
    PFN_RBStorage s_RenderbufferStorage = nullptr;
    PFN_FBRB      s_FramebufferRenderbuffer = nullptr;
    bool s_fboLoaded = false;
    bool s_fboOk = false;

    bool LoadFboFunctions()
    {
        if (s_fboLoaded)
            return s_fboOk;
        s_fboLoaded = true;
        s_GenFramebuffers        = (PFN_GenFB)wglGetProcAddress("glGenFramebuffers");
        s_BindFramebuffer        = (PFN_BindFB)wglGetProcAddress("glBindFramebuffer");
        s_FramebufferTexture2D   = (PFN_FBTex2D)wglGetProcAddress("glFramebufferTexture2D");
        s_GenRenderbuffers       = (PFN_GenRB)wglGetProcAddress("glGenRenderbuffers");
        s_BindRenderbuffer       = (PFN_BindRB)wglGetProcAddress("glBindRenderbuffer");
        s_RenderbufferStorage    = (PFN_RBStorage)wglGetProcAddress("glRenderbufferStorage");
        s_FramebufferRenderbuffer= (PFN_FBRB)wglGetProcAddress("glFramebufferRenderbuffer");
        s_fboOk = s_GenFramebuffers && s_BindFramebuffer && s_FramebufferTexture2D &&
                  s_GenRenderbuffers && s_BindRenderbuffer && s_RenderbufferStorage &&
                  s_FramebufferRenderbuffer;
        return s_fboOk;
    }

    void Normalize3(float v[3])
    {
        const float len = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (len > 1e-6f) { v[0] /= len; v[1] /= len; v[2] /= len; }
    }

    void Cross3(const float a[3], const float b[3], float out[3])
    {
        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
    }

    float Dot3(const float a[3], const float b[3])
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    // Loads a perspective projection (column-major) into the current matrix.
    void LoadPerspective(float fovYDeg, float aspect, float znear, float zfar)
    {
        const float f = 1.0f / std::tan(fovYDeg * 0.5f * 3.14159265f / 180.0f);
        float m[16] = { 0 };
        m[0]  = f / aspect;
        m[5]  = f;
        m[10] = (zfar + znear) / (znear - zfar);
        m[11] = -1.0f;
        m[14] = (2.0f * zfar * znear) / (znear - zfar);
        glLoadMatrixf(m);
    }

    // Loads a look-at view (column-major) into the current matrix.
    void LoadLookAt(const float eye[3], const float center[3], const float up[3])
    {
        float fwd[3] = { center[0] - eye[0], center[1] - eye[1], center[2] - eye[2] };
        Normalize3(fwd);
        float side[3]; Cross3(fwd, up, side); Normalize3(side);
        float up2[3];  Cross3(side, fwd, up2);

        float m[16];
        m[0] = side[0]; m[4] = side[1]; m[8]  = side[2];  m[12] = -Dot3(side, eye);
        m[1] = up2[0];  m[5] = up2[1];  m[9]  = up2[2];   m[13] = -Dot3(up2, eye);
        m[2] = -fwd[0]; m[6] = -fwd[1]; m[10] = -fwd[2];  m[14] =  Dot3(fwd, eye);
        m[3] = 0.0f;    m[7] = 0.0f;    m[11] = 0.0f;     m[15] = 1.0f;
        glLoadMatrixf(m);
    }
}

CObjectThumbnail& CObjectThumbnail::GetInstance()
{
    static CObjectThumbnail instance;
    return instance;
}

void CObjectThumbnail::BeginFrame()
{
    m_budget = MAX_RENDERS_PER_FRAME;
}

void CObjectThumbnail::FreeTexture(unsigned int tex)
{
    if (tex != 0)
        glDeleteTextures(1, &tex);
}

void CObjectThumbnail::Invalidate()
{
    for (auto& kv : m_cache)
        if (kv.second != 0)
            glDeleteTextures(1, &kv.second);
    m_cache.clear();
}

bool CObjectThumbnail::EnsureTargets()
{
    if (m_init)
        return true;
    if (!LoadFboFunctions())
        return false;  // driver lacks FBO support (very unlikely)

    s_GenFramebuffers(1, &m_fbo);
    s_GenRenderbuffers(1, &m_depth);
    s_BindRenderbuffer(GL_RENDERBUFFER, m_depth);
    s_RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, THUMB_SIZE, THUMB_SIZE);
    s_BindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    s_FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
    s_BindRenderbuffer(GL_RENDERBUFFER, 0);
    s_BindFramebuffer(GL_FRAMEBUFFER, 0);
    m_init = true;
    return true;
}

unsigned int CObjectThumbnail::Get(int type)
{
    auto it = m_cache.find(type);
    if (it != m_cache.end())
        return it->second;
    if (m_budget <= 0)
        return 0;  // try again next frame
    --m_budget;
    const unsigned int tex = RenderSlotToTexture(type);
    m_cache[type] = tex;
    return tex;
}

unsigned int CObjectThumbnail::RenderSlotToTexture(int type)
{
    if (type < 0)
        return 0;
    BMD* b = &Models[type];
    if (b->NumMeshs <= 0 || b->Meshs == nullptr)
        return 0;
    if (!EnsureTargets())
        return 0;

    // Persistent color texture for this model.
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, THUMB_SIZE, THUMB_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Save GL state we touch.
    GLint prevFBO = 0; glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    GLint vp[4];       glGetIntegerv(GL_VIEWPORT, vp);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION); glPushMatrix();
    glMatrixMode(GL_MODELVIEW);  glPushMatrix();

    s_BindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    s_FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    glViewport(0, 0, THUMB_SIZE, THUMB_SIZE);
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Unlit, full-bright so the texture reads clearly without a scene light.
    b->BodyScale = 1.0f;
    b->BodyOrigin[0] = b->BodyOrigin[1] = b->BodyOrigin[2] = 0.0f;
    b->BodyHeight = 0.0f;
    b->CurrentAction = 0;
    b->LightEnable = false;
    b->ContrastEnable = false;
    b->BodyLight[0] = b->BodyLight[1] = b->BodyLight[2] = 1.0f;

    BoneScale = 1.0f;
    vec3_t angle = { 0.0f, 0.0f, 0.0f };
    vec3_t head  = { 0.0f, 0.0f, 0.0f };
    b->Animation(BoneTransform, 0.0f, 0.0f, 0, angle, head, false, false);
    vec3_t bbMin = { 0, 0, 0 }, bbMax = { 0, 0, 0 };
    OBB_t obb;
    b->Transform(BoneTransform, bbMin, bbMax, &obb, true);

    // Frame the model from its bounding box (MU objects are Z-up). Fall back to a
    // sensible default when the box comes back degenerate.
    float cx = (bbMin[0] + bbMax[0]) * 0.5f;
    float cy = (bbMin[1] + bbMax[1]) * 0.5f;
    float cz = (bbMin[2] + bbMax[2]) * 0.5f;
    const float sx = bbMax[0] - bbMin[0];
    const float sy = bbMax[1] - bbMin[1];
    const float sz = bbMax[2] - bbMin[2];
    float radius = 0.5f * std::fmax(sx, std::fmax(sy, sz));
    if (!(radius > 1.0f) || radius > 100000.0f)
    {
        // Degenerate/invalid box: assume a typical MU object size.
        cx = 0.0f; cy = 0.0f; cz = 80.0f;
        radius = 160.0f;
    }

    const float fovDeg = 35.0f;
    const float dist = radius / std::tan(fovDeg * 0.5f * 3.14159265f / 180.0f) * 1.8f;

    float dir[3] = { 1.0f, -1.0f, 0.8f };
    Normalize3(dir);
    const float eye[3]    = { cx + dir[0] * dist, cy + dir[1] * dist, cz + dir[2] * dist };
    const float center[3] = { cx, cy, cz };
    const float up[3]     = { 0.0f, 0.0f, 1.0f };

    // Generous near/far so nothing clips regardless of true model size.
    const float znear = std::fmax(2.0f, dist * 0.05f);
    const float zfar  = dist + radius * 8.0f + 4000.0f;
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    LoadPerspective(fovDeg, 1.0f, znear, zfar);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    LoadLookAt(eye, center, up);

    b->RenderBody(RENDER_TEXTURE, 1.0f, -1, 1.0f, 0.0f, 0.0f);

    // Restore.
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glPopAttrib();
    s_BindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFBO);
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

#endif // _EDITOR
