#include "stdafx.h"

#ifdef _EDITOR

#include "SavedGameRenderState.h"

#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

// The flags of the game's render state functions that ZzzOpenglUtil.h does
// not declare, and the bone scale the models multiply by.
extern bool AlphaTestEnable;
extern int AlphaBlendType;
extern float BoneScale;

namespace MuEditor::Effects
{
namespace
{
// The values of AlphaBlendType the render state functions set.
constexpr int NoBlend = 0;
constexpr int GlowBlend = 3;

// The blend of each value of AlphaBlendType (ZzzOpenglUtil.cpp): 1 is
// EnableLightMap, 2 EnableAlphaTest, 3 EnableAlphaBlend, 4
// EnableAlphaBlendMinus, 5, 6 and 7 EnableAlphaBlend2, 3 and 4.
void ApplyBlend(int blendType)
{
    mu::IMuRenderer& renderer = mu::GetRenderer();
    AlphaBlendType = blendType;
    switch (blendType)
    {
    case 1:
        renderer.SetBlendMode(mu::BlendMode::LightMap);
        return;
    case 2:
    case 6:
        renderer.SetBlendMode(mu::BlendMode::Alpha);
        return;
    case 3:
        renderer.SetBlendMode(mu::BlendMode::Glow);
        return;
    case 4:
        renderer.SetBlendMode(mu::BlendMode::Subtract);
        return;
    case 5:
        renderer.SetBlendMode(mu::BlendMode::Luminance);
        return;
    case 7:
        renderer.SetBlendMode(mu::BlendMode::Mixed);
        return;
    default:
        renderer.DisableBlend();
        AlphaBlendType = NoBlend;
        return;
    }
}

void ApplyFlags(bool texture, bool depthTest, bool cullFace, bool depthMask, bool alphaTest)
{
    mu::IMuRenderer& renderer = mu::GetRenderer();
    TextureEnable = texture;
    renderer.SetTexture2D(texture);
    DepthTestEnable = depthTest;
    renderer.SetDepthTest(depthTest);
    CullFaceEnable = cullFace;
    renderer.SetCullFace(cullFace);
    DepthMaskEnable = depthMask;
    renderer.SetDepthMask(depthMask);
    AlphaTestEnable = alphaTest;
    renderer.SetAlphaTest(alphaTest);
}
} // namespace

SavedGameRenderState::SavedGameRenderState()
    : m_texture(TextureEnable), m_depthTest(DepthTestEnable), m_cullFace(CullFaceEnable), m_depthMask(DepthMaskEnable),
      m_alphaTest(AlphaTestEnable), m_blendType(AlphaBlendType), m_fog(FogEnable), m_boneScale(BoneScale)
{
    FogEnable = false;
    mu::GetRenderer().SetFogEnabled(false);
    ApplyBlend(NoBlend);
    ApplyFlags(true, true, false, true, false);
}

// The render state functions turn fog off with the glow blend and on with
// the others while the game has fog.
SavedGameRenderState::~SavedGameRenderState()
{
    FogEnable = m_fog;
    ApplyBlend(m_blendType);
    ApplyFlags(m_texture, m_depthTest, m_cullFace, m_depthMask, m_alphaTest);
    mu::GetRenderer().SetFogEnabled(m_fog && m_blendType != GlowBlend);
    BoneScale = m_boneScale;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
