#include "stdafx.h"

#include "Core/Utilities/Random.h"
#include "ItemRenderStyles.h"
#include "ItemModelTable.h"

#include "Core/Globals/_enum.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string_view>
#include <vector>

namespace Render::Items::Styles
{
namespace
{
// A recipe draws the model and returns true, or returns false when the style
// is not for this drawing (the drawing code then draws the model plainly).
using StyleFunction = bool (*)(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType);
// A recipe that styles share with a texture of their own each.
using TexturedStyleFunction = bool (*)(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType,
                                       int Texture);
// `Texture` is the texture of the glow pass.
using GlowFunction = void (*)(BMD* b, const OBJECT* o, DrawLook& look, float Alpha, int RenderType, int Texture);

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

    bool Render(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType) const
    {
        return texturedRender != nullptr ? texturedRender(b, o, look, modelType, alpha, renderType, texture)
                                         : render(b, o, look, modelType, alpha, renderType);
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

// The clock in reference frames (ADR 0001), for looks that cycle.
double ReferenceFrames()
{
    return WorldTime * REFERENCE_FPS * 0.001;
}

// For styles that only shine below +3: the drawing code draws the model.
bool RenderPlainly(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    return false;
}

// The light a set's chrome layer is drawn in: a share of the model's light, per channel.
struct ChromeTint
{
    float red;
    float green;
    float blue;
};

constexpr ChromeTint ThunderHawkChromeTint{0.85f, 0.85f, 1.2f};
constexpr ChromeTint HurricaneChromeTint{0.3f, 0.8f, 1.f};

// Pixels the alpha test drops (BITMAP_t::hasHoles), not only an alpha channel: a .tga can be solid.
bool TextureHasHoles(const BMD* b, int meshIndex)
{
    return Bitmaps.GetTexture(b->IndexTexture[b->Meshs[meshIndex].Texture])->hasHoles;
}

// Draws the model with a chrome layer (BITMAP_CHROME + 1) in the tint of the model's light, or in the model's own
// light without a tint, mesh by mesh with RenderBody's rules (texture scripts, the hidden mesh), so any model can
// have the look. A mesh whose texture has holes gets the chrome under its texture: solid and on both faces, so it
// fills the holes (the Magic Gladiator sets' grooves) and the texture covers the rest. A mesh whose texture has none
// gets the chrome added over its texture, and so do a glowing mesh (it adds its light, as RenderBody's two passes
// drew it) and every mesh of a see-through model (a character fading out after death), where a solid layer would
// show through the texture. Doppelgangers are drawn plainly.
void RenderSetChrome(BMD* b, const DrawLook& look, const std::optional<ChromeTint>& tint)
{
    const bool seeThrough = look.alpha < 0.99f;
    // An added chrome layer on a see-through model scales BodyLight by the alpha, so the light is put back then too.
    const bool keepLight = tint.has_value() || seeThrough;
    vec3_t Light;
    if (keepLight)
    {
        VectorCopy(b->BodyLight, Light);
    }
    const auto chrome = [&](int meshIndex, int flags)
    {
        if (tint)
        {
            Vector(tint->red * Light[0], tint->green * Light[1], tint->blue * Light[2], b->BodyLight);
        }
        b->RenderScriptedMesh(meshIndex, RENDER_TEXTURE | RENDER_CHROME | flags, look.alpha, look.blendMesh,
                              look.blendMeshLight, look.blendMeshU, look.blendMeshV, BITMAP_CHROME + 1);
        if (keepLight)
        {
            VectorCopy(Light, b->BodyLight);
        }
    };
    const auto texture = [&](int meshIndex)
    {
        b->RenderScriptedMesh(meshIndex, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight,
                              look.blendMeshU, look.blendMeshV);
    };

    b->BeginRender(look.alpha);
    for (int i = 0; i < b->NumMeshs; ++i)
    {
        if (b->IsMeshHidden(i, look.hiddenMesh))
        {
            continue;
        }
        if (!seeThrough && !b->IsMeshGlowing(i, look.blendMesh) && TextureHasHoles(b, i))
        {
            chrome(i, RENDER_TWOSIDED);
            texture(i);
        }
        else
        {
            texture(i);
            chrome(i, RENDER_BRIGHT);
        }
    }
    b->EndRender();
}

bool RenderStormCrow(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    RenderSetChrome(b, look, std::nullopt);
    return true;
}

bool RenderThunderHawk(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    RenderSetChrome(b, look, ThunderHawkChromeTint);
    return true;
}

bool RenderWingsOfDarkness(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Vector(0.8f, 0.6f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, 0.5f, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh, BITMAP_CHROME + 1);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    return true;
}

bool RenderWingOfStorm(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 0.7f, 0.5f, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, look.hiddenMesh);

    // Four texture frames, four reference frames each, from the clock: a draw keeps no count.
    const int textureFrame = static_cast<int>(std::fmod(ReferenceFrames(), 16.0)) / 4;
    float fU = textureFrame * 0.25f;
    Vector(0.9f, 0.6f, 0.3f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, look.blendMeshLight, fU, look.blendMeshV,
                  look.hiddenMesh);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    return true;
}

bool RenderWingOfRuin(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    float Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.3f;
    Vector(0.1f + Luminosity, 0.1f + Luminosity, 0.1f + Luminosity, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.8f;
    Vector(0.0f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, BITMAP_3RDWING_LAYER);
    return true;
}

bool RenderCapeOfEmperor(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    else
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    return true;
}

bool RenderWingsOfDespair(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME6, look.alpha, look.blendMesh, look.blendMeshLight,
                  look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderWingOfDimension(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderDivineSwordOfArchangel(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, Alpha, 0, look.blendMeshLight, look.blendMeshU,
                  WorldTime * 0.0001f, BITMAP_CHROME);
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderArchangelStaff(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, look.blendMeshLight, look.blendMeshU,
                  -WorldTime * 0.0001f, BITMAP_CHROME);
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderDivineScepterOfArchangel(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, Alpha, 0, look.blendMeshLight, look.blendMeshU,
                  -WorldTime * 0.0001f, BITMAP_CHROME);
    return true;
}

bool RenderGreatReignCrossbow(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    Vector(0.8f * Light[0], 0.f, 0.8f * Light[2], b->BodyLight);
    b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, look.blendMeshLight, -WorldTime * 0.0002f,
                  look.blendMeshV, BITMAP_CHROME);
    VectorCopy(Light, b->BodyLight);
    b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  BITMAP_CHROME + 1);
    VectorCopy(Light, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    return true;
}

bool RenderPumpkinOfLuck(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_DARK | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, -WorldTime * 0.0002f,
                  look.blendMeshV, BITMAP_CHROME);
    return true;
}

void SpawnPumpkinOfLuck(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t vPos, vRelativePos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    b->TransformPosition(bones[8], vRelativePos, vPos, true);
    float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
    Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, o->Light);
    CreateSprite(BITMAP_LIGHT, vPos, 1.5f, o->Light, o, 0.5f);
    b->TransformPosition(bones[10], vRelativePos, vPos, true);
    CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
    b->TransformPosition(bones[11], vRelativePos, vPos, true);
    CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
}

bool RenderSkillParchment(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.0f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh, BITMAP_ROOLOFPAPER_EFFECT_R);
    return true;
}

bool RenderRuneBlade(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    b->BeginRender(1.f);
    b->RenderMesh(3, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, sinf(WorldTime * 0.01f), 1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_CHROME | RENDER_TEXTURE, 1.f, 0, look.blendMeshLight, look.blendMeshU, WorldTime * 0.001f,
                  BITMAP_CHROME);

    float Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
    Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, look.blendMeshLight, WorldTime * 0.0001f,
                  -WorldTime * 0.0005f);
    b->EndRender();
    return true;
}

bool RenderDragonSpear(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, 1, look.blendMeshLight, WorldTime * 0.0001f, WorldTime * 0.0005f);
    return true;
}

bool RenderElementalMace(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);

    float time = WorldTime * 0.001f;
    float Luminosity = sinf(time) * 0.5f + 0.3f;
    Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE, 1.f, 2, look.blendMeshLight, time, -WorldTime * 0.0005f);
    return true;
}

bool RenderDarkHorse(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    Vector(0.8f, 0.4f, 0.1f, b->BodyLight);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderDarkRaven(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    Vector(0.3f, 0.8f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderBattleScepter(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    look.blendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
    b->BeginRender(1.f);
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV, look.hiddenMesh);
    b->EndRender();
    return true;
}

bool RenderMasterScepter(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    Vector(0.1f, 0.3f, 1.f, b->BodyLight);
    look.blendMesh = 0;
    look.blendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    Vector(0.6f, 0.8f, 1.f, b->BodyLight);
    look.blendMesh = 1;
    look.blendMeshLight = 1.f;
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, WorldTime * 0.0003f,
                  BITMAP_CHROME);
    return true;
}

bool RenderFlamberge(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // b->RenderBody( RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
    // 5 );

    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    look.blendMesh = 1;
    Vector(1.f, 0.f, 0.2f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                  look.blendMeshU, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    float fV;
    fV = (((int)(WorldTime * 0.05) % 16) / 4) * 0.25f;
    b->RenderMesh(5, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 5, look.blendMeshLight, look.blendMeshU, fV);
    return true;
}

bool RenderSwordBreaker(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                  look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderRuneBastardSword(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    look.blendMesh = 1;
    look.blendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderFrostMace(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 1);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderDeadlyStaff(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    look.blendMesh = 1;
    look.blendMeshLight = 1.f;
    Vector(1.f, 0.5f, 0.5f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderImperialStaff(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    look.blendMesh = 0;
    look.blendMeshLight = fabs(sinf(WorldTime * 0.001f)) + 0.1f;
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  BITMAP_SOCKETSTAFF);
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  BITMAP_SOCKETSTAFF);
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  BITMAP_SOCKETSTAFF);
    return true;
}

bool RenderStaff32(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                  look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderCrimsonGlory(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                  look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderSalamanderShield(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 1);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderFrostBarrier(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    look.blendMesh = 2;
    look.blendMeshLight = absf((sinf(WorldTime * 0.001f)));
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderGuardianShield(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderCrossShield(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                  look.blendMeshU, -(int)WorldTime % 2000 * 0.0005f);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

// The light of a blended mesh that pulses slowly between 0.4 and 1.
float BlendMeshPulse()
{
    return sinf(WorldTime * 0.002f) * 0.3f + 0.7f;
}

bool RenderOldScroll(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, 0.7f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 1);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 4, BlendMeshPulse(), look.blendMeshU, look.blendMeshV, 0);
    return true;
}

bool RenderIllusionSorcererCovenant(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float sine = float(sinf(WorldTime * 0.00004f) * 0.15f) + 0.5f;
    b->RenderBody(RenderType, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 0);
    b->RenderBody(RenderType, 1.f, 0, sine, look.blendMeshU, look.blendMeshV, 1);
    return true;
}

bool RenderScrollOfBlood(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, 0.7f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 0);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, BlendMeshPulse(), look.blendMeshU, look.blendMeshV, 1);
    return true;
}

bool RenderCursedCastleWater(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight * 1.5f, look.blendMeshU, look.blendMeshV);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, Alpha, look.blendMesh, look.blendMeshLight / 4.0f, WorldTime * 0.005f,
                  -WorldTime * 0.005f, 0);
    return true;
}

void SpawnCursedCastleWater(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t Light;
    vec3_t vRelativePos, vWorldPos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    Vector(1.f, 0.f, 0.0f, Light);

    b->TransformPosition(bones[1], vRelativePos, vWorldPos, true);
    CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
    b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, 2.f / 7.f, o->Position, o->Angle);
}

bool RenderCondorFlame(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, 0.9, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderCondorFeather(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, 0.9, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderThirdClassQuestItem(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType,
                               int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, Texture);
    return true;
}

bool RenderAbyssalEye(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, fLumi, look.blendMeshU, look.blendMeshV,
                  BITMAP_ITEM_EFFECT_DEYE_R);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 0.2f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderFreeTicket(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, Texture);
    return true;
}

bool RenderChaosCard(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 1);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, Texture);
    return true;
}

bool RenderRareItemTicketWithBrightMesh1(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType,
                                         int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, Texture);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderRareItemTicket(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, Texture);
    return true;
}

bool RenderTalismanOfLuck(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.5f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV,
                  BITMAP_LUCKY_CHARM_EFFECT53);
    return true;
}

bool RenderSeal(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType, int Texture)
{
    float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, Texture);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderElitePotion(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_CHROME4, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderTextured(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    return true;
}

bool RenderTexturedAllMeshes(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderGreatScepter(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    float Luminosity = WorldTime * 0.0005f;

    look.hiddenMesh = 2;
    look.blendMesh = 2;
    look.blendMeshLight = 1.f;
    b->BeginRender(1.f);
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(2, RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, Luminosity, Luminosity);
    b->EndRender();
    return true;
}

bool RenderLordScepter(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    look.blendMeshLight = 1.f;
    look.blendMeshU = WorldTime * 0.0008f;
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    look.blendMeshLight = sinf(WorldTime * 0.001f) * 0.3f + 0.7f;
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, Alpha, 0, look.blendMeshLight, 0.f, look.blendMeshV);
    return true;
}

bool RenderKnightBlade(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);

    float Luminosity = sinf(WorldTime * 0.0008f) * 0.7f + 0.5f;
    b->RenderMesh(2, RENDER_TEXTURE, Alpha, 2, Luminosity, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, Alpha, 1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    //. 날
    b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, Alpha, 3, look.blendMeshLight, WorldTime * 0.0001f,
                  WorldTime * 0.0005f);
    return true;
}

bool RenderDarkReignBlade(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, 0, look.blendMeshLight, WorldTime * 0.0005f, WorldTime * 0.0005f);
    look.hiddenMesh = 0;
    b->StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, WorldTime * 0.0005f);
    return true;
}

bool RenderHurricane(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Doppelgangers are drawn plainly.
    if (RenderType & RENDER_DOPPELGANGER)
    {
        return false;
    }
    RenderSetChrome(b, look, HurricaneChromeTint);
    return true;
}

bool RenderSylphWindBow(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.0f);

    b->StreamMesh = 0;
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    if (Type == MODEL_BONE_BLADE)
    {
        b->BodyLight[0] = 1.0f;
        b->BodyLight[1] = 0.7f;
        b->BodyLight[2] = 0.4f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      (int)WorldTime % 4000 * 0.0004f - 0.7f, look.blendMeshV, BITMAP_CHROME7);
        b->BodyLight[0] = 0.7f;
        b->BodyLight[1] = 0.7f;
        b->BodyLight[2] = 0.7f;
    }
    else if (Type == MODEL_GRAND_VIPER_STAFF)
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      (int)WorldTime % 2000 * 0.0002f - 0.3f, (int)WorldTime % 2000 * 0.0002f - 0.3f,
                      BITMAP_CHROME_ENERGY);
    else
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      (int)WorldTime % 4000 * 0.0002f - 0.3f, (int)WorldTime % 4000 * 0.0002f - 0.3f, BITMAP_CHROME6);
    b->StreamMesh = -1;

    b->EndRender();
    return true;
}

bool RenderExplosionBlade(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);

    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshU);
    float Luminosity = sinf(WorldTime * 0.0015f) * 0.03f + 0.3f;
    Vector(Luminosity, Luminosity, Luminosity + 0.1f, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, -2, look.blendMeshLight, look.blendMeshU, look.blendMeshU);

    float Luminosity4 = sinf(WorldTime * 0.0025f) * 0.5f + 0.7f;
    Vector(0.4f, 0.4f, 0.8f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE, 0.5f, -2, look.blendMeshLight, look.blendMeshU, look.blendMeshU);
    return true;
}

bool RenderSylphidRay(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    return true;
}

bool RenderSwordDancer(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    look.hiddenMesh = 1;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, 0.5f, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV, BITMAP_LAVA);
    b->RenderMesh(1, RENDER_TEXTURE, 0.7f, 1, look.blendMeshLight, look.blendMeshU, WorldTime * 0.0009f);
    return true;
}

bool RenderAlbatrossBow(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    look.hiddenMesh = 1;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, 1.0f, 1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME5, 0.5f, -1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderPlatinaStaff(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    look.hiddenMesh = 1;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 1, look.blendMeshLight, WorldTime * 0.0009f,
                  WorldTime * 0.0009f);
    return true;
}

bool RenderOnlyMesh2WithoutSkin(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    return true;
}

bool RenderOnlyMesh0WithoutSkin(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    return true;
}

bool RenderOnlyMesh1WithoutSkin(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (b->HideSkin == true)
    {
        b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
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

bool RenderViolentWindToEternalWingHelm(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
        b->RenderMesh(anMesh[Type - (MODEL_MISTERY_HELM)], RENDER_TEXTURE, look.alpha, look.blendMesh,
                      look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    }
    else
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    return true;
}

bool RenderViolentWindToEternalWingArmor(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV, BITMAP_INVEN_ARMOR + nTexture);
        for (int i = 1; i < b->NumMeshs; ++i)
            b->RenderMesh(i, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
    }
    else
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    return true;
}

bool RenderViolentWindToEternalWingPants(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV, BITMAP_INVEN_PANTS + nTexture);
        for (int i = 1; i < b->NumMeshs; ++i)
            b->RenderMesh(i, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
    }
    else
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    return true;
}

bool RenderCharacterCard(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV, _R_Type);
    return true;
}

bool RenderDivineAndSuccubusSkin(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV, nTexture);
        for (int i = 1; i < b->NumMeshs; ++i)
        {
            b->RenderMesh(i, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
        }
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    return true;
}

bool RenderBrova(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(4, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(5, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, 0.4f, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, 0.8f, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderStrikerScepter(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(4, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(5, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(6, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, 0.5f, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderArrowViperBow(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
    b->BeginRender(1.f);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(6, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE, 1.f, 3, Luminosity, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(4, RENDER_TEXTURE, 1.f, 4, Luminosity, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(5, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);

    b->EndRender();
    return true;
}

bool RenderLostMap(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Models[o->Type].StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, WorldTime * 0.0005f);
    Models[o->Type].StreamMesh = -1;
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderSymbolOfKundun(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->StreamMesh = 1;
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, WorldTime * 0.0005f);
    b->StreamMesh = -1;

    Vector(1.f, 0.5f, 0.f, b->BodyLight);
    // Up and down between 0.01 and 1 by 0.01 a reference frame, from the clock: a draw keeps no count.
    const float sweep = static_cast<float>(std::fmod(ReferenceFrames() * 0.01, 1.98));
    const float fMeshLight = 0.01f + (sweep < 0.99f ? sweep : 1.98f - sweep);
    b->RenderMesh(2, RENDER_TEXTURE, 1.0f, 2, fMeshLight, look.blendMeshU, look.blendMeshV);

    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 0.3f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderStaffOfKundun(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, Alpha, 1, 0.2f, look.blendMeshU, look.blendMeshV);

    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 0, sinf(WorldTime * 0.005f), look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(0, RENDER_CHROME4 | RENDER_BRIGHT, Alpha, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderDemonicStick(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, 1.f, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderStormBlitzStick(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    float fLumi = (sinf(WorldTime * 0.002f) + 0.5f) * 0.5f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, fLumi, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderGreatLordScepter(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, 1.f, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 3, 1.f, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderGrandSoulShield(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
    b->RenderMesh(2, RENDER_COLOR, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);

    VectorCopy(Light, b->BodyLight);
    b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, 1.f, 2, look.blendMeshLight, look.blendMeshU, WorldTime * 0.01f,
                  BITMAP_CHROME);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, 1.f, 1, look.blendMeshLight, (float)(Random::DrawRand(o, 1) % 10) * 0.1f,
                  (float)(Random::DrawRand(o, 2) % 10) * 0.1f);

    float Luminosity = sinf(WorldTime * 0.001f) * 0.4f + 0.6f;
    Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, look.blendMeshLight, WorldTime * 0.0001f,
                  WorldTime * 0.0005f);
    b->EndRender();
    return true;
}

bool RenderElementalShield(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    b->RenderMesh(1, RENDER_TEXTURE, 0.8f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE, 0.5f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);

    b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE, 1.f, 2, look.blendMeshLight, WorldTime * 0.0005f, look.blendMeshV);
    b->RenderMesh(3, RENDER_TEXTURE, 1.f, 3, look.blendMeshLight, (float)(Random::DrawRand(o, 1) % 10) * 0.1f,
                  (float)(Random::DrawRand(o, 2) % 10) * 0.1f);
    b->EndRender();
    return true;
}

bool RenderMonsterBattleBow(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Only in the hands of the Metal Balrog and the Orc Archer of Doom
    // (RENDER_EXTRA); otherwise drawn plainly.
    if (!(RenderType & RENDER_EXTRA))
    {
        return false;
    }
    RenderType -= RENDER_EXTRA;
    Vector(0.1f, 0.1f, 0.1f, b->BodyLight);
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderSiegePotion(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    if (look.hiddenMesh == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
        Vector(0.1f, 0.5f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_METAL | RENDER_BRIGHT, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    else if (look.hiddenMesh == 0)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
        Vector(0.1f, 0.5f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_METAL | RENDER_BRIGHT, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    b->EndRender();
    return true;
}

bool RenderContractSummon(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    if (look.hiddenMesh == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    }
    else if (look.hiddenMesh == 0)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    }
    b->EndRender();
    return true;
}

bool RenderLifeStone(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->BeginRender(1.f);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    Vector(0.f, 0.5f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 1.f, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->EndRender();
    return true;
}

bool RenderAmmunition(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (g_isCharacterBuff(o, eBuff_InfinityArrow))
    {
        Vector(1.f, 0.8f, 0.2f, b->BodyLight);
        b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
        b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
    else
    {
        b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    }
    return true;
}

bool RenderHelperNpcPlate(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Only on the helper NPCs (Luke and Leo the Helper, Helper Ellen), the only
    // characters with the flag of the PC room look; otherwise drawn plainly.
    if (o->m_bpcroom != TRUE)
    {
        return false;
    }
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    Vector(0.9f, 0.7f, 1.0f, b->BodyLight);

    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, BITMAP_FENRIR_THUNDER);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_METAL, look.alpha, 1, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  BITMAP_FENRIR_THUNDER);
    VectorCopy(Light, b->BodyLight);
    return true;
}

bool RenderSocketSeed(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, look.hiddenMesh);
    return true;
}

bool RenderSocketSeedSphere(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Each sphere has the color of its seed; other items are drawn plainly.
    if (Type < MODEL_SEED_SPHERE_FIRE_1 || Type > MODEL_SEED_SPHERE_EARTH_5)
    {
        return false;
    }
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);

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
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, 1);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, 1);
    return true;
}

bool RenderGambleItem(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    int _angle = int(o->Angle[1]) % 360;
    float _meshLight1;
    if (0 < _angle && _angle <= 180)
    {
        _meshLight1 = 0.2f - (sinf(Q_PI * (_angle) / 180.0f) * 0.2f);
    }
    else
    {
        _meshLight1 = 0.2f;
    }
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, 0.35f, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, _meshLight1, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, 2, _meshLight1, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderTalismanOfResurrection(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    float Luminosity = (sinf(WorldTime * 0.003f) + 1) * 0.3f + 0.3f;
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, Luminosity, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    return true;
}

bool RenderTalismanOfMobility(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, look.hiddenMesh);
    return true;
}

bool RenderTalismanOfGuardian(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderTalismanOfItemProtection(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, look.hiddenMesh);
    return true;
}

bool RenderInvitationToSantaVillage(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME4, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);

    float Luminosity = (sinf(WorldTime * 0.003f) + 1) * 0.3f + 0.6f;
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, Luminosity, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderLuckyCoin(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderChromeMesh1(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderBoostAura(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.2f, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderChromeMesh0(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderSealedBox(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    return true;
}

bool RenderGoldenOrSilverBox(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderSmallCapeOfLord(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    return true;
}

bool RenderMesh0WithTexture(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType, int Texture)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, -1, look.blendMeshLight, look.blendMeshU, look.blendMeshV, Texture);
    return true;
}

bool RenderJewelryCase(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    return true;
}

bool RenderSkeletonTransformationRing(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, 0.5f, 0, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    return true;
}

bool RenderPhoenixSoulStar(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_BRIGHT, look.alpha * fLumi, 0, look.blendMeshLight * fLumi, look.blendMeshU,
                  look.blendMeshV, BITMAP_PHOENIXSOULWING);

    Vector(.15f, 1.f, .25f, b->BodyLight);
    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME3, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    return true;
}

bool RenderPhoenixSoulHelmet(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    if (b->HideSkin == true)
    {
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV, -1);
        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV, -1);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, look.alpha * fLumi, 2, look.blendMeshLight * fLumi,
                      (double)(-int(WorldTime) % 1000) * 0.00009f, look.blendMeshV, -1);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, look.alpha * fLumi, 2, look.blendMeshLight * fLumi,
                      (double)(-int(WorldTime) % 1000) * 0.00009f, look.blendMeshV, -1);
    }
    return true;
}

bool RenderPhoenixSoulArmor(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  -1);
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, -1);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, look.alpha * fLumi, 1, look.blendMeshLight * fLumi,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, look.blendMeshV, -1);
    b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  -1);
    return true;
}

bool RenderPhoenixSoulBoots(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  -1);
    b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, look.blendMeshV, -1);
    b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, look.alpha * fLumi, 1, look.blendMeshLight * fLumi,
                  (double)(-int(WorldTime) % 1000) * 0.00009f, look.blendMeshV, -1);
    return true;
}

bool RenderLuckyItem(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
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
        b->RenderMesh(nIndex, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
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
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, nIndex);
            for (int i = 1; i < b->NumMeshs; ++i)
                b->RenderMesh(i, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                              look.blendMeshV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, look.hiddenMesh);
    }
    else
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
    }
    return true;
}

// The bone in the light of its level (a little darker), then a red chrome
// layer running over it; the level glow comes on top as for other items.
bool RenderBloodBone(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    look.blendMeshU = sinf(gMapManager.WorldActive * 0.0001f);
    look.blendMeshV = -WorldTime * 0.0005f;
    vec3_t light;
    VectorCopy(b->BodyLight, light);
    Vector(light[0] * 0.9f, light[1] * 0.9f, light[2] * 0.9f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    Vector(.9f, .1f, .1f, b->BodyLight);
    b->StreamMesh = 0;
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, look.hiddenMesh, BITMAP_CHROME);
    b->StreamMesh = -1;
    VectorCopy(light, b->BodyLight);
    return true;
}

// The cloak is see-through: it is added to what is behind it, so a darker
// light would make it fade. Its brightest color is always this bright.
constexpr float InvisibilityCloakBrightness = 0.8f;

// The cloak in the color of its level, as visible at every level and on any
// ground, its first mesh pulsing; the level glow comes on top as for other
// items.
bool RenderInvisibilityCloak(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    vec3_t light;
    VectorCopy(b->BodyLight, light);
    const float brightest = std::max({light[0], light[1], light[2]});
    if (brightest > 0.f)
    {
        VectorScale(light, InvisibilityCloakBrightness / brightest, b->BodyLight);
    }
    else
    {
        Vector(InvisibilityCloakBrightness, InvisibilityCloakBrightness, InvisibilityCloakBrightness, b->BodyLight);
    }
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, BlendMeshPulse(), look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    VectorCopy(light, b->BodyLight);
    return true;
}

// ------------------------------------------------ glow passes

// The whole staff glows, then its second mesh again, white; the object keeps
// blending that mesh.
void RenderDeadlyStaffGlow(BMD* b, const OBJECT* o, DrawLook& look, float Alpha, int RenderType, int Texture)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV, -1,
                  Texture);
    look.blendMesh = 1;
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(1, RenderType, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
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
    {"bloodBone", RenderBloodBone},
    {"invisibilityCloak", RenderInvisibilityCloak},
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

using SpawnFunction = void (*)(BMD* b, OBJECT* o, vec34_t* bones);

// The styles whose items spawn lights or animate where they are drawn.
const struct
{
    const char* name;
    SpawnFunction spawn;
} StyleSpawns[] = {
    {"pumpkinOfLuck", SpawnPumpkinOfLuck},
    {"cursedCastleWater", SpawnCursedCastleWater},
};

ItemModelTable<SpawnFunction> g_itemSpawns{[](const Data::Items::ItemModelDefinition& model)
                                           {
                                               for (const auto& style : StyleSpawns)
                                                   if (model.renderStyle == style.name)
                                                       return style.spawn;
                                               return SpawnFunction{};
                                           }};

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

std::span<const std::string_view> GetNames()
{
    static const std::vector<std::string_view> names = []
    {
        std::vector<std::string_view> sorted;
        for (const RenderStyle& style : RenderStyles)
        {
            sorted.emplace_back(style.name);
        }
        std::ranges::sort(sorted);
        return sorted;
    }();
    return names;
}

void SpawnHelperNpcPlateLights(OBJECT* o)
{
    BMD* b = &Models[MODEL_PLATE_ARMOR];
    b->BodyScale = o->Scale;
    VectorCopy(o->Position, b->BodyOrigin);

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

bool Render(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType)
{
    const RenderStyle* style = g_itemStyles.Find(modelType);
    return style != nullptr && style->Render(b, o, look, modelType, alpha, renderType);
}

bool Spawns(int modelType)
{
    return g_itemSpawns.Find(modelType) != nullptr;
}

void Spawn(BMD* b, OBJECT* o, int modelType, float (*bones)[3][4])
{
    if (const SpawnFunction spawn = g_itemSpawns.Find(modelType))
        spawn(b, o, bones);
}

const ShineBelowPlus3* FindShineBelowPlus3(int modelType)
{
    const RenderStyle* style = g_itemStyles.Find(modelType);
    return style != nullptr ? style->shine : nullptr;
}

bool RenderGlow(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType, int texture)
{
    const RenderStyle* style = g_itemStyles.Find(modelType);
    if (style == nullptr || style->glow == nullptr)
    {
        return false;
    }
    style->glow(b, o, look, alpha, renderType, texture);
    return true;
}
} // namespace Render::Items::Styles
