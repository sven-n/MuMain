#include "stdafx.h"
#include "SceneUBO.h"
#include <cstring>
#include <cassert>

SceneUBO& SceneUBO::Instance()
{
    static SceneUBO instance;
    return instance;
}

SceneUBO::~SceneUBO()
{
    Destroy();
}

void SceneUBO::Create()
{
    if (m_UBOHandle.IsValid()) return;

    m_UBOHandle = RHI::CreateUniformBlock(sizeof(m_Data), 1); // Binding point 1
    Upload();
}

void SceneUBO::Destroy()
{
    if (m_UBOHandle.IsValid()) {
        RHI::DestroyUniformBlock(m_UBOHandle);
        m_UBOHandle = {};
    }
}

void SceneUBO::Bind()
{
    // No-op since DXP-14: RHI::CreateUniformBlock binds the block to its slot once, at
    // creation (both GL's glBindBufferBase and D3D11's VSSetConstantBuffers/
    // PSSetConstantBuffers persist across draws) -- kept as a public method since callers
    // may still invoke it defensively, but there is no per-call rebind work left to do.
}

void SceneUBO::SetFog(float start, float end, const float color[3], bool enabled)
{
    m_Data.fogStart = start;
    m_Data.fogEnd = end;
    if (color) {
        m_Data.fogColor[0] = color[0];
        m_Data.fogColor[1] = color[1];
        m_Data.fogColor[2] = color[2];
        m_Data.fogColor[3] = 1.0f;
    }
    m_Data.fogEnabled = enabled ? 1.0f : 0.0f;
    Upload();
}

void SceneUBO::SetFogEnabled(bool enabled)
{
    m_Data.fogEnabled = enabled ? 1.0f : 0.0f;
    Upload();
}

void SceneUBO::Upload()
{
    if (!m_UBOHandle.IsValid()) return;
    RHI::UpdateUniformBlock(m_UBOHandle, &m_Data, sizeof(m_Data));
}
