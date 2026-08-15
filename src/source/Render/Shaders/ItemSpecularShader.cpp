#include "stdafx.h"
#include "Render/Shaders/ItemSpecularShader.h"

// ===========================================================================
// Singleton
// ===========================================================================
CItemSpecularShader& CItemSpecularShader::Instance()
{
    static CItemSpecularShader s_instance;
    return s_instance;
}

bool CItemSpecularShader::Init()
{
    // No GL program to compile anymore (DXP-02 cleanup) — this class is pure
    // variant/tint bookkeeping, always available once the engine has started.
    m_bInitialized = true;
    m_bSupported   = true;
    return true;
}

void CItemSpecularShader::Shutdown()
{
    m_bSupported    = false;
    m_activeVariant = SHADER_VARIANT_NONE;
}

bool CItemSpecularShader::Begin(EShaderVariant variant,
                                GLuint /*baseTex*/, GLuint /*animChromeTex*/,
                                GLuint /*metalTex*/, GLuint /*chrome1Tex*/,
                                const float* bodyLight, const float* specularTint)
{
    if (!IsSupported() || variant <= SHADER_VARIANT_NONE || variant >= SHADER_VARIANT_COUNT)
        return false;

    m_activeVariant = variant;
    if (specularTint) {
        m_specularTint[0] = specularTint[0];
        m_specularTint[1] = specularTint[1];
        m_specularTint[2] = specularTint[2];
    } else if (bodyLight) {
        m_specularTint[0] = bodyLight[0];
        m_specularTint[1] = bodyLight[1];
        m_specularTint[2] = bodyLight[2];
    }

    return true;
}

void CItemSpecularShader::End()
{
    m_activeVariant = SHADER_VARIANT_NONE;
}
