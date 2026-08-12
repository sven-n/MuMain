#pragma once

#include "stdafx.h"

// PassthroughShader handles textured and untextured geometry rendered via modern VBOs.
class PassthroughShader {
public:
    static PassthroughShader& Instance();

    void Create();
    void Destroy();

    void Bind();
    void Unbind();

    void SetTexture(GLuint texID, int slot = 0);
    void SetUseTexture(bool use);
    void SetUseFog(bool use);
    // FFP GL_TEXTURE_ENV_MODE=GL_ADD port (DXP-08a): fragColor = texture.rgb + vertexColor.rgb
    // (alpha stays multiplicative) instead of the default modulate (texture * vertexColor).
    void SetTexCombineAdd(bool add);

    GLuint GetProgram() const { return m_Program; }
    bool IsCreated() const { return m_Program != 0; }

    // GLP-19: IR merges consecutive Begin/End pairs into one draw and needs to know whether they
    // share shader state. Exposes the dirty-check sentinels rather than the requested values --
    // the sentinels are what actually reached the GPU, which is what the batch was drawn with.
    struct StateSnapshot { int useTexture; int useFog; int texCombineAdd; };
    StateSnapshot GetStateSnapshot() const
    {
        return StateSnapshot{ m_LastUseTexture, m_LastUseFog, m_LastTexCombineAdd };
    }

private:
    PassthroughShader() = default;
    ~PassthroughShader();

    PassthroughShader(const PassthroughShader&) = delete;
    PassthroughShader& operator=(const PassthroughShader&) = delete;

    void CreateGL();
    void DestroyGL();
    void BindGL();

    // ---- GL state ----
    GLuint m_Program = 0;
    GLuint m_VertShader = 0;
    GLuint m_FragShader = 0;

    GLint m_LocTex = -1;
    GLint m_LocUseTexture = -1;
    GLint m_LocUseFog = -1;
    GLint m_LocAlphaRef = -1;
    GLint m_LocTexCombineAdd = -1;

    // Sentinel guaranteed to differ from any real g_AlphaRef value (-1.0f or a GL_GREATER
    // threshold), forcing the first Bind() after Create() to upload u_AlphaRef.
    float m_LastAlphaRef = -12345.0f;

    // Tri-state (-1 = unset, 0/1 = last value sent) so the first call after program (re)creation
    // always uploads.
    int m_LastUseTexture = -1;
    int m_LastUseFog = -1;
    int m_LastTexCombineAdd = -1;
};
