#ifndef ITEM_SPECULAR_SHADER_H
#define ITEM_SPECULAR_SHADER_H

#pragma once

#include "stdafx.h"

// Item-specular tier variant/tint bookkeeping.
//
// Historically this class also owned a set of GLSL 1.20 programs (gl_MultiTexCoord0/1/2,
// gl_ModelViewProjectionMatrix) that drew the multi-texture specular blend directly via
// immediate-mode client-array texcoord streams. That drawing responsibility has moved to
// BMDMeshShader's render modes 4-7 (DXP-02), which reproduce each variant's UV/blend formulas
// on the GPU-skinned static VBO path — see BMDMeshShader.cpp and BMD::RenderMesh(). This class
// now only tracks which variant is active and the per-item tint color, so ZzzObject.cpp's
// existing Begin()/End() call sites (which select the variant per equipped item) and
// BMD::RenderMesh()'s GPU path (which reads GetActiveVariant()/GetSpecularTint()) stay in sync
// without threading new parameters through the whole call chain.
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
    static CItemSpecularShader& Instance();

    bool Init();
    void Shutdown();
    bool IsSupported() const { return m_bInitialized && m_bSupported; }

    /**
     * @brief Marks a variant active for the item about to be rendered via BMDMeshShader's GPU
     *        item-specular render modes (4-7). Texture IDs are accepted for call-site source
     *        compatibility with ZzzObject.cpp but are no longer used here — BMD::RenderMesh()
     *        resolves its own texture IDs directly (see the DXP-02 increments in ZzzBMD.cpp).
     * @param specularTint Per-item RGB color vector from PartObjectColor to preserve base armor color.
     */
    bool Begin(EShaderVariant variant,
               GLuint baseTex, GLuint animChromeTex, GLuint metalTex, GLuint chrome1Tex,
               const float* bodyLight, const float* specularTint = nullptr);
    void End();

    EShaderVariant GetActiveVariant() const { return m_activeVariant; }

    // Called by ZzzObject before Begin() so ZzzBMD.cpp can compute the correct
    // g_chrome[] UV array for RenderArrayTexCoords1 (animated) during the base pass.
    // Pass the RENDER_CHROME2 or RENDER_CHROME4 flag constant for the variant.
    void  SetChromePrepassFlags(int flags) { m_chromePrepassFlags = flags; }
    int   GetChromePrepassFlags() const    { return m_chromePrepassFlags; }

    // Per-item tint stored by Begin() so BMD::RenderMesh()'s GPU chrome path (TASK-28 increment 2)
    // can read it without threading a new parameter through RenderMesh()'s call chain.
    const float* GetSpecularTint() const { return m_specularTint; }

private:
    CItemSpecularShader() = default;
    ~CItemSpecularShader() = default;

    bool           m_bInitialized = false;
    bool           m_bSupported   = false;
    int            m_chromePrepassFlags = 0; // RENDER_CHROME / RENDER_CHROME2 / RENDER_CHROME4
    EShaderVariant m_activeVariant = SHADER_VARIANT_NONE;
    float          m_specularTint[3] = { 1.f, 1.f, 1.f };
};

#endif // ITEM_SPECULAR_SHADER_H
