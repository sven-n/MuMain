#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"

// Scene UBO for shared fog and environment parameters (binding point 1).
// Layout matches GLSL layout(std140, binding=1) uniform SceneData.
// DXP-14: backed by RHI::CreateUniformBlock/UpdateUniformBlock instead of raw GL calls (see
// GlobalUBO.h's equivalent note).
class SceneUBO {
public:
    static SceneUBO& Instance();

    void Create();
    void Destroy();
    void Bind();

    void SetFog(float start, float end, const float color[3], bool enabled);
    void SetFogEnabled(bool enabled);

    bool IsCreated() const { return m_UBOHandle.IsValid(); }

private:
    SceneUBO() = default;
    ~SceneUBO();

    SceneUBO(const SceneUBO&) = delete;
    SceneUBO& operator=(const SceneUBO&) = delete;

    RHI::BufferHandle m_UBOHandle;

    struct {
        float fogColor[4] = { 0.f, 0.f, 0.f, 0.f };
        float fogStart    = 0.f;
        float fogEnd      = 0.f;
        float fogEnabled  = 0.f;
        float padding     = 0.f;
    } m_Data;

    void Upload();
};
