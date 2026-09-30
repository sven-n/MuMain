#include "stdafx.h"

#include "ItemRenderStyles.h"
#include "ItemModelTable.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <string_view>

namespace Render::Items::Styles
{
namespace
{
// A recipe draws the model and returns true, or returns false when the style
// is not for this drawing (the drawing code then draws the model plainly).
using StyleFunction = bool (*)(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType);
// A recipe that styles share with a texture of their own each.
using TexturedStyleFunction = bool (*)(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture);
// `Texture` is the texture of the glow pass.
using GlowFunction = void (*)(BMD* b, OBJECT* o, float Alpha, int RenderType, int Texture);

struct RenderStyle
{
    // `glowPass`: the glow pass of items +7 and up, for the few styles that
    // glow differently from their "glow" values.
    constexpr RenderStyle(const char* styleName, StyleFunction recipe, GlowFunction glowPass = nullptr)
        : name(styleName), render(recipe), glow(glowPass)
    {
    }

    // Styles that shine below +3; the shine is a named constant.
    constexpr RenderStyle(const char* styleName, StyleFunction recipe, const ShineBelowPlus3* shineBelowPlus3)
        : name(styleName), render(recipe), shine(shineBelowPlus3)
    {
    }

    // Styles that share a recipe and only differ in its texture; they cannot
    // leave the texture out.
    constexpr RenderStyle(const char* styleName, TexturedStyleFunction recipe, int recipeTexture,
                          const ShineBelowPlus3* shineBelowPlus3 = nullptr)
        : name(styleName), texturedRender(recipe), texture(recipeTexture), shine(shineBelowPlus3)
    {
    }

    bool Render(BMD* b, OBJECT* o, int modelType, float alpha, int renderType) const
    {
        return texturedRender != nullptr ? texturedRender(b, o, modelType, alpha, renderType, texture)
                                         : render(b, o, modelType, alpha, renderType);
    }

    const char* name;
    StyleFunction render = nullptr;
    TexturedStyleFunction texturedRender = nullptr;
    int texture = 0;
    GlowFunction glow = nullptr;
    const ShineBelowPlus3* shine = nullptr;
};

// ------------------------------------------------ the shine below +3

constexpr ShineBelowPlus3 HarmonyShine{
    1.f, {{{1.5f, RENDER_CHROME2 | RENDER_BRIGHT, 1.5f}, {1.f, RENDER_CHROME4 | RENDER_BRIGHT, 1.f}}}};
constexpr ShineBelowPlus3 SealShine{
    0.9f, {{{1.5f, RENDER_CHROME2 | RENDER_BRIGHT, 1.5f}, {1.f, RENDER_CHROME4 | RENDER_BRIGHT, 1.f}}}};
// Drawn with the light the model has.
constexpr ShineBelowPlus3 CursedCastleWaterShine{
    std::nullopt, {{{0.5f, RENDER_TEXTURE | RENDER_BRIGHT, 0.5f}, {1.f, RENDER_CHROME4 | RENDER_BRIGHT, 1.f}}}};

// ------------------------------------------------ the recipes

// For styles that only shine below +3: the drawing code draws the model.
bool RenderPlainly(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    return false;
}

bool RenderStormCrow(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderBody(RENDER_TEXTURE | RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
    return true;
}

bool RenderThunderHawk(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    Vector(0.85f * Light[0], 0.85f * Light[1], 1.2f * Light[2], b->BodyLight);
    b->RenderBody(RENDER_TEXTURE | RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
    VectorCopy(Light, b->BodyLight);
    return true;
}

bool RenderWingsOfDarkness(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Vector(0.8f, 0.6f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, 0.5f, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderWingOfStorm(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 0.7f, 0.5f, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);

    static float s_iTexAni = 0;
    s_iTexAni += FPS_ANIMATION_FACTOR;
    if (s_iTexAni > 15)
        s_iTexAni = 0;
    float fU = ((int)s_iTexAni / 4) * 0.25f;
    Vector(0.9f, 0.6f, 0.3f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, fU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    return true;
}

bool RenderWingOfRuin(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    float Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.3f;
    Vector(0.1f + Luminosity, 0.1f + Luminosity, 0.1f + Luminosity, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.8f;
    Vector(0.0f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_3RDWING_LAYER);
    return true;
}

bool RenderCapeOfEmperor(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    return true;
}

bool RenderWingsOfDespair(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME6, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderWingOfDimension(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderDivineSwordOfArchangel(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  WorldTime * 0.0001f, BITMAP_CHROME);
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderArchangelStaff(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  -WorldTime * 0.0001f, BITMAP_CHROME);
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderDivineScepterOfArchangel(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  -WorldTime * 0.0001f, BITMAP_CHROME);
    return true;
}

bool RenderGreatReignCrossbow(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    Vector(0.8f * Light[0], 0.f, 0.8f * Light[2], b->BodyLight);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, -WorldTime * 0.0002f,
                  o->BlendMeshTexCoordV, BITMAP_CHROME);
    VectorCopy(Light, b->BodyLight);
    b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_CHROME + 1);
    VectorCopy(Light, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderPumpkinOfLuck(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_DARK | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, -WorldTime * 0.0002f,
                  o->BlendMeshTexCoordV, BITMAP_CHROME);

    vec3_t vPos, vRelativePos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    b->TransformPosition(BoneTransform[8], vRelativePos, vPos, true);
    float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
    Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, o->Light);
    CreateSprite(BITMAP_LIGHT, vPos, 1.5f, o->Light, o, 0.5f);
    b->TransformPosition(BoneTransform[10], vRelativePos, vPos, true);
    CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
    b->TransformPosition(BoneTransform[11], vRelativePos, vPos, true);
    CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
    return true;
}

bool RenderSkillParchment(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.0f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh, BITMAP_ROOLOFPAPER_EFFECT_R);
    return true;
}

bool RenderRuneBlade(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    b->BeginRender(1.f);
    b->RenderMesh(3, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, sinf(WorldTime * 0.01f), 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_CHROME | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  WorldTime * 0.001f, BITMAP_CHROME);

    float Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
    Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, o->BlendMeshLight, WorldTime * 0.0001f,
                  -WorldTime * 0.0005f);
    b->EndRender();
    return true;
}

bool RenderDragonSpear(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, 1, o->BlendMeshLight, WorldTime * 0.0001f, WorldTime * 0.0005f);
    return true;
}

bool RenderElementalMace(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);

    float time = WorldTime * 0.001f;
    float Luminosity = sinf(time) * 0.5f + 0.3f;
    Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE, 1.f, 2, o->BlendMeshLight, time, -WorldTime * 0.0005f);
    return true;
}

bool RenderDarkHorse(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    Vector(0.8f, 0.4f, 0.1f, b->BodyLight);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderDarkRaven(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    Vector(0.3f, 0.8f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderBattleScepter(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
    b->BeginRender(1.f);
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    b->EndRender();
    return true;
}

bool RenderMasterScepter(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    Vector(0.1f, 0.3f, 1.f, b->BodyLight);
    o->BlendMesh = 0;
    o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Vector(0.6f, 0.8f, 1.f, b->BodyLight);
    o->BlendMesh = 1;
    o->BlendMeshLight = 1.f;
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0003f,
                  BITMAP_CHROME);
    return true;
}

bool RenderFlamberge(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // b->RenderBody( RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
    // 5 );

    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    o->BlendMesh = 1;
    Vector(1.f, 0.f, 0.2f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    float fV;
    fV = (((int)(WorldTime * 0.05) % 16) / 4) * 0.25f;
    b->RenderMesh(5, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 5, o->BlendMeshLight, o->BlendMeshTexCoordU, fV);
    return true;
}

bool RenderSwordBreaker(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderRuneBastardSword(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    o->BlendMesh = 1;
    o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderFrostMace(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderDeadlyStaff(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    o->BlendMesh = 1;
    o->BlendMeshLight = 1.f;
    Vector(1.f, 0.5f, 0.5f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderImperialStaff(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    o->BlendMesh = 0;
    o->BlendMeshLight = fabs(sinf(WorldTime * 0.001f)) + 0.1f;
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_SOCKETSTAFF);
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_SOCKETSTAFF);
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_SOCKETSTAFF);
    return true;
}

bool RenderStaff32(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderCrimsonGlory(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderSalamanderShield(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  1);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderFrostBarrier(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    o->BlendMesh = 2;
    o->BlendMeshLight = absf((sinf(WorldTime * 0.001f)));
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderGuardianShield(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderCrossShield(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0005f);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderOldScroll(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float sine = float(sinf(WorldTime * 0.002f) * 0.3f) + 0.7f;
    b->RenderBody(RenderType, 0.7f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 4, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
    return true;
}

bool RenderIllusionSorcererCovenant(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float sine = float(sinf(WorldTime * 0.00004f) * 0.15f) + 0.5f;
    b->RenderBody(RenderType, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
    b->RenderBody(RenderType, 1.f, 0.5f, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    return true;
}

bool RenderScrollOfBlood(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float sine = float(sinf(WorldTime * 0.002f) * 0.3f) + 0.7f;
    b->RenderBody(RenderType, 0.7f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    return true;
}

bool RenderCursedCastleWater(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;

    float PlaySpeed = 0.f;
    PlaySpeed = b->Actions[b->CurrentAction].PlaySpeed;

    b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, 2.f / 7.f, o->Position, o->Angle);
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight * 1.5f, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight / 4.0f, WorldTime * 0.005f,
                  -WorldTime * 0.005f, 0);

    vec3_t Light;
    vec3_t vRelativePos, vWorldPos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    Vector(1.f, 0.f, 0.0f, Light);

    b->TransformPosition(BoneTransform[1], vRelativePos, vWorldPos, true);
    CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
    return true;
}

bool RenderCondorFlame(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, 0.9, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderCondorFeather(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, 0.9, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderThirdClassQuestItem(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    return true;
}

bool RenderAbyssalEye(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  BITMAP_ITEM_EFFECT_DEYE_R);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 0.2f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderFreeTicket(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    return true;
}

bool RenderChaosCard(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, 1);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    return true;
}

bool RenderRareItemTicketWithBrightMesh1(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderRareItemTicket(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    return true;
}

bool RenderTalismanOfLuck(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  BITMAP_LUCKY_CHARM_EFFECT53);
    return true;
}

bool RenderSeal(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderElitePotion(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_CHROME4, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderTextured(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderTexturedAllMeshes(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderGreatScepter(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    float Luminosity = WorldTime * 0.0005f;

    o->HiddenMesh = 2;
    o->BlendMesh = 2;
    o->BlendMeshLight = 1.f;
    b->BeginRender(1.f);
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    b->RenderMesh(2, RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, Luminosity, Luminosity);
    b->EndRender();
    return true;
}

bool RenderLordScepter(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    o->BlendMeshLight = 1.f;
    o->BlendMeshTexCoordU = WorldTime * 0.0008f;
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.3f + 0.7f;
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, Alpha, 0, o->BlendMeshLight, 0.f, o->BlendMeshTexCoordV);
    return true;
}

bool RenderKnightBlade(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);

    float Luminosity = sinf(WorldTime * 0.0008f) * 0.7f + 0.5f;
    b->RenderMesh(2, RENDER_TEXTURE, Alpha, 2, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    //. 날
    b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, Alpha, 3, o->BlendMeshLight, WorldTime * 0.0001f,
                  WorldTime * 0.0005f);
    return true;
}

bool RenderDarkReignBlade(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, WorldTime * 0.0005f, WorldTime * 0.0005f);
    o->HiddenMesh = 0;
    b->StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0005f);
    return true;
}

bool RenderHurricane(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    Vector(Light[0] * 0.3f, Light[1] * 0.8f, Light[1] * 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE | RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
    VectorCopy(Light, b->BodyLight);
    return true;
}

bool RenderSylphWindBow(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.0f);

    b->StreamMesh = 0;
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    if (Type == MODEL_BONE_BLADE)
    {
        b->BodyLight[0] = 1.0f;
        b->BodyLight[1] = 0.7f;
        b->BodyLight[2] = 0.4f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                      (int)WorldTime % 4000 * 0.0004f - 0.7f, o->BlendMeshTexCoordV, BITMAP_CHROME7);
        b->BodyLight[0] = 0.7f;
        b->BodyLight[1] = 0.7f;
        b->BodyLight[2] = 0.7f;
    }
    else if (Type == MODEL_GRAND_VIPER_STAFF)
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                      (int)WorldTime % 2000 * 0.0002f - 0.3f, (int)WorldTime % 2000 * 0.0002f - 0.3f,
                      BITMAP_CHROME_ENERGY);
    else
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                      (int)WorldTime % 4000 * 0.0002f - 0.3f, (int)WorldTime % 4000 * 0.0002f - 0.3f, BITMAP_CHROME6);
    b->StreamMesh = -1;

    b->EndRender();
    return true;
}

bool RenderExplosionBlade(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordU);
    float Luminosity = sinf(WorldTime * 0.0015f) * 0.03f + 0.3f;
    Vector(Luminosity, Luminosity, Luminosity + 0.1f, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, -2, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordU);

    o->Alpha = 0.5f;
    float Luminosity4 = sinf(WorldTime * 0.0025f) * 0.5f + 0.7f;
    Vector(0.4f, 0.4f, 0.8f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -2, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordU);
    return true;
}

bool RenderSylphidRay(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

bool RenderSwordDancer(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    o->HiddenMesh = 1;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, 0.5f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  BITMAP_LAVA);
    b->RenderMesh(1, RENDER_TEXTURE, 0.7f, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0009f);
    return true;
}

bool RenderAlbatrossBow(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    o->HiddenMesh = 1;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, 1.0f, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME5, 0.5f, -1, o->BlendMeshLight,
                  o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderPlatinaStaff(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    o->HiddenMesh = 1;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 1, o->BlendMeshLight, WorldTime * 0.0009f,
                  WorldTime * 0.0009f);
    return true;
}

bool RenderOnlyMesh2WithoutSkin(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

bool RenderOnlyMesh0WithoutSkin(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

bool RenderOnlyMesh1WithoutSkin(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

// The six sets from Violent Wind to Eternal Wing (39 to 44) each have a mesh
// or an inventory texture of their own.
constexpr int ViolentWindToEternalWingSetCount = 6;

bool IsViolentWindToEternalWingSet(int Type, int firstSet)
{
    return Type >= firstSet && Type < firstSet + ViolentWindToEternalWingSetCount;
}

bool RenderViolentWindToEternalWingHelm(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    // Other items are drawn plainly.
    if (!IsViolentWindToEternalWingSet(Type, MODEL_MISTERY_HELM))
    {
        return false;
    }
    if (b->HideSkin)
    {
        int anMesh[6] = {2, 1, 0, 2, 1, 2};
        b->RenderMesh(anMesh[Type - (MODEL_MISTERY_HELM)], RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                      o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderViolentWindToEternalWingArmor(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    // Other items are drawn plainly.
    if (!IsViolentWindToEternalWingSet(Type, MODEL_MISTERY_ARMOR))
    {
        return false;
    }
    if (b->HideSkin)
    {
        int nTexture = Type - (MODEL_MISTERY_ARMOR);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, BITMAP_INVEN_ARMOR + nTexture);
        for (int i = 1; i < b->NumMeshs; ++i)
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                          o->BlendMeshTexCoordV);
    }
    else
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderViolentWindToEternalWingPants(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    // Other items are drawn plainly.
    if (!IsViolentWindToEternalWingSet(Type, MODEL_MISTERY_PANTS))
    {
        return false;
    }
    if (b->HideSkin)
    {
        int nTexture = Type - (MODEL_MISTERY_PANTS);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, BITMAP_INVEN_PANTS + nTexture);
        for (int i = 1; i < b->NumMeshs; ++i)
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                          o->BlendMeshTexCoordV);
    }
    else
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderCharacterCard(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
    int _R_Type = 0;
    switch (Type)
    {
    case MODEL_MAGIC_GLADIATOR_CHARACTER_CARD:
        _R_Type = BITMAP_CHARACTERCARD_R_MA;
        break;
    case MODEL_DARK_LORD_CHARACTER_CARD:
        _R_Type = BITMAP_CHARACTERCARD_R_DA;
        break;
    case MODEL_SUMMONER_CHARACTER_CARD:
        _R_Type = BITMAP_CHARACTERCARD_R;
        break;
    default: // other items are drawn plainly
        return false;
    }
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  _R_Type);
    return true;
}

bool RenderDivineAndSuccubusSkin(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    int nTexture = 0;
    switch (Type)
    {
    case MODEL_FAITH_ARMOR:
        nTexture = BITMAP_SKIN_ARMOR_DEVINE;
        break;
    case MODEL_FAITH_PANTS:
        nTexture = BITMAP_SKIN_PANTS_DEVINE;
        break;
    case MODEL_ARMOR + 53:
        nTexture = BITMAP_SKIN_ARMOR_SUCCUBUS;
        break;
    case MODEL_PANTS + 53:
        nTexture = BITMAP_SKIN_PANTS_SUCCUBUS;
        break;
    default: // other items are drawn plainly
        return false;
    }
    if (b->HideSkin)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, nTexture);
        for (int i = 1; i < b->NumMeshs; ++i)
        {
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                          o->BlendMeshTexCoordV);
        }
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

bool RenderBrova(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(5, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, 0.4f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, 0.8f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderStrikerScepter(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(5, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(6, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderArrowViperBow(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
    b->BeginRender(1.f);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(6, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE, 1.f, 3, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(4, RENDER_TEXTURE, 1.f, 4, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(5, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

    b->EndRender();
    return true;
}

bool RenderLostMap(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Models[o->Type].StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0005f);
    Models[o->Type].StreamMesh = -1;
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderSymbolOfKundun(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0005f);
    b->StreamMesh = -1;

    Vector(1.f, 0.5f, 0.f, b->BodyLight);
    static float fMeshLight = 0.500f;
    static float fAdd = 0.01f;
    if (fMeshLight > 1.f)
    {
        fMeshLight = 1.00f;
        fAdd = -0.01f;
    }
    if (fMeshLight < 0.01f)
    {
        fMeshLight = 0.01f;
        fAdd = 0.01f;
    }
    b->RenderMesh(2, RENDER_TEXTURE, 1.0f, 2, fMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    fMeshLight += fAdd;

    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 0.3f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderStaffOfKundun(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, Alpha, 1, 0.2f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 0, sinf(WorldTime * 0.005f), o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_CHROME4 | RENDER_BRIGHT, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderDemonicStick(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderStormBlitzStick(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    float fLumi = (sinf(WorldTime * 0.002f) + 0.5f) * 0.5f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderGreatLordScepter(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 3, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderGrandSoulShield(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    b->BeginRender(1.f);

    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    Vector(Light[0] * 0.3f, Light[1] * 0.3f, Light[2] * 0.3f, b->BodyLight);
    b->RenderMesh(2, RENDER_COLOR, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

    VectorCopy(Light, b->BodyLight);
    b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, 1.f, 2, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.01f,
                  BITMAP_CHROME);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, 1, o->BlendMeshLight, (float)(rand() % 10) * 0.1f,
                  (float)(rand() % 10) * 0.1f);

    float Luminosity = sinf(WorldTime * 0.001f) * 0.4f + 0.6f;
    Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, o->BlendMeshLight, WorldTime * 0.0001f,
                  WorldTime * 0.0005f);
    b->EndRender();
    return true;
}

bool RenderElementalShield(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    b->RenderMesh(1, RENDER_TEXTURE, 0.8f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE, 0.5f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE, 1.f, 2, o->BlendMeshLight, WorldTime * 0.0005f, o->BlendMeshTexCoordV);
    b->RenderMesh(3, RENDER_TEXTURE, 1.f, 3, o->BlendMeshLight, (float)(rand() % 10) * 0.1f,
                  (float)(rand() % 10) * 0.1f);
    b->EndRender();
    return true;
}

bool RenderMonsterBattleBow(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Only in the hands of the Metal Balrog and the Orc Archer of Doom
    // (RENDER_EXTRA); otherwise drawn plainly.
    if (!(RenderType & RENDER_EXTRA))
    {
        return false;
    }
    RenderType -= RENDER_EXTRA;
    Vector(0.1f, 0.1f, 0.1f, b->BodyLight);
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderSiegePotion(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    if (o->HiddenMesh == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        Vector(0.1f, 0.5f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_METAL | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else if (o->HiddenMesh == 0)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        Vector(0.1f, 0.5f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_METAL | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    b->EndRender();
    return true;
}

bool RenderContractSummon(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    if (o->HiddenMesh == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else if (o->HiddenMesh == 0)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    b->EndRender();
    return true;
}

bool RenderLifeStone(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Vector(0.f, 0.5f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->EndRender();
    return true;
}

bool RenderAmmunition(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (g_isCharacterBuff(o, eBuff_InfinityArrow))
    {
        Vector(1.f, 0.8f, 0.2f, b->BodyLight);
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
}

bool RenderHelperNpcPlate(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Only on the helper NPCs (Luke and Leo the Helper, Helper Ellen), the only
    // characters with the flag of the PC room look; otherwise drawn plainly.
    if (o->m_bpcroom != TRUE)
    {
        return false;
    }
    if (Type == MODEL_PLATE_ARMOR)
    {
        vec3_t EndRelative, EndPos;
        Vector(0.f, 0.f, 0.f, EndRelative);

        b->TransformPosition(o->BoneTransform[0], EndRelative, EndPos, true);

        Vector(0.4f, 0.6f, 0.8f, o->Light);
        CreateSprite(BITMAP_LIGHT, EndPos, 6.0f, o->Light, o, 0.5f);

        float Luminosity;
        Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
        Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f, o->Light);
        CreateSprite(BITMAP_LIGHT, EndPos, 2.0f, o->Light, o);
    }

    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    Vector(0.9f, 0.7f, 1.0f, b->BodyLight);

    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_FENRIR_THUNDER);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_METAL, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_FENRIR_THUNDER);
    VectorCopy(Light, b->BodyLight);
    return true;
}

bool RenderSocketSeed(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Each seed has a color; other items are drawn plainly.
    if (Type < MODEL_SEED_FIRE || Type > MODEL_SEED_EARTH)
    {
        return false;
    }
    int iCategoryIndex = Type - (MODEL_SEED_FIRE) + 1;
    switch (iCategoryIndex)
    {
    case 1: // 0~9
        Vector(0.9f, 0.1f, 0.2f, b->BodyLight);
        break;
    case 2: // 10~15
        Vector(0.4f, 0.5f, 1.0f, b->BodyLight);
        break;
    case 3: // 16~20
        Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
        break;
    case 4: // 21~28
        Vector(0.4f, 1.0f, 0.6f, b->BodyLight);
        break;
    case 5: // 29~33
        Vector(1.0f, 0.8f, 0.4f, b->BodyLight);
        break;
    case 6: // 34~40
        Vector(1.0f, 0.4f, 1.0f, b->BodyLight);
        break;
    }
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderSocketSeedSphere(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    // Each sphere has the color of its seed; other items are drawn plainly.
    if (Type < MODEL_SEED_SPHERE_FIRE_1 || Type > MODEL_SEED_SPHERE_EARTH_5)
    {
        return false;
    }
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);

    int iCategoryIndex = (Type - (MODEL_SEED_SPHERE_FIRE_1)) % 6 + 1;
    switch (iCategoryIndex)
    {
    case 1: // 0~9
        Vector(0.9f, 0.1f, 0.2f, b->BodyLight);
        break;
    case 2: // 10~15
        Vector(0.4f, 0.5f, 1.0f, b->BodyLight);
        break;
    case 3: // 16~20
        Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
        break;
    case 4: // 21~28
        Vector(0.4f, 1.0f, 0.6f, b->BodyLight);
        break;
    case 5: // 29~33
        Vector(1.0f, 0.8f, 0.4f, b->BodyLight);
        break;
    case 6: // 34~40
        Vector(1.0f, 0.4f, 1.0f, b->BodyLight);
        break;
    }
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, 1);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, 1);
    return true;
}

bool RenderGambleItem(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    int _angle = int(b->BodyAngle[1]) % 360;
    float _meshLight1;
    if (0 < _angle && _angle <= 180)
    {
        _meshLight1 = 0.2f - (sinf(Q_PI * (_angle) / 180.0f) * 0.2f);
    }
    else
    {
        _meshLight1 = 0.2f;
    }
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, 0.35f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, _meshLight1, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, 2, _meshLight1, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    return true;
}

bool RenderTalismanOfResurrection(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    float Luminosity = (sinf(WorldTime * 0.003f) + 1) * 0.3f + 0.3f;
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, Luminosity, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderTalismanOfMobility(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderTalismanOfGuardian(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderTalismanOfItemProtection(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderInvitationToSantaVillage(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME4, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);

    float Luminosity = (sinf(WorldTime * 0.003f) + 1) * 0.3f + 0.6f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, Luminosity, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderLuckyCoin(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderChromeMesh1(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderBoostAura(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.2f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderChromeMesh0(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderSealedBox(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderGoldenOrSilverBox(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderSmallCapeOfLord(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

bool RenderMesh0WithTexture(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, int Texture)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  Texture);
    return true;
}

bool RenderJewelryCase(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return true;
}

bool RenderSkeletonTransformationRing(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, 0.5f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    return true;
}

bool RenderPhoenixSoulStar(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->RenderMesh(0, RENDER_BRIGHT, o->Alpha * fLumi, 0, o->BlendMeshLight * fLumi, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, BITMAP_PHOENIXSOULWING);

    Vector(.15f, 1.f, .25f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME3, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    return true;
}

bool RenderPhoenixSoulHelmet(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    if (b->HideSkin == true)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, -1);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, -1);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 2, o->BlendMeshLight * fLumi,
                      (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 2, o->BlendMeshLight * fLumi,
                      (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
    }
    return true;
}

bool RenderPhoenixSoulArmor(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, -1);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, -1);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 1, o->BlendMeshLight * fLumi,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, -1);
    return true;
}

bool RenderPhoenixSoulBoots(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, -1);
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 1, o->BlendMeshLight * fLumi,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
    return true;
}

bool RenderLuckyItem(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    bool bHide = false;
    int nIndex = 0;
    if (Type == MODEL_HELM + 65)
        nIndex = 2;
    else if (Type == MODEL_HELM + 70)
        nIndex = 1;
    if (nIndex > 0)
        bHide = true;

    if (bHide)
    {
        b->RenderMesh(nIndex, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
    }
    else if (b->HideSkin)
    {
        if (Type == MODEL_ARMOR + 65)
            nIndex = BITMAP_INVEN_ARMOR + 6;
        else if (Type == MODEL_ARMOR + 70)
            nIndex = BITMAP_INVEN_ARMOR + 7;
        else if (Type == MODEL_PANTS + 65)
            nIndex = BITMAP_INVEN_PANTS + 6;
        else if (Type == MODEL_PANTS + 70)
            nIndex = BITMAP_INVEN_PANTS + 7;

        if (nIndex > 0)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                          o->BlendMeshTexCoordV, nIndex);
            for (int i = 1; i < b->NumMeshs; ++i)
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                              o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                          o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
}

// ------------------------------------------------ glow passes

// The whole staff glows, then its second mesh again, white; the object keeps
// blending that mesh.
void RenderDeadlyStaffGlow(BMD* b, OBJECT* o, float Alpha, int RenderType, int Texture)
{
    b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1,
                  Texture);
    o->BlendMesh = 1;
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
}

// ------------------------------------------------ the names

const RenderStyle RenderStyles[] = {
    {"stormCrow", RenderStormCrow},
    {"thunderHawk", RenderThunderHawk},
    {"wingsOfDarkness", RenderWingsOfDarkness},
    {"wingOfStorm", RenderWingOfStorm},
    {"wingOfRuin", RenderWingOfRuin},
    {"capeOfEmperor", RenderCapeOfEmperor},
    {"wingsOfDespair", RenderWingsOfDespair},
    {"wingOfDimension", RenderWingOfDimension},
    {"divineSwordOfArchangel", RenderDivineSwordOfArchangel},
    {"archangelStaff", RenderArchangelStaff},
    {"divineScepterOfArchangel", RenderDivineScepterOfArchangel},
    {"greatReignCrossbow", RenderGreatReignCrossbow},
    {"pumpkinOfLuck", RenderPumpkinOfLuck},
    {"skillParchment", RenderSkillParchment},
    {"runeBlade", RenderRuneBlade},
    {"dragonSpear", RenderDragonSpear},
    {"elementalMace", RenderElementalMace},
    {"darkHorse", RenderDarkHorse},
    {"darkRaven", RenderDarkRaven},
    {"battleScepter", RenderBattleScepter},
    {"masterScepter", RenderMasterScepter},
    {"flamberge", RenderFlamberge},
    {"swordBreaker", RenderSwordBreaker},
    {"runeBastardSword", RenderRuneBastardSword},
    {"frostMace", RenderFrostMace},
    {"deadlyStaff", RenderDeadlyStaff, RenderDeadlyStaffGlow},
    {"imperialStaff", RenderImperialStaff},
    {"staff32", RenderStaff32},
    {"crimsonGlory", RenderCrimsonGlory},
    {"salamanderShield", RenderSalamanderShield},
    {"frostBarrier", RenderFrostBarrier},
    {"guardianShield", RenderGuardianShield},
    {"crossShield", RenderCrossShield},
    {"oldScroll", RenderOldScroll},
    {"illusionSorcererCovenant", RenderIllusionSorcererCovenant, &HarmonyShine},
    {"scrollOfBlood", RenderScrollOfBlood},
    {"cursedCastleWater", RenderCursedCastleWater, &CursedCastleWaterShine},
    {"harmonyShine", RenderPlainly, &HarmonyShine},
    {"condorFlame", RenderCondorFlame},
    {"condorFeather", RenderCondorFeather},
    {"deathBeamKnightFlame", RenderThirdClassQuestItem, BITMAP_ITEM_EFFECT_DBSTONE_R},
    {"hellMinerHorn", RenderThirdClassQuestItem, BITMAP_ITEM_EFFECT_HELLHORN_R},
    {"darkPhoenixFeather", RenderThirdClassQuestItem, BITMAP_ITEM_EFFECT_PFEATHER_R},
    {"abyssalEye", RenderAbyssalEye},
    {"eventTicket", RenderFreeTicket, BITMAP_FREETICKET_R},
    {"chaosCard", RenderChaosCard, BITMAP_CHAOSCARD_R},
    {"rareItemTicket1", RenderRareItemTicketWithBrightMesh1, BITMAP_RAREITEM1_R},
    {"rareItemTicket2", RenderRareItemTicketWithBrightMesh1, BITMAP_RAREITEM2_R},
    {"rareItemTicket3", RenderRareItemTicketWithBrightMesh1, BITMAP_RAREITEM3_R},
    {"rareItemTicket4", RenderRareItemTicket, BITMAP_RAREITEM4_R},
    {"rareItemTicket", RenderRareItemTicket, BITMAP_RAREITEM5_R},
    {"talismanOfLuck", RenderTalismanOfLuck},
    {"sealOfAscension", RenderSeal, BITMAP_LUCKY_SEAL_EFFECT43, &SealShine},
    {"sealOfWealth", RenderSeal, BITMAP_LUCKY_SEAL_EFFECT44, &SealShine},
    {"sealOfSustenance", RenderSeal, BITMAP_LUCKY_SEAL_EFFECT45, &SealShine},
    {"elitePotion", RenderElitePotion},
    {"textured", RenderTextured},
    {"rareItemTicket7", RenderRareItemTicket, BITMAP_RAREITEM7},
    {"rareItemTicket8", RenderRareItemTicket, BITMAP_RAREITEM8},
    {"rareItemTicket9", RenderRareItemTicket, BITMAP_RAREITEM9},
    {"rareItemTicket10", RenderRareItemTicket, BITMAP_RAREITEM10},
    {"rareItemTicket11", RenderRareItemTicket, BITMAP_RAREITEM11},
    {"rareItemTicket12", RenderRareItemTicket, BITMAP_RAREITEM12},
    {"openAccessTicketToDoppelganger", RenderFreeTicket, BITMAP_DOPPLEGANGGER_FREETICKET},
    {"openAccessTicketToVarka", RenderFreeTicket, BITMAP_BARCA_FREETICKET},
    {"openAccessTicketToVarka7", RenderFreeTicket, BITMAP_BARCA7TH_FREETICKET},
    {"summonerCharacterCard", RenderChaosCard, BITMAP_CHARACTERCARD_R},
    {"chaosCardGold", RenderChaosCard, BITMAP_NEWCHAOSCARD_GOLD_R},
    {"chaosCardRare", RenderChaosCard, BITMAP_NEWCHAOSCARD_RARE_R},
    {"chaosCardMini", RenderChaosCard, BITMAP_NEWCHAOSCARD_MINI_R},
    {"texturedAllMeshes", RenderTexturedAllMeshes},
    {"greatScepter", RenderGreatScepter},
    {"lordScepter", RenderLordScepter},
    {"knightBlade", RenderKnightBlade},
    {"darkReignBlade", RenderDarkReignBlade},
    {"hurricane", RenderHurricane},
    {"sylphWindBow", RenderSylphWindBow},
    {"explosionBlade", RenderExplosionBlade},
    {"sylphidRay", RenderSylphidRay},
    {"swordDancer", RenderSwordDancer},
    {"albatrossBow", RenderAlbatrossBow},
    {"platinaStaff", RenderPlatinaStaff},
    {"onlyMesh2WithoutSkin", RenderOnlyMesh2WithoutSkin},
    {"onlyMesh0WithoutSkin", RenderOnlyMesh0WithoutSkin},
    {"onlyMesh1WithoutSkin", RenderOnlyMesh1WithoutSkin},
    {"violentWindToEternalWingHelm", RenderViolentWindToEternalWingHelm},
    {"violentWindToEternalWingArmor", RenderViolentWindToEternalWingArmor},
    {"violentWindToEternalWingPants", RenderViolentWindToEternalWingPants},
    {"characterCard", RenderCharacterCard},
    {"divineAndSuccubusSkin", RenderDivineAndSuccubusSkin},
    {"brova", RenderBrova},
    {"strikerScepter", RenderStrikerScepter},
    {"arrowViperBow", RenderArrowViperBow},
    {"lostMap", RenderLostMap},
    {"symbolOfKundun", RenderSymbolOfKundun},
    {"staffOfKundun", RenderStaffOfKundun},
    {"demonicStick", RenderDemonicStick},
    {"stormBlitzStick", RenderStormBlitzStick},
    {"greatLordScepter", RenderGreatLordScepter},
    {"grandSoulShield", RenderGrandSoulShield},
    {"elementalShield", RenderElementalShield},
    {"monsterBattleBow", RenderMonsterBattleBow},
    {"siegePotion", RenderSiegePotion},
    {"contractSummon", RenderContractSummon},
    {"lifeStone", RenderLifeStone},
    {"ammunition", RenderAmmunition},
    {"helperNpcPlate", RenderHelperNpcPlate},
    {"socketSeed", RenderSocketSeed},
    {"socketSeedSphere", RenderSocketSeedSphere},
    {"gambleItem", RenderGambleItem},
    {"talismanOfResurrection", RenderTalismanOfResurrection},
    {"talismanOfMobility", RenderTalismanOfMobility},
    {"talismanOfGuardian", RenderTalismanOfGuardian},
    {"talismanOfItemProtection", RenderTalismanOfItemProtection},
    {"invitationToSantaVillage", RenderInvitationToSantaVillage},
    {"luckyCoin", RenderLuckyCoin},
    {"chromeMesh1", RenderChromeMesh1},
    {"boostAura", RenderBoostAura},
    {"chromeMesh0", RenderChromeMesh0},
    {"sealedBox", RenderSealedBox},
    {"goldenOrSilverBox", RenderGoldenOrSilverBox},
    {"smallCapeOfLord", RenderSmallCapeOfLord},
    {"packageBoxA", RenderMesh0WithTexture, BITMAP_PACKAGEBOX_RED},
    {"packageBoxB", RenderMesh0WithTexture, BITMAP_PACKAGEBOX_BLUE},
    {"packageBoxC", RenderMesh0WithTexture, BITMAP_PACKAGEBOX_GOLD},
    {"packageBoxD", RenderMesh0WithTexture, BITMAP_PACKAGEBOX_GREEN},
    {"packageBoxE", RenderMesh0WithTexture, BITMAP_PACKAGEBOX_PUPLE},
    {"packageBoxF", RenderMesh0WithTexture, BITMAP_PACKAGEBOX_SKY},
    {"accountServiceItem", RenderMesh0WithTexture, BITMAP_INGAMESHOP_PRIMIUM6},
    {"dayPass", RenderMesh0WithTexture, BITMAP_INGAMESHOP_COMMUTERTICKET4},
    {"hourPass", RenderMesh0WithTexture, BITMAP_INGAMESHOP_SIZECOMMUTERTICKET3},
    {"jewelryCase", RenderJewelryCase},
    {"skeletonTransformationRing", RenderSkeletonTransformationRing},
    {"phoenixSoulStar", RenderPhoenixSoulStar},
    {"phoenixSoulHelmet", RenderPhoenixSoulHelmet},
    {"phoenixSoulArmor", RenderPhoenixSoulArmor},
    {"phoenixSoulBoots", RenderPhoenixSoulBoots},
    {"luckyItem", RenderLuckyItem},
};

const RenderStyle* FindStyle(std::string_view name)
{
    const auto found = std::find_if(std::begin(RenderStyles), std::end(RenderStyles),
                                    [name](const RenderStyle& style) { return style.name == name; });
    return found != std::end(RenderStyles) ? found : nullptr;
}

// The render style of every item type, taken from the item model database
// after each build of it.
ItemModelTable<const RenderStyle*> g_itemStyles{[](const Data::Items::ItemModelDefinition& model)
                                                {
                                                    // Names that are not styles are drawn plainly; the model
                                                    // loader reports them.
                                                    return model.renderStyle.empty() ? nullptr
                                                                                     : FindStyle(model.renderStyle);
                                                }};
} // namespace

bool Exists(std::string_view name)
{
    return FindStyle(name) != nullptr;
}

bool Render(BMD* b, OBJECT* o, int modelType, float alpha, int renderType)
{
    const RenderStyle* style = g_itemStyles.Find(modelType);
    return style != nullptr && style->Render(b, o, modelType, alpha, renderType);
}

const ShineBelowPlus3* FindShineBelowPlus3(int modelType)
{
    const RenderStyle* style = g_itemStyles.Find(modelType);
    return style != nullptr ? style->shine : nullptr;
}

bool RenderGlow(BMD* b, OBJECT* o, int modelType, float alpha, int renderType, int texture)
{
    const RenderStyle* style = g_itemStyles.Find(modelType);
    if (style == nullptr || style->glow == nullptr)
    {
        return false;
    }
    style->glow(b, o, alpha, renderType, texture);
    return true;
}
} // namespace Render::Items::Styles
