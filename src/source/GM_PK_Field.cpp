//*****************************************************************************
// file    : GM_PK_Field.cpp
//*****************************************************************************

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>

#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "ZzzOpenData.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "Random.h"

namespace
{
    struct MonsterDefinition
    {
        int monsterType;
        EMonsterModelType monsterModelId;
        int objectModelId;
        float scale;
        bool assignLifetime;
        int lifetime;
    };

    constexpr std::array<MonsterDefinition, 12> kMonsterDefinitions{ {
        {MONSTER_ZOMBIE_FIGHTER, EMonsterModelType::MONSTER_MODEL_ZOMBIE_FIGHTER, MODEL_ZOMBIE_FIGHTER, 1.0f, false, 0},
        {MONSTER_ZOMBIER, EMonsterModelType::MONSTER_MODEL_ZOMBIE_FIGHTER, MODEL_ZOMBIE_FIGHTER, 1.0f, false, 0},
        {MONSTER_GLADIATOR, EMonsterModelType::MONSTER_MODEL_GLADIATOR, MODEL_GLADIATOR, 1.0f, false, 0},
        {MONSTER_HELL_GLADIATOR, EMonsterModelType::MONSTER_MODEL_GLADIATOR, MODEL_GLADIATOR, 1.0f, false, 0},
        {MONSTER_SLAUGHTERER, EMonsterModelType::MONSTER_MODEL_SLAUGTHERER, MODEL_SLAUGHTERER, 0.7f, false, 0},
        {MONSTER_ASH_SLAUGHTERER, EMonsterModelType::MONSTER_MODEL_SLAUGTHERER, MODEL_SLAUGHTERER, 0.7f, false, 0},
        {MONSTER_BLOOD_ASSASSIN, EMonsterModelType::MONSTER_MODEL_BLOOD_ASSASSIN, MODEL_BLOOD_ASSASSIN, 1.0f, true, 100},
        {MONSTER_CRUEL_BLOOD_ASSASSIN, EMonsterModelType::MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN, MODEL_CRUEL_BLOOD_ASSASSIN, 1.0f, true, 100},
        {MONSTER_COLD_BLOODED_ASSASSIN, EMonsterModelType::MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN, MODEL_CRUEL_BLOOD_ASSASSIN, 1.0f, true, 100},
        {MONSTER_BURNING_LAVA_GIANT, EMonsterModelType::MONSTER_MODEL_BURNING_LAVA_GIANT, MODEL_BURNING_LAVA_GIANT, 1.0f, false, 0},
        {MONSTER_LAVA_GIANT, EMonsterModelType::MONSTER_MODEL_LAVA_GIANT, MODEL_LAVA_GIANT, 1.0f, false, 0},
        {MONSTER_RUTHLESS_LAVA_GIANT, EMonsterModelType::MONSTER_MODEL_LAVA_GIANT, MODEL_LAVA_GIANT, 1.0f, false, 0},
    } };

    const MonsterDefinition* FindMonsterDefinition(int monsterType)
    {
        const auto it = std::find_if(kMonsterDefinitions.begin(), kMonsterDefinitions.end(),
            [monsterType](const MonsterDefinition& def) { return def.monsterType == monsterType; });
        return (it != kMonsterDefinitions.end()) ? &(*it) : nullptr;
    }

    constexpr int kBlurSampleCount = 5;
}

CGM_PK_FieldPtr CGM_PK_Field::Make()
{
    CGM_PK_FieldPtr pkfield(new CGM_PK_Field);
    pkfield->Init();
    return pkfield;
}

CGM_PK_Field::CGM_PK_Field()
{
    Init();
}

CGM_PK_Field::~CGM_PK_Field()
{
    Destroy();
}

void CGM_PK_Field::Init()
{
    //n/a
}

void CGM_PK_Field::Destroy()
{
    //n/a
}

CHARACTER* CGM_PK_Field::CreateMonster(int type, int positionX, int positionY, int key)
{
    if (!gMapManager.IsPKField())
    {
        return nullptr;
    }

    const MonsterDefinition* definition = FindMonsterDefinition(type);
    if (definition == nullptr)
    {
        return nullptr;
    }

    OpenMonsterModel(definition->monsterModelId);

    CHARACTER* character = CreateCharacter(key, definition->objectModelId, positionX, positionY);
    if (character == nullptr)
    {
        return nullptr;
    }

    character->Object.Scale = definition->scale;
    character->Object.m_iAnimation = 0;
    character->Weapon[0].Type = -1;
    character->Weapon[1].Type = -1;

    if (definition->assignLifetime)
    {
        character->Object.LifeTime = definition->lifetime;
    }

    return character;
}

void CGM_PK_Field::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!gMapManager.IsPKField())
        return;

    if (o->Type >= MODEL_GLADIATOR && o->Type <= MODEL_CRUEL_BLOOD_ASSASSIN)
    {
        float Start_Frame = 0.0f;
        float End_Frame = 0.0f;

        switch (o->Type)
        {
        case MODEL_SLAUGHTERER:
        {
            Start_Frame = 4.0f;
            End_Frame = 10.0f;
        }
        break;
        case MODEL_GLADIATOR:
        {
            Start_Frame = 3.0f;
            End_Frame = 7.0f;
        }
        break;
        case MODEL_BLOOD_ASSASSIN:
        case MODEL_CRUEL_BLOOD_ASSASSIN:
        {
            Start_Frame = 3.0f;
            End_Frame = 8.0f;
        }
        break;
        default:
            return;
        }

        if ((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK2))
        {
            BMD* b = &Models[o->Type];
            vec3_t  Light;

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fDelay = 5.0f;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / fDelay;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < fDelay; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                switch (o->Type)
                {
                case MODEL_SLAUGHTERER:
                {
                    Vector(0.3f, 0.3f, 0.3f, Light);
                    b->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);
                }
                break;
                case MODEL_GLADIATOR:
                {
                    Vector(0.0f, 0.3f, 0.2f, Light);
                    b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);
                }
                break;
                case MODEL_BLOOD_ASSASSIN:
                case MODEL_CRUEL_BLOOD_ASSASSIN:
                {
                    if (o->Type == MODEL_BLOOD_ASSASSIN)
                    {
                        Vector(0.9f, 0.2f, 0.1f, Light);
                    }
                    else	//o->Type == MODEL_MONSTER01+161
                    {
                        Vector(0.2f, 0.9f, 0.1f, Light);
                    }
                    b->TransformPosition(BoneTransform[40], StartRelative, StartPos, false);

                    b->TransformByBoneMatrix(EndPos, BoneTransform[40]);
                    CreateBlur(c, StartPos, EndPos, Light, 5, false, 0);

                    b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 5, false, 1);
                    b->TransformPosition(BoneTransform[53], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 5, false, 2);
                    b->TransformPosition(BoneTransform[14], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[14], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 5, false, 3);
                    b->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 5, false, 4);
                }
                break;

                default:
                    break;
                }
                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
}
bool CGM_PK_Field::CreateObject(OBJECT* o)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    if (o->Type >= 0 && o->Type <= 6)
    {
        o->CollisionRange = -300;
        return true;
    }

    return false;
}
bool CGM_PK_Field::MoveObject(OBJECT* o)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    switch (o->Type)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        o->HiddenMesh = -2;
        return true;

    default:
        break;
    }

    return false;
}

bool CGM_PK_Field::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    vec3_t Light;

    switch (o->Type)
    {
    case 0:
    {
        o->HiddenMesh = -2;
        float fLumi = ((sinf(WorldTime * 0.001f) + 1.f) * 0.5f) * 100.0f;

        int nRanDelay = o->Position[0];
        nRanDelay = nRanDelay % 3 + 1;
        int nRanTemp = 30;
        nRanTemp = nRanTemp * nRanDelay;
        int nRanGap = 10;
        if (nRanTemp != 90.0f)
        {
            nRanGap = 40;
        }

        if (fLumi >= nRanTemp && fLumi <= nRanTemp + nRanGap)
        {
            Vector(1.0f, 1.0f, 1.0f, Light);
            for (int i = 0; i < 20; ++i)
            {
                CreateParticleFpsChecked(BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 6, o->Scale, o);
            }
        }
    }
    return true;
    case 1:
    {
        o->HiddenMesh = -2;
        if (rand_fps_check(3))
        {
            Vector(1.0f, 1.0f, 1.0f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 60, o->Scale, o);
        }
    }
    return true;
    case 2:
    {
        o->HiddenMesh = -2;
        vec3_t  Light;
        if (rand_fps_check(3))
        {
            Vector(0.f, 0.f, 0.f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 16, o->Scale, o);
        }
    }
    return true;
    case 3:
    {
        o->HiddenMesh = -2;
        if (rand_fps_check(4))
        {
            float fRed = Random::RangeFloat(0, 2) * 0.01f + 0.015f;
            Vector(fRed, 0.0f, 0.0f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 11, o->Scale, o);
        }
    }
    return true;
    case 4:
    {
        o->HiddenMesh = -2;
        Vector(1.0f, 0.4f, 0.4f, Light);
        vec3_t vAngle;
        if (rand_fps_check(3))
        {
            Vector(Random::RangeFloat(120, 159), 0.f, Random::RangeFloat(0, 29), vAngle);
            VectorAdd(vAngle, o->Angle, vAngle);
            CreateJoint(BITMAP_JOINT_SPARK, o->Position, o->Position, vAngle, 4, o, o->Scale);
            CreateParticle(BITMAP_SPARK, o->Position, vAngle, Light, 9, o->Scale);
        }
    }
    return true;
    case 5:
    {
        o->HiddenMesh = -2;
        if (rand_fps_check(3))
        {
            Vector(0.3f, 0.3f, 0.3f, o->Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 21, o->Scale);
        }
    }
    return true;
    case 6:
    {
        o->HiddenMesh = -2;

        vec3_t vLightFire;
        Vector(1.0f, 0.2f, 0.0f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 2.0f * o->Scale, vLightFire, o);

        vec3_t vLight;
        Vector(1.0f, 1.0f, 1.0f, vLight);
        switch (Random::RangeInt(0, 2))
        {
        case 0:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK1, o->Position, o->Angle, vLight, 0, o->Scale);
            break;
        case 1:
            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, vLight, 4, o->Scale);
            break;
        case 2:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK3, o->Position, o->Angle, vLight, 0, o->Scale);
            break;
        }
    }
    return true;
    }
    return false;
}

bool CGM_PK_Field::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    if (o->Type >= MODEL_ZOMBIE_FIGHTER && o->Type <= MODEL_BURNING_LAVA_GIANT)
    {
        RenderMonster(o, b, ExtraMon);

        return true;
    }
    else if (o->Type == 15)
    {
        b->StreamMesh = 0;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 10000 * 0.0001f);
        b->StreamMesh = -1;

        return true;
    }
    else if (o->Type == 67)
    {
        b->StreamMesh = 1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime % 10000 * 0.0001f);
        b->StreamMesh = -1;
        vec3_t light;
        Vector(1.0f, 0.0f, 0.0f, light);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        VectorCopy(light, b->BodyLight);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.2f, 0, 0.2f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        vec3_t vLightFire, Position, vPos;
        Vector(1.0f, 0.0f, 0.0f, vLightFire);
        Vector(0.0f, 0.0f, 0.0f, vPos);

        b->TransformPosition(BoneTransform[6], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, o->Scale * 5.0f, vLightFire, o);

        Vector(0.0f, 0.0f, -350.0f, vPos);
        b->TransformPosition(BoneTransform[6], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, o->Scale * 5.0f, vLightFire, o);

        if (o->AnimationFrame >= 35 && o->AnimationFrame <= 37)
        {
            o->PKKey = -1;
        }

        if (o->AnimationFrame >= 1 && o->AnimationFrame <= 2 && o->PKKey != 1)
        {
            o->AnimationFrame = 1;

            int test = Random::RangeInt(0, 999);
            if (test >= 0 && test < 2)
            {
                o->PKKey = 1;
            }
            else
            {
                o->PKKey = -1;
            }
        }
        vec3_t p, Pos, Light;
        Vector(0.4f, 0.1f, 0.1f, Light);
        //Vector(Random::RangeFloat(-30, -11), Random::RangeFloat(-30, -11), 0.0f, p);
        Vector(-150.0f, 0.0f, 0.0f, p);
        b->TransformPosition(BoneTransform[4], p, Pos, false);
        if (o->AnimationFrame >= 35.0f && o->AnimationFrame < 50.0f)
            CreateParticleFpsChecked(BITMAP_SMOKE, Pos, o->Angle, Light, 63, o->Scale * 1.5f);

        return true;
    }
    else if (o->Type == 68)
    {
        b->StreamMesh = 1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime % 10000 * 0.0001f);
        b->StreamMesh = -1;

        vec3_t light;
        Vector(1.0f, 0.0f, 0.0f, light);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        VectorCopy(light, b->BodyLight);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.2f, 0, 0.2f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        vec3_t vLightFire, Position, vPos;
        Vector(1.0f, 0.0f, 0.0f, vLightFire);
        Vector(0.0f, 0.0f, 0.0f, vPos);

        b->TransformPosition(BoneTransform[6], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, o->Scale * 5.0f, vLightFire, o);

        Vector(0.0f, 0.0f, -350.0f, vPos);
        b->TransformPosition(BoneTransform[6], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, o->Scale * 5.0f, vLightFire, o);

        vec3_t p, Pos, Light;
        //Vector(0.08f, 0.08f, 0.08f, Light);
        Vector(0.3f, 0.1f, 0.1f, Light);
        Vector(Random::RangeFloat(-30, -11), Random::RangeFloat(-30, -11), 0.0f, p);
        b->TransformPosition(BoneTransform[4], p, Pos, false);
        if (o->AnimationFrame >= 7.0f && o->AnimationFrame < 13.0f)
            CreateParticleFpsChecked(BITMAP_SMOKE, Pos, o->Angle, Light, 18, o->Scale * 1.5f);

        return true;
    }
    return false;
}

void CGM_PK_Field::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!gMapManager.IsPKField())
        return;

    switch (o->Type)
    {
    case 16:		//song_lava2 fade in-out
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    break;
    default:
        break;
    }
}

bool CGM_PK_Field::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (!gMapManager.IsPKField())
        return false;

    switch (o->Type)
    {
    case MODEL_BLOOD_ASSASSIN:
    case MODEL_CRUEL_BLOOD_ASSASSIN:
    {
        if (o->CurrentAction == MONSTER01_DIE)
        {
            int iBones[] = { 5, 6, 7 };		// Neck/ Head/ HeadNub
            vec3_t vLight, vPos, vRelative;
            Vector(1.0f, 1.0f, 1.0f, vLight);
            vec3_t vLightFire;
            if (o->Type == MODEL_BLOOD_ASSASSIN)
            {
                Vector(1.0f, 0.2f, 0.0f, vLightFire);
            }
            else
            {
                Vector(0.2f, 1.0f, 0.0f, vLightFire);
            }
            for (int i = 0; i < 3; ++i)
            {
                float fScale = 1.2f;
                if (i >= 1)
                {
                    b->TransformByObjectBone(vPos, o, iBones[i]);
                    CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLightFire, o);

                    fScale = 0.7f;
                    Vector(Random::RangeFloat(-5, 4), Random::RangeFloat(-5, 4), Random::RangeFloat(-5, 4), vRelative);
                    b->TransformByObjectBone(vPos, o, iBones[i], vRelative);
                }
                else
                {
                    b->TransformByObjectBone(vPos, o, iBones[i]);
                    vPos[2] += 50.0f;
                    CreateSprite(BITMAP_LIGHT, vPos, 2.5f, vLightFire, o);

                    Vector(Random::RangeFloat(-10, 9), Random::RangeFloat(-10, 9), Random::RangeFloat(-10, 9), vRelative);
                    b->TransformByObjectBone(vPos, o, iBones[i], vRelative);
                }
                if (o->Type == MODEL_BLOOD_ASSASSIN)
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        float fScale = Random::RangeFloat(18, 22) * 0.03f;
                        switch (Random::RangeInt(0, 2))
                        {
                        case 0:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK1, vPos, o->Angle, vLight, 0, fScale);
                            break;
                        case 1:
                            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 4, fScale);
                            break;
                        case 2:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK3, vPos, o->Angle, vLight, 0, fScale);
                            break;
                        }
                    }
                }
                else		//o->Type == MODEL_MONSTER01+161
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        float fScale = Random::RangeFloat(18, 22) * 0.03f;
                        Vector(0.6f, 0.9f, 0.1f, o->Light);
                        switch (Random::RangeInt(0, 2))
                        {
                        case 0:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK1_MONO, vPos, o->Angle, o->Light, 0, fScale);
                            break;
                        case 1:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK2_MONO, vPos, o->Angle, o->Light, 4, fScale);
                            break;
                        case 2:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, vPos, o->Angle, o->Light, 0, fScale);
                            break;
                        }
                    }
                }
            }
        }
    }
    return true;
    }
    return false;
}

bool CGM_PK_Field::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    vec3_t vPos, vLight;

    switch (o->Type)
    {
    case MODEL_ZOMBIE_FIGHTER:
    {
    }
    return true;
    case MODEL_GLADIATOR:
    {
    }
    return true;
    case MODEL_SLAUGHTERER:
    {
        vec3_t p, Position;
        if (rand_fps_check(4))
        {
            Vector(0.0f, 50.0f, 0.0f, p);
            b->TransformPosition(o->BoneTransform[6], p, Position, true);

            Vector(1.0f, 1.0f, 1.0f, o->Light);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, o->Light, 61);
        }
    }
    return true;
    case MODEL_BLOOD_ASSASSIN:
    case MODEL_CRUEL_BLOOD_ASSASSIN:
    {
        int iBones[] = { 37, 11, 70, 65, 6 };

        switch (o->Type)
        {
        case MODEL_BLOOD_ASSASSIN:
            Vector(0.9f, 0.2f, 0.1f, vLight);	//red
            break;
        case MODEL_CRUEL_BLOOD_ASSASSIN:
            Vector(0.3f, 0.9f, 0.2f, vLight);	//green
            break;
        }
        if (o->CurrentAction != MONSTER01_DIE)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (Random::RangeInt(0, 3) > 0) continue;

                b->TransformByObjectBone(vPos, o, iBones[i]);
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 50, 1.0f);
                CreateParticleFpsChecked(BITMAP_SMOKELINE1 + Random::RangeInt(0, 2), vPos, o->Angle, vLight, 0, 0.01f);
            }

            if (o->CurrentAction == MONSTER01_ATTACK1)
            {
                vec3_t TempPos;
                Vector(0.0f, 0.0f, 0.0f, TempPos);

                b->TransformByObjectBone(vPos, o, iBones[4]);

                TempPos[1] = (Hero->Object.Position[1] - vPos[1]) * 0.5f;
                TempPos[0] = (Hero->Object.Position[0] - vPos[0]) * 0.5f;
                TempPos[2] = 0.0f;

                VectorNormalize(TempPos);
                VectorScale(TempPos, 50.0f, TempPos);

                VectorAdd(vPos, TempPos, vPos);

                CreateParticleFpsChecked(BITMAP_LIGHT + 2, vPos, o->Angle, vLight, 7, 0.5f);

                switch (o->Type)
                {
                case MODEL_BLOOD_ASSASSIN:
                    Vector(0.9f, 0.4f, 0.1f, vLight);	//red
                    break;
                case MODEL_CRUEL_BLOOD_ASSASSIN:
                    Vector(0.6f, 0.9f, 0.2f, vLight);	//green
                    break;
                }
                CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 29, 1.0f);
            }
        }
        else					//o->CurrentAction == MONSTER01_DIE
        {
            if ((int)o->LifeTime == 100)
            {
                o->LifeTime = 90;
                o->m_bRenderShadow = false;

                vec3_t vRelativePos, vWorldPos, Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

                Vector(0.f, 0.f, 0.f, vRelativePos);

                b->TransformPosition(o->BoneTransform[5], vRelativePos, vWorldPos, true);
                switch (o->Type)
                {
                case MODEL_BLOOD_ASSASSIN:
                    CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                    break;

                case MODEL_CRUEL_BLOOD_ASSASSIN:
                    CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                    break;
                }
            }
        }
    }
    return true;
    case MODEL_LAVA_GIANT:
    case MODEL_BURNING_LAVA_GIANT:
    {
        auto fRotation = (float)((int)(WorldTime * 0.1f) % 360);
        float fAngle = (sinf(WorldTime * 0.003f) + 1.0f) * 0.4f + 1.5f;
        vec3_t vWorldPos, vLight;
        switch (o->Type)
        {
        case MODEL_LAVA_GIANT:
        {
            Vector(0.5f, 0.1f, 0.9f, vLight);
        }
        break;
        case MODEL_BURNING_LAVA_GIANT:
        {
            Vector(0.9f, 0.4f, 0.1f, vLight);
        }
        break;
        }

        b->TransformByObjectBone(vWorldPos, o, 3);
        CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);
        fAngle = (sinf(WorldTime * 0.003f) + 1.0f) * 0.4f + 0.5f;
        b->TransformByObjectBone(vWorldPos, o, 37);
        CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);
        b->TransformByObjectBone(vWorldPos, o, 43);
        CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);

        vec3_t vRelative, vPos;
        Vector(1.0f, 1.0f, 1.0f, vLight);
        int iModel = (o->Type == MODEL_BURNING_LAVA_GIANT) ? MODEL_LAVAGIANT_FOOTPRINT_R : MODEL_LAVAGIANT_FOOTPRINT_V;

        if (o->CurrentAction == MONSTER01_WALK)
        {
            if (o->AnimationFrame >= 2.0f && o->AnimationFrame <= 2.4f && rand_fps_check(1))
            {
                Vector(0.0f, 0.0f, 0.0f, vPos);
                b->TransformPosition(o->BoneTransform[36], vPos, vRelative, true);
                CreateEffect(iModel, vRelative, o->Angle, vLight, 0, o, -1, 0, 0, 0, 1.3f);
                CreateParticle(BITMAP_SMOKE, vRelative, o->Angle, vLight, 62, 1.0f);
            }
            else if (o->AnimationFrame >= 7.0f && o->AnimationFrame <= 7.4f && rand_fps_check(1))
            {
                Vector(0.0f, 0.0f, 0.0f, vPos);
                b->TransformPosition(o->BoneTransform[42], vPos, vRelative, true);
                CreateEffect(iModel, vRelative, o->Angle, vLight, 0, o, -1, 0, 0, 0, 1.3f);
                CreateParticle(BITMAP_SMOKE, vRelative, o->Angle, vLight, 62, 1.0f);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->AnimationFrame >= 6.0f && o->AnimationFrame <= 6.4f && rand_fps_check(1))
            {
                Vector(0.0f, 0.0f, 0.0f, vPos);
                b->TransformPosition(o->BoneTransform[42], vPos, vRelative, true);
                CreateEffect(iModel, vRelative, o->Angle, vLight, 0, o, -1, 0, 0, 0, 1.6f);
                CreateParticle(BITMAP_SMOKE, vRelative, o->Angle, vLight, 62, 4.0f);
            }
        }

        vec3_t p, Position;
        if (rand_fps_check(4))
        {
            Vector(0.0f, 50.0f, 0.0f, p);
            b->TransformPosition(o->BoneTransform[7], p, Position, true);
            switch (o->Type)
            {
            case MODEL_LAVA_GIANT:
                Vector(0.5f, 0.1f, 0.9f, o->Light);
                break;
            case MODEL_BURNING_LAVA_GIANT:
                Vector(0.9f, 0.4f, 0.1f, o->Light);
                break;
            }
            CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light, 61);
        }
    }
    return true;
    }
    return false;
}

bool CGM_PK_Field::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    auto fRotation = (float)((int)(WorldTime * 0.1f) % 360);
    float fAngle = (sinf(WorldTime * 0.004f) + 1.0f) * 0.4f + 0.2f;
    vec3_t vWorldPos, vLight;
    Vector(0.1f, 0.4f, 0.5f, vLight);

    switch (o->Type)
    {
    case MODEL_ZOMBIE_FIGHTER:
    {
        b->TransformByObjectBone(vWorldPos, o, 9);
        CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);

        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        DisableDepthTest();
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight * fAngle, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        EnableDepthTest();
    }
    return true;
    case MODEL_GLADIATOR:
    {
        b->TransformByObjectBone(vWorldPos, o, 9);
        CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);

        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        DisableDepthTest();
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight * fAngle, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        EnableDepthTest();
    }
    return true;
    case MODEL_SLAUGHTERER:
    {
        float fBlendMeshLight = 0.0f;
        fBlendMeshLight = (sinf(WorldTime * 0.003f) + 1.0f) * 0.5f;

        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BUGBEAR_R);
    }
    return true;
    case MODEL_BLOOD_ASSASSIN:
    case MODEL_CRUEL_BLOOD_ASSASSIN:
    {
        if (o->CurrentAction == MONSTER01_DIE)
        {
            if ((int)o->LifeTime == 100)
            {
                switch (o->Type)
                {
                case MODEL_BLOOD_ASSASSIN:
                {
                    CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY, o->Position, o->Angle, o->Light, 0, o, 0, 0);
                }
                break;
                case MODEL_CRUEL_BLOOD_ASSASSIN:
                {
                    CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY, o->Position, o->Angle, o->Light, 0, o, 0, 0);
                }
                break;
                }
            }
        }
        else
        {
            float fBlendMeshLight = 0.0f;
            fBlendMeshLight = (sinf(WorldTime * 0.005f) + 1.0f) * 0.3f + 0.3f;

            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            switch (o->Type)
            {
            case MODEL_BLOOD_ASSASSIN:
            {
                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PKMON02);

                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, 3, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PKMON01);
            }
            break;
            case MODEL_CRUEL_BLOOD_ASSASSIN:
            {
                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PKMON04);

                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, 3, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PKMON03);
            }
            break;
            }
            Vector(b->BodyLight[0] * 0.65f, b->BodyLight[0] * 0.65f, b->BodyLight[0] * 0.65f, b->BodyLight);
            b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            Vector(b->BodyLight[0] * 2.0f, b->BodyLight[0] * 1.0f, b->BodyLight[0] * 0.4f, b->BodyLight);
            b->RenderMesh(2, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
    }
    return true;
    case MODEL_LAVA_GIANT:
    case MODEL_BURNING_LAVA_GIANT:
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        float fBlendMeshLight = 0.0f;
        fBlendMeshLight = (sinf(WorldTime * 0.001f));

        switch (o->Type)
        {
        case MODEL_LAVA_GIANT:
        case MODEL_BURNING_LAVA_GIANT:
        {
            float fAlpha = 1.0f;
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            int iTexture = (o->Type == MODEL_LAVA_GIANT) ? BITMAP_PKMON06 : BITMAP_PKMON05;

            if (fBlendMeshLight < 0)
            {
                //fBlendMeshLight = fabs(fBlendMeshLight);
                fBlendMeshLight = -(fBlendMeshLight);
                //b->RenderMesh(0,RENDER_TEXTURE|RENDER_DARK, fAlpha, 0,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, iTexture);
                b->RenderMesh(0, RENDER_TEXTURE, fAlpha, 0, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, iTexture);
            }
            else
            {
                for (int i = 0; i < 3; ++i)
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, fAlpha, 0, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, iTexture);
            }
        }
        break;
        }
    }
    return true;
    case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD:
    case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY:
    case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    }
    return false;
}

bool CGM_PK_Field::CreateFireSpark(PARTICLE* o)
{
    if (!gMapManager.IsPKField())
    {
        return false;
    }

    o->Type = BITMAP_FIRE_SNUFF;
    o->Scale = Random::RangeFloat(0, 49) / 100.f + 0.4f;
    vec3_t Position;
    Vector(Hero->Object.Position[0] + Random::RangeFloat(-800, 799), Hero->Object.Position[1] + Random::RangeFloat(-500, 899), Hero->Object.Position[2] + Random::RangeFloat(50, 349), Position);

    VectorCopy(Position, o->Position);
    VectorCopy(Position, o->StartPosition);
    o->Velocity[0] = -Random::RangeFloat(64, 127) * 0.1f;
    if (Position[1] < CameraPosition[1] + 400.f)
    {
        o->Velocity[0] = -o->Velocity[0] + 2.2f;
    }
    o->Velocity[1] = Random::RangeFloat(-16, 15) * 0.1f;
    o->Velocity[2] = Random::RangeFloat(-16, 15) * 0.1f;
    o->TurningForce[0] = Random::RangeFloat(-8, 7) * 0.1f;
    o->TurningForce[1] = Random::RangeFloat(-32, 31) * 0.1f;
    o->TurningForce[2] = Random::RangeFloat(-8, 7) * 0.1f;

    Vector(1.f, 0.f, 0.f, o->Light);

    return true;
}

bool CGM_PK_Field::PlayMonsterSound(OBJECT* o)
{
    if (!gMapManager.IsPKField())
        return false;

    float fDis_x, fDis_y;
    fDis_x = o->Position[0] - Hero->Object.Position[0];
    fDis_y = o->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 500.0f)
        return true;

    switch (o->Type)
    {
    case MODEL_ZOMBIE_FIGHTER:
    {
        if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_ATTACK);
        }
        else if (MONSTER01_SHOCK == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE01);
        }
        else if (MONSTER01_WALK == o->CurrentAction)
        {
            if (rand_fps_check(20))
            {
                PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE01);
            }
        }
        else if (MONSTER01_DIE == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_DEATH);
        }

        //	SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE02,
        //	SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE02,
    }
    return true;
    case MODEL_GLADIATOR:
    {
        if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_ATTACK);
        }
        else if (MONSTER01_SHOCK == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE01);
        }
        else if (MONSTER01_WALK == o->CurrentAction)
        {
            if (rand_fps_check(20))
            {
                PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_MOVE01);
            }
        }
        else if (MONSTER01_DIE == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_DEATH);
        }

        //	SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE02,
        //	SOUND_PKFIELD_RAISEDGLADIATOR_MOVE02,
    }
    return true;
    case MODEL_SLAUGHTERER:
    {
        if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_ATTACK);
        }
        else if (MONSTER01_SHOCK == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_DAMAGE01);
        }
        else if (MONSTER01_WALK == o->CurrentAction)
        {
            if (rand_fps_check(20))
            {
                PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_MOVE01);
            }
        }
        else if (MONSTER01_DIE == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_DEATH);
        }

        //SOUND_PKFIELD_ASHESBUTCHER_DAMAGE02,
        //SOUND_PKFIELD_ASHESBUTCHER_MOVE02,
    }
    return true;
    case MODEL_BLOOD_ASSASSIN:
    case MODEL_CRUEL_BLOOD_ASSASSIN:
    {
        if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_ATTACK);
        }
        else if (MONSTER01_SHOCK == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_DAMAGE01);
        }
        else if (MONSTER01_WALK == o->CurrentAction)
        {
            if (rand_fps_check(20))
            {
                PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_MOVE01);
            }
        }
        else if (MONSTER01_DIE == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_DEDTH);
        }

        //	SOUND_PKFIELD_BLOODASSASSIN_DAMAGE02,
        //	SOUND_PKFIELD_BLOODASSASSIN_MOVE01,
    }
    return true;
    case MODEL_LAVA_GIANT:
    case MODEL_BURNING_LAVA_GIANT:
    {
        if (MONSTER01_ATTACK1 == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK01);
        }
        else if (MONSTER01_ATTACK2 == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK02);
        }
        else if (MONSTER01_SHOCK == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE01);
        }
        else if (MONSTER01_WALK == o->CurrentAction)
        {
            if (rand_fps_check(20))
            {
                PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE01);
            }
        }
        else if (MONSTER01_DIE == o->CurrentAction)
        {
            PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_DEATH);
        }

        //SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE02,
        //SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE02,
    }
    return true;
    }

    return false;
}

void CGM_PK_Field::PlayObjectSound(OBJECT* o)
{
}

void CGM_PK_Field::PlayBGM()
{
    if (gMapManager.IsPKField())
    {
        PlayMp3(MUSIC_PKFIELD);
    }
    else
    {
        StopMp3(MUSIC_PKFIELD);
    }
}