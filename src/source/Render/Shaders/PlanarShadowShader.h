#ifndef PLANAR_SHADOW_SHADER_H
#define PLANAR_SHADOW_SHADER_H

#pragma once

#include "stdafx.h"
#include <SDL3/SDL.h>

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
    // Only valid to call while Begin() is active.
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
};

#endif // PLANAR_SHADOW_SHADER_H
