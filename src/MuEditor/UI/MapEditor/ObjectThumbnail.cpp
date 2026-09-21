#include "stdafx.h"

#ifdef _EDITOR

#include "ObjectThumbnail.h"

#include "Render/Models/ZzzBMD.h"        // BMD / Models[] / BoneTransform / RENDER_TEXTURE / OBB_t
#include "Render/Renderer/MuRenderer.h"  // mu::GetRenderer()
#include "UI/Console/MuEditorConsoleUI.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

// Global bone scale the model Transform/Animation multiply by; the game sets it
// per object (Calc_RenderObject). Must be 1 for an un-scaled preview.
extern float BoneScale;

namespace
{
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

    // Guarantees EndOffscreenCapture() runs even if something in between throws or
    // an early return gets added later - otherwise the capture stays "open" forever
    // and BeginOffscreenCapture() refuses every future call, silently breaking
    // every thumbnail after this one for the rest of the process.
    class ScopedOffscreenCapture
    {
    public:
        ~ScopedOffscreenCapture() { mu::GetRenderer().EndOffscreenCapture(); }
    };

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
        mu::GetRenderer().LoadMatrix(m);
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
        mu::GetRenderer().ReleaseTexture(tex);
}

void CObjectThumbnail::Invalidate()
{
    for (auto& kv : m_cache)
        if (kv.second != 0)
            mu::GetRenderer().ReleaseTexture(kv.second);
    m_cache.clear();
    m_failCount.clear();
    m_pendingTypes.clear();
}

unsigned int CObjectThumbnail::Get(int type)
{
    auto it = m_cache.find(type);
    if (it != m_cache.end())
        return it->second;
    if (m_budget <= 0)
        return 0;  // try again next frame
    if (std::find(m_pendingTypes.begin(), m_pendingTypes.end(), type) == m_pendingTypes.end())
    {
        m_pendingTypes.push_back(type);
        --m_budget;
    }
    return 0;  // result appears once ProcessPendingRequests() has run
}

bool CObjectThumbnail::RequestSlotPreview(int slot)
{
    if (m_scratchPending)
        return false;  // previous request not delivered yet - caller must wait
    m_scratchSlot = slot;
    m_scratchPending = true;
    m_scratchHasResult = false;
    return true;
}

unsigned int CObjectThumbnail::PollSlotPreview()
{
    if (m_scratchPending || !m_scratchHasResult)
        return 0;
    m_scratchHasResult = false;
    return m_scratchResult;
}

void CObjectThumbnail::ProcessPendingRequests()
{
    // No active frame right now (window minimized/occluded/not focused - e.g.
    // the user alt-tabbed away) - BeginOffscreenCapture() can't succeed for
    // ANY request regardless of the model, so don't even try: leave everything
    // queued for next time instead of spending retry budget on a "failure"
    // that has nothing to do with the model itself and will stop happening
    // the moment the window is active again.
    if (!mu::GetRenderer().IsFrameActive())
        return;

    for (const int type : m_pendingTypes)
    {
        const unsigned int tex = RenderNow(type);
        if (tex != 0)
        {
            m_cache[type] = tex;
            m_failCount.erase(type);
        }
        else if (++m_failCount[type] >= MAX_TRANSIENT_RETRIES)
        {
            // Given up - cache the failure so Get() stops retrying it.
            m_cache[type] = 0;
            m_failCount.erase(type);
        }
        // else: leave uncached - Get() will queue another attempt later.
    }
    m_pendingTypes.clear();

    if (m_scratchPending)
    {
        const unsigned int tex = RenderNow(m_scratchSlot);
        if (tex != 0)
        {
            m_scratchResult = tex;
            m_scratchHasResult = true;
            m_scratchPending = false;
            m_scratchFailCount = 0;
        }
        else if (++m_scratchFailCount >= MAX_TRANSIENT_RETRIES)
        {
            m_scratchResult = 0;
            m_scratchHasResult = true;
            m_scratchPending = false;
            m_scratchFailCount = 0;
        }
        // else: leave m_scratchPending set - retries the same slot next frame.
    }
}

unsigned int CObjectThumbnail::RenderNow(int type)
{
    if (type < 0)
        return 0;
    BMD* b = &Models[type];
    if (b->NumMeshs <= 0 || b->Meshs == nullptr)
    {
        char msg[96];
        snprintf(msg, sizeof(msg), "[MapEditor] Thumbnail skip: type %d not loaded (NumMeshs=%d, Meshs=%p)",
                 type, b->NumMeshs, static_cast<void*>(b->Meshs));
        g_MuEditorConsoleUI.LogEditor(msg);
        return 0;
    }

    // Renders into its own dedicated texture via the renderer's offscreen capture -
    // draw calls issued before EndOffscreenCapture() never reach the main frame, so
    // this can't interfere with (or be interfered with by) normal game rendering.
    const std::uint32_t tex = mu::GetRenderer().BeginOffscreenCapture(0u, THUMB_SIZE, THUMB_SIZE);
    if (tex == 0u)
    {
        char msg[96];
        snprintf(msg, sizeof(msg), "[MapEditor] Thumbnail FAILED: type %d BeginOffscreenCapture returned 0", type);
        g_MuEditorConsoleUI.LogEditor(msg);
        return 0;
    }
    const ScopedOffscreenCapture endCaptureOnReturn; // EndOffscreenCapture() on every exit path below

    glMatrixMode(GL_PROJECTION); glPushMatrix();
    glMatrixMode(GL_MODELVIEW);  glPushMatrix();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    // Foliage/tree models rely on alpha-blended or alpha-cutout leaf textures to
    // render solid - with blending off, translucent leaf clusters either vanish or
    // render as sparse fragments instead of a filled canopy.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    // Half the box's diagonal, not half its largest single axis: a box that's
    // wide AND tall (not just cube-shaped) needs the full diagonal to guarantee
    // every corner stays inside the frame from an arbitrary viewing angle: using
    // only the largest axis put the camera too close, so non-cubic objects (most
    // of them) stuck out past the thumbnail's edges.
    float radius = 0.5f * std::sqrt(sx * sx + sy * sy + sz * sz);
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
    gluPerspective(fovDeg, 1.0f, znear, zfar);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    LoadLookAt(eye, center, up);

    b->RenderBody(RENDER_TEXTURE, 1.0f, -1, 1.0f, 0.0f, 0.0f);

    // Restore.
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();

    return tex;
}

#endif // _EDITOR
