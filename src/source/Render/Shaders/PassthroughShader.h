#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"

// PassthroughShader handles textured and untextured geometry rendered via modern VBOs.
// DXP-14 increment 1: first shader with an HLSL port -- Create()/Destroy()/Bind() branch on
// g_RenderBackend; the D3D11 path compiles HLSL via D3DCompile and packs the four scalar
// GLSL uniforms (u_UseTexture/u_UseFog/u_AlphaRef/u_TexCombineAdd, which have no "loose
// uniform" equivalent in HLSL) into one small cbuffer instead.
class PassthroughShader {
public:
    static PassthroughShader& Instance();

    void Create();
    void Destroy();

    void Bind();
    void Unbind();

    // DXP-15 increment 1: D3D11 branch binds through RHI::BindTexture (texID doubles as the
    // RHI::TextureHandle id on both backends, see PassthroughShader.cpp).
    void SetTexture(GLuint texID, int slot = 0);
    void SetUseTexture(bool use);
    void SetUseFog(bool use);
    // FFP GL_TEXTURE_ENV_MODE=GL_ADD port (DXP-08a): fragColor = texture.rgb + vertexColor.rgb
    // (alpha stays multiplicative) instead of the default modulate (texture * vertexColor).
    void SetTexCombineAdd(bool add);

    GLuint GetProgram() const { return m_Program; }
    bool IsCreated() const { return m_Program != 0 || m_D3DVertexShader != nullptr; }

private:
    PassthroughShader() = default;
    ~PassthroughShader();

    PassthroughShader(const PassthroughShader&) = delete;
    PassthroughShader& operator=(const PassthroughShader&) = delete;

    void CreateGL();
    void DestroyGL();
    void BindGL();

    void CreateD3D11();
    void DestroyD3D11();
    void BindD3D11();
    void UploadD3D11Flags();

    // ---- GL state ----
    GLuint m_Program = 0;
    GLuint m_VertShader = 0;
    GLuint m_FragShader = 0;

    GLint m_LocTex = -1;
    GLint m_LocUseTexture = -1;
    GLint m_LocUseFog = -1;
    GLint m_LocAlphaRef = -1;
    GLint m_LocTexCombineAdd = -1;

    // ---- D3D11 state (DXP-14) ----
    // Shader objects stored as void* so this header doesn't need <d3d11.h> -- the .cpp casts
    // to ID3D11VertexShader*/ID3D11PixelShader*/ID3D11SamplerState* under its own D3D11 block.
    // Only valid when g_RenderBackend == RenderBackend::D3D11.
    void* m_D3DVertexShader = nullptr;
    void* m_D3DPixelShader  = nullptr;
    // DXP-16 increment 3: no longer owns a sampler -- RHI::BindTexture() binds each texture's own
    // recorded filter/wrap sampler at bind time (RHI_D3D11.cpp's GetOrCreateSampler).
    RHI::BufferHandle m_D3DFlagsCBuffer; // PassFlags cbuffer, slot 4 -- packs the 4 fields below
    int m_D3DUseTexture = 1;
    int m_D3DUseFog = 0;
    int m_D3DTexCombineAdd = 0;

    // ---- Shared per-draw dirty-check state ----
    // Sentinel guaranteed to differ from any real g_AlphaRef value (-1.0f or a GL_GREATER
    // threshold), forcing the first Bind() after Create() to upload u_AlphaRef. Doubles as the
    // D3D11 PassFlags cbuffer's alphaRef field (always a real value post-Create on both backends).
    float m_LastAlphaRef = -12345.0f;

    // Tri-state (-1 = unset, 0/1 = last value sent) so the first call after program (re)creation
    // always uploads, same pattern as m_LastAlphaRef. GL-path dirty-check only -- the D3D11
    // path always re-uploads its cbuffer on a Set* call (m_D3DUseTexture/m_D3DUseFog/
    // m_D3DTexCombineAdd above hold its real current values instead).
    int m_LastUseTexture = -1;
    int m_LastUseFog = -1;
    int m_LastTexCombineAdd = -1;
};
