#include "stdafx.h"
#include "GlobalUBO.h"
#include "Render/Core/ImmediateRenderer.h" // GLP-19 -- IR::Flush() before matrices change
#include <cstring>
#include <cassert>

GlobalUBO& GlobalUBO::Instance()
{
    static GlobalUBO instance;
    return instance;
}

GlobalUBO::~GlobalUBO()
{
    Destroy();
}

static void MultMat4(float* out, const float* a, const float* b)
{
    // out = a * b (column-major order)
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a[row + k * 4] * b[k + col * 4];
            }
            out[row + col * 4] = sum;
        }
    }
}

void GlobalUBO::Create()
{
    if (m_UBOHandle.IsValid()) return;

    // Total size = 4 * 64 bytes (matrices) + 16 bytes (vec4 u_Time) = 272 bytes, binding point 0.
    const size_t bufferSize = 4 * 16 * sizeof(float) + 4 * sizeof(float);
    m_UBOHandle = RHI::CreateUniformBlock(bufferSize, 0);

    RecomputeMVP();
    Upload();
}

void GlobalUBO::Destroy()
{
    if (m_UBOHandle.IsValid()) {
        RHI::DestroyUniformBlock(m_UBOHandle);
        m_UBOHandle = {};
    }
}

void GlobalUBO::SetView(const float m[16])
{
    std::memcpy(m_View, m, sizeof(m_View));
    RecomputeMVP();
    Upload();
}

void GlobalUBO::SetProj(const float m[16])
{
    std::memcpy(m_Proj, m, sizeof(m_Proj));
    RecomputeMVP();
    Upload();
}

void GlobalUBO::SetModel(const float m[16])
{
    std::memcpy(m_Model, m, sizeof(m_Model));
    RecomputeMVP();
    Upload();
}

void GlobalUBO::SetModel(const float origin[3], float scale)
{
    float m[16] = {
        scale, 0.0f,  0.0f,  0.0f,
        0.0f,  scale, 0.0f,  0.0f,
        0.0f,  0.0f,  scale, 0.0f,
        origin[0], origin[1], origin[2], 1.0f
    };
    SetModel(m);
}

void GlobalUBO::SetOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    float proj[16] = { 0 };
    proj[0]  = 2.0f / (right - left);
    proj[5]  = 2.0f / (top - bottom);
    proj[10] = -2.0f / (zFar - zNear);
    proj[12] = -(right + left) / (right - left);
    proj[13] = -(top + bottom) / (top - bottom);
    proj[14] = -(zFar + zNear) / (zFar - zNear);
    proj[15] = 1.0f;

    float viewIdentity[16] = {
        1.f,0.f,0.f,0.f,
        0.f,1.f,0.f,0.f,
        0.f,0.f,1.f,0.f,
        0.f,0.f,0.f,1.f
    };

    std::memcpy(m_Proj, proj, sizeof(m_Proj));
    std::memcpy(m_View, viewIdentity, sizeof(m_View));
    RecomputeMVP();
    Upload();
}

void GlobalUBO::PushModel()
{
    assert(m_ModelStackDepth < 8 && "GlobalUBO model stack overflow!");
    if (m_ModelStackDepth < 8) {
        std::memcpy(m_ModelStack[m_ModelStackDepth], m_Model, sizeof(m_Model));
        m_ModelStackDepth++;
    }
}

void GlobalUBO::PopModel()
{
    assert(m_ModelStackDepth > 0 && "GlobalUBO model stack underflow!");
    if (m_ModelStackDepth > 0) {
        m_ModelStackDepth--;
        std::memcpy(m_Model, m_ModelStack[m_ModelStackDepth], sizeof(m_Model));
        RecomputeMVP();
        Upload();
    }
}

void GlobalUBO::PushView()
{
    assert(m_ViewStackDepth < 8 && "GlobalUBO view stack overflow!");
    if (m_ViewStackDepth < 8) {
        std::memcpy(m_ViewStack[m_ViewStackDepth], m_View, sizeof(m_View));
        m_ViewStackDepth++;
    }
}

void GlobalUBO::PopView()
{
    assert(m_ViewStackDepth > 0 && "GlobalUBO view stack underflow!");
    if (m_ViewStackDepth > 0) {
        m_ViewStackDepth--;
        std::memcpy(m_View, m_ViewStack[m_ViewStackDepth], sizeof(m_View));
        RecomputeMVP();
        Upload();
    }
}

void GlobalUBO::SetTime(float worldTime, float deltaTime)
{
    m_Time[0] = worldTime;
    m_Time[1] = deltaTime;
    m_Time[2] = 0.0f;
    m_Time[3] = 0.0f;
    Upload();
}

void GlobalUBO::RecomputeMVP()
{
    float vp[16];
    MultMat4(vp, m_Proj, m_View);
    MultMat4(m_MVP, vp, m_Model);
}

void GlobalUBO::Upload()
{
    // GLP-19: the view/proj/model matrices are batch state for IR, and they live here rather than
    // in any PassthroughShader uniform -- so IR's state key cannot see them. Every mutator
    // (SetView/SetProj/SetModel/SetOrtho/PopView) funnels through this one function, which makes it
    // the only hook that cannot be missed. Flush BEFORE the new matrices reach the GPU, so a batch
    // accumulated under the old ones is drawn under the old ones.
    // Without this, BeginSprite/EndSprite (view push/pop) and BeginBitmap/EndBitmap (proj+view
    // swap) let UI text quads accumulate across a projection change -- observed as garbled glyphs.
    IR::Flush();

    if (!m_UBOHandle.IsValid()) return;

    // layout(std140, binding=0) -- contiguous, matches the HLSL cbuffer mirror byte-for-byte:
    // Offset 0:   u_View  (64 bytes)
    // Offset 64:  u_Proj  (64 bytes)
    // Offset 128: u_Model (64 bytes)
    // Offset 192: u_MVP   (64 bytes)
    // Offset 256: u_Time  (16 bytes)
    unsigned char packed[272];
    std::memcpy(packed +   0, m_View,  64);
    std::memcpy(packed +  64, m_Proj,  64);
    std::memcpy(packed + 128, m_Model, 64);
    std::memcpy(packed + 192, m_MVP,   64);
    std::memcpy(packed + 256, m_Time,  16);

    RHI::UpdateUniformBlock(m_UBOHandle, packed, sizeof(packed));
}
