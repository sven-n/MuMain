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
// The values of AlphaBlendType and the functions that set them
// (ZzzOpenglUtil.cpp).
constexpr int NoBlend = 0;        // DisableAlphaBlend
constexpr int LightMapBlend = 1;  // EnableLightMap
constexpr int AlphaTestBlend = 2; // EnableAlphaTest
constexpr int GlowBlend = 3;      // EnableAlphaBlend
constexpr int SubtractBlend = 4;  // EnableAlphaBlendMinus
constexpr int LuminanceBlend = 5; // EnableAlphaBlend2
constexpr int AlphaBlend3 = 6;    // EnableAlphaBlend3
constexpr int MixedBlend = 7;     // EnableAlphaBlend4

void ApplyBlend(int blendType)
{
    mu::IMuRenderer& renderer = mu::GetRenderer();
    AlphaBlendType = blendType;
    switch (blendType)
    {
    case LightMapBlend:
        renderer.SetBlendMode(mu::BlendMode::LightMap);
        return;
    case AlphaTestBlend:
    case AlphaBlend3:
        renderer.SetBlendMode(mu::BlendMode::Alpha);
        return;
    case GlowBlend:
        renderer.SetBlendMode(mu::BlendMode::Glow);
        return;
    case SubtractBlend:
        renderer.SetBlendMode(mu::BlendMode::Subtract);
        return;
    case LuminanceBlend:
        renderer.SetBlendMode(mu::BlendMode::Luminance);
        return;
    case MixedBlend:
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
