#include "stdafx.h"

#include "ItemEffects.h"
#include "ItemModelLookup.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <string_view>

namespace Render::Items::Effects
{
namespace
{
// An effect runs before the model is drawn, and may change the level the
// model glows like. Drawn: it drew the model itself, nothing more is drawn.
using EffectFunction = Result (*)(BMD* b, OBJECT* o, int Type, float Alpha, int& Level);
// Draws the model below +3 instead of the plain drawing.
using ShineFunction = void (*)(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float* Light);

struct ItemEffect
{
    const char* name;
    EffectFunction apply = nullptr;
    ShineFunction renderBelowPlus3 = nullptr;
};

// ------------------------------------------------ the effects

Result ApplyBillOfBalrog(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    Vector(0.5f, 0.5f, 1.5f, b->BodyLight);
    b->StreamMesh = 0;
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
    b->StreamMesh = -1;
    return Result::Applied;
}

Result ApplyMeshesPerLevel(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->StreamMesh = 0;
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    if (Level == 1)
    {
    }
    else if (Level == 2)
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.75f, 0.65f, 0.5f, b->BodyLight);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, BITMAP_CHROME);
    }
    else if (Level == 3)
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.75f, 0.65f, 0.5f, b->BodyLight);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, BITMAP_CHROME);
        b->RenderMesh(2, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, BITMAP_CHROME);
    }
    b->StreamMesh = -1;
    return Result::Drawn;
}

Result ApplyFirecracker(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    b->StreamMesh = 0;
    o->BlendMeshLight = 1.f;
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    Vector(1.f, 0.f, 0.f, b->BodyLight);
    b->LightEnable = true;
    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    b->StreamMesh = -1;
    return Result::Drawn;
}

Result ApplyGmGift(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    Vector(1.f, 1.f, 1.f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    b->LightEnable = true;
    Vector(0.1f, 0.6f, 0.4f, b->BodyLight);
    o->Alpha = 0.5f;
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
    return Result::Drawn;
}

Result ApplyDarkLordScroll(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    b->BeginRender(o->Alpha);
    o->BlendMeshLight = 1.f;
    b->RenderMesh(0, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
    b->RenderMesh(1, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->RenderMesh(2, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 2, 1 - o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    b->EndRender();
    return Result::Drawn;
}

Result ApplySpirit(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    switch (Level)
    {
    case 0:
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
        break;

    case 1:
        Vector(0.3f, 0.8f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV, o->HiddenMesh);
        break;
    }
    return Result::Drawn;
}

Result ApplyPotion(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    if (Level > 0)
        Level = 7;
    return Result::Applied;
}

Result ApplyFruits(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    switch (Level)
    {
    case 0:
        Vector(0.0f, 0.5f, 1.0f, b->BodyLight);
        break;
    case 1:
        Vector(1.0f, 0.2f, 0.0f, b->BodyLight);
        break;
    case 2:
        Vector(1.0f, 0.8f, 0.0f, b->BodyLight);
        break;
    case 3:
        Vector(0.6f, 0.8f, 0.4f, b->BodyLight);
        break;
    }
    b->RenderBody(RENDER_METAL, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh, BITMAP_CHROME + 1);
    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
    return Result::Drawn;
}

Result ApplyBloodBone(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->BlendMeshTexCoordU = sinf(gMapManager.WorldActive * 0.0001f);
    o->BlendMeshTexCoordV = -WorldTime * 0.0005f;
    Vector(.9f, .9f, .9f, b->BodyLight);
    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh);
    Vector(.9f, .1f, .1f, b->BodyLight);
    Models[o->Type].StreamMesh = 0;
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
    Models[o->Type].StreamMesh = -1;
    return Result::Drawn;
}

Result ApplyInvisibilityCloak(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    Vector(0.8f, 0.8f, 0.8f, b->BodyLight);
    float sine = float(sinf(WorldTime * 0.002f) * 0.3f) + 0.7f;

    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  o->HiddenMesh);
    return Result::Drawn;
}

Result ApplyDevilsEye(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    float sine = (float)sinf(WorldTime * 0.002f) * 10.f + 15.65f;

    o->BlendMesh = 1;
    o->BlendMeshLight = sine;
    o->BlendMeshTexCoordV = (int)WorldTime % 2000 * 0.0005f;
    o->Alpha = 2.0f;

    float Luminosity = sine;
    Vector(Luminosity / 5.0f, Luminosity / 5.0f, Luminosity / 5.0f, o->Light);
    return Result::Applied;
}

Result ApplyDevilsKey(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
    vec3_t p, Position, EffLight;
    Vector(0.f, 0.f, 0.f, p);

    float Scale = Luminosity * 0.8f;
    Vector(Luminosity * 2, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

    b->TransformPosition(BoneTransform[1], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);

    b->TransformPosition(BoneTransform[2], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
    return Result::Applied;
}

Result ApplyDevilsInvitation(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
    vec3_t p, Position, EffLight;
    Vector(0.f, 0.f, 0.f, p);

    float Scale = Luminosity * 0.8f;
    Vector(Luminosity * 2, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

    b->TransformPosition(BoneTransform[9], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);

    b->TransformPosition(BoneTransform[10], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
    return Result::Applied;
}

Result ApplyRena(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.25f + 0.75f;
    vec3_t EffLight;

    Vector(Luminosity * 1.f, Luminosity * 0.5f, Luminosity * 0.f, EffLight);
    CreateSprite(BITMAP_SPARK + 1, o->Position, 2.5f, EffLight, o);
    return Result::Applied;
}

Result ApplyWingsOfDragon(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->BlendMeshLight = (float)(sinf(WorldTime * 0.001f) + 1.f) / 4.f;
    return Result::Applied;
}

Result ApplyWingsOfSoul(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->BlendMeshLight = (float)sinf(WorldTime * 0.001f) + 1.1f;
    return Result::Applied;
}

Result ApplyRedSpirit(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.4f + 0.6f;
    return Result::Applied;
}

Result ApplyStaffOfKundun(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
    return Result::Applied;
}

Result ApplyDivineSet(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->BlendMeshLight = 1.f;
    return Result::Applied;
}

Result ApplyHiddenMeshByLevel(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    switch (Level)
    {
    case 0:
        o->HiddenMesh = 1;
        break;
    case 1:
        o->HiddenMesh = 0;
        break;
    }
    return Result::Applied;
}

Result ApplyHideMesh1(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    o->HiddenMesh = 1;
    return Result::Applied;
}

Result ApplyWingsOfDarkness(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    vec3_t posCenter, p, Position, Light;
    float Scale = sinf(WorldTime * 0.004f) * 0.3f + 0.3f;

    Scale = (Scale * 10.f) + 20.f;

    Vector(0.6f, 0.3f, 0.8f, Light);

    Vector(0.f, 0.f, 0.f, p);

    for (int i = 0; i < 5; ++i)
    {
        b->TransformPosition(BoneTransform[22 - i], p, posCenter, true);
        b->TransformPosition(BoneTransform[30 - i], p, Position, true);
        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_THUNDER, Position, posCenter, o->Angle, 14, o, Scale);
            CreateJoint(BITMAP_JOINT_SPIRIT, posCenter, Position, o->Angle, 4, o, Scale + 5);
        }

        CreateSprite(BITMAP_FLARE_BLUE, posCenter, Scale / 28.f, Light, o);
    }

    for (int i = 0; i < 5; ++i)
    {
        b->TransformPosition(BoneTransform[7 - i], p, posCenter, true);
        b->TransformPosition(BoneTransform[11 + i], p, Position, true);
        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_THUNDER, Position, posCenter, o->Angle, 14, o, Scale);
            CreateJoint(BITMAP_JOINT_SPIRIT, posCenter, Position, o->Angle, 4, o, Scale + 5);
        }

        CreateSprite(BITMAP_FLARE_BLUE, posCenter, Scale / 28.f, Light, o);
    }
    return Result::Applied;
}

Result ApplyWingOfStorm(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    vec3_t vRelativePos, vPos, vLight;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    Vector(0.f, 0.f, 0.f, vPos);
    Vector(0.f, 0.f, 0.f, vLight);

    float fLuminosity = absf(sinf(WorldTime * 0.0004f)) * 0.4f;
    Vector(0.5f + fLuminosity, 0.5f + fLuminosity, 0.5f + fLuminosity, vLight);
    int iBone[] = {9, 20, 19, 10, 18, 28, 27, 36, 35, 38, 37, 53, 48, 62, 70, 72, 71, 78, 79, 80, 87, 90, 91, 106, 102};
    float fScale = 0.f;

    for (int i = 0; i < 25; ++i)
    {
        b->TransformPosition(BoneTransform[iBone[i]], vRelativePos, vPos, true);
        fScale = 0.5f; // (rand()%10) * 0.05f + 0.3f;
        CreateSprite(BITMAP_CLUD64, vPos, fScale, vLight, o, WorldTime * 0.01f, 1);
    }

    int iBoneThunder[] = {11, 21, 29, 63, 81, 89};
    if (rand_fps_check(2))
    {
        for (int i = 0; i < 6; ++i)
        {
            b->TransformPosition(BoneTransform[iBoneThunder[i]], vRelativePos, vPos, true);
            if (rand_fps_check(20))
            {
                Vector(0.6f, 0.6f, 0.9f, vLight);
                CreateEffect(MODEL_FENRIR_THUNDER, vPos, o->Angle, vLight, 1, o);
            }
        }
    }

    int iBoneLight[] = {64, 61, 69, 77, 86, 98, 97, 99, 104, 103, 105, 12, 8, 17, 26, 34, 52, 44, 51, 50, 49, 45};

    fScale = absf(sinf(WorldTime * 0.003f)) * 0.2f;

    for (int i = 0; i < 22; ++i)
    {
        b->TransformPosition(BoneTransform[iBoneLight[i]], vRelativePos, vPos, true);
        if (iBoneLight[i] == 12 || iBoneLight[i] == 64 || iBoneLight[i] == 98 || iBoneLight[i] == 52)
        {
            Vector(0.9f, 0.0f, 0.0f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, fScale + 1.4f, vLight, o);
        }
        else
        {
            Vector(0.8f, 0.5f, 0.2f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, fScale + 0.3f, vLight, o);
        }
    }
    return Result::Applied;
}

Result ApplyWingOfEternal(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.003f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.003f)) * 0.3f;

    Vector(0.5f + Luminosity, 0.5f + Luminosity, 0.6f + Luminosity, Light);
    // int iRedFlarePos[] = { 25, 32, 53, 15, 9, 35 };
    int iRedFlarePos[] = {24, 31, 15, 8, 53, 35};
    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(BoneTransform[iRedFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 1.3f, Light, o);
    }

    Vector(0.1f, 0.1f, 0.9f, Light);
    // int iGreenFlarePos[] = { 23, 22, 24, 34, 5, 31, 14, 12, 27, 8, 6, 7, 16, 13, 56, 37, 58, 40, 39, 38 };
    int iGreenFlarePos[] = {22, 23, 25, 29, 30, 28, 32, 13, 16, 14, 12, 9, 7, 6, 57, 58, 40, 39};

    for (int i = 0; i < 18; ++i)
    {
        b->TransformPosition(BoneTransform[iGreenFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 1.5f, Light, o);
    }
    int iGreenFlarePos2[] = {56, 38, 51, 45};

    for (int i = 0; i < 4; ++i)
    {
        b->TransformPosition(BoneTransform[iGreenFlarePos2[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 0.5f, Light, o);
    }
    return Result::Applied;
}

Result ApplyWingOfIllusion(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;

    Vector(0.5f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, Light);
    int iRedFlarePos[] = {5, 6, 7, 8, 18, 19, 23, 24, 25, 27, 37, 38};
    for (int i = 0; i < 12; ++i)
    {
        b->TransformPosition(BoneTransform[iRedFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 0.6f, Light, o);
    }

    Vector(0.0f + Luminosity, 0.5f + Luminosity, 0.0f + Luminosity, Light);
    int iGreenFlarePos[] = {4, 9, 13, 14, 26, 32, 31, 33};

    for (int i = 0; i < 8; ++i)
    {
        b->TransformPosition(BoneTransform[iGreenFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
    }

    Vector(1.0f, 1.0f, 1.0f, Light);
    float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
    Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, Light);
    CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o, 0.5f);
    if (rand_fps_check(2))
    {
        b->TransformPosition(BoneTransform[13], p, Position, true);
        CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 5, 0.5f);
        b->TransformPosition(BoneTransform[31], p, Position, true);
        CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 5, 0.5f);
    }
    return Result::Applied;
}

Result ApplyWingOfRuin(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.003f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.003f)) * 0.3f;

    Vector(0.7f + Luminosity, 0.5f + Luminosity, 0.8f + Luminosity, Light);
    int iRedFlarePos[] = {6, 15, 24, 56, 47, 38};
    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(BoneTransform[iRedFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 1.5f, Light, o);
    }

    Vector(0.6f, 0.4f, 0.7f, Light);
    int iSparkPos[] = {7,  16, 25, 57, 48, 39, 11, 22, 31, 63, 54, 40, 10, 21, 30, 62, 53, 41, 9,
                       20, 29, 61, 52, 42, 8,  19, 28, 60, 51, 43, 18, 27, 59, 50, 17, 26, 58, 49};
    int iNumParticle = 1;

    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.1f);
    }

    for (int i = 6; i < 18; ++i)
    {
        b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.3f);
    }

    for (int i = 18; i < 30; ++i)
    {
        b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.5f);
    }

    for (int i = 30; i < 38; ++i)
    {
        b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.7f);
    }
    return Result::Applied;
}

Result ApplyWingOfDimension(BMD* b, OBJECT* o, int Type, float Alpha, int& Level)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;

    Vector((1.0f + Luminosity) / 2.f, (0.7f + Luminosity) / 2.f, (0.2f + Luminosity) / 2.f, Light);
    int iFlarePos0[] = {7, 30, 31, 43, 8, 20};

    int icnt;
    for (icnt = 0; icnt < 2; ++icnt)
    {
        b->TransformPosition(BoneTransform[iFlarePos0[icnt]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 2.0f, Light, o);
    }
    Vector((1.0f + Luminosity) / 4.f, (0.7f + Luminosity) / 4.f, (0.2f + Luminosity) / 4.f, Light);
    for (; icnt < 6; ++icnt)
    {
        b->TransformPosition(BoneTransform[iFlarePos0[icnt]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 0.5f, Light, o);
    }

    Vector((0.5f + Luminosity) / 2.f, (0.1f + Luminosity) / 2.f, (0.4f + Luminosity) / 2.f, Light);
    int iGreenFlarePos[] = {29, 38, 42, 19, 15, 6};

    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(BoneTransform[iGreenFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 2.0f, Light, o);
    }
    return Result::Applied;
}

// ------------------------------------------------ the shine below +3

void RenderCursedCastleWater(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float* Light)
{
    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
    RenderPartObjectBodyColor2(b, o, Type, 0.5f, RENDER_TEXTURE | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 0.5f);
    RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
}

void RenderHarmonyShine(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float* Light)
{
    VectorCopy(Light, b->BodyLight);
    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
    RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
    RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
}

void RenderSealShine(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float* Light)
{
    Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
    RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
    RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
}

// ------------------------------------------------ the names

const ItemEffect ItemEffects[] = {
    {"billOfBalrog", ApplyBillOfBalrog},
    {"meshesPerLevel", ApplyMeshesPerLevel},
    {"firecracker", ApplyFirecracker},
    {"gmGift", ApplyGmGift},
    {"darkLordScroll", ApplyDarkLordScroll},
    {"spirit", ApplySpirit},
    {"potion", ApplyPotion},
    {"fruits", ApplyFruits},
    {"bloodBone", ApplyBloodBone},
    {"invisibilityCloak", ApplyInvisibilityCloak},
    {"devilsEye", ApplyDevilsEye},
    {"devilsKey", ApplyDevilsKey},
    {"devilsInvitation", ApplyDevilsInvitation},
    {"rena", ApplyRena},
    {"wingsOfDragon", ApplyWingsOfDragon},
    {"wingsOfSoul", ApplyWingsOfSoul},
    {"redSpirit", ApplyRedSpirit},
    {"staffOfKundun", ApplyStaffOfKundun},
    {"divineSet", ApplyDivineSet},
    {"hiddenMeshByLevel", ApplyHiddenMeshByLevel},
    {"hideMesh1", ApplyHideMesh1},
    {"wingsOfDarkness", ApplyWingsOfDarkness},
    {"wingOfStorm", ApplyWingOfStorm},
    {"wingOfEternal", ApplyWingOfEternal},
    {"wingOfIllusion", ApplyWingOfIllusion},
    {"wingOfRuin", ApplyWingOfRuin},
    {"wingOfDimension", ApplyWingOfDimension},
    {"cursedCastleWater", nullptr, RenderCursedCastleWater},
    {"harmonyShine", nullptr, RenderHarmonyShine},
    {"sealShine", nullptr, RenderSealShine},
};

const ItemEffect* FindEffect(std::string_view name)
{
    const auto found = std::find_if(std::begin(ItemEffects), std::end(ItemEffects),
                                    [name](const ItemEffect& effect) { return effect.name == name; });
    return found != std::end(ItemEffects) ? found : nullptr;
}

// The effect of every item type, taken from the item model database after
// each build of it.
ItemModelTable<const ItemEffect*> g_itemEffects{[](const Data::Items::ItemModelDefinition& model)
                                                {
                                                    // Names that are not effects are drawn without one; the
                                                    // model loader reports them.
                                                    return model.effect.empty() ? nullptr : FindEffect(model.effect);
                                                }};
} // namespace

bool Exists(std::string_view name)
{
    return FindEffect(name) != nullptr;
}

Result Apply(BMD* b, OBJECT* o, int modelType, float alpha, int& level)
{
    const ItemEffect* effect = g_itemEffects.Find(modelType);
    if (effect == nullptr || effect->apply == nullptr)
    {
        return Result::None;
    }
    return effect->apply(b, o, modelType, alpha, level);
}

bool RenderBelowPlus3(BMD* b, OBJECT* o, int modelType, float alpha, int renderType, float* light)
{
    const ItemEffect* effect = g_itemEffects.Find(modelType);
    if (effect == nullptr || effect->renderBelowPlus3 == nullptr)
    {
        return false;
    }
    effect->renderBelowPlus3(b, o, modelType, alpha, renderType, light);
    return true;
}
} // namespace Render::Items::Effects
