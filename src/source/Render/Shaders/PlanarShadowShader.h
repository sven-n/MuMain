#ifndef PLANAR_SHADOW_SHADER_H
#define PLANAR_SHADOW_SHADER_H

#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include <SDL3/SDL.h>

// DXP-14 increment 4: HLSL port, same Create/Destroy/Bind-style backend-branch pattern as the
// other 3 shaders (see PassthroughShader.h for the general shape) -- here applied to
// Init/Shutdown/Begin/End/Draw. Two scope narrowings vs. the GLSL, both because this class's
// public surface stays unchanged (RHI.h design doc Q1) while D3D11's stricter input-layout
// model can't cleanly support what GL's VAO/attribute model does for free:
// 1. DrawGPUSkinned() stays GL-only this increment (documented no-op under D3D11, same pattern
//    as the other shaders' SetTexture) -- it draws directly from BMD::m_VAO_StaticGPU, a buffer
//    shaped for BMDMeshShader's VertexLayout::BMDMesh, not PlanarShadow's own PosOnly layout;
//    wiring a D3D11 GPU-skinned shadow draw is DXP-15/16's job (needs BMD's real GPU path first).
// 2. The D3D11 vertex shader twin only takes a_Pos (matches VertexLayout::PosOnly's own "position-
//    only" design intent, RHI.h) -- the GLSL's a_BoneIndex + bone-skinning branch is CPU-path-only
//    under D3D11 for the same reason as (1): D3D11's CreateInputLayout requires every VS input
//    semantic to have a matching layout element, so a shader declaring a_BoneIndex could never
//    bind against a position-only buffer. Ported when DrawGPUSkinned's D3D11 path lands.
class CPlanarShadowShader
{
public:
    static CPlanarShadowShader& Instance();

    bool Init();
    void Shutdown();
    bool IsSupported() const { return m_bInitialized && m_bSupported; }

    // `mvp` is the camera view*projection matrix (DXP-06: replaces the old FFP
    // gl_ModelViewProjectionMatrix read) — callers already have this via BMD::RenderMesh()'s
    // GetCurrentMVP() cache, since shadow vertices are pre-transformed to world space on the CPU.
    bool Begin(const float* bodyOrigin, const float mvp[16], float sx = 2000.0f, float sy = 4000.0f, float alpha = 0.45f);
    void End();

    // Uploads `vertexCount` positions (flat vec3 array, `vertexCount*3` floats, non-indexed
    // triangle list — matches the CPU callers' existing glDrawArrays submission) and draws them
    // through this shader's own VAO/VBO. Only valid to call while Begin() is active.
    void Draw(const float* vertices, int vertexCount);

    // DXP-20 increment 3: draws directly from a caller-owned VAO (BMD::m_VAO_StaticGPU, the same
    // rest-pose-position + bone-index buffer BMDMeshShader's GPU skinning path draws from) instead
    // of re-skinning on the CPU into `vertices` first. Skins in-shader from BoneMatrices (binding 2,
    // populated by BoneUBO::UploadBones() — caller must upload before calling this), then applies
    // the same ground-skew projection as Draw(). `skinOrigin`/`skinScale` mirror RenderMesh's
    // gpuBodyOrigin/gpuBodyScale (nullptr/1.0 when bone matrices already encode world position).
    // Only valid to call while Begin() is active. GL-only this increment -- see class comment.
    void DrawGPUSkinned(GLuint vao, int baseCorner, int vertexCount, const float* skinOrigin, float skinScale);

    bool IsActive() const { return m_bActive; }

private:
    CPlanarShadowShader();
    ~CPlanarShadowShader();

    bool LoadGLFunctions();
    bool CompileShaders();
    void CreateBuffers();
    void DestroyBuffers();

    bool InitGL();
    void ShutdownGL();
    bool BeginGL(const float* bodyOrigin, const float mvp[16], float sx, float sy, float alpha);
    void EndGL();
    void DrawGL(const float* vertices, int vertexCount);

    bool InitD3D11();
    void ShutdownD3D11();
    bool BeginD3D11(const float* bodyOrigin, const float mvp[16], float sx, float sy, float alpha);
    void EndD3D11();
    void DrawD3D11(const float* vertices, int vertexCount);

    bool   m_bInitialized = false;
    bool   m_bSupported   = false;
    bool   m_bActive      = false;

    GLuint m_hProgram        = 0;
    GLuint m_hVertexShader   = 0;
    GLuint m_hFragmentShader = 0;

    GLint  m_locBodyOrigin   = -1;
    GLint  m_locSx           = -1;
    GLint  m_locSy           = -1;
    GLint  m_locShadowAlpha  = -1;
    GLint  m_locMVP          = -1;
    GLint  m_locUseGPUSkin   = -1; // DXP-20: 1 = skin a_Pos via BoneMatrices[a_BoneIndex] before the skew
    GLint  m_locSkinOrigin   = -1; // DXP-20: mirrors RenderMesh's gpuBodyOrigin (world placement of skinned rest pose)
    GLint  m_locSkinScale    = -1; // DXP-20: mirrors RenderMesh's gpuBodyScale

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    size_t m_VBOCapacity = 0;

    // ---- D3D11 state (DXP-14 increment 4) ----
    void* m_D3DVertexShader = nullptr;
    void* m_D3DPixelShader  = nullptr;
    // ShadowFlags cbuffer, slot 7 (0=GlobalMatrices, 1=SceneData, 2=BoneUBO, 3=DynamicLights
    // reserved, 4=Passthrough's PassFlags, 5=Terrain's TerrainFlags, 6=BMDMesh's BMDFlags).
    RHI::BufferHandle m_D3DFlagsCBuffer;
    // Own Dynamic vertex buffer (RHI::VertexLayout::PosOnly) -- separate from m_VAO/m_VBO above,
    // which stay GL-only (nothing under D3D11 draws through this shader's real game call sites
    // yet, DXP-15/16's job -- this buffer only serves the DXP-14 smoke test).
    RHI::BufferHandle m_D3DVBO;
    size_t m_D3DVBOCapacity = 0;
};

#endif // PLANAR_SHADOW_SHADER_H
