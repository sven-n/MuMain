#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"

// TerrainShader handles multi-texture blended terrain rendering using VBOs.
// DXP-14 increment 2: HLSL port, same Create()/Destroy()/Bind() backend-branch pattern as
// PassthroughShader (DXP-14 increment 1) -- see that class's header for the general shape.
class TerrainShader {
public:
    static TerrainShader& Instance();

    void Create();
    void Destroy();

    void Bind();
    void Unbind();

    // GL-only this increment -- D3D11 texture binding lands with DXP-16 (world pipelines),
    // the first real caller of a textured terrain draw under this backend.
    void SetBaseTexture(GLuint texID);
    void SetOverlayTexture(GLuint texID);

    // Per-tile UV scale, mirroring legacy FaceTexture()'s `Width = 64.f/b->Width` (already divided
    // by TERRAIN_SCALE by the caller so it's ready to multiply world-space a_Pos.xy directly).
    // Base and overlay get independent scales because tex1/tex2 can be differently-sized bitmaps.
    void SetUVScale(float baseScaleX, float baseScaleY, float overlayScaleX, float overlayScaleY);

    // TASK-30 phase 2: water UV scroll. `waterMove` set once per frame (matches the CPU `WaterMove`
    // global, itself recomputed once per frame in RenderTerrain()); the base/overlay water flags are
    // set once per tile draw, alongside SetBaseTexture()/SetOverlayTexture().
    void SetWaterMove(float waterMove);
    void SetWaterFlags(bool baseIsWater, bool overlayIsWater);

    // Fixed-function alpha test threshold mirror (DXP-01). Called once per tile draw, alongside
    // SetWaterFlags() — dirty-checked internally so it's a no-op glUniform-wise almost always.
    void SyncAlphaRef();

    GLuint GetProgram() const { return m_Program; }
    bool IsCreated() const { return m_Program != 0 || m_D3DVertexShader != nullptr; }

private:
    TerrainShader() = default;
    ~TerrainShader();

    TerrainShader(const TerrainShader&) = delete;
    TerrainShader& operator=(const TerrainShader&) = delete;

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

    GLint m_LocBaseTex = -1;
    GLint m_LocOverlayTex = -1;
    GLint m_LocWaterMove = -1;
    GLint m_LocBaseIsWater = -1;
    GLint m_LocOverlayIsWater = -1;
    GLint m_LocAlphaRef = -1;
    GLint m_LocBaseUVScale = -1;
    GLint m_LocOverlayUVScale = -1;

    // ---- D3D11 state (DXP-14) ----
    // Shader objects stored as void* so this header doesn't need <d3d11.h> -- the .cpp casts
    // under its own D3D11 block. Only valid when g_RenderBackend == RenderBackend::D3D11.
    void* m_D3DVertexShader = nullptr;
    void* m_D3DPixelShader  = nullptr;
    // DXP-16 increment 3: no longer owns a sampler -- RHI::BindTexture() binds each texture's own
    // recorded filter/wrap sampler at bind time (RHI_D3D11.cpp's GetOrCreateSampler). See
    // TerrainShader.cpp's CreateD3D11 comment for why a shared per-shader sampler was the bug.
    // TerrainFlags cbuffer, slot 5 (0=GlobalMatrices, 1=SceneData, 2=BoneUBO, 3=DynamicLights
    // reserved, 4=PassthroughShader's PassFlags -- each shader with loose GLSL uniforms gets
    // its OWN slot number, not a shared one: D3D11 constant-buffer slot bindings live on the
    // context, not the shader object, so two shaders reusing one slot would each stomp the
    // other's binding the moment both existed at once, whereas per-shader slots never collide).
    RHI::BufferHandle m_D3DFlagsCBuffer;
    float m_D3DWaterMove = 0.0f;
    int   m_D3DBaseIsWater = 0;
    int   m_D3DOverlayIsWater = 0;
    // -1 sentinel (both flags are always 0/1) forces the first SetWaterFlags() call after Create()
    // to upload, same trick as m_LastAlphaRef below -- dirty-checks the per-tile write on both
    // backends: D3D11's UploadD3D11Flags() cbuffer rewrite and GL's glUniform1i pair (GLP-04) --
    // shared by both branches of SetWaterFlags() so back-to-back tiles sharing the same water-ness
    // (the common case) skip the redundant write instead of paying it on every one of the
    // thousands of tile draws in a frustum pass.
    int   m_LastBaseIsWater = -1;
    int   m_LastOverlayIsWater = -1;

    // Sentinel guaranteed to differ from any real g_AlphaRef value, forcing the first
    // SyncAlphaRef() call after Create() to upload u_AlphaRef. Doubles as the D3D11
    // TerrainFlags cbuffer's alphaRef field (always a real value post-Create on both backends).
    float m_LastAlphaRef = -12345.0f;

    // Dirty-check cache for SetUVScale, same reasoning as m_LastAlphaRef -- avoid a redundant
    // glUniform2f pair on the (common) case of consecutive tiles sharing both textures.
    float m_LastBaseUVScale[2] = { -12345.0f, -12345.0f };
    float m_LastOverlayUVScale[2] = { -12345.0f, -12345.0f };
};
