///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Utilities/DrawSpawnCheck.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "ZzzEffect.h"
#include "Render/Effects/EffectDraw.h"
#include "Audio/DSPlaySound.h"
#include "Network/Server/WSclient.h"
#include "UI/Core/WindowSystem.h"
#include "Scenes/MainScene.h"
#include "Render/Renderer/MuRenderer.h"

#include <algorithm>
#include <array>

namespace
{
constexpr float kSpriteFrameMin = 0.2f;
constexpr float kSpriteFrameMax = 1.0f;
constexpr float kSpriteFrameStep = 0.1f;

float ComputeFrameDelta()
{
    return kSpriteFrameStep * FPS_ANIMATION_FACTOR;
}

float UpdateAnimationFrame(float currentFrame, bool isVisible)
{
    const float delta = ComputeFrameDelta();
    currentFrame += isVisible ? delta : -delta;
    return std::clamp(currentFrame, kSpriteFrameMin, kSpriteFrameMax);
}
} // namespace

OBJECT Sprites[MAX_SPRITES];

namespace
{
// Sprites created inside a bloom exclusion - a character's gem sparkles, item
// glows and eye lights - are drawn inside one, so post-processing leaves the
// character's Classic look alone.
std::array<bool, MAX_SPRITES> s_spriteBloomExcluded{};
// Sprites a RenderSprites pass already drew this frame; a later pass in the same frame draws only newer ones.
std::array<bool, MAX_SPRITES> s_spriteDrawn{};
} // namespace

std::vector<bool> SpritesDrawnThisFrame()
{
    return std::vector<bool>(s_spriteDrawn.begin(), s_spriteDrawn.end());
}

void SetSpritesDrawnThisFrame(const std::vector<bool>& drawn)
{
    std::copy_n(drawn.begin(), std::min(drawn.size(), s_spriteDrawn.size()), s_spriteDrawn.begin());
}

void ClearSprites()
{
    for (OBJECT& sprite : Sprites)
        sprite.Live = false;
}

void BeginSpriteDraws()
{
    s_spriteDrawn.fill(false);
}

int CreateSprite(int Type, vec3_t Position, float Scale, vec3_t Light, OBJECT* Owner, float Rotation, int SubType)
{
    Diagnostics::DrawSpawnCheck::Note("sprite");
    if (g_pOption && !g_pOption->GetRenderAllEffects())
    {
        return false;
    }

    for (int i = 0; i < MAX_SPRITES; i++)
    {
        OBJECT* o = &Sprites[i];
        if (!o->Live)
        {
            o->Live = true;
            o->Type = Type;
            o->SubType = SubType;
            o->Owner = Owner;
            s_spriteBloomExcluded[i] = mu::GetRenderer().IsBloomExcluded();
            s_spriteDrawn[i] = false;
            o->AnimationFrame = 1.f;
            o->Visible = true;
            o->Scale = Scale;
            o->Angle[2] = Rotation;
            VectorCopy(Position, o->Position);
            VectorCopy(Position, o->StartPosition);
            VectorCopy(Light, o->Light);
            return i;
        }
    }
    return false;
}

bool IsSpriteBloomExcluded(int sprite)
{
    return s_spriteBloomExcluded[sprite];
}

Render::EffectDrawing::SpriteDraw DescribeSprite(const OBJECT& sprite, bool bloomExcluded)
{
    using Render::EffectDrawing::Blend;
    const OBJECT* o = &sprite;
    Render::EffectDrawing::SpriteDraw draw;
    draw.texture = o->Type;
    VectorCopy(o->Position, draw.position);
    VectorCopy(o->Light, draw.light);
    draw.rotation = o->Angle[2];
    draw.bloomExcluded = bloomExcluded;
    if (o->Type == BITMAP_FORMATION_MARK)
        draw.state.blend = Blend::AlphaTest;
    else if (o->SubType == 0)
        draw.state.blend = Blend::Glow;
    else if (o->SubType == 1)
        draw.state.blend = Blend::Subtract;
    else if (o->SubType == 2)
        draw.state.blend = Blend::AlphaTest;
    else if (o->SubType == 3)
        draw.state.blend = Blend::Blend2;

    float Scale = UpdateAnimationFrame(o->AnimationFrame, o->Visible) * o->Scale;

    BITMAP_t* pBitmap = Bitmaps.GetTexture(o->Type);
    float Width = pBitmap->Width * Scale;
    float Height = pBitmap->Height * Scale;

    if (o->Type == BITMAP_FORMATION_MARK)
    {
        float u = 0.0f, v = 0.0f, uw, vw;
        uw = 0.33f;
        vw = 0.33f;
        switch (o->SubType)
        {
        case 0:
            u = 0.f;
            v = 0.f;
            break;

        case 1:
            u = 0.33f;
            v = 0.f;
            break;

        case 2:
            u = 0.66f;
            v = 0.f;
            break;

        case 3:
            u = 0.f;
            v = 0.33f;
            break;

        case 4:
            u = 0.33f;
            v = 0.33f;
            break;

        case 5:
            u = 0.66f;
            v = 0.33f;
            break;

        case 6:
            u = 0.f;
            v = 0.66f;
            break;

        case 7:
            u = 0.33f;
            v = 0.66f;
            break;
        }

        draw.width = 64.f;
        draw.height = 64.f;
        draw.u = u;
        draw.v = v;
        draw.uWidth = uw;
        draw.vHeight = vw;
    }
    else
    {
        draw.width = Width;
        draw.height = Height;
    }
    return draw;
}

void RenderSprites(BYTE byRenderOneMore)
{
    if (g_pOption && !g_pOption->GetRenderAllEffects())
    {
        return;
    }
    if (IsSpritesDisabledDebug())
    {
        return;
    }

    for (int i = 0; i < MAX_SPRITES; i++)
    {
        const OBJECT* o = &Sprites[i];
        if (byRenderOneMore == 1)
        {
            if (o->Position[2] > 350.f)
            {
                continue;
            }
        }
        else if (byRenderOneMore == 2)
        {
            if (o->Position[2] <= 300.f)
            {
                s_spriteDrawn[i] = true;
                continue;
            }
        }

        if (o->Live && !s_spriteDrawn[i])
        {
            Render::EffectDrawing::ActiveEffectDrawer().DrawSprite(DescribeSprite(*o, s_spriteBloomExcluded[i]));

            if (byRenderOneMore == 0 || byRenderOneMore == 2)
            {
                s_spriteDrawn[i] = true;
            }
        }
    }
}
