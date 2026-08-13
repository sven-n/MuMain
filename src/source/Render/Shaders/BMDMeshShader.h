#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"

// BMDMeshShader handles 3D character, monster, item, and map meshes
// DXP-14 increment 3: HLSL port, same Create()/Destroy()/Bind() backend-branch pattern as
// PassthroughShader/TerrainShader (DXP-14 increments 1-2) -- see PassthroughShader.h for the
// general shape. Bind()'s existing 21-parameter signature stays as-is (RHI.h design doc Q1);
// the D3D11 path packs all of them into one BMDFlags cbuffer instead of per-uniform glUniform*
// calls.
class BMDMeshShader {
public:
    static BMDMeshShader& Instance();

    void Create();
    void Destroy();

    void Bind(int renderMode = 0, float waveOffsetU = 0.0f, float waveOffsetV = 0.0f, GLuint texID = 0, const float mvp[16] = nullptr, int useGPUSkin = 0, const float bodyOrigin[3] = nullptr, float bodyScale = 1.0f, float chromeWave = 0.0f, GLuint chromeTex1ID = 0, const float specularTint[3] = nullptr, GLuint metalTexID = 0, GLuint chromeTex2ID = 0, float chromeWave2 = 0.0f, float chromeLightVecX = 0.0f, float chromeLightVecY = 0.0f, const float lightDir[3] = nullptr, const float bodyLight[3] = nullptr, int lightEnable = 0, float alpha = 1.0f, int chromeVariant = 0, float chromeTimeTerm = 0.0f);
    void Unbind();

    // GL-only -- called from BindGL(). D3D11 texture binding (DXP-16 increment 2) goes through
    // BindD3D11() directly instead (RHI::BindTexture per texID param), mirroring BindGL's own
    // renderMode-gated slot selection rather than routing through this setter.
    void SetTexture(GLuint texID, int slot = 0);

    GLuint GetProgram() const { return m_Program; }
    bool IsCreated() const { return m_Program != 0 || m_D3DVertexShader != nullptr; }

private:
    BMDMeshShader() = default;
    ~BMDMeshShader();

    BMDMeshShader(const BMDMeshShader&) = delete;
    BMDMeshShader& operator=(const BMDMeshShader&) = delete;

    void CreateGL();
    void DestroyGL();
    void BindGL(int renderMode, float waveOffsetU, float waveOffsetV, GLuint texID, const float mvp[16], int useGPUSkin, const float bodyOrigin[3], float bodyScale, float chromeWave, GLuint chromeTex1ID, const float specularTint[3], GLuint metalTexID, GLuint chromeTex2ID, float chromeWave2, float chromeLightVecX, float chromeLightVecY, const float lightDir[3], const float bodyLight[3], int lightEnable, float alpha, int chromeVariant, float chromeTimeTerm);

    void CreateD3D11();
    void DestroyD3D11();
    void BindD3D11(int renderMode, float waveOffsetU, float waveOffsetV, GLuint texID, int useGPUSkin, const float bodyOrigin[3], float bodyScale, float chromeWave, GLuint chromeTex1ID, const float specularTint[3], GLuint metalTexID, GLuint chromeTex2ID, float chromeWave2, float chromeLightVecX, float chromeLightVecY, const float lightDir[3], const float bodyLight[3], int lightEnable, float alpha, int chromeVariant, float chromeTimeTerm, const float mvp[16]);

    GLuint m_Program = 0;
    GLuint m_VertShader = 0;
    GLuint m_FragShader = 0;

    GLint m_LocTex        = -1;
    GLint m_LocChromeTex1 = -1; // RenderMode 4+ (item specular): sampler, "Chrome1" texture (BITMAP_CHROME)
    GLint m_LocMetalTex   = -1; // RenderMode 5+ (item specular tier 2+, CHROME_METAL/FULL_SPECULAR): static MatCap sampler (BITMAP_SHINY)
    GLint m_LocChromeTex2 = -1; // RenderMode 6/7 (item specular tier 3/4): animated sampler (BITMAP_CHROME2)

    // GLS-09: all non-sampler BMDMeshShader uniforms, consolidated into one std140 UBO
    // (binding slot 6) instead of ~18 discrete glUniform* calls per draw. Field order/
    // offsets copied verbatim from dx-only-port's already-shipped, static_assert-verified
    // D3D11 BMDFlagsCB (BMDMeshShader.cpp:673-696 on that branch) so a future merge of that
    // branch's D3D11 backend can share this exact layout instead of reconciling two.
    struct BMDFlagsCB {
        float   mvpDraw[16];
        int32_t useGPUSkin;
        int32_t renderMode;
        float   waveOffsetU;
        float   waveOffsetV;
        float   bodyOrigin[3];
        float   bodyScale;
        float   chromeWave;
        float   chromeWave2;
        float   chromeLightVec[2];
        int32_t chromeVariant;
        float   chromeTimeTerm;
        float   alpha;
        float   alphaRef;
        float   lightDir[3];
        int32_t lightEnable;
        float   bodyLight[3];
        float   _pad0;
        float   specularTint[3];
        float   _pad1;
    };
    static_assert(sizeof(BMDFlagsCB) == 176, "BMDFlagsCB must match the GLSL BMDFlags block layout byte-for-byte");

    RHI::BufferHandle m_BMDFlagsUBO;

    // ---- D3D11 state (DXP-14 increment 3) ----
    // Shader objects stored as void* so this header doesn't need <d3d11.h> -- the .cpp casts
    // under its own D3D11 block. Only valid when g_RenderBackend == RenderBackend::D3D11.
    void* m_D3DVertexShader = nullptr;
    void* m_D3DPixelShader  = nullptr;
    // DXP-16 increment 3: no longer owns a sampler -- RHI::BindTexture() binds each texture's own
    // recorded filter/wrap sampler at bind time (RHI_D3D11.cpp's GetOrCreateSampler).
    // BMDFlags cbuffer, slot 6 (0=GlobalMatrices, 1=SceneData, 2=BoneUBO, 3=DynamicLights
    // reserved, 4=PassthroughShader's PassFlags, 5=TerrainShader's TerrainFlags -- each shader
    // with loose GLSL uniforms gets its own slot, see TerrainShader.h's field comment for why).
    // D3D11 uses its own file-local BMDFlagsCB (BMDMeshShader.cpp, anonymous namespace) --
    // byte-identical layout to the GL struct above but kept as a separate definition since the
    // header must stay <d3d11.h>-free; unifying them is a possible future cleanup, not required
    // for correctness (RHI::UpdateUniformBlock only needs a matching byte layout, not the same type).
    RHI::BufferHandle m_D3DFlagsCBuffer;
};
