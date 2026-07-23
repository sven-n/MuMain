#ifndef ITEM_SPECULAR_SHADER_H
#define ITEM_SPECULAR_SHADER_H

#pragma once

#include "stdafx.h"
#include <SDL3/SDL.h>

// 4 variants mapping exactly to the 4 fixed-function pipeline tiers.
//
// The original engine renders items with up to 4 additive passes:
//   Pass 1: Base * BodyLight * bodyLightScale
//   Pass 2: Chrome (animated UV = g_chrome + blendOffset)  [V3/V4 only]
//   Pass 3: Metal  (static UV  = g_chrome)                 [V2/V3/V4]
//   Pass 4: Chrome1(static UV  = g_chrome)                 [V1/V2/V3/V4]
//
// Each GLSL variant combines all its passes into one draw call.
// UV coord streams:
//   GL_TEXTURE0 coord (v_TexCoord0): base mesh UVs
//   GL_TEXTURE1 coord (v_TexCoord1): animated chrome UV (g_chrome + blendOffset)
//   GL_TEXTURE2 coord (v_TexCoord2): static  chrome UV (g_chrome only)
//   GL_TEXTURE3 coord (v_TexCoord2): static  chrome UV (same, reused for Chrome1)

enum EShaderVariant
{
    SHADER_VARIANT_NONE           = 0,
    SHADER_VARIANT_CHROME_1       = 1,  // +7/+8:   Base + Chrome1(static)
    SHADER_VARIANT_CHROME_METAL   = 2,  // +9/+10:  Base + Chrome1(static) + Metal(static)
    SHADER_VARIANT_FULL_SPECULAR_V1 = 3,// +11/+12: Base + Chrome2(anim) + Metal(static) + Chrome1(static)
    SHADER_VARIANT_FULL_SPECULAR_V2 = 4,// +13/+15: Base + Chrome2(anim) + Metal(static) + Chrome1(static)
    SHADER_VARIANT_COUNT          = 5
};

class CItemSpecularShader
{
public:
    struct ShaderProgram
    {
        GLuint hProgram        = 0;
        GLuint hVertexShader   = 0;
        GLuint hFragmentShader = 0;

        // Sampler uniforms
        GLint locBaseTex       = -1;
        GLint locAnimChromeTex = -1;   // GL_TEXTURE1 (animated)
        GLint locMetalTex      = -1;   // GL_TEXTURE2 (static)
        GLint locChrome1Tex    = -1;   // GL_TEXTURE3 (static)

        // Light uniform
        GLint locBodyLight     = -1;   // pre-scaled light (Light * bodyLightScale)

        // GPU UV uniforms
        GLint locWorldTime     = -1;
        GLint locPrepassFlags  = -1;
    };

    static CItemSpecularShader& Instance();

    bool Init();
    void Shutdown();
    bool IsSupported() const { return m_bInitialized && m_bSupported; }

    bool Begin(EShaderVariant variant,
               GLuint baseTex, GLuint animChromeTex, GLuint metalTex, GLuint chrome1Tex,
               const float* bodyLight);
    void End();

    void SubmitTexCoords(const float texBase[][2], const float texAnim[][2], const float texStatic[][2]);
    void DisableTexCoords();

    EShaderVariant GetActiveVariant() const { return m_activeVariant; }

    // Called by ZzzObject before Begin() so ZzzBMD.cpp can compute the correct
    // g_chrome[] UV array for RenderArrayTexCoords1 (animated) during the base pass.
    // Pass the RENDER_CHROME2 or RENDER_CHROME4 flag constant for the variant.
    void  SetChromePrepassFlags(int flags) { m_chromePrepassFlags = flags; }
    int   GetChromePrepassFlags() const    { return m_chromePrepassFlags; }

private:
    CItemSpecularShader();
    ~CItemSpecularShader();

    bool LoadGLFunctions();
    bool CompileShaders();

    bool           m_bInitialized = false;
    bool           m_bSupported   = false;
    int            m_chromePrepassFlags = 0; // RENDER_CHROME / RENDER_CHROME2 / RENDER_CHROME4
    EShaderVariant m_activeVariant = SHADER_VARIANT_NONE;

    ShaderProgram m_programs[SHADER_VARIANT_COUNT];
};

#endif // ITEM_SPECULAR_SHADER_H
