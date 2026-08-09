#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"

// Global UBO for shared matrix and scene uniforms (binding point 0).
// Layout matches GLSL layout(std140, binding=0) uniform GlobalMatrices.
// DXP-14: backed by RHI::CreateUniformBlock/UpdateUniformBlock instead of raw GL calls, so
// this class works unchanged under both backends (RHI_D3D11's HLSL cbuffer mirrors the same
// std140 layout -- see PassthroughShader.cpp's GlobalMatrices cbuffer).
class GlobalUBO {
public:
    static GlobalUBO& Instance();

    void Create();
    void Destroy();

    void SetView(const float m[16]);
    void SetProj(const float m[16]);
    void SetModel(const float m[16]);
    void SetModel(const float origin[3], float scale);
    void SetOrtho(float left, float right, float bottom, float top, float zNear = -1.0f, float zFar = 1.0f);

    void PushModel();
    void PopModel();

    void PushView();
    void PopView();

    void SetTime(float worldTime, float deltaTime);

    // DXP-07d: CPU-side mirrors of the current matrices, for shadow-compare consumers
    // (e.g. ZzzBMD.cpp's GetCurrentMVP()) that need to check their own GL reads against what
    // GlobalUBO already has on the render thread, without re-deriving proj*view*model themselves.
    const float* GetView()  const { return m_View; }
    const float* GetProj()  const { return m_Proj; }
    const float* GetModel() const { return m_Model; }
    const float* GetMVP()   const { return m_MVP; }

    bool IsCreated() const { return m_UBOHandle.IsValid(); }

private:
    GlobalUBO() = default;
    ~GlobalUBO();

    GlobalUBO(const GlobalUBO&) = delete;
    GlobalUBO& operator=(const GlobalUBO&) = delete;

    RHI::BufferHandle m_UBOHandle;

    float m_View[16]  = { 1.f,0.f,0.f,0.f,  0.f,1.f,0.f,0.f,  0.f,0.f,1.f,0.f,  0.f,0.f,0.f,1.f };
    float m_Proj[16]  = { 1.f,0.f,0.f,0.f,  0.f,1.f,0.f,0.f,  0.f,0.f,1.f,0.f,  0.f,0.f,0.f,1.f };
    float m_Model[16] = { 1.f,0.f,0.f,0.f,  0.f,1.f,0.f,0.f,  0.f,0.f,1.f,0.f,  0.f,0.f,0.f,1.f };
    float m_MVP[16]   = { 1.f,0.f,0.f,0.f,  0.f,1.f,0.f,0.f,  0.f,0.f,1.f,0.f,  0.f,0.f,0.f,1.f };
    float m_Time[4]   = { 0.f, 0.f, 0.f, 0.f };

    float m_ModelStack[8][16];
    int   m_ModelStackDepth = 0;

    float m_ViewStack[8][16];
    int   m_ViewStackDepth = 0;

    unsigned char m_LastUploaded[272]; // must match Upload()'s packed buffer size
    bool m_HasUploaded = false;

    void RecomputeMVP();
    void Upload();
};
