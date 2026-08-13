#ifndef PLANAR_SHADOW_SHADER_H
#define PLANAR_SHADOW_SHADER_H

#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"
#include <SDL3/SDL.h>

// DXP-14 increment 4: HLSL port, same Create/Destroy/Bind-style backend-branch pattern as the
// other 3 shaders (see PassthroughShader.h for the general shape) -- here applied to
// Init/Shutdown/Begin/End/Draw.
//
// DXP-21 part 2: DrawGPUSkinned() now has a real D3D11 path, drawing directly from
// BMD::m_D3D11StaticVB (RHI::VertexLayout::BMDMesh) instead of the CPU-replayed vertex list.
// It uses a SECOND, separate D3D11 vertex shader object (m_D3DVertexShaderGPUSkin) rather than
// adding a_BoneIndex/the skin branch to the existing CPU-path VS twin (g_szPlanarVertHLSL):
// D3D11's CreateInputLayout requires every semantic a VS declares in its input signature to have
// a matching element in the descriptor array used to build the layout, and the CPU path's own
// buffer (m_D3DVBO, VertexLayout::PosOnly) is a bare 12-byte position-only stream with no bone
// index at all -- one shader can't structurally serve both a 12-byte PosOnly buffer and a
// 52-byte BMDMesh buffer. The GPU-skin VS is bound with VertexLayout::BMDMesh, whose input
// layout is already registered by BMDMeshShader (RHI.h: BMDMeshShader is BMDMesh's bytecode
// producer) -- this class deliberately does NOT call RegisterVertexShaderBytecode(BMDMesh, ...)
// itself, since the GPU-skin VS's inputs (a_Pos/a_BoneIndex) are a strict subset of what that
// already-registered layout provides. Both VS objects still read the same slot-7 ShadowFlags
// cbuffer (mvp/bodyOrigin/sx/sy/alpha cached from Begin(), reused by DrawGPUSkinned so a full
// cbuffer rewrite -- RHI::UpdateUniformBlock has no partial-update variant -- stays correct).
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

    // DXP-20 increment 3 (GL) / DXP-21 part 2 (D3D11): draws directly from a caller-owned static
    // GPU buffer (BMD::m_VAO_StaticGPU under GL, BMD::m_D3D11StaticVB under D3D11 — the same
    // rest-pose-position + bone-index buffer BMDMeshShader's GPU skinning path draws from) instead
    // of re-skinning on the CPU into `vertices` first. Skins in-shader from BoneMatrices (binding 2,
    // populated by BoneUBO::UploadBones() — caller must upload before calling this), then applies
    // the same ground-skew projection as Draw(). `skinOrigin`/`skinScale` mirror RenderMesh's
    // gpuBodyOrigin/gpuBodyScale (nullptr/1.0 when bone matrices already encode world position).
    // Only valid to call while Begin() is active. `vao` is read under GL, `d3dVB` under D3D11 —
    // widened to two explicit params (DXP-21) rather than overloading `vao` to mean different
    // things per backend.
    void DrawGPUSkinned(GLuint vao, RHI::BufferHandle d3dVB, int baseCorner, int vertexCount, const float* skinOrigin, float skinScale);

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
    void DrawGPUSkinnedD3D11(RHI::BufferHandle d3dVB, int baseCorner, int vertexCount, const float* skinOrigin, float skinScale);

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
    // DXP-21 part 2: second VS object for DrawGPUSkinned's D3D11 path (a_Pos+a_BoneIndex,
    // VertexLayout::BMDMesh) — see class comment for why this can't just be a branch inside
    // m_D3DVertexShader.
    void* m_D3DVertexShaderGPUSkin = nullptr;
    // ShadowFlags cbuffer, slot 7 (0=GlobalMatrices, 1=SceneData, 2=BoneUBO, 3=DynamicLights
    // reserved, 4=Passthrough's PassFlags, 5=Terrain's TerrainFlags, 6=BMDMesh's BMDFlags).
    RHI::BufferHandle m_D3DFlagsCBuffer;
    // Own Dynamic vertex buffer (RHI::VertexLayout::PosOnly) -- separate from m_VAO/m_VBO above,
    // which stay GL-only (nothing under D3D11 draws through this shader's real game call sites
    // yet, DXP-15/16's job -- this buffer only serves the DXP-14 smoke test).
    RHI::BufferHandle m_D3DVBO;
    size_t m_D3DVBOCapacity = 0;

    // DXP-21 part 2: mvp/bodyOrigin/sx/sy/alpha cached from BeginD3D11, so DrawGPUSkinned can
    // rebuild the full ShadowFlagsCB (RHI::UpdateUniformBlock has no partial-update variant --
    // every call must supply the whole cbuffer) without threading those params through the
    // public DrawGPUSkinned signature.
    float m_D3DCachedMVP[16] = {};
    float m_D3DCachedBodyOrigin[3] = {};
    float m_D3DCachedSx = 0.0f;
    float m_D3DCachedSy = 0.0f;
    float m_D3DCachedAlpha = 0.0f;
};

#endif // PLANAR_SHADOW_SHADER_H
