#include "stdafx.h"

#include "Render/Effects/EffectDraw.h"

#include "Render/Renderer/MuRenderer.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "World/GameMaps/GMHellas.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <cstring>
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
    if (state.depthMask == Toggle::On)
        EnableDepthMask();
    else if (state.depthMask == Toggle::Off)
        DisableDepthMask();
    switch (state.blend)
    {
    case Blend::Unchanged:
        break;
    case Blend::Off:
        DisableAlphaBlend();
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
    if (state.texture >= 0)
        BindTexture(state.texture);
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

    void DrawQuads(const QuadsDraw& draw) override
    {
        mu::GetRenderer().RenderQuad3D(draw.vertices, draw.texture);
    }

    void DrawDecal(const DecalDraw& draw) override
    {
        vec3_t light = {draw.light[0], draw.light[1], draw.light[2]};
        if (draw.water)
            RenderWaterTerrain(draw.texture, draw.x, draw.y, draw.sizeX, draw.sizeY, light, draw.rotation, draw.alpha,
                               draw.height);
        else
            RenderTerrainAlphaBitmap(draw.texture, draw.x, draw.y, draw.sizeX, draw.sizeY, light, draw.rotation,
                                     draw.alpha, draw.height);
    }

    void DrawPlane(const PlaneDraw& draw) override
    {
        float rotation[3][4];
        std::memcpy(rotation, draw.rotation, sizeof(rotation));
        mu::GetRenderer().PushMatrix();
        mu::GetRenderer().Translate(draw.position[0], draw.position[1], draw.position[2]);
        RenderPlane3D(draw.width, draw.height, rotation);
        mu::GetRenderer().PopMatrix();
    }

    void DrawCircle(const CircleDraw& draw) override
    {
        vec3_t position = {draw.position[0], draw.position[1], draw.position[2]};
        RenderCircle(draw.texture, position, draw.scaleBottom, draw.scaleTop, draw.height, draw.rotation, draw.lightTop,
                     draw.textureV);
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
