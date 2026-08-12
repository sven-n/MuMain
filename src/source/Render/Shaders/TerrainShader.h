#pragma once

#include "stdafx.h"

// TerrainShader handles multi-texture blended terrain rendering using VBOs.
class TerrainShader {
public:
    static TerrainShader& Instance();

    void Create();
    void Destroy();

    void Bind();
    void Unbind();

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
    bool IsCreated() const { return m_Program != 0; }

private:
    TerrainShader() = default;
    ~TerrainShader();

    TerrainShader(const TerrainShader&) = delete;
    TerrainShader& operator=(const TerrainShader&) = delete;

    void CreateGL();
    void DestroyGL();
    void BindGL();

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

    // Sentinel guaranteed to differ from any real g_AlphaRef value, forcing the first
    // SyncAlphaRef() call after Create() to upload u_AlphaRef.
    float m_LastAlphaRef = -12345.0f;

    // Dirty-check cache for SetUVScale, same reasoning as m_LastAlphaRef -- avoid a redundant
    // glUniform2f pair on the (common) case of consecutive tiles sharing both textures.
    float m_LastBaseUVScale[2] = { -12345.0f, -12345.0f };
    float m_LastOverlayUVScale[2] = { -12345.0f, -12345.0f };
};
