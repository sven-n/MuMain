#include "stdafx.h"

#include "Render/Effects/EffectDraw.h"

#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <optional>

namespace Render::EffectDrawing
{
namespace
{
void Apply(const EffectState& state)
{
    if (state.additiveTexture != Toggle::Unchanged)
        mu::GetRenderer().SetTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
                                    state.additiveTexture == Toggle::On ? GL_ADD : GL_MODULATE);
    if (state.depthTest == Toggle::On)
        EnableDepthTest();
    else if (state.depthTest == Toggle::Off)
        DisableDepthTest();
    switch (state.blend)
    {
    case Blend::Unchanged:
        break;
    case Blend::AlphaTest:
        EnableAlphaTest();
        break;
    case Blend::AlphaTestKeepDepthMask:
        EnableAlphaTest(false);
        break;
    case Blend::Glow:
        EnableAlphaBlend();
        break;
    case Blend::Subtract:
        EnableAlphaBlendMinus();
        break;
    case Blend::Blend2:
        EnableAlphaBlend2();
        break;
    case Blend::Blend3:
        EnableAlphaBlend3();
        break;
    }
}

class DefaultEffectDrawer final : public EffectDrawer
{
public:
    void SetState(const EffectState& state) override
    {
        Apply(state);
    }

    void DrawSprite(const SpriteDraw& draw) override
    {
        Apply(draw.state);
        std::optional<mu::ScopedBloomExclusion> exclusion;
        if (draw.bloomExcluded)
            exclusion.emplace();
        vec3_t position = {draw.position[0], draw.position[1], draw.position[2]};
        vec3_t light = {draw.light[0], draw.light[1], draw.light[2]};
        RenderSprite(draw.texture, position, draw.width, draw.height, light, draw.rotation, draw.u, draw.v, draw.uWidth,
                     draw.vHeight);
    }
};

DefaultEffectDrawer s_default;
EffectDrawer* s_active = &s_default;
} // namespace

EffectDrawer& ActiveEffectDrawer()
{
    return *s_active;
}

void SetEffectDrawer(EffectDrawer* drawer)
{
    s_active = drawer ? drawer : &s_default;
}
} // namespace Render::EffectDrawing
