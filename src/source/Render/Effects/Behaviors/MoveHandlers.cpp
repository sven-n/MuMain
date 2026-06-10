#include "stdafx.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "Render/Effects/ZzzEffect.h"
#include "Audio/DSPlaySound.h"
#include "UI/Legacy/UIManager.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Character/CharacterManager.h"
#include "UI/NewUI/NewUISystem.h"
#include "Engine/Object/ZzzInterface.h"
#include "MoveHandlers.h"


namespace Render::Effects::Behaviors
{
    // MODEL_ARROW_AUTOLOAD
    bool Move_MODEL_ARROW_AUTOLOAD(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            OBJECT* pOwn = o->Owner;
            if ((int)o->LifeTime % 10 == 0)
            {
                CreateEffectFpsChecked(MODEL_ARROW_AUTOLOAD, pOwn->Position, pOwn->Angle, pOwn->Light, 1, pOwn);
            }
        }
        else if (o->SubType == 1)
        {
            vec3_t tmp = { -10.f, 5.f, 10.f };

            if (o->LifeTime < 20)
            {
                float flumi = (float)(o->LifeTime - 20) / 20.f;
                VectorScale(o->Direction, flumi, o->Light);
            }

            OBJECT* pOwner = o->Owner;
            VectorTransform(tmp, pOwner->BoneTransform[47], o->Position);
            VectorScale(o->Position, pOwner->Scale, o->Position);
            VectorAddScaled(o->Position, pOwner->Position, o->Position, FPS_ANIMATION_FACTOR);
            VectorCopy(pOwner->Angle, o->Angle);
            o->Angle[0] += 85.f; o->Angle[1] += -17.f; o->Angle[2] += (20.f) * FPS_ANIMATION_FACTOR;
        }
    }
        return true;
    }

    // MODEL_INFINITY_ARROW
    bool Move_MODEL_INFINITY_ARROW(OBJECT* o, int index, float Luminosity)
    {
    {
        vec3_t tmp = { 0.f, 0.f, 0.f };
        OBJECT* pOwner = o->Owner;
        if (o->SubType == 1)
        {
            o->Light[0] *= pow(0.95f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.95f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.95f, FPS_ANIMATION_FACTOR);
        }
        VectorTransform(tmp, pOwner->BoneTransform[29], o->Position);
        VectorScale(o->Position, pOwner->Scale, o->Position);
        VectorAddScaled(o->Position, pOwner->Position, o->Position, FPS_ANIMATION_FACTOR);
        VectorCopy(pOwner->Angle, o->Angle);
    }
        return true;
    }

    // MODEL_INFINITY_ARROW1, MODEL_INFINITY_ARROW2, MODEL_INFINITY_ARROW3
    bool Move_MODEL_INFINITY_ARROW1(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            o->Light[0] *= pow(0.95f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.95f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.95f, FPS_ANIMATION_FACTOR);
            VectorCopy(o->Owner->Position, o->Position);
        }
        else if (o->SubType == 1)
        {
            o->Light[0] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            if ((int)o->LifeTime == 40)
                CreateEffectFpsChecked(MODEL_INFINITY_ARROW3, o->Position, o->Angle, o->Light, 2, o);
            else
                if ((int)o->LifeTime == 20)
                    CreateEffectFpsChecked(MODEL_INFINITY_ARROW3, o->Position, o->Angle, o->Light, 3, o);
            VectorCopy(o->Owner->Position, o->Position);
        }
        else if (o->SubType == 2)
        {
            o->Light[0] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            VectorCopy(o->Owner->Position, o->Position);
        }
        else if (o->SubType == 3)
        {
            o->Light[0] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.98f, FPS_ANIMATION_FACTOR);
            VectorCopy(o->Owner->Position, o->Position);
        }
    }
        return true;
    }

    // MODEL_SHIELD_CRASH
    bool Move_MODEL_SHIELD_CRASH(OBJECT* o, int index, float Luminosity)
    {
    {
        float ftmp = 0.f;
        vec3_t nPos, nLight;

        if (o->LifeTime >= 0 && o->LifeTime < 8)
        {
            ftmp = (float)o->LifeTime / 8;
            VectorScale(o->Direction, ftmp, o->Light);
        }

        if (o->LifeTime >= 8 && o->LifeTime < 24)
        {
            ftmp = 1.f - ((float)(o->LifeTime - 24) / 16);
            VectorScale(o->Direction, ftmp, o->Light);
        }

        if ((int)o->LifeTime == 23)
        {
            if (o->SubType == 1)
            {
                Vector(0.3f, 0.3f, 0.8f, nLight);
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Owner->Position, o->Owner->Angle,
                    nLight, 9, o->Owner, -1, 0, 0, 0);
            }
            else if (o->SubType == 2)
            {
                vec3_t vShockColor = { 0.8f, 0.3f, 0.3f };
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Owner->Position, o->Owner->Angle,
                    vShockColor, 9, o->Owner, -1, 0, 0, 0);
            }
            else
            {
                vec3_t vShockColor = { 1.f, 1.f, 1.f };
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Owner->Position, o->Owner->Angle,
                    vShockColor, 9, o->Owner, -1, 0, 0, 0);
            }
        }

        VectorCopy(o->Owner->Position, o->Position);
        VectorCopy(o->Owner->Position, nPos);

        if (o->SubType == 1)
        {
            Vector(0.f, 1.f, 5.f, nLight);
            CreateEffectFpsChecked(MODEL_SKILL_INFERNO, nPos, o->Angle, nLight, 10, o, 30, 0);
            nPos[2] += 120.f;
            CreateParticleFpsChecked(BITMAP_SMOKE, nPos, o->Angle, o->Light, 11, 2.f);
        }
        else if (o->SubType == 2)
        {
            Vector(0.8f, 0.3f, 0.3f, nLight);
            CreateEffectFpsChecked(MODEL_SKILL_INFERNO, nPos, o->Angle, nLight, 10, o, 30, 0);
        }
        else if (o->SubType == 0)
        {
            Vector(0.0f, 0.8f, 1.5f, nLight);
            CreateEffectFpsChecked(MODEL_SKILL_INFERNO, nPos, o->Angle, nLight, 2, o, 30, 0);
            nPos[2] += 120.f;
            CreateParticleFpsChecked(BITMAP_SMOKE, nPos, o->Angle, o->Light, 11, 2.f);
        }
    }
        return true;
    }

    // MODEL_SHIELD_CRASH2
    bool Move_MODEL_SHIELD_CRASH2(OBJECT* o, int index, float Luminosity)
    {
    {
        float ftmp = 0.f;
        if (o->LifeTime >= 0 && o->LifeTime < 8)
        {
            ftmp = (float)o->LifeTime / 8;
            VectorScale(o->Direction, ftmp, o->Light);
        }

        if (o->LifeTime >= 8 && o->LifeTime < 24)
        {
            ftmp = 1.f - ((float)(o->LifeTime - 24) / 16);
            VectorScale(o->Direction, ftmp, o->Light);
        }

        VectorCopy(o->Owner->Position, o->Position);
    }
        return true;
    }

    // MODEL_IRON_RIDER_ARROW
    bool Move_MODEL_IRON_RIDER_ARROW(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Scale *= pow(1.05f, FPS_ANIMATION_FACTOR);
        if (o->LifeTime < 5)
        {
            o->Light[0] *= pow(0.7f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.7f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.7f, FPS_ANIMATION_FACTOR);
        }
        vec3_t vPos;
        VectorScale(o->Direction, o->Velocity, vPos);
        VectorAddScaled(o->Position, vPos, o->Position, FPS_ANIMATION_FACTOR);
        CreateSprite(BITMAP_LIGHT + 3, o->Position, 4.0f, o->Light, o);
        CreateSprite(BITMAP_DS_EFFECT, o->Position, 2.5f, o->Light, o);
        CreateParticleFpsChecked(BITMAP_SPARK + 1, o->Position, o->Angle, o->Light, 10, 3.0f);

        VectorCopy(o->Position, o->EyeLeft);
        CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 3, NULL, 0);
    }
        return true;
    }

    // MODEL_MULTI_SHOT3
    bool Move_MODEL_MULTI_SHOT3(OBJECT* o, int index, float Luminosity)
    {
        o->Scale += (0.25f) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = (float)o->LifeTime / 18.f;
        o->Alpha = o->BlendMeshLight;
        return true;
    }

    // MODEL_MULTI_SHOT1
    bool Move_MODEL_MULTI_SHOT1(OBJECT* o, int index, float Luminosity)
    {
        o->Scale += (0.2f) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = (float)o->LifeTime / 18.f;
        o->Alpha = o->BlendMeshLight;
        return true;
    }

    // MODEL_MULTI_SHOT2
    bool Move_MODEL_MULTI_SHOT2(OBJECT* o, int index, float Luminosity)
    {
        o->Scale += (0.3f) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = (float)o->LifeTime / 18.f;
        o->Alpha = o->BlendMeshLight;
        return true;
    }

    // MODEL_KENTAUROS_ARROW
    bool Move_MODEL_KENTAUROS_ARROW(OBJECT* o, int index, float Luminosity)
    {
    {
        if ((int)o->LifeTime == 24)
        {
            vec3_t vDir;
            vec34_t vMat;
            Vector(0.f, -1.f, 0.f, vDir);
            AngleMatrix(o->Angle, vMat);
            VectorRotate(vDir, vMat, o->Direction);
            CreateJointFpsChecked(BITMAP_FLARE + 1, o->Position, o->Position, o->Angle, 17, o, 15.f, 40);
        }
        if (o->LifeTime <= 24)
        {
            o->Scale *= pow(1.05f, FPS_ANIMATION_FACTOR);
            vec3_t vLight = { 0.3f, 0.5f, 1.f };
            vec3_t vPos;
            VectorScale(o->Direction, o->Velocity, vPos);
            VectorAddScaled(o->Position, vPos, o->Position, FPS_ANIMATION_FACTOR);
            //			CreateParticle(BITMAP_FLAME, o->Position, o->Angle, vLight, 8, 3.5f);
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 26, 0.2f);
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 26, 0.2f);
            //			CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 9, o, 10.f, 40 );
        }
        //			else
        o->Alpha += (0.001f) * FPS_ANIMATION_FACTOR;
    }
        return true;
    }

    // MODEL_WARP3, MODEL_WARP2, MODEL_WARP, MODEL_WARP6, MODEL_WARP5, MODEL_WARP4
    bool Move_MODEL_WARP3(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            float fTemp1 = sinf(WorldTime * 0.0011f) * 0.2f;
            float fTemp2 = sinf(WorldTime * 0.0017f) * 0.2f;
            float fTemp3 = sinf(WorldTime * 0.0013f) * 0.2f;
            Vector(fTemp1 + 0.01f, fTemp2 + 0.01f, fTemp3 + 0.01f, o->Light);
            o->Angle[1] += (4.0f + o->Gravity) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 1)
        {
            float fTemp1 = sinf(WorldTime * 0.0011f) * 0.05f;
            Vector(0.0f + fTemp1, 0.2f + fTemp1, 0.1f + fTemp1, o->Light);
            o->Angle[1] += (2.0f + o->Gravity) * FPS_ANIMATION_FACTOR;
        }
    }
        return true;
    }

    // MODEL_GHOST
    bool Move_MODEL_GHOST(OBJECT* o, int index, float Luminosity)
    {
    {
        if (rand() % 40 == 1)
        {
            o->Velocity = o->Velocity * -1.0f;
            o->Gravity = o->Velocity * ((float)(rand() % 3) + 5.0f);
        }
        if (rand() % 40 == 1)
        {
            o->Velocity = o->Velocity * -1.0f;
            o->Distance = o->Velocity * ((float)(rand() % 3) + 1.0f);
        }

        vec3_t vTemp;
        VectorSubtract(o->Position, o->StartPosition, vTemp);
        float fTemp = VectorLength(vTemp);

        if (fTemp >= 500.f && o->Kind == 0)
        {
            o->LifeTime = 50;
            o->Kind = 1;
        }

        o->Angle[2] += (sinf((int)WorldTime % 10000 * 0.0001f) * o->Gravity) * FPS_ANIMATION_FACTOR;
        o->Angle[0] += (sinf((int)WorldTime % 10000 * 0.0001f) * o->Distance / 3.0f) * FPS_ANIMATION_FACTOR;

        Vector(o->BlendMeshLight / 10.f, o->BlendMeshLight / 10.f, o->BlendMeshLight / 4.f, o->Light);
        CreateParticleFpsChecked(BITMAP_LIGHT, o->Position, o->Angle, o->Light, 9, o->Scale);
    }
        return true;
    }

    // MODEL_TREE_ATTACK
    bool Move_MODEL_TREE_ATTACK(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
    {
        Vector(1.0f, 1.0f, 1.0f, Light);
        o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
        if (o->Scale >= 1.0f)
        {
            o->Alpha -= (0.07f) * FPS_ANIMATION_FACTOR;
            o->Scale = 1.0f;
        }
    }
        return true;
    }

    // MODEL_BUTTERFLY01
    bool Move_MODEL_BUTTERFLY01(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Height;
    {
        if (o->Kind > 0)
            o->Angle[2] += (rand() % 10) * FPS_ANIMATION_FACTOR;
        else
            o->Angle[2] -= (rand() % 10) * FPS_ANIMATION_FACTOR;

        if (rand_fps_check(32))
        {
            o->Direction[2] = (float)(rand() % 15 - 7) * 1.f;
        }
        o->Direction[2] += ((float)(rand() % 15 - 7) * 0.2f) * FPS_ANIMATION_FACTOR;
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height + 50.f)
        {
            o->Direction[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->Direction[2] += (1.f) * FPS_ANIMATION_FACTOR;
        }
        if (o->Position[2] > Height + 150.f)
        {
            o->Direction[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->Direction[2] -= (1.f) * FPS_ANIMATION_FACTOR;
        }
        o->Position[2] += ((float)(rand() % 15 - 7) * 0.3f) * FPS_ANIMATION_FACTOR;

        float  Luminosity = (float)(rand() % 32 + 64) * 0.01f;
        if (o->SubType == 0) {
            Vector(Luminosity * 0.4f, Luminosity * 0.8f, Luminosity * 0.6f, Light);
        }
        else if (o->SubType == 1) {
            Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 0.8f, Light);
        }
        else if (o->SubType == 2) {
            Vector(Luminosity * 0.6f, Luminosity * 0.8f, Luminosity * 0.4f, Light);
        }
        else if (o->SubType == 3)
        {
            Vector(Luminosity * 0.7f, Luminosity * 0.9f, Luminosity * 0.5f, Light);
            if (rand_fps_check(2))
            {
                vec3_t Position;
                Vector((float)(rand() % 16 - 8), (float)(rand() % 16 - 8), (float)(rand() % 16 - 8), Position);
                VectorAdd(Position, o->Position, Position);
                CreateParticleFpsChecked(BITMAP_SPARK, Position, o->Angle, o->Light, 7);
            }
        }
        CreateSprite(BITMAP_LIGHT, o->Position, 1.f, Light, o);
    }
        return true;
    }

    // MODEL__SPEAR
    bool Move_MODEL__SPEAR(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
    {
        vec3_t Angle, p, Position;
        Vector(0.f, 0.f, 0.f, Position);
        float Matrix[3][4];
        Vector(0.f, -100.f, 0.f, p);
        for (int j = 0; j < 3; j++)
        {
            Vector((float)(rand() % 90), 0.f, (float)(rand() % 360), Angle);
            AngleMatrix(Angle, Matrix);
            VectorRotate(p, Matrix, Position);
            VectorSubtract(o->Position, Position, Position);
            //Position[2] += 30.f;
            CreateJointFpsChecked(BITMAP_JOINT_HEALING, Position, o->Position, Angle, 6, NULL, 5.f);
        }
    }
        return true;
    }

    // MODEL_HALLOWEEN_CANDY_BLUE, MODEL_HALLOWEEN_CANDY_ORANGE, MODEL_HALLOWEEN_CANDY_YELLOW, MODEL_HALLOWEEN_CANDY_RED, MODEL_HALLOWEEN_CANDY_HOBAK, MODEL_HALLOWEEN_CANDY_STAR
    bool Move_MODEL_HALLOWEEN_CANDY_BLUE(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        if (o->SubType == 0 || o->SubType == 1)
        {
            o->Angle[o->m_iAnimation] += (20.f) * FPS_ANIMATION_FACTOR;
            o->Position[2] += ((o->Gravity * 0.5f)) * FPS_ANIMATION_FACTOR;
            o->Gravity -= (1.5f) * FPS_ANIMATION_FACTOR;

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                o->Position[2] = Height;
                o->Gravity = -o->Gravity * 0.3f;
                o->LifeTime -= 2 * FPS_ANIMATION_FACTOR;
            }

            VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);

            if (o->Type == MODEL_HALLOWEEN_CANDY_HOBAK)
            {
                if (rand_fps_check(3))
                {
                    CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, o->Light, 5, 0.4f + rand() % 10 * 0.01f);
                }
            }
        }
        return true;
    }

    // MODEL_HALLOWEEN_EX
    bool Move_MODEL_HALLOWEEN_EX(OBJECT* o, int index, float Luminosity)
    {
        return true;
    }

    // MODEL_XMAS_EVENT_BOX, MODEL_XMAS_EVENT_CANDY, MODEL_XMAS_EVENT_TREE, MODEL_XMAS_EVENT_SOCKS
    bool Move_MODEL_XMAS_EVENT_BOX(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        o->Angle[o->m_iAnimation] += (20.f) * FPS_ANIMATION_FACTOR;
        o->Position[2] += ((o->Gravity * 0.5f)) * FPS_ANIMATION_FACTOR;
        o->Gravity -= (1.5f) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height + 3.f)
        {
            o->Position[2] = Height + 3.f;
            o->Gravity = -o->Gravity * 0.3f;
            o->LifeTime -= 2 * FPS_ANIMATION_FACTOR;
        }

        VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        return true;
    }

    // MODEL_XMAS_EVENT_ICEHEART
    bool Move_MODEL_XMAS_EVENT_ICEHEART(OBJECT* o, int index, float Luminosity)
    {
        o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
        if (o->Owner != NULL)
        {
            if (o->Owner->CurrentAction != PLAYER_SANTA_2)
            {
                o->Live = false;
            }
        }
        return true;
    }

    // MODEL_NEWYEARSDAY_EVENT_BEKSULKI, MODEL_NEWYEARSDAY_EVENT_CANDY, MODEL_NEWYEARSDAY_EVENT_MONEY, MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN, MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED, MODEL_NEWYEARSDAY_EVENT_PIG, MODEL_NEWYEARSDAY_EVENT_YUT
    bool Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        o->Angle[o->m_iAnimation] += (10.f) * FPS_ANIMATION_FACTOR;
        o->Position[0] += ((o->Direction[0] * 1.2f)) * FPS_ANIMATION_FACTOR;
        o->Position[1] += ((o->Direction[1] * 1.2f)) * FPS_ANIMATION_FACTOR;
        o->Position[2] += ((o->Gravity * 1.5f)) * FPS_ANIMATION_FACTOR;
        o->Gravity -= (1.5f) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.3f;
            o->LifeTime -= (2) * FPS_ANIMATION_FACTOR;
        }

        VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        return true;
    }

    // MODEL_MOONHARVEST_MOON
    bool Move_MODEL_MOONHARVEST_MOON(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Angle[2] += (5.0f) * FPS_ANIMATION_FACTOR;
            if (o->LifeTime < 25)
            {
                o->Light[0] -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[1] -= (0.06f) * FPS_ANIMATION_FACTOR;
                o->Light[2] -= (0.05f) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 1)
        {
            VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
            CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, o->Position, o->Angle, o->Light, 0, 1.0f);
            //CreateParticle(BITMAP_SMOKE, o->Position, o->Angle,o->Light, 23, 1.0f);
            //CreateParticle(BITMAP_SMOKE, o->Position, o->Angle,o->Light, 8, 1.0f);
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 11, 1.0f);
            if (rand_fps_check(2))
            {
                CreateParticleFpsChecked(BITMAP_WATERFALL_3, o->Position, o->Angle, o->Light, 3, 3.f);
            }
        }
        else if (o->SubType == 2)
        {
            o->Scale += (0.01f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_MOONHARVEST_GAM, MODEL_MOONHARVEST_SONGPUEN1, MODEL_MOONHARVEST_SONGPUEN2
    bool Move_MODEL_MOONHARVEST_GAM(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        o->Angle[0] += (10.f) * FPS_ANIMATION_FACTOR;
        o->Position[0] += ((o->Direction[0] * 2.2f)) * FPS_ANIMATION_FACTOR;
        o->Position[1] += ((o->Direction[1] * 2.2f)) * FPS_ANIMATION_FACTOR;
        o->Position[2] += ((o->Gravity * 1.5f)) * FPS_ANIMATION_FACTOR;
        o->Gravity -= (1.5f) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.3f;
            o->LifeTime -= (2) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_SPEARSKILL
    bool Move_MODEL_SPEARSKILL(OBJECT* o, int index, float Luminosity)
    {
        return true;
    }

    // MODEL_SUMMONER_WRISTRING_EFFECT
    bool Move_MODEL_SUMMONER_WRISTRING_EFFECT(OBJECT* o, int index, float Luminosity)
    {
    {
        OBJECT* pObject = o->Owner;
        BMD* pModel = &Models[pObject->Type];
        //pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
        vec3_t vPos, vRelative;
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        pModel->TransformPosition(pObject->BoneTransform[37], vRelative, vPos, false);
        VectorScale(vPos, pModel->BodyScale, vPos);
        VectorAdd(vPos, pObject->Position, o->Position);

        if (pObject->Live)
            o->LifeTime = 100.f; //무한

        BMD* b = &Models[o->Type];
        b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 5.f, o->Position, o->Angle);
    }
        return true;
    }

    // MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT
    bool Move_MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            OBJECT* pObject = o->Owner;
            o->Position[0] = pObject->Position[0] + cosf(WorldTime * 0.003f + o->Skill * 0.024f) * 60.0f;
            o->Position[1] = pObject->Position[1] + sinf(WorldTime * 0.003f + o->Skill * 0.024f) * 60.0f;
            o->Position[2] = pObject->Position[2] + (sinf(WorldTime * 0.0010f + o->Skill * 0.024f) + 2.0f) * 80.0f - 60.0f;

            if (o->StartPosition[0] != o->Position[0] - pObject->Position[0])
            {
                float fAngle = CreateAngle(o->StartPosition[0], o->StartPosition[1],
                    o->Position[0] - pObject->Position[0], o->Position[1] - pObject->Position[1]);
                o->Angle[2] = fAngle + 0;
            }
            VectorSubtract(o->Position, pObject->Position, o->StartPosition);

            if (o->Kind == 1)
            {
                if (o->Alpha > 0.0f) o->Alpha -= 0.03f;
                else DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, o->Owner);
            }
            else
            {
                if (Hero->SafeZone/* || rand_fps_check(100)*/ || sinf(WorldTime * 0.0004f + o->Skill * 0.024f) < 0.3f)
                    o->Kind = 1;
                if (o->Alpha < 1.0f) o->Alpha += 0.03f;
            }

            if (pObject->Live)
                o->LifeTime = 100.f;

            if (rand_fps_check(1))
            {
                vec3_t vLight;
                Vector(o->Alpha * 0.3f, o->Alpha * 0.3f, o->Alpha * 0.3f, vLight);
                CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, vLight, 1, 1, pObject);
            }
        }
        else if (o->SubType == 1)
        {
            if (rand_fps_check(2))
            {
                OBJECT* pObject = o->Owner;
                if (pObject->Live == false)
                {
                    o->Live = false;
                    return true;
                }
                BMD* pModel = &Models[pObject->Type];
                int iNumBones = pModel->NumBones;
                CreateEffectFpsChecked(BITMAP_FIRE_CURSEDLICH, pObject->Position, pObject->Angle, pObject->Light, 0, pObject, -1, rand() % iNumBones);
            }
        }
        return true;
    }

    // MODEL_SUMMONER_EQUIP_HEAD_NEIL
    bool Move_MODEL_SUMMONER_EQUIP_HEAD_NEIL(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            OBJECT* pObject = o->Owner;
            o->Position[0] = pObject->Position[0] + cosf(WorldTime * 0.003f + o->Skill * 0.024f) * 60.0f;
            o->Position[1] = pObject->Position[1] + sinf(WorldTime * 0.003f + o->Skill * 0.024f) * 60.0f;
            o->Position[2] = pObject->Position[2] + (sinf(WorldTime * 0.0010f + o->Skill * 0.024f) + 2.0f) * 80.0f - 60.0f;

            if (o->StartPosition[0] != o->Position[0] - pObject->Position[0])
            {
                float fAngle = CreateAngle(o->StartPosition[0], o->StartPosition[1],
                    o->Position[0] - pObject->Position[0], o->Position[1] - pObject->Position[1]);
                o->Angle[2] = fAngle + 0;
            }
            VectorSubtract(o->Position, pObject->Position, o->StartPosition);

            if (o->Kind == 1)
            {
                if (o->Alpha > 0.0f) o->Alpha -= 0.03f;
                else DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, o->Owner);
            }
            else
            {
                if (Hero->SafeZone/* || rand_fps_check(100)*/ || sinf(WorldTime * 0.0004f + o->Skill * 0.024f) < 0.3f)
                    o->Kind = 1;
                if (o->Alpha < 1.0f) o->Alpha += 0.03f;
            }

            if (pObject->Live)
                o->LifeTime = 100.f;

            vec3_t vLight;
            Vector(o->Alpha, o->Alpha, o->Alpha, vLight);
            if (rand_fps_check(1))
            {
                CreateParticleFpsChecked(BITMAP_LIGHT + 2, o->Position, o->Angle, o->Light, 3, 0.30f, pObject);
            }

            if (rand_fps_check(2))
            {
                CreateParticleFpsChecked(BITMAP_LIGHT + 2, o->Position, o->Angle, o->Light, 3, 0.30f, pObject);
            }
            //DeleteJoint(MODEL_SPEARSKILL, o, 15);
        }
        else if (o->SubType == 1)
        {
            OBJECT* pObject = o->Owner;
            if (pObject->Live == false)
            {
                o->Live = false;
                return true;
            }
            vec3_t vColor;
            Vector(1.0f, 1.0f, 1.0f, vColor);
            CreateParticleFpsChecked(BITMAP_LIGHT + 2, pObject->Position, pObject->Angle, vColor, 5, 0.1f, pObject);
        }
        return true;
    }

    // MODEL_SUMMONER_SUMMON_SAHAMUTT
    bool Move_MODEL_SUMMONER_SUMMON_SAHAMUTT(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        float Matrix[3][4];
    {
        vec3_t vTempPosition;
        VectorCopy(o->Position, vTempPosition);

        if (o->LifeTime < 20)
        {
            SetAction(o, 0);
            o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->CurrentAction == 0 && o->Alpha < 0.3f)
        {
            o->Alpha += (0.05f) * FPS_ANIMATION_FACTOR;
            {
                o->Angle[2] = CreateAngle2D(o->Position, o->HeadTargetAngle);
                float dx = o->HeadTargetAngle[0] - o->Position[0];
                float dy = o->HeadTargetAngle[1] - o->Position[1];
                o->Distance = sqrtf(dx * dx + dy * dy);
            }
        }
        else
        {
            SetAction(o, 1);
            if (o->AnimationFrame >= 12.0f)
            {
                o->AnimationFrame = 12.0f;
            }

            if (o->AnimationFrame >= 11.0f)
            {
                if (o->Alpha > 0)
                    o->Alpha -= (0.3f) * FPS_ANIMATION_FACTOR;
                else
                    o->Alpha = 0;
            }
            else if (o->AnimationFrame < 3.0f && o->Alpha < 0.7f)
            {
                o->Alpha += (0.05f) * FPS_ANIMATION_FACTOR;
            }

            if (o->AnimationFrame > 4.0f && o->AnimationFrame < 12.0f)
            {
                AngleMatrix(o->Angle, Matrix);
                vec3_t vMoveDir, Position;
                if (o->AnimationFrame < 10.0f)
                {
                    Vector(0, o->Distance / -13.0f, 0, vMoveDir);
                }
                else
                {
                    Vector(0, o->Distance / -45.0f, 0, vMoveDir);
                }
                VectorRotate(vMoveDir, Matrix, Position);
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
            }
            if (o->AnimationFrame <= 4.0f)
            {
                o->Angle[2] = CreateAngle2D(o->Position, o->HeadTargetAngle);
                float dx = o->HeadTargetAngle[0] - o->Position[0];
                float dy = o->HeadTargetAngle[1] - o->Position[1];
                o->Distance = sqrtf(dx * dx + dy * dy);
            }
        }
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);

        VectorSubtract(vTempPosition, o->Position, vTempPosition);

        if (o->AnimationFrame >= 11.0f)
        {
            CreateBomb3(o->Position, o->SubType);
            if (rand_fps_check(2))
                PlayBuffer(SOUND_SUMMON_EXPLOSION);
        }

        BMD* pModel = &Models[o->Type];
        pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
        vec3_t vPos, vRelative, vLight;
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        int iPositions[] = { 13, 23, 39, 49, 3, 4, 5, 61 };
        for (int i = 0; i < 8; ++i)
        {
            pModel->TransformPosition(BoneTransform[iPositions[i]], vRelative, vPos, false);
            VectorAdd(vPos, vTempPosition, vPos);
            Vector(o->Alpha * 0.3f, o->Alpha * 0.3f, o->Alpha * 0.3f, vLight);
            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 2, 5, o);
            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 2, 4, o);
            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 2, 3, o);
        }
    }
        return true;
    }

    // MODEL_SUMMONER_SUMMON_NEIL
    bool Move_MODEL_SUMMONER_SUMMON_NEIL(OBJECT* o, int index, float Luminosity)
    {
        float Matrix[3][4];
    {
        if (o->LifeTime < 20) o->Alpha -= 0.05f;
        else if (o->Alpha < 0.7f) o->Alpha += 0.04f;

        if (o->AnimationFrame > 8 && o->Skill == 0)
        {
            o->Skill = 1;
            CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_NIFE1, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
            if (o->SubType >= 1)
                CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_NIFE2, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
            if (o->SubType >= 2)
                CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_NIFE3, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
        }
        if (o->AnimationFrame > 10 && o->Skill == 1)
        {
            o->Skill = 2;

            AngleMatrix(o->Angle, Matrix);
            vec3_t vMoveDir, vPosition;
            Vector(0, -60.0f, 0, vMoveDir);
            VectorRotate(vMoveDir, Matrix, vPosition);
            VectorAdd(o->Position, vPosition, vPosition);
            CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, vPosition, o->Angle, o->Light, o->SubType, o);

            CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
            if (o->SubType >= 1)
                CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_GROUND2, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
            if (o->SubType >= 2)
                CreateEffectFpsChecked(MODEL_SUMMONER_SUMMON_NEIL_GROUND3, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);

            PlayBuffer(SOUND_SUMMON_REQUIEM);
        }
        // 			if (o->AnimationFrame > 11 && o->AnimationFrame < 12)
        // 				EarthQuake = (float)(-rand()%2-5)*0.1f;
    }
        return true;
    }

    // MODEL_SUMMONER_SUMMON_LAGUL
    bool Move_MODEL_SUMMONER_SUMMON_LAGUL(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            int anEffectVolume[3] = { 5, 4, 3 };
            if (rand() % anEffectVolume[(int)o->PKKey] == 0)
            {
                vec3_t vPos, vLight;

                VectorCopy(o->HeadTargetAngle, vPos);
                vPos[0] += (float)(rand() % 500 - 250);
                vPos[1] += (float)(rand() % 500 - 250);
                Vector(1.0f, 1.0f, 1.0f, vLight);
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 57, 3.5f);

                VectorCopy(o->HeadTargetAngle, vPos);
                vPos[0] += (float)(rand() % 400 - 200);
                vPos[1] += (float)(rand() % 400 - 200);
                Vector(0.6f, 0.1f, 1.f, vLight);
                CreateEffectFpsChecked(BITMAP_CLOUD, vPos, o->Angle, vLight, 0, NULL, -1, 0, 0, 0, 2.0f);

                Vector(0.6f, 0.1f, 1.f, vLight);
                CreateParticleFpsChecked(BITMAP_TWINTAIL_WATER, vPos, o->Angle, vLight, 1);
            }
        }
        else if (o->SubType == 1)
        {
            SetAction(o, 0);

            if (o->LifeTime < 30)
                o->Alpha = o->LifeTime / 40.f;
            else if (o->LifeTime > 144)
                o->Alpha = (160 - o->LifeTime) / 20.f;
        }
    }
        return true;
    }

    // BITMAP_MAGIC
    bool Move_BITMAP_MAGIC(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            CreateEffectFpsChecked(BITMAP_MAGIC, o->Position, o->Angle, o->Light, 1);
            if (o->LifeTime > 5 && o->LifeTime < 10)
            {
                CreateParticleFpsChecked(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
            }
        }
        else if (o->SubType == 2 || o->SubType == 3 || o->SubType == 7)
        {
            if (o->LifeTime > 5 && o->LifeTime < 10)
            {
                if (o->SubType == 3)
                    CreateParticleFpsChecked(BITMAP_FLARE, o->Position, o->Angle, o->Light, 10, 0.19f, o);
                else
                    CreateParticleFpsChecked(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
            }
        }
        else if (o->SubType == 4)
        {
            CreateParticleFpsChecked(BITMAP_FLARE, o->Position, o->Angle, o->Light, 12, 0.19f, o);
        }
        else if (o->SubType == 8)
        {
            o->Scale += (1.8f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 9)
        {
            if (o->LifeTime < 20) o->Alpha -= 0.05f;
            else if (o->Alpha < 1.0f) o->Alpha += 0.05f;

            o->HeadAngle[0] += (4.0f) * FPS_ANIMATION_FACTOR;
            o->HeadAngle[1] -= (8.0f) * FPS_ANIMATION_FACTOR;
            o->HeadAngle[2] += (4.0f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 10)
        {
            if (o->LifeTime < 20) o->Alpha -= 0.03f;
            else if (o->Alpha < 1.0f) o->Alpha += 0.05f;
        }
        else if (o->SubType == 11)
        {
            o->HeadAngle[0] += (2.0f) * FPS_ANIMATION_FACTOR;
            o->HeadAngle[1] -= (2.0f) * FPS_ANIMATION_FACTOR;
            o->HeadAngle[2] += (2.0f) * FPS_ANIMATION_FACTOR;

            if (o->LifeTime <= 10)
            {
                o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[0] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 12)
        {
            if (o->Alpha > 0.0f)
            {
                o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Alpha = 0.0f;
            }

            o->Scale += (o->Alpha * 1.0f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 5.0f)
            {
                //o->Scale = 5.0f;
                o->Light[0] *= pow(0.5f, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(0.5f, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(0.5f, FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 13)
        {
            o->Scale *= pow(1.1f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.95f, o->Light);
            if (o->Scale > 8)
            {
                o->Live = false;
            }
            if (o->Scale > 4)
            {
                VectorScale(o->Light, 0.5f, o->Light);
            }
        }
        else if (o->SubType == 14)
        {
            o->Scale *= pow(1.1f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.95f, o->Light);
            if (o->Scale > 8)
            {
                o->Live = false;
            }
            if (o->Scale > 4)
            {
                VectorScale(o->Light, 0.5f, o->Light);
            }
        }
        return true;
    }

    // BITMAP_OUR_INFLUENCE_GROUND, BITMAP_ENEMY_INFLUENCE_GROUND
    bool Move_BITMAP_OUR_INFLUENCE_GROUND(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            if (o->Owner == NULL || o->Owner->Live == false)
                o->Live = false;

            VectorCopy(o->Owner->Position, o->Position);

            o->Alpha -= (0.02f) * FPS_ANIMATION_FACTOR;
            o->Scale += (0.01f) * FPS_ANIMATION_FACTOR;

            if (o->Alpha < 0.f)
            {
                o->Alpha = 1.0f;
                o->Scale = 0.6f;
            }

            if (o->LifeTime < 25)
                o->AlphaTarget -= (0.02f) * FPS_ANIMATION_FACTOR;
            else if (o->AlphaTarget < 1.0f)
                o->AlphaTarget += (0.02f) * FPS_ANIMATION_FACTOR;

            if (1 <= o->LifeTime)
                o->LifeTime = 50;
        }
        return true;
    }

    // BITMAP_ORORA
    bool Move_BITMAP_ORORA(OBJECT* o, int index, float Luminosity)
    {
        if (o->Owner == NULL || o->Owner->Live == false)
            o->Live = false;
        else
        {
            if (o->LifeTime <= 5)
            {
                o->Live = false;
                CreateEffectFpsChecked(o->Type, o->Position, o->Angle, o->Light, o->SubType, o->Owner);
            }
        }
        return true;
    }

    // BITMAP_JOINT_THUNDER
    bool Move_BITMAP_JOINT_THUNDER(OBJECT* o, int index, float Luminosity)
    {
    {
        float fScale = (rand() % 400 + 100) / 10.0f;
        vec3_t vPosition, vStartPosition;
        VectorCopy(o->StartPosition, vStartPosition);
        VectorCopy(o->Position, vPosition);
        vStartPosition[0] += rand() % 100 - 50;
        vStartPosition[1] += rand() % 100 - 50;
        vPosition[0] += rand() % 100 - 50;
        vPosition[1] += rand() % 100 - 50;

        if (rand_fps_check(2))
        {
            if (o->SubType == 1)
            {
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, vStartPosition, vPosition, o->Angle, 33, NULL, fScale);
            }
            else
            {
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, vStartPosition, vPosition, o->Angle, 16, NULL, fScale);
            }
        }

        vec3_t vLight = { 0.45f, 0.45f, 0.7f };

        if (o->LifeTime > 10)
        {
            if (rand_fps_check(2))
                CreateEffectFpsChecked(BITMAP_MAGIC + 1, vPosition, o->Angle, vLight, 11, o);
        }

        if (rand_fps_check(4))
            CreateParticleFpsChecked(BITMAP_SMOKE, vPosition, o->Angle, vLight, 54, 2.8f);

        if (rand_fps_check(4))
        {
            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, vLight, 13);
        }

        if (o->LifeTime > 5)
        {
            Vector(0.15f, 0.15f, 0.4f, vLight);
            if (rand_fps_check(5))
                CreateEffectFpsChecked(BITMAP_CHROME_ENERGY2, vPosition, o->Angle, vLight, 0);
        }

        if (o->Owner != NULL && rand_fps_check(5) && o->Owner->BoneTransform != NULL)
        {
            BMD* pTargetModel = &Models[o->Owner->Type];
            int iNumBones = pTargetModel->NumBones;
            float fRandom;
            vec3_t vLight, vRelativePos, vPos, vAngle;
            Vector(0.0f, 0.0f, 0.0f, vRelativePos);
            for (int i = 0; i < iNumBones; ++i)
            {
                if (iNumBones > 100 && rand() % iNumBones > iNumBones / 10) continue;
                else if (iNumBones > 50 && rand() % iNumBones > iNumBones / 5) continue;
                else if (iNumBones > 20 && rand() % iNumBones > iNumBones / 2) continue;
                VectorCopy(o->Owner->Position, pTargetModel->BodyOrigin);
                pTargetModel->TransformPosition(o->Owner->BoneTransform[i], vRelativePos, vPos, true);

                Vector(0.2f, 0.2f, 0.8f, vLight);
                fRandom = 3.0f + ((float)(rand() % 20 - 10) * 0.1f);
                CreateParticleFpsChecked(BITMAP_LIGHT, vPos, vAngle, vLight, 5, fRandom);
            }
        }
    }
        return true;
    }

    // BITMAP_IMPACT
    bool Move_BITMAP_IMPACT(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Matrix[3][4];
    {
        float ScaleBk = 0.f;
        if (o->Scale < 2.f)
        {
            o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
            ScaleBk = o->Scale;
        }
        else
        {
            if (o->Scale < 2.4f)
                o->Scale += (0.02f) * FPS_ANIMATION_FACTOR;
            else
                o->Scale = 2.f;

            if (o->Scale >= 2.2f)
                ScaleBk = 2.2f - (o->Scale - 2.2f);
            else
                ScaleBk = o->Scale;
        }
        vec3_t Light, P, dp;

        if (o->LifeTime <= 40)
        {
            o->Alpha -= (0.1f) * FPS_ANIMATION_FACTOR;
            o->BlendMeshLight -= (0.1f) * FPS_ANIMATION_FACTOR;
        }
        float Matrix[3][4];
        Vector(-10.f, -30.f, 0.f, P);
        Vector(o->Light[0] * o->Alpha, o->Light[1] * o->Alpha, o->Light[2] * o->Alpha, Light);
        AngleMatrix(o->Owner->Angle, Matrix);
        VectorRotate(P, Matrix, dp);
        VectorAdd(dp, o->Owner->Position, o->Position);
        o->Position[2] += (130) * FPS_ANIMATION_FACTOR;
        CreateSprite(BITMAP_IMPACT, o->Position, ScaleBk, Light, o);
        CreateSprite(BITMAP_SHINY + 1, o->Position, ScaleBk, Light, o, -WorldTime * 0.08f);
        if (o->Scale > 1.f)
            CreateSprite(BITMAP_ORORA, o->Position, ScaleBk, Light, o, -WorldTime * 0.08f);
        if (o->Scale > 2.f)
            CreateSprite(BITMAP_ORORA, o->Position, ScaleBk, Light, o, WorldTime * 0.08f);
    }
        return true;
    }

    // BITMAP_FLAME
    bool Move_BITMAP_FLAME(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
        if (o->SubType == 0)
        {
            for (int j = 0; j < 6; j++)
            {
                Vector((float)(rand() % 50 - 25), (float)(rand() % 50 - 25), 0.f, Position);
                VectorAdd(Position, o->Position, Position);
                CreateParticleFpsChecked(BITMAP_FLAME, Position, o->Angle, Light);
            }
            if (rand_fps_check(8))
            {
                CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light);
            }

            Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            if (o->Owner == &Hero->Object && (int)o->LifeTime % 20 == 0)
            {
                o->LifeTime = ((int)o->LifeTime / 20) * 19.9f;
                AttackCharacterRange(o->Skill, o->Position, 150.f, o->Weapon, o->PKKey);
            }
        }
        else if (o->SubType == 1 || o->SubType == 2)
        {
            for (int j = 0; j < 18; j++)
            {
                if (rand_fps_check(1))
                {
                    Vector(0.f, 250.f, 0.f, p);
                    Vector(0.f, 0.f, j * 20.f, Angle);
                    AngleMatrix(Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(Position, o->Position, Position);
                    Position[0] += rand() % 64 - 32;
                    Position[1] += rand() % 64 - 32;
                    if (o->SubType == 1)
                        CreateParticleFpsChecked(BITMAP_FLAME, Position, o->Angle, Light, 0, 1.2f);
                    else if (o->SubType == 2)
                        CreateParticleFpsChecked(BITMAP_FIRE + 3, Position, o->Angle, Light, 13, 2.5f);
                }
            }
        }
        else if (o->SubType == 3)
        {
            for (int j = 0; j < 3; j++)
            {
                if (rand_fps_check(1))
                {
                    CreateParticleFpsChecked(BITMAP_FLAME, o->Position, o->Angle, Light, 6);
                    Vector((float)(rand() % 10 - 5), (float)(rand() % 10 - 5), 40.f, Position);
                    VectorAdd(Position, o->Position, Position);
                    CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, o->Angle, Light, 0, 2.8f);
                }
            }
            Vector((float)(rand() % 10 - 5), (float)(rand() % 10 - 5), -40.f, Position);
            VectorAdd(Position, o->Position, Position);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 21, 0.8f);

            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light, 12);

            Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        }
        else if (o->SubType == 4)
        {
            o->Scale += (20.f) * FPS_ANIMATION_FACTOR;
            for (int j = 0; j < 18; j++)
            {
                if (rand_fps_check(1))
                {
                    Vector(0.f, 150.f - o->Scale, 0.f, p);
                    Vector(0.f, 0.f, j * 20.f, Angle);
                    AngleMatrix(Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(Position, o->Position, Position);
                    Position[0] += rand() % 64 - 32;
                    Position[1] += rand() % 64 - 32;
                    CreateParticleFpsChecked(BITMAP_FLAME, Position, o->Angle, Light, 0, 1.2f);
                }
            }
            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light, 12);
        }
        else if (o->SubType == 5)
        {
            Vector((float)(rand() % 32 - 16), (float)(rand() % 32 - 16), 0.f, Position);
            VectorAdd(Position, o->Position, Position);
            CreateParticleFpsChecked(BITMAP_FLAME, Position, o->Angle, Light, 0, o->Scale);
        }
        else if (o->SubType == 6)
        {
            Vector((float)(rand() % 32 - 16), (float)(rand() % 32 - 16), 0.f, Position);
            VectorAdd(Position, o->Position, Position);
            CreateParticleFpsChecked(BITMAP_FLAME, Position, o->Angle, o->Light, 12, o->Scale);
        }
        return true;
    }

    // MODEL_RAKLION_BOSS_CRACKEFFECT
    bool Move_MODEL_RAKLION_BOSS_CRACKEFFECT(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Alpha -= (0.03f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_RAKLION_BOSS_MAGIC
    bool Move_MODEL_RAKLION_BOSS_MAGIC(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Alpha -= (0.03f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // BITMAP_FIRE_HIK2_MONO
    bool Move_BITMAP_FIRE_HIK2_MONO(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
        if (o->SubType == 0)
        {
            for (int i = 0; i < 2; ++i)
            {
                Vector((float)(rand() % 30 - 15), (float)(rand() % 30 - 15), 0.f, p);
                VectorAdd(p, o->Position, Position);
                if (rand() % 3 != 0)
                {
                    CreateParticleFpsChecked(BITMAP_FLAME, Position, o->Angle, o->Light, 11, 1.4f);
                }

                if (rand_fps_check(8))
                {
                    CreateEffectFpsChecked(MODEL_ICE_SMALL, Position, o->Angle, o->Light, 0);
                }
            }
        }
        else if (o->SubType == 1)
        {
            for (int j = 0; j < 9; j++)
            {
                if (rand_fps_check(1))
                {
                    Vector(0.f, 250.f, 0.f, p);
                    Vector(0.f, 0.f, j * 40.f, Angle);
                    AngleMatrix(Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(Position, o->Position, Position);
                    Position[0] += rand() % 64 - 32;
                    Position[1] += rand() % 64 - 32;
                    CreateParticleFpsChecked(BITMAP_FIRE + 3, Position, o->Angle, Light, 13, 2.5f);
                }
            }
        }
        return true;
    }

    // BITMAP_CLOUD
    bool Move_BITMAP_CLOUD(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Light[0] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            o->Scale += (0.03f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_ALICE_BUFFSKILL_EFFECT, MODEL_ALICE_BUFFSKILL_EFFECT2
    bool Move_MODEL_ALICE_BUFFSKILL_EFFECT(OBJECT* o, int index, float Luminosity)
    {
        float Matrix[3][4];
    {
        if (o->SubType == 0 || o->SubType == 1 || o->SubType == 2)
        {
            if (o->Owner == NULL || o->Owner->Live == false)
            {
                o->Live = false;
            }

            VectorCopy(o->Owner->Position, o->Position);
            o->Position[2] += (100) * FPS_ANIMATION_FACTOR;
            if (o->LifeTime > 20)
            {
                o->Alpha += (0.05f) * FPS_ANIMATION_FACTOR;
                o->BlendMeshLight += (0.05f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->BlendMeshLight -= (0.05f) * FPS_ANIMATION_FACTOR;
                if (o->Alpha < 0.f)
                {
                    o->Live = false;
                }
            }

            if (o->Type == MODEL_ALICE_BUFFSKILL_EFFECT)
            {
                o->Angle[2] += (8.f) * FPS_ANIMATION_FACTOR;
            }
            else if (o->Type == MODEL_ALICE_BUFFSKILL_EFFECT2)
            {
                o->Angle[2] -= (8.f) * FPS_ANIMATION_FACTOR;
            }

            o->Scale += (0.035f) * FPS_ANIMATION_FACTOR;

            float fRot = (WorldTime * 0.0006f) * 360.0f;
            vec3_t vLight;

            // flare01
            if (o->SubType == 0)
            {
                Vector(0.8f * o->Alpha, 0.1f * o->Alpha, 0.9f * o->Alpha, vLight);
            }
            else if (o->SubType == 1)
            {
                Vector(1.0f * o->Alpha, 1.0f * o->Alpha, 1.0f * o->Alpha, vLight);
            }
            else if (o->SubType == 2)
            {
                Vector(0.8f * o->Alpha, 0.5f * o->Alpha, 0.2f * o->Alpha, vLight);
            }

            if (o->SubType == 0 || o->SubType == 2)
            {
                CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 0);
                CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 0);
            }
            else if (o->SubType == 1)
            {
                CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 1);
                CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 1);
            }

            // shiny04
            if (o->SubType == 0)
            {
                Vector(0.7f * o->Alpha, 0.6f * o->Alpha, 0.9f * o->Alpha, vLight);
            }
            else if (o->SubType == 1)
            {
                Vector(1.0f * o->Alpha, 1.0f * o->Alpha, 1.0f * o->Alpha, vLight);
            }
            else if (o->SubType == 2)
            {
                Vector(0.8f * o->Alpha, 0.5f * o->Alpha, 0.2f * o->Alpha, vLight);
            }

            if (o->SubType == 0 || o->SubType == 2)
            {
                CreateSprite(BITMAP_SHINY + 5, o->Position, 2.0f, vLight, o, fRot);
                CreateSprite(BITMAP_SHINY + 5, o->Position, 1.0f, vLight, o, -fRot);
            }
            else if (o->SubType == 1)
            {
                CreateSprite(BITMAP_SHINY + 5, o->Position, 2.0f, vLight, o, fRot, 1);
                CreateSprite(BITMAP_SHINY + 5, o->Position, 1.0f, vLight, o, -fRot, 1);
            }

            vec3_t vAngle, vPos, vWorldPos;
            float Matrix[3][4];
            Vector(0.f, -200.f, 0.f, vPos);
            for (int i = 0; i < 3; ++i)
            {
                Vector((float)(rand() % 90), 0.f, (float)(rand() % 360), vAngle);
                AngleMatrix(vAngle, Matrix);
                VectorRotate(vPos, Matrix, vWorldPos);
                VectorSubtract(o->Position, vWorldPos, vWorldPos);

                if (o->SubType == 0)
                {
                    Vector(0.7f, 0.5f, 0.7f, vLight);
                }
                else if (o->SubType == 1)
                {
                    Vector(1.0f, 1.0f, 1.0f, vLight);
                }
                else if (o->SubType == 2)
                {
                    Vector(0.8f, 0.5f, 0.2f, vLight);
                }

                if (o->SubType == 0 || o->SubType == 2)
                {
                    CreateJointFpsChecked(BITMAP_JOINT_HEALING, vWorldPos, o->Position, vAngle, 15, o, 5.f, 0, 0, 0, 0, vLight);
                }
                else if (o->SubType == 1)
                {
                    CreateJointFpsChecked(BITMAP_JOINT_HEALING, vWorldPos, o->Position, vAngle, 16, o, 5.f, 0, 0, 0, 0, vLight);
                }
            }
        }
        else if (o->SubType == 3 || o->SubType == 4)
        {
            if (o->Owner == NULL || o->Owner->Live == false)
            {
                o->Live = false;
            }
            else
            {
                o->LifeTime = 100;

                BMD* pModel = &Models[o->Owner->Type];
                int iBone = rand() % pModel->NumBones;

                vec3_t vRelativePos, vWorldPos;
                Vector(0.f, 0.f, 0.f, vRelativePos);

                if (!pModel->Bones[iBone].Dummy)
                {
                    pModel->TransformPosition(o->Owner->BoneTransform[iBone], vRelativePos, vWorldPos, false);
                    VectorScale(vWorldPos, pModel->BodyScale, vWorldPos);
                    VectorAdd(vWorldPos, o->Owner->Position, vWorldPos);

                    if (o->SubType == 3)
                    {
                        CreateParticleFpsChecked(BITMAP_LIGHT + 2, vWorldPos, o->Angle, o->Light, 6, o->Scale);
                        iBone = rand() % pModel->NumBones;
                        pModel->TransformPosition(o->Owner->BoneTransform[iBone], vRelativePos, vWorldPos, false);
                        VectorScale(vWorldPos, pModel->BodyScale, vWorldPos);
                        VectorAdd(vWorldPos, o->Owner->Position, vWorldPos);
                        CreateParticleFpsChecked(BITMAP_LIGHT + 2, vWorldPos, o->Angle, o->Light, 6, o->Scale);
                    }
                    else if (o->SubType == 4)
                    {
                        if (rand_fps_check(2))
                        {
                            vWorldPos[2] -= 20.f;
                            CreateParticleFpsChecked(BITMAP_TWINTAIL_WATER, vWorldPos, o->Angle, o->Light, 2);
                        }
                    }
                }
            }
        }
    }
        return true;
    }

    // MODEL_LIGHTNING_SHOCK
    bool Move_MODEL_LIGHTNING_SHOCK(OBJECT* o, int index, float Luminosity)
    {
        float Matrix[3][4];
        float Height;
    {
        vec3_t vLight;

        if (o->SubType == 0)
        {
            if (o->Owner->AnimationFrame > 6.0f || o->LifeTime < 15)
            {
                Vector(0.f, -20.f, -75.f - o->Velocity, o->Direction);
                o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;
                o->Velocity += (o->Gravity) * FPS_ANIMATION_FACTOR;
            }

            OBJECT* pObject = o;
            OBJECT* pOwner = pObject->Owner;
            BMD* pOwnerModel = &Models[pOwner->Type];

            float fRot = (WorldTime * 0.0006f) * 360.0f;
            float fScale = 0.8f;

            // shiny
            Vector(1.0f, 0.4f, 0.4f, vLight);
            CreateSprite(BITMAP_SHINY + 1, o->Position, 4.0f * fScale, vLight, o, fRot);
            CreateSprite(BITMAP_SHINY + 1, o->Position, 3.0f * fScale, vLight, o, -fRot);
            // magic_ground
            Vector(1.0f, 0.2f, 0.2f, vLight);
            CreateSprite(BITMAP_MAGIC, o->Position, 1.0f * fScale, vLight, o, fRot);
            CreateSprite(BITMAP_MAGIC, o->Position, 0.5f * fScale, vLight, o, -fRot);
            // pin_light
            Vector(1.0f, 0.4f, 0.4f, vLight);
            CreateSprite(BITMAP_PIN_LIGHT, o->Position, 2.0f * fScale, vLight, o, (float)(rand() % 360));
            CreateSprite(BITMAP_PIN_LIGHT, o->Position, 2.0f * fScale, vLight, o, (float)(rand() % 360));

            Vector(1.0f, 0.3f, 0.3f, vLight);
            CreateParticleFpsChecked(BITMAP_MAGIC, o->Position, o->Angle, vLight, 0, 1.f * fScale);

            vec3_t vPos, vLightFlare, vRelative;
            Vector(1.0f, 0.7f, 0.4f, vLight);
            Vector(1.0f, 0.2f, 0.1f, vLightFlare);
            for (int i = 0; i < 11; ++i)
            {
                fScale = (float)(rand() % 80 + 32) * 0.01f * 1.0f;
                Vector(o->Position[0] + (rand() % 70 - 35) * 1.0f, o->Position[1] + (rand() % 70 - 35) * 1.0f,
                    o->Position[2] + (rand() % 70 - 35) * 1.0f, vPos);
                CreateSprite(BITMAP_LIGHT, vPos, 2.2f, vLightFlare, pObject);
                if (rand() % 3 > 0) continue;
                CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, pObject->Angle, vLight, 0, fScale);
            }

            Vector(1.0f, 0.5f, 0.4f, vLight);

            for (int i = 0; i < 2; ++i)
            {
                fScale = (float)(rand() % 60 + 22) * 0.01f * 1.0f;
                int iBone = rand() % 41;
                Vector((rand() % 30 - 15) * 1.0f, (rand() % 30 - 15) * 1.0f, (rand() % 30 - 15) * 1.0f, vRelative);
                pOwnerModel->TransformByObjectBone(vPos, pOwner, iBone, vRelative);
                CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, pObject->Angle, vLight, 0, fScale);
            }

            float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                CreateEffectFpsChecked(MODEL_LIGHTNING_SHOCK, o->Position, o->Angle, o->Light, 1, o);
                EffectDestructor(o);
            }
        }
        else if (o->SubType == 1)
        {
            OBJECT* pObject = o;

            float fScale = 0.8f;
            vec3_t vPos, vLightFlare;
            Vector(1.0f, 0.7f, 0.4f, vLight);
            Vector(1.0f, 0.2f, 0.1f, vLightFlare);
            for (int i = 0; i < 11; ++i)
            {
                fScale = (float)(rand() % 80 + 32) * 0.01f * 1.0f;
                Vector(o->Position[0] + (rand() % 70 - 35) * 1.0f, o->Position[1] + (rand() % 70 - 35) * 1.0f,
                    o->Position[2] + (rand() % 70 - 35) * 1.0f, vPos);
                CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, pObject->Angle, vLight, 0, fScale);	// 전기
            }

            vec34_t Matrix;
            vec3_t vAngle, vDirection, vPosition;
            float fAngle;
            Vector(1.0f, 0.0f, 0.0f, vLight);

            for (int i = 0; i < 6; ++i)
            {
                fScale = (float)(rand() % 60 + 22) * 0.01f * 1.0f;
                Vector(0.f, rand() % 400, 0.f, vDirection);
                fAngle = o->Angle[2] + rand() % 360;
                Vector(0.f, 0.f, fAngle, vAngle);
                AngleMatrix(vAngle, Matrix);
                VectorRotate(vDirection, Matrix, vPosition);
                VectorAdd(vPosition, o->Position, vPosition);
                vPosition[2] = RequestTerrainHeight(vPosition[0], vPosition[1]) + 20;

                CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPosition, pObject->Angle, vLight, 0, fScale);	// 전기
            }

            VectorCopy(o->Position, vPosition);
            vPosition[2] = RequestTerrainHeight(vPosition[0], vPosition[1]) + 10;

            Vector(1.0f, 0.0f, 0.0f, vLight);

            for (int i = 0; i < 2; i++)
                CreateParticleFpsChecked(BITMAP_SMOKE, vPosition, o->Angle, vLight, 58);

            if (rand_fps_check(2))
            {
                Vector(1.0f, 0.0f, 0.0f, vLight);
                CreateParticleFpsChecked(BITMAP_SMOKE, vPosition, o->Angle, vLight, 54, 2.8f);
            }

            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, vPosition, o->Angle, vLight, 13, o);
        }
        else if (o->SubType == 2)
        {
            if (o->Owner != NULL && o->Owner->BoneTransform != NULL)
            {
                BMD* pTargetModel = &Models[o->Owner->Type];
                int iNumBones = pTargetModel->NumBones;
                float fRandom;
                vec3_t vLight, vRelativePos, vPos, vAngle;
                Vector(0.0f, 0.0f, 0.0f, vRelativePos);
                for (int i = 0; i < iNumBones; i++)
                {
                    VectorCopy(o->Owner->Position, pTargetModel->BodyOrigin);
                    pTargetModel->TransformPosition(o->Owner->BoneTransform[i], vRelativePos, vPos, true);

                    Vector(0.8f, 0.0f, 0.0f, vLight);
                    if (rand_fps_check(60))
                    {
                        fRandom = 5.0f + ((float)(rand() % 20 - 10) * 0.1f);
                        CreateParticleFpsChecked(BITMAP_LIGHT, vPos, vAngle, vLight, 5, fRandom);
                    }
                    Vector(1.0f, 0.8f, 0.4f, vLight);
                    if (rand_fps_check(5))
                    {
                        fRandom = (float)(rand() % 70 + 22) * 0.01f * 1.0f;
                        CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, vAngle, vLight, 0, fRandom);
                    }
                }

                Vector(1.0f, 0.0f, 0.0f, vLight);
                //for(i=0; i<2; i++)
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 58);

                if (rand_fps_check(4))
                {
                    Vector(1.0f, 0.0f, 0.0f, vLight);
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 54, 2.8f);
                }
            }
        }
    }
        return true;
    }

    // MODEL_SKILL_BLAST
    bool Move_MODEL_SKILL_BLAST(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Height;
        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            Vector(0.f, 0.f, 0.f, o->Direction);
            vec3_t Position;
            Vector(o->Position[0], o->Position[1], o->Position[2] + 80.f, Position);
            for (int j = 0; j < 6; j++)
            {
                if (rand_fps_check(1))
                {
                    CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light);
                }
            }
            CreateParticleFpsChecked(BITMAP_SHINY + 4, Position, o->Angle, Light);
            CreateParticleFpsChecked(BITMAP_EXPLOTION, Position, o->Angle, Light);
            if (o->Owner == &Hero->Object)
                AttackCharacterRange(o->Skill, o->Position, 150.f, o->Weapon, o->PKKey);
            o->Live = false;
        }
        VectorCopy(o->Position, o->EyeLeft);
        //CreateSprite(BITMAP_SHINY+4,o->Position,2.5f,Light,o);
        Vector(Luminosity * 0.2f, Luminosity * 0.4f, Luminosity * 1.f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        return true;
    }

    // MODEL_WAVE
    bool Move_MODEL_WAVE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (o->Scale > 2.f)
        {
            o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
            o->Position[0] -= (1.f) * FPS_ANIMATION_FACTOR;
            o->Position[2] -= (1.5f) * FPS_ANIMATION_FACTOR;
            o->BlendMeshLight = o->LifeTime / 30.f;
        }
        else
        {
            o->Scale += (1.2f) * FPS_ANIMATION_FACTOR;
            o->Position[0] -= (1.2f) * FPS_ANIMATION_FACTOR;
            o->Position[2] -= (1.8f) * FPS_ANIMATION_FACTOR;
        }

        Vector(-Luminosity * 0.5f, -Luminosity * 0.5f, -Luminosity * 0.5f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 5, PrimaryTerrainLight);
        return true;
    }

    // MODEL_TAIL
    bool Move_MODEL_TAIL(OBJECT* o, int index, float Luminosity)
    {
        o->Position[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
        o->Gravity += (60.f) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = o->LifeTime / 20.f;
        return true;
    }

    // MODEL_WAVE_FORCE
    bool Move_MODEL_WAVE_FORCE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        o->BlendMeshLight = o->LifeTime / 20.f;
        Vector(-Luminosity * 0.5f, -Luminosity * 0.5f, -Luminosity * 0.5f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 5, PrimaryTerrainLight);
        return true;
    }

    // MODEL_SKILL_INFERNO
    bool Move_MODEL_SKILL_INFERNO(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (o->SubType == 2)
        {
            o->Scale += (0.04f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 8)
        {
            o->Scale += (0.04f) * FPS_ANIMATION_FACTOR;
            if (o->LifeTime < 10)
            {
                o->Light[0] *= pow(0.8f, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(0.8f, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 3)
        {
            VectorCopy(o->Owner->Position, o->Position);
            o->Position[0] = o->Owner->Owner->Position[0] + 150.f;
        }
        else if (o->SubType == 4)
        {
            VectorCopy(o->Owner->Position, o->Position);

            o->Scale += (0.003f) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.8f * o->Scale * 30.f) * FPS_ANIMATION_FACTOR;
            o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 5)
        {
            o->Position[2] += (2.f) * FPS_ANIMATION_FACTOR;
            o->Angle[2] += (20.f) * FPS_ANIMATION_FACTOR;
            o->BlendMeshLight = o->LifeTime / 20.f;
            Vector(Luminosity * 0.1f, Luminosity * 0.3f, Luminosity * 0.8f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 5, PrimaryTerrainLight);
        }
        else if (o->SubType == 6)
        {
            o->Scale += (0.01f) * FPS_ANIMATION_FACTOR;
            o->BlendMeshLight = o->LifeTime / 5.f * 0.1f;
            Vector(Luminosity * 0.8f, Luminosity * 0.3f, Luminosity * 0.1f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        }

        if (o->SubType < 4)
        {
            o->BlendMeshLight = o->LifeTime / 20.f;
            if (o->SubType != 2)
            {
                Vector(-Luminosity * 0.5f, -Luminosity * 0.5f, -Luminosity * 0.5f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 5, PrimaryTerrainLight);
            }
        }
        else if (o->SubType == 8)
        {
            o->BlendMeshLight = o->LifeTime / 20.f;
        }
        else if (o->SubType == 9)
        {
            o->BlendMeshLight = o->LifeTime / 80.f;
            o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Light[0] *= pow(1.0f / (1.04f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.04f), FPS_ANIMATION_FACTOR);
            o->Alpha -= (0.01f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 10)
        {
            o->Scale += (0.04f) * FPS_ANIMATION_FACTOR;
            BMD* b = &Models[o->Type];
            VectorCopy(o->Light, b->BodyLight);
        }
        return true;
    }

    // MODEL_MAGIC_CIRCLE1
    bool Move_MODEL_MAGIC_CIRCLE1(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (o->SubType == 2)
        {
            o->Scale += (0.015f) * FPS_ANIMATION_FACTOR;
            VectorCopy(o->Owner->Position, o->Position);
            Vector(0.1f, 0.0f, 0.0f, o->Light);
        }
        else if (o->SubType == 1)
        {
            o->Scale += (0.01f) * FPS_ANIMATION_FACTOR;
            Vector(Luminosity * 1.0f, Luminosity * 0.0f, Luminosity * 0.0f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        }
        else
        {
            VectorCopy(o->Owner->Position, o->Position);
            o->Scale += (0.01f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_PROTECT
    bool Move_MODEL_PROTECT(OBJECT* o, int index, float Luminosity)
    {
        o->BlendMeshLight = 1.3f;

        VectorCopy(o->Owner->Position, o->Position);
        o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
        if (!o->Owner->Live || !o->Owner->Visible)
            o->Live = false;
        return true;
    }

    // MODEL_POISON
    bool Move_MODEL_POISON(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        o->BlendMeshLight = o->LifeTime * 0.1f;
        o->Alpha = o->LifeTime * 0.1f;
        Vector(Luminosity * 0.3f, Luminosity * 1.f, Luminosity * 0.6f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        return true;
    }

    // MODEL_SAW
    bool Move_MODEL_SAW(OBJECT* o, int index, float Luminosity)
    {
        o->Angle[2] -= (30.f) * FPS_ANIMATION_FACTOR;
        return true;
    }

    // MODEL_LASER
    bool Move_MODEL_LASER(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType != 0 && o->SubType != 3)
        {
            o->Direction[1] -= (o->Velocity) * FPS_ANIMATION_FACTOR;
            o->Velocity += (1.f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_SKILL_WHEEL1
    bool Move_MODEL_SKILL_WHEEL1(OBJECT* o, int index, float Luminosity)
    {
        CreateEffectFpsChecked(MODEL_SKILL_WHEEL2, o->Position, o->Angle, o->Light, 4 - o->LifeTime, o->Owner, o->PKKey, o->Skill, o->Kind);
        return true;
    }

    // MODEL_SKILL_FISSURE
    bool Move_MODEL_SKILL_FISSURE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
        vec3_t Position;
    {
        if ((int)o->LifeTime == 8)
        {
            for (int i = 0; i < 16; ++i)
            {
                Angle[0] = -10.f;
                Angle[1] = 0.f;
                Angle[2] = i * 10.f;
                vec3_t Position;
                VectorCopy(o->Position, Position);
                Position[2] += 100.f;

                //if ( (i%8)==0 )
                    //CreateEffect(BITMAP_MAGIC+1,o->Position,Angle,o->Light,4,o);

                Position[0] += rand() % 600 - 200;
                Position[1] += rand() % 600 - 400;
                CreateJointFpsChecked(BITMAP_FLARE, Position, Position, Angle, 24, NULL, 90);
            }
        }
        else if ((int)o->LifeTime == 2)
        {
            Vector(0.f, 0.f, rand() % 360, Angle);
            CreateEffectFpsChecked(MODEL_FISSURE, o->Position, Angle, o->Light, 0, o);
            CreateEffectFpsChecked(MODEL_FISSURE_LIGHT, o->Position, Angle, o->Light, 0, o);
            o->Live = false;
        }
    }
        return true;
    }

    // MODEL_FISSURE
    bool Move_MODEL_FISSURE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
    {
        double fLevel = sinf(Q_PI * double(o->LifeTime) / 120.f) * -0.5f;
        vec3_t Light;
        Vector(fLevel, fLevel, fLevel + 0.1f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 10, PrimaryTerrainLight);

        EarthQuake = (float)(rand() % 4 - 2) * 0.1f;

        if (rand_fps_check(2)) {
            Vector(0.f, (float)(rand() % 150) + 300.f, 0.f, p);
            Vector(0.f, 0.f, (float)(rand() % 360), Angle);
            AngleMatrix(Angle, Matrix);
            VectorRotate(p, Matrix, Position);
            VectorAdd(Position, o->Position, Position);

            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, Position, o->Angle, o->Light, 0);
        }

        BMD* pModel = &Models[o->Type];
        pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);
        Vector(1.f, 1.f, 1.f, Light);
        for (int i = 0; i < pModel->NumBones; i++) {
            if (!pModel->Bones[i].Dummy && rand_fps_check(12))
            {
                Vector(0.f, 0.f, 0.f, p);
                pModel->TransformPosition(BoneTransform[i], p, Position, true);
                CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, o->Angle, Light, 3, 4.f, o);
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 21, 1.f);
            }
        }
    }
        return true;
    }

    // MODEL_SKILL_FURY_STRIKE
    bool Move_MODEL_SKILL_FURY_STRIKE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
    {
        vec3_t p, Position, Pos[5];
        vec3_t Angle;
        short  scale = 150;
        float  Matrix[3][4], ang[5];
        int    TargetX, TargetY;

        Vector(0.f, 0.f, 0.f, Angle);
        Vector(0.f, 0.f, 0.f, p);

        if ((int)o->LifeTime == 11)
        {
            vec3_t  light;
            Vector(1.f, 1.f, 1.f, light);
            Vector(0.f, 0.f, 0.f, Angle);

            if (o->Kind != 3)
            {
                Vector(-25.f, -80.f, 0.f, p);
                AngleMatrix(o->Owner->Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, o->StartPosition);
            }

            float AddHeight = 25.f;

            if (gMapManager.InHellas() == true)
            {
                if (o->Kind == 0 || o->Kind == 2)
                {
                    int PositionX = (int)(o->StartPosition[0] / TERRAIN_SCALE);
                    int PositionY = (int)(o->StartPosition[1] / TERRAIN_SCALE);
                    AddWaterWave(PositionX, PositionY, 2, -1000);
                }
                else
                {
                    AddHeight = 100.f;
                }
            }

            o->StartPosition[2] = RequestTerrainHeight(o->StartPosition[0], o->StartPosition[1]) + AddHeight;
            AddHeight = 3.f;

            CreateParticleFpsChecked(BITMAP_EXPLOTION, o->StartPosition, Angle, light, 0, 0.5f);

            if (o->Kind == 0)
            {
                for (int j = 0; j < 8; j++)
                {
                    if (rand_fps_check(1))
                    {
                        Vector((float)(rand() % 60 - 60.f), 0.f, (float)(rand() % 30 + 90), Angle);
                        VectorAdd(Angle, o->Angle, Angle);
                        VectorCopy(o->StartPosition, Position);
                        Position[0] += rand() % 20 - 10;
                        Position[1] += rand() % 20 - 10;
                        CreateJointFpsChecked(BITMAP_JOINT_SPARK, Position, Position, Angle);
                        if (rand_fps_check(8)) CreateParticleFpsChecked(BITMAP_SPARK, Position, Angle, Light);
                    }
                }
            }
            Vector(0.f, 0.f, 0.f, Angle);

            if (o->Kind == 0)
                CreateEffectFpsChecked(MODEL_WAVE, o->StartPosition, Angle, o->Light);

            o->StartPosition[2] -= 27; //* FPS_ANIMATION_FACTOR;

            if (o->Owner != NULL)
            {
                if (o->Owner->Type != MODEL_WEREWOLF_HERO)
                {
                    CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 3, o->StartPosition, Angle, o->Light, 0, o, scale);
                    CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 1, o->StartPosition, Angle, o->Light, 0, o, scale);
                    CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 2, o->StartPosition, Angle, o->Light, 0, o, scale);
                }
            }

            for (int i = 0; i < 5; ++i)
            {
                Vector(0.f, (float)(rand() % 150 + 100), 0.f, p);
                Vector(0.f, 0.f, (float)(o->SubType + (i * 72.f)), Angle);
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->StartPosition, Position);

                Position[2] += 3;
                scale = rand() % 50 + 40;

                Vector(0.f, 0.f, 45 + (float)(rand() % 30 - 15), Angle);

                TargetX = (int)(Position[0] / TERRAIN_SCALE);
                TargetY = (int)(Position[1] / TERRAIN_SCALE);

                WORD wall = TerrainWall[TERRAIN_INDEX(TargetX, TargetY)];

                if ((wall & TW_NOMOVE) != TW_NOMOVE && (wall & TW_NOGROUND) != TW_NOGROUND && (wall & TW_WATER) != TW_WATER)
                {
                    if (gMapManager.InHellas() == true)
                    {
                        AddHeight = 100.f;
                    }
                    Position[2] = RequestTerrainHeight(Position[0], Position[1]) + AddHeight;

                    CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 4, Position, Angle, o->Light, 0, o->Owner, scale);
                    CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 5, Position, Angle, o->Light, 0, o->Owner, scale);
                }
            }
        }
        else if ((int)o->LifeTime == 10)
        {
            for (int j = 0; j < 5; ++j)
            {
                VectorCopy(o->StartPosition, Pos[j]);
                ang[j] = 0.f;
            }

            int count = 0;
            int random;

            for (int j = 0; j < 4; ++j)
            {
                Vector(0.f, rand() % 15 + 85.f, 0.f, p);

                if (j >= 3) count = rand();

                for (int i = 0; i < 5; ++i)
                {
                    if ((count % 2) == 0) random = rand() % 30 + 50;
                    else                random = -(rand() % 30 + 50);

                    ang[i] += random;
                    Angle[2] = ang[i] + (i * (rand() % 10 + 62));

                    AngleMatrix(Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(Position, Pos[i], Pos[i]);

                    TargetX = (int)(Pos[i][0] / TERRAIN_SCALE);
                    TargetY = (int)(Pos[i][1] / TERRAIN_SCALE);

                    WORD wall = TerrainWall[TERRAIN_INDEX(TargetX, TargetY)];

                    if ((wall & TW_NOMOVE) != TW_NOMOVE || (wall & TW_NOGROUND) != TW_NOGROUND || (wall & TW_WATER) != TW_WATER)
                    {
                        if (gMapManager.InHellas() == true)
                            Pos[i][2] = RequestTerrainHeight(Pos[i][0], Pos[i][1]) + 100.f;
                        else
                            Pos[i][2] = RequestTerrainHeight(Pos[i][0], Pos[i][1]) + 3;
                        Angle[2] += 270.f;

                        CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 7, Pos[i], Angle, o->Light, 0, o->Owner, 100);
                        CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE + 8, Pos[i], Angle, o->Light, 0, o->Owner, 100);
                    }
                }

                count++;
            }
            o->LifeTime = 0;

            PlayBuffer(SOUND_FURY_STRIKE3);
        }
        else
        {
            auto count = (float)o->LifeTime;
            float	addAngle = 15.f;
            if (o->LifeTime > 9 && o->LifeTime < 16)
            {
                count = 12.5;
                addAngle = 18.f;
                if ((int)o->LifeTime == 15)
                    o->Gravity *= pow(-1, FPS_ANIMATION_FACTOR);
            }

            if (o->Kind == 0)
            {
                if ((int)o->LifeTime == 13)
                {
                    vec3_t  pos;
                    Vector(0.f, 0.f, 0.f, Angle);
                    Vector(-25.f, -40.f, 0.f, p);
                    AngleMatrix(o->Owner->Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(Position, o->Position, pos);

                    vec3_t  position;
                    //                            pos[2] -= 50.f;
                    for (int i = 0; i < 4; ++i)
                    {
                        position[0] = pos[0]; position[1] = pos[1];
                        position[2] = pos[2] - (i * 50);
                        CreateEffectFpsChecked(MODEL_TAIL, position, Angle, o->Light);
                    }
                    pos[0] += rand() % 30 + 20;
                    pos[2] += rand() % 500 - 250;

                    for (int i = 0; i < 4; ++i)
                    {
                        position[0] = pos[0]; position[1] = pos[1];
                        position[2] = pos[2] - (i * 30);
                        CreateEffectFpsChecked(MODEL_TAIL, position, Angle, o->Light);
                    }
                    PlayBuffer(SOUND_FURY_STRIKE2);
                }
            }

            float   angle = (20 - count) * addAngle;

            o->Angle[0] += (80) * FPS_ANIMATION_FACTOR;
            o->Direction[1] = sinf((float)((angle * Q_PI) / 180.f)) * 260;
            VectorCopy(o->StartPosition, o->Position);
            if (count < 12.5 || count>12.5)
            {
                o->Gravity += (8.f) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Gravity -= (8.f) * FPS_ANIMATION_FACTOR;
            }

            AngleMatrix(o->HeadAngle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAdd(Position, o->StartPosition, o->Position);

            o->Position[2] += o->Gravity;
            o->Position[2] += 200;
        }
    }
        return true;
    }

    // MODEL_BALGAS_SKILL
    bool Move_MODEL_BALGAS_SKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        o->BlendMeshLight = o->LifeTime * 0.1f;
        //        o->BlendMeshTexCoordU = -(float)o->LifeTime*0.2f;
        Vector(Luminosity * 0.3f, Luminosity * 0.6f, Luminosity, Light);
        {
            BMD* b = &Models[o->Type];
            b->CurrentAction = o->CurrentAction;
            b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity * 2.0f, o->Position, o->Angle);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        }
        return true;
    }

    // MODEL_CHANGE_UP_EFF
    bool Move_MODEL_CHANGE_UP_EFF(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
    {
        vec3_t  Loc;
        int STwo = 2;
        if (rand_fps_check(STwo))
        {
            //               Vector(0.f,0.f,(float)(rand()%360),o->Angle);

            if (o->SubType == 2)
            {
                VectorCopy(o->Owner->Position, o->Position);
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 22.0f;
            }

            Loc[0] = o->Position[0] + rand() % 200 - 100;
            Loc[1] = o->Position[1] + rand() % 200 - 100;
            Loc[2] = o->Position[2] - 200;
            if (o->SubType == 0 || o->SubType == 2)
            {
                CreateJointFpsChecked(BITMAP_FLARE, Loc, Loc, o->Angle, 50, NULL, 40);
            }
            //				else
            //				if(o->SubType == 1)
            //					CreateJoint(BITMAP_FLARE,Loc,Loc,o->Angle,25,o,50.f);
                                //CreateJoint(BITMAP_FLARE,Loc,Loc,o->Angle,51,NULL,30);

            if (o->LifeTime > 40)
                CreateEffectFpsChecked(BITMAP_MAGIC, o->Position, o->Angle, Light, 4, o, 4.0f);
        }
        if (o->LifeTime <= 1 && o->SubType == 0)
            SetPlayerStop(Hero);

        if (o->SubType == 0)
        {
            BMD* b = &Models[o->Type];
            b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 3.f, o->Position, o->Angle);
        }
        else if (o->SubType == 2)
        {
            BMD* b = &Models[o->Type];
            b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 3.f, o->Position, o->Angle);
        }
    }
        return true;
    }

    // MODEL_CHANGE_UP_NASA
    bool Move_MODEL_CHANGE_UP_NASA(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            if ((int)o->LifeTime == 100)
            {
                CreateEffectFpsChecked(MODEL_CHANGE_UP_NASA, o->Position, o->Angle, o->Light, 1, o);
            }
            else
                if ((int)o->LifeTime == 70)
                {
                    CreateEffectFpsChecked(MODEL_CHANGE_UP_NASA, o->Position, o->Angle, o->Light, 2, o);
                }
            if ((int)o->LifeTime == 40)
            {
                CreateEffectFpsChecked(MODEL_CHANGE_UP_NASA, o->Position, o->Angle, o->Light, 3, o);
            }
        }
        if (o->SubType >= 1 && o->SubType <= 3)
        {
            o->Scale += (0.02f) * FPS_ANIMATION_FACTOR;
            o->BlendMeshLight = o->LifeTime * 0.005f;
        }
        else
            if (o->SubType == 0)
                o->BlendMeshLight = o->LifeTime * 0.01f;
    }
        return true;
    }

    // MODEL_CHANGE_UP_CYLINDER
    bool Move_MODEL_CHANGE_UP_CYLINDER(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
    {
        if (o->SubType == 2)
        {
            VectorCopy(o->Owner->Position, o->Position);
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        }

        vec3_t  Go;
        AngleMatrix(Angle, Matrix);
        VectorRotate(Go, Matrix, Position);
        VectorAdd(Position, o->Position, Position);

        if (o->SubType == 1)
        {
            o->BlendMeshLight = o->LifeTime * 0.015f;
            o->Scale += (0.08f) * FPS_ANIMATION_FACTOR;
        }
        //			Vector(Luminosity*0.3f,Luminosity*0.6f,Luminosity,Light);
        //			AddTerrainLight(Position[0],Position[1],Light,3,PrimaryTerrainLight);
    }
        return true;
    }

    // MODEL_DARK_ELF_SKILL
    bool Move_MODEL_DARK_ELF_SKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
    {
        vec3_t  Go;
        if (o->Scale < 0.8f)
            o->Scale += (0.2f) * FPS_ANIMATION_FACTOR;
        else
            o->Scale -= (0.4f) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = o->LifeTime * 0.1f;
        o->BlendMeshTexCoordU = -(float)o->LifeTime * 0.2f;
        Vector(0.0f, -500.f, 0.0f, Go);
        vec3_t	 	  Angle;
        VectorCopy(o->Angle, Angle);
        Angle[2] += 7;

        AngleMatrix(Angle, Matrix);
        VectorRotate(Go, Matrix, Position);
        VectorAdd(Position, o->Position, Position);

        for (int j = 0; j < 6; j++)
        {
            if (rand_fps_check(1))
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, o->Light, 25);
        }
        Vector(Luminosity * 0.3f, Luminosity * 0.6f, Luminosity, Light);
        AddTerrainLight(Position[0], Position[1], Light, 3, PrimaryTerrainLight);
    }
        return true;
    }

    // MODEL_MAGIC2
    bool Move_MODEL_MAGIC2(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        o->BlendMeshLight = o->LifeTime * 0.1f;
        o->BlendMeshTexCoordU = -(float)o->LifeTime * 0.2f;
        //VectorAdd(o->Position,o->Direction,o->Position);

        for (int j = 0; j < 4; j++)
            if (rand_fps_check(1))
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 3);

        Vector(Luminosity * 0.3f, Luminosity * 0.6f, Luminosity, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);

        if (o->SubType == 2)
        {
            o->HiddenMesh = 0;
            vec3_t Light2;
            VectorCopy(o->Angle, Angle);
            VectorCopy(o->Position, Position);
            Angle[2] += rand() % 10 - 5;
            if (rand_fps_check(1))
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 11, (float)(rand() % 32 + 80) * 0.015f);

            Vector(1.f, 1.f, 1.f, Light2);
            Position[2] += 50.f;
            CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light2, NULL, (float)(rand() % 360));
            CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light2, NULL, (float)(rand() % 360));

            CreateSprite(BITMAP_LIGHT, Position, 3.5f, Light, NULL, (float)(rand() % 360));
        }
        return true;
    }

    // MODEL_SUMMON
    bool Move_MODEL_SUMMON(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->LifeTime >= 30)
        {
            o->Light[0] *= pow(1.03f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.03f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.03f, FPS_ANIMATION_FACTOR);
        }
        else
        {
            o->Light[0] *= pow(1.0f / (1.08f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.08f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.08f), FPS_ANIMATION_FACTOR);
        }
        o->Angle[2] += (1.0f) * FPS_ANIMATION_FACTOR;
    }
        return true;
    }

    // MODEL_STORM2
    bool Move_MODEL_STORM2(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
    {
        if (o->SubType == 0)
        {
            o->Angle[1] += (40.0f) * FPS_ANIMATION_FACTOR;

            if (o->LifeTime <= 5)
            {
                o->Light[0] *= pow(1.0f / (1.01f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.01f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.01f), FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 1)
        {
            vec3_t Pos1, Pos2, Direction;

            float Matrix[3][4];

            for (int i = 0; i < 2; i++)
            {
                Angle[0] = o->Angle[0];
                Angle[1] = o->Angle[1];
                Angle[2] = o->Angle[2] / 3.0f + 180.0f * i;

                Vector(0.0f, 200.0f, 0.0f, Direction);
                AngleMatrix(Angle, Matrix);
                VectorRotate(Direction, Matrix, Position);
                VectorAdd(Position, o->Position, Position);

                Vector(0.3f, 0.5f, 0.6f, Light);
                CreateParticleFpsChecked(BITMAP_CLUD64, Position, o->Angle, Light, 1, 1.0f);
            }

            Vector(90.f, 0.f, 0.0f, Angle);

            Pos1[0] = o->Position[0] + rand() % 600 - 300.0f;
            Pos1[1] = o->Position[1] + rand() % 600 - 300.0f;
            Pos1[2] = o->Position[2];

            Pos2[0] = Pos1[0] + rand() % 100 - 50.0f;
            Pos2[1] = Pos1[1] + rand() % 100 - 50.0f;
            Pos2[2] = Pos1[2];

            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Pos1, Pos2, Angle, 20, o, 20.f);

            Pos2[0] = Pos1[0] + rand() % 100 - 50.0f;
            Pos2[1] = Pos1[1] + rand() % 100 - 50.0f;
            Pos2[2] = Pos1[2];
            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, Pos2, o->Angle, Light, 2);

            o->Angle[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Light[0] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);

            o->Alpha += (0.01f) * FPS_ANIMATION_FACTOR;

            if (o->Light[0] <= 0.05f)
                o->Live = false;
        }
        else if (o->SubType == 2)
        {
            o->Angle[1] += (40.0f) * FPS_ANIMATION_FACTOR;

            if (o->LifeTime <= 5)
            {
                o->Light[0] *= pow(1.0f / (1.01f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.01f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.01f), FPS_ANIMATION_FACTOR);
            }

            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light, 2);
        }
    }
        return true;
    }

    // MODEL_STORM3
    bool Move_MODEL_STORM3(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
    {
        if (o->LifeTime <= 30)
        {
            o->Light[0] *= pow(1.0f / (1.1f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.1f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.1f), FPS_ANIMATION_FACTOR);
        }
        o->Angle[1] += (30.0f) * FPS_ANIMATION_FACTOR;

        Vector(90.f, 0.f, o->Angle[2], Angle);

        for (int i = 0; i < 5; i++)
        {
            if (rand_fps_check(2))
            {
                vec3_t Pos1, Pos2;

                Pos1[0] = o->StartPosition[0] + rand() % 2000 - 1000.0f;
                Pos1[1] = o->StartPosition[1] + rand() % 2000 - 1000.0f;
                Pos1[2] = o->StartPosition[2];

                Pos2[0] = Pos1[1] + rand() % 200 - 100.0f;
                Pos2[1] = Pos1[1] + rand() % 200 - 100.0f;

                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Pos1, Pos2, Angle, 20, o, 20.f);
            }
        }
    }
        return true;
    }

    // MODEL_MAYASTONE1, MODEL_MAYASTONE2, MODEL_MAYASTONE3
    bool Move_MODEL_MAYASTONE1(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Height;
    {
        bool success = false;

        VectorCopy(o->HeadAngle, o->Angle);

        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
            success = true;

        if (success)
        {
            o->Position[2] = Height;
            Vector(0.f, 0.f, 0.f, o->Direction);
            vec3_t Position;
            Vector(o->Position[0], o->Position[1], o->Position[2] + 80.f, Position);

            BYTE smokeNum = 15;

            Vector(0.0f, 0.6f, 1.f, Light);
            Vector(0.f, 0.f, 0.f, Angle);
            CreateEffectFpsChecked(MODEL_SKILL_INFERNO, o->Position, Angle, Light, 2, o, 30, 0);

            for (int i = 0; i < smokeNum; ++i)
            {
                Position[0] = o->Position[0] + (rand() % 160 - 80);
                Position[1] = o->Position[1] + (rand() % 160 - 100);
                Position[2] = o->Position[2] + 50;

                Vector(0.3f, 0.5f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 11, (float)(rand() % 32 + 80) * 0.025f);
            }

            for (int j = 0; j < 6; j++)
                CreateEffectFpsChecked(MODEL_MAYASTONE4 + rand() % 2, o->Position, o->Angle, Light);

            CreateParticleFpsChecked(BITMAP_EXPLOTION, Position, o->Angle, Light, 0, 4.0f);

            o->Live = false;
        }
    }
        return true;
    }

    // MODEL_MAYASTONE4, MODEL_MAYASTONE5
    bool Move_MODEL_MAYASTONE4(OBJECT* o, int index, float Luminosity)
    {
        float Height;
    {
        VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        VectorScale(o->Direction, 0.9f, o->Direction);
        o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
        o->Gravity -= (3.f) * FPS_ANIMATION_FACTOR;
        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.5f;
            o->LifeTime -= (4) * FPS_ANIMATION_FACTOR;
            o->Angle[0] -= (o->Scale * 128.f) * FPS_ANIMATION_FACTOR;
        }
        else
            o->Angle[0] -= (o->Scale * 32.f) * FPS_ANIMATION_FACTOR;

        if (rand_fps_check(10))
            CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, o->Light, 15);
    }
        return true;
    }

    // MODEL_MAYASTONEFIRE
    bool Move_MODEL_MAYASTONEFIRE(OBJECT* o, int index, float Luminosity)
    {
        float Height;
    {
        VectorCopy(o->HeadAngle, o->Angle);

        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
            o->Live = false;
    }
        return true;
    }

    // MODEL_MAYAHANDSKILL
    bool Move_MODEL_MAYAHANDSKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        float Matrix[3][4];
    {
        if (o->SubType == 0)
        {
            o->Light[0] *= pow(1.0f / (1.17f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.17f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.17f), FPS_ANIMATION_FACTOR);

            vec3_t vPos;
            VectorCopy(o->Position, vPos);
            vPos[2] += 100.f;
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->StartPosition, 17, 10.0f);

            vec3_t Direction;
            Vector(0.0f, (-80.0f - rand() % 30) * o->Scale, 0.0f, Direction);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(Direction, Matrix, Position);
            VectorAdd(Position, o->Position, o->Position);
            CreateEffectFpsChecked(MODEL_MAYAHANDSKILL, o->Position, o->Angle, o->StartPosition, 1, NULL, -1, 0, 0, 0, o->Scale + 0.2f);

            if (o->Light[0] <= 0.05f)
                o->Live = false;
        }
        else if (o->SubType == 1)
        {
            o->Light[0] *= pow(1.0f / (1.17f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.17f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.17f), FPS_ANIMATION_FACTOR);

            vec3_t vPos;
            VectorCopy(o->Position, vPos);
            vPos[2] += 100.f;
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->StartPosition, 17, 10.0f);
        }
    }
        return true;
    }

    // MODEL_CIRCLE
    bool Move_MODEL_CIRCLE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
        vec3_t Position;
        o->BlendMeshLight = o->LifeTime * 0.1f;
        if (o->SubType == 1)
        {
            if (o->LifeTime > (44 - o->Owner->m_bySkillCount))
            {
                vec3_t Angle = { 0.0f, 0.0f, 0.0f };
                int iCount = 36;
                for (int i = 0; i < iCount; ++i)
                {
                    Angle[0] = -10.f;
                    Angle[1] = 0.f;
                    Angle[2] = i * (10.f + rand() % 10);
                    vec3_t Position;
                    VectorCopy(o->Position, Position);
                    Position[2] += 100.f;
                    CreateJointFpsChecked(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 6, o, 60.f, 0, 0);

                    if ((int)o->LifeTime == (44 - o->Owner->m_bySkillCount + 1))
                    {
                        Angle[2] = i * 10.f;
                        CreateJointFpsChecked(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 7, o, 60.f, 0, 0);
                    }
                }
            }
        }
        else if (o->SubType == 2)
        {
            if (o->LifeTime > 240)
            {
                o->BlendMeshLight = (250 - o->LifeTime) * 0.1f;
            }
            else
            {
                o->BlendMeshLight = o->LifeTime * 0.1f;
            }
        }
        else if (o->SubType == 3)
        {
            if (o->LifeTime > 10)
            {
                o->BlendMeshLight = (30 - o->LifeTime) * 0.1f;
            }
            else
            {
                o->BlendMeshLight = o->LifeTime * 0.1f;
            }
        }
        else if (o->SubType == 4)
        {
            if (o->LifeTime > (44 - o->Owner->m_bySkillCount))
            {
                vec3_t Angle, Position;
                for (int i = 0; i < 36; ++i)
                {
                    Angle[0] = -10.f;
                    Angle[1] = 0.f;
                    Angle[2] = i * 10.f;

                    VectorCopy(o->Position, Position);
                    Position[2] += 100.f;
                    CreateJointFpsChecked(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 22, o, 2.0f, 0, 0);
                    CreateJointFpsChecked(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 23, o, 1.75f, 0, 0);
                }
            }
        }
        return true;
    }

    // MODEL_CIRCLE_LIGHT
    bool Move_MODEL_CIRCLE_LIGHT(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
        if (o->SubType != 3 && o->SubType != 4)
        {
            int value = 4;

            if (o->LifeTime >= 30)
                o->BlendMeshLight = (40 - o->LifeTime) * 0.1f;
            else
                o->BlendMeshLight = o->LifeTime * 0.1f;
            o->BlendMeshTexCoordU = -(float)o->LifeTime * 0.01f;
            EarthQuake = (float)(rand() % 6 - 6) * 0.1f;

            if (rand_fps_check(value))
            {
                vec3_t p, Position;
                vec3_t Angle;
                float Matrix[3][4];
                Vector(0.f, (float)(rand() % 300), 0.f, p);
                Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, Position);
                CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, Position, o->Angle, o->Light, 1);
            }
            Vector(Luminosity * 1.f, Luminosity * 0.8f, Luminosity * 0.2f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        }
        else if (o->SubType == 3)
        {
            int value = 2;

            if (o->LifeTime >= 240)
                o->BlendMeshLight = (250 - o->LifeTime) * 0.1f;
            else
                o->BlendMeshLight = o->LifeTime * 0.1f;

            o->BlendMeshLight = std::min<float>(0.5f, o->BlendMeshLight);
            o->BlendMeshTexCoordU = -(float)o->LifeTime * 0.01f;
            if (o->LifeTime > 30 && rand_fps_check(value))
            {
                vec3_t p, Position;
                vec3_t Angle;
                float Matrix[3][4];
                Vector(0.f, (float)(rand() % 200), 0.f, p);
                Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, Position);

                CreateParticleFpsChecked(BITMAP_FLARE_BLUE, Position, o->Angle, o->Light, 0);
                if (o->LifeTime > 40)
                {
                    Position[2] += 600.f;
                    Angle[2] = 45.f;
                    CreateJointFpsChecked(BITMAP_FLARE_BLUE, Position, Position, Angle, 19, NULL, 40);
                }
            }
            Vector(o->BlendMeshLight, o->BlendMeshLight, o->BlendMeshLight, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        }
        else if (o->SubType == 4)
        {
            int value = 5;

            if (o->LifeTime >= 10)
                o->BlendMeshLight = (20 - o->LifeTime) * 0.1f;
            else
                o->BlendMeshLight = o->LifeTime * 0.1f;

            o->BlendMeshLight = std::min<float>(0.5f, o->BlendMeshLight);
            o->BlendMeshTexCoordU = -(float)o->LifeTime * 0.01f;
            if (rand_fps_check(value) && o->LifeTime > 5)
            {
                vec3_t p, Position;
                vec3_t Angle;
                float Matrix[3][4];
                Vector(0.f, (float)(rand() % 100), 0.f, p);
                Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, Position);

                CreateParticleFpsChecked(BITMAP_FLARE_BLUE, Position, o->Angle, o->Light, 0);
                Position[2] += 600.f;
                Angle[2] = 45.f;
                CreateJointFpsChecked(BITMAP_FLARE_BLUE, Position, Position, Angle, 19, NULL, 40);
            }
            Vector(o->BlendMeshLight, o->BlendMeshLight, o->BlendMeshLight, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        }
        return true;
    }

    // MODEL_ICE_SMALL, MODEL_METEO1, MODEL_METEO2, MODEL_BOSS_ATTACK, MODEL_EFFECT_SAPITRES_ATTACK_2
    bool Move_MODEL_ICE_SMALL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        float Matrix[3][4];
        float Height;
        if (o->SubType == 0 || o->SubType == 10 || o->SubType == 12)
        {
            VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        }
        else if (o->Type == MODEL_EFFECT_SAPITRES_ATTACK_2 && o->SubType == 14)
        {
            auto fMoveSpeed = (float)(rand() % 5 + 25);
            o->Position[0] -= ((o->Direction[0] * fMoveSpeed)) * FPS_ANIMATION_FACTOR;
            o->Position[1] -= ((o->Direction[1] * fMoveSpeed)) * FPS_ANIMATION_FACTOR;
            o->Position[2] -= ((o->Direction[2] * fMoveSpeed)) * FPS_ANIMATION_FACTOR;

            if (o->LifeTime <= 10)
            {
                o->BlendMeshLight -= (0.1f) * FPS_ANIMATION_FACTOR;
            }
        }
        else
        {
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
        }
        VectorScale(o->Direction, 0.9f, o->Direction);
        o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
        if (o->SubType == 0 || o->SubType == 10 || o->SubType == 12 || o->SubType == 13)
        {
            o->Gravity -= (3.f) * FPS_ANIMATION_FACTOR;
            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                o->Position[2] = Height;
                o->Gravity = -o->Gravity * 0.5f;
                o->LifeTime -= (4) * FPS_ANIMATION_FACTOR;
                o->Angle[0] -= (o->Scale * 128.f) * FPS_ANIMATION_FACTOR;
            }
            else
                o->Angle[0] -= (o->Scale * 32.f) * FPS_ANIMATION_FACTOR;

            if ((o->SubType == 0 || o->SubType == 12 || o->SubType == 13) && rand_fps_check(10))
            {
                if (o->Type == MODEL_ICE_SMALL || o->Type == MODEL_METEO1 || o->Type == MODEL_METEO2)
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light);
                else if (o->Type == MODEL_STONE1 || o->Type == MODEL_STONE2)
                {
                    CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, o->Light, 1 + rand() % 3);
                }
            }
        }
        else if (o->SubType == 1)
            o->Gravity += (0.5f) * FPS_ANIMATION_FACTOR;
        else if (o->Type != MODEL_EFFECT_SAPITRES_ATTACK_2)
        {
            o->Angle[2] += (20.f) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.5f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_SKULL
    bool Move_MODEL_SKULL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
        if (o->SubType == 1 && o->Owner != NULL)
        {
            if (((int)o->LifeTime % 10) == 0)
            {
                VectorCopy(o->Position, o->m_vDeadPosition);
            }

            VectorCopy(o->Owner->Position, o->StartPosition);
            o->StartPosition[2] += (200.f) * FPS_ANIMATION_FACTOR;

            MoveHumming(o->Position, o->Angle, o->StartPosition, 10.f);

            o->HeadAngle[0] += ((float)(rand() % 32 - 16) * 0.2f) * FPS_ANIMATION_FACTOR;
            o->HeadAngle[2] += ((float)(rand() % 32 - 16) * 0.8f) * FPS_ANIMATION_FACTOR;
            o->Angle[0] += (o->HeadAngle[0]) * FPS_ANIMATION_FACTOR;
            o->Angle[2] += (o->HeadAngle[2]) * FPS_ANIMATION_FACTOR;
            o->HeadAngle[0] *= pow(0.6f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);

            if (o->LifeTime < 10)
            {
                o->Alpha *= pow(1.0f / (1.1f), FPS_ANIMATION_FACTOR);
                o->BlendMeshLight *= pow(1.0f / (1.2f), FPS_ANIMATION_FACTOR);

                if (o->Angle[0] <= 80)
                {
                    o->Angle[0] += (5.f) * FPS_ANIMATION_FACTOR;
                }
            }
            else
            {
                Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
                if (o->Position[2] < Height + 100.f)
                {
                    o->HeadAngle[0] = 0.f;
                    o->Angle[0] = -5.f;
                }
                if (o->Position[2] > Height + 400.f)
                {
                    o->HeadAngle[0] = 0.f;
                    o->Angle[0] = 5.f;
                }
            }

            CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light, 5, 1.2f);

            Vector(Luminosity, Luminosity * 0.3f, Luminosity * 0.1f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        }
        return true;
    }

    // MODEL_CURSEDTEMPLE_STATUE_PART1, MODEL_CURSEDTEMPLE_STATUE_PART2
    bool Move_MODEL_CURSEDTEMPLE_STATUE_PART1(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        VectorCopy(o->Light, Light);
        if (o->Position[2] > 290)//o->StartPosition[2])
        {
            o->Direction[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Angle[2] -= ((float)(rand() % 10)) * FPS_ANIMATION_FACTOR;
        }
        else if (o->Position[2] < 290)
        {
            o->Position[2] = 290;
            o->Direction[0] = 0;
            o->Direction[1] = 0;
            o->Direction[2] = 0;
            EarthQuake = (float)(rand() % 4 + 1) * 0.1f;
        }
        else
        {
            o->Alpha -= (0.01f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_XMAS2008_SNOWMAN_HEAD
    bool Move_MODEL_XMAS2008_SNOWMAN_HEAD(OBJECT* o, int index, float Luminosity)
    {
        float Height;
    {
        if (o->LifeTime < 45)
        {
            o->Position[0] += ((o->Direction[0] * 1.2f)) * FPS_ANIMATION_FACTOR;
            o->Position[1] += ((o->Direction[1] * 1.2f)) * FPS_ANIMATION_FACTOR;
            o->Position[2] += ((o->Gravity * 1.5f)) * FPS_ANIMATION_FACTOR;
            o->Gravity -= (1.5f) * FPS_ANIMATION_FACTOR;

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                o->Position[2] = Height;
                o->Gravity = -o->Gravity * 0.3f;
                o->LifeTime -= (2) * FPS_ANIMATION_FACTOR;
            }

            VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        }
    }
        return true;
    }

    // MODEL_XMAS2008_SNOWMAN_BODY
    bool Move_MODEL_XMAS2008_SNOWMAN_BODY(OBJECT* o, int index, float Luminosity)
    {
    {
        BMD* b = &Models[o->Type];
        b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 5.f, o->Position, o->Angle);
    }
        return true;
    }

    // MODEL_DOPPELGANGER_SLIME_CHIP
    bool Move_MODEL_DOPPELGANGER_SLIME_CHIP(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
        o->HeadAngle[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
        VectorCopy(o->Light, Light);

        o->Position[0] += (o->HeadAngle[0]) * FPS_ANIMATION_FACTOR;
        o->Position[1] += (o->HeadAngle[1]) * FPS_ANIMATION_FACTOR;
        o->Position[2] += (o->HeadAngle[2]) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]) + 20;

        if (o->LifeTime < 10)
        {
            o->Light[0] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
        }

        if (o->Position[2] + o->Direction[2] <= Height)
        {
            o->Position[2] = Height;
            o->HeadAngle[0] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[1] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[2] += (1.6f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            if (o->HeadAngle[2] < 5.0f)
                o->HeadAngle[2] = 0;
        }
        else
        {
            o->Scale += (sinf(WorldTime * 0.015f) * 0.1f) * FPS_ANIMATION_FACTOR;
        }

        if (o->SubType == 0)
        {
            o->Angle[0] += (0.35f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            o->Angle[1] += (0.35f * o->LifeTime) * FPS_ANIMATION_FACTOR;
        }
        else
        {
            o->Angle[0] -= (0.35f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            o->Angle[1] -= (0.35f * o->LifeTime) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_WATER_WAVE
    bool Move_MODEL_WATER_WAVE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        o->Scale += (0.02f) * FPS_ANIMATION_FACTOR;
        o->Position[2] = o->StartPosition[2] + o->Gravity;

        o->Velocity -= (10.f) * FPS_ANIMATION_FACTOR;
        o->Gravity -= (20.f) * FPS_ANIMATION_FACTOR;
        if (o->Gravity < 60) o->Gravity = 60;

        Vector(0.f, o->Velocity, 0.f, o->Direction);
        Vector(Luminosity * 0.1f, Luminosity * 0.3f, Luminosity * 0.6f, Light);

        for (int j = 0; j < 4; j++)
        {
            CreateParticleFpsChecked(BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 1);
        }

        AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        return true;
    }

    // MODEL_STAFF_OF_DESTRUCTION
    bool Move_MODEL_STAFF_OF_DESTRUCTION(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Height;
        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            VectorCopy(o->Position, Position);
            Position[2] += 80;
            CreateParticleFpsChecked(BITMAP_EXPLOTION, Position, o->Angle, Light);

            for (int j = 0; j < 6; j++)
            {
                CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light);
            }
            o->Live = false;
        }
        Vector(Luminosity * 0.2f, Luminosity * 0.4f, Luminosity * 1.f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        return true;
    }

    // MODEL_PIERCING
    bool Move_MODEL_PIERCING(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (o->Owner->Live == false)
        {
            o->LifeTime = -1;
            return true;
        }
        /*
                    Vector(Luminosity*0.9f,Luminosity*0.4f,Luminosity*0.6f,Light);
            AddTerrainLight(o->StartPosition[0],o->StartPosition[1],Light,2,PrimaryTerrainLight);
        */
        o->Gravity += (90.f) * FPS_ANIMATION_FACTOR;
        VectorCopy(o->Owner->Angle, o->Angle);
        VectorCopy(o->Owner->Angle, o->HeadAngle);
        VectorCopy(o->Position, o->StartPosition);
        VectorCopy(o->Owner->Position, o->Position);
        if (o->SubType == 1)
        {
            Vector(1.f, 1.f, 1.f, Light);

            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 3, NULL, 20.f, 7); //  전기
            CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 8 + 8) * 0.2f, Light, o, (float)(rand() % 360));
        }

        if (gMapManager.InHellas())
        {
            if (o->Owner != NULL && o->Owner->Owner != NULL && o->Owner->Owner == (&Hero->Object))
            {
                int PositionX = (int)(o->Position[0] / TERRAIN_SCALE);
                int PositionY = (int)(o->Position[1] / TERRAIN_SCALE);
                AddWaterWave(PositionX, PositionY, 2, -200);
            }
        }

        o->HeadAngle[1] = o->Gravity;
        return true;
    }

    // MODEL_ARROW_BEST_CROSSBOW
    bool Move_MODEL_ARROW_BEST_CROSSBOW(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        if (o->LifeTime > 25)
        {
            o->BlendMeshLight = (30 - o->LifeTime) / 40.f;
        }
        else
        {
            o->BlendMeshLight = 1.f;
        }
        o->Angle[1] += (rand() % 60 + 30.f) * FPS_ANIMATION_FACTOR;
        VectorCopy(o->Position, o->EyeLeft);

        VectorCopy(o->Angle, Angle);
        AngleMatrix(Angle, Matrix);
        VectorRotate(o->Direction, Matrix, Position);
        VectorAdd(o->EyeLeft, Position, o->EyeLeft);

        Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        CheckClientArrow(o);
        return true;
    }

    // MODEL_ARROW_DOUBLE
    bool Move_MODEL_ARROW_DOUBLE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        o->Angle[1] += (30.f) * FPS_ANIMATION_FACTOR;
        VectorCopy(o->Position, o->EyeLeft);
        Vector(Luminosity * 0.2f, Luminosity * 0.4f, Luminosity * 1.f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        CheckClientArrow(o);
        return true;
    }

    // MODEL_ARROW_HOLY
    bool Move_MODEL_ARROW_HOLY(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (o->SubType == 1)
        {
            o->Angle[1] += (60.f) * FPS_ANIMATION_FACTOR;

            if ((int)o->LifeTime == 13)
                CreateEffectFpsChecked(MODEL_PIERCING, o->Position, o->Angle, o->Light, 3, o);
            CheckClientArrow(o);

            Vector(Luminosity * 0.9f, Luminosity * 0.4f, Luminosity * 0.6f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        }
        else
        {
            o->Angle[1] += (30.f) * FPS_ANIMATION_FACTOR;

            Vector(Luminosity * 0.2f, Luminosity * 0.4f, Luminosity * 1.f, Light);
            AddTerrainLight(o->StartPosition[0], o->StartPosition[1], Light, 2, PrimaryTerrainLight);

            if (o->SubType != 0)
            {
                if ((int)o->LifeTime == 13)
                {
                    CreateEffectFpsChecked(MODEL_PIERCING, o->Position, o->Angle, o->Light, 0, o);
                }
                else if ((int)o->LifeTime == 30)
                {
                    o->AttackPoint[0] = 0;
                    o->Kind = 1;
                }
            }
        }
        return true;
    }

    // MODEL_ARROW
    bool Move_MODEL_ARROW(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
        Vector(Luminosity * 0.8f, Luminosity * 0.5f, Luminosity * 0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);

        if (o->SubType == 3 || o->SubType == 4)
        {
            if (o->SubType == 3)
            {
                o->Angle[0] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            }
            if (o->Angle[0] > 50)
            {
                o->Angle[0] = 50.f;
            }

            int PositionX = (int)(o->Position[0] / TERRAIN_SCALE);
            int PositionY = (int)(o->Position[1] / TERRAIN_SCALE);
            int WallIndex = TERRAIN_INDEX_REPEAT(PositionX, PositionY);
            int Wall = TerrainWall[WallIndex] & TW_NOGROUND;

            if (Wall != TW_NOGROUND)
            {
                float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
                if (o->Position[2] < Height)
                {
                    o->Position[2] = Height + 10;
                    Vector(0.f, 0.f, 0.f, o->Direction);
                }
            }
            CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light, 5);
        }
        else if (o->SubType == 5)
        {
            Vector(Luminosity * 0.1f, Luminosity * 0.6f, Luminosity * 0.3f, Light);
            CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light);
            CheckClientArrow(o);
        }
        else
        {
            CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light);
            CheckClientArrow(o);
        }
        return true;
    }

    // MODEL_ARROW_STEEL, MODEL_ARROW_THUNDER, MODEL_ARROW_LASER, MODEL_ARROW_V, MODEL_ARROW_SAW, MODEL_ARROW_NATURE, MODEL_ARROW_WING, MODEL_LACEARROW
    bool Move_MODEL_ARROW_STEEL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        if (o->Type == MODEL_ARROW_NATURE)
        {
            if (o->SubType == 1) {
                Vector(0.1f, 0.4f, 0.1f, Light);
                CreateSprite(BITMAP_LIGHTNING + 1, o->Position, 0.3f, Light, o, (int)WorldTime * 0.1f);
                CreateSprite(BITMAP_LIGHTNING + 1, o->Position, 0.7f, Light, o, -(int)WorldTime * 0.1f);

                Vector(Luminosity * 0.2f, Luminosity * 0.8f, Luminosity * 0.2f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);

                o->Angle[1] += (60.f) * FPS_ANIMATION_FACTOR;

                if (o->LifeTime > 25 && ((int)o->LifeTime % 2) == 0)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        Vector((float)(rand() % 32 - 16), (float)(rand() % 64 - 32), (float)(rand() % 32 - 16), Position);
                        VectorAdd(Position, o->Position, Position);

                        Vector(0.4f, 1.f, 0.2f, Light);
                        CreateParticleFpsChecked(BITMAP_FLARE, Position, o->Angle, Light, 5, 0.2f);
                    }
                }
            }
            else
            {
                CreateSprite(BITMAP_LIGHTNING + 1, o->Position, 0.5f, o->Light, o, (int)WorldTime * 0.1f);
                CreateSprite(BITMAP_LIGHTNING + 1, o->Position, 1.f, o->Light, o, -(int)WorldTime * 0.1f);
                for (int j = 0; j < 4; j++)
                {
                    Vector((float)(rand() % 32 - 16), (float)(rand() % 64 - 32), (float)(rand() % 32 - 16), Position);
                    VectorAdd(Position, o->Position, Position);
                    CreateParticleFpsChecked(BITMAP_FLOWER01 + rand() % 3, Position, o->Angle, o->Light);
                    //CreateParticle(BITMAP_BUBBLE,Position,o->Angle,o->Light);
                }
                o->Angle[1] += (30.f) * FPS_ANIMATION_FACTOR;

                Vector(Luminosity * 0.6f, Luminosity * 0.8f, Luminosity * 0.8f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            }
        }
        else if (o->Type == MODEL_LACEARROW)
        {
            o->Angle[1] += (60.f) * FPS_ANIMATION_FACTOR;

            if (((int)o->LifeTime % 2) == 0)
            {
                for (int j = 0; j < 3; j++)
                {
                    Vector((float)(rand() % 32 - 16), (float)(rand() % 64 - 32), (float)(rand() % 32 - 16), Position);
                    VectorAdd(Position, o->Position, Position);

                    Vector(0.4f, 0.2f, 1.f, Light);
                    CreateParticleFpsChecked(BITMAP_FLARE, Position, o->Angle, Light, 5, 0.2f);
                }
            }
            VectorCopy(o->Position, o->EyeLeft);
            CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 3, NULL, 0);

            Vector(Luminosity * 0.6f, Luminosity * 0.2f, Luminosity * 0.8f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        }
        else if (o->Type == MODEL_ARROW_WING)
        {
            CreateSprite(BITMAP_LIGHTNING + 1, o->Position, 0.5f, o->Light, o, (int)WorldTime * 0.1f);
            CreateSprite(BITMAP_LIGHTNING + 1, o->Position, 1.f, o->Light, o, -(int)WorldTime * 0.1f);
            for (int j = 0; j < 4; j++)
            {
                Vector((float)(rand() % 16 - 8), (float)(rand() % 16 - 8), (float)(rand() % 16 - 8), Position);
                VectorAdd(Position, o->Position, Position);
                CreateParticleFpsChecked(BITMAP_BUBBLE, Position, o->Angle, o->Light, 1);
            }
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 0);

            Vector(Luminosity * 0.6f, Luminosity * 0.8f, Luminosity * 0.8f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        }
        CheckClientArrow(o);
        return true;
    }

    // MODEL_DARK_SCREAM_FIRE, MODEL_DARK_SCREAM
    bool Move_MODEL_DARK_SCREAM_FIRE(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->Type == MODEL_DARK_SCREAM_FIRE)
        {
            o->Scale -= (0.14f) * FPS_ANIMATION_FACTOR;
        }
        if (o->Type == MODEL_DARK_SCREAM)
        {
            o->Scale -= (0.04f) * FPS_ANIMATION_FACTOR;
        }
        if (o->Scale < 0.1f)
        {
            o->Scale = 0.f;
        }

        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 3.f;

        CreateParticleFpsChecked(BITMAP_FLAME, o->Position, o->Angle, o->Light, 8, (o->Scale - 0.4f) * 3.5f);
        if (o->Type == MODEL_DARK_SCREAM)
        {
            CheckClientArrow(o);
        }
    }
        return true;
    }

    // MODEL_CURSEDTEMPLE_HOLYITEM
    bool Move_MODEL_CURSEDTEMPLE_HOLYITEM(OBJECT* o, int index, float Luminosity)
    {
    {
        if (!o->Owner->Live)
        {
            o->Live = false;
            return true;
        }

        if (o->LifeTime < 10) o->LifeTime = 999999;

        BMD* b = &Models[o->Owner->Type];
        vec3_t vRelativePos, vWorldPos;
        Vector(70.f, 0.f, 0.f, vRelativePos);

        VectorCopy(o->Owner->Position, b->BodyOrigin);
        b->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, vWorldPos, true);

        //vWorldPos[0] = o->Owner->Position[0];
        //vWorldPos[1] = o->Owner->Position[1];

        Vector(1.0f, 1.0f, 1.0f, o->Light);
        VectorCopy(vWorldPos, o->Position);

        b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 3.f, o->Position, o->Angle);
    }
        return true;
    }

    // MODEL_CURSEDTEMPLE_PRODECTION_SKILL
    bool Move_MODEL_CURSEDTEMPLE_PRODECTION_SKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
    {
        if (!o->Owner->Live)
        {
            o->Live = false;
            return true;
        }

        if (o->LifeTime < 10) o->LifeTime = 999999;

        vec3_t RelativePos, Position;
        BMD* herobmd = &Models[o->Owner->Type];
        VectorCopy(o->Owner->Position, herobmd->BodyOrigin);
        Vector(-23.f, 0.f, 0.f, RelativePos);
        herobmd->TransformPosition(o->Owner->BoneTransform[2], RelativePos, Position, true);

        Vector(1.0f, 1.0f, 1.0f, o->Light);
        VectorCopy(Position, o->Position);
        VectorCopy(o->Owner->Angle, o->Angle);

        o->Alpha = 0.3f;
        o->Scale = 1.0f;

        BMD* effbmd = &Models[o->Type];

        effbmd->SetBodyLight(o->Light);
        effbmd->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 3.f, o->Position, o->Angle);

        CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Owner->Position, o->Angle, o->Light, 10, o->Owner);
    }
        return true;
    }

    // MODEL_CURSEDTEMPLE_RESTRAINT_SKILL
    bool Move_MODEL_CURSEDTEMPLE_RESTRAINT_SKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
    {
        if (!o->Owner->Live)
        {
            o->Live = false;
            return true;
        }

        if (o->LifeTime < 10) o->LifeTime = 999999;

        Vector(1.0f, 1.0f, 1.0f, o->Light);
        VectorCopy(o->Owner->Position, o->Position);
        VectorCopy(o->Owner->Angle, o->Angle);
        o->Alpha = 0.6f;
        o->Scale = 1.0f;

        BMD* b = &Models[o->Type];
        b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 3.f, o->Position, o->Angle);

        vec3_t vRelativePos, Light;
        Vector(0.4f, 0.4f, 0.8f, Light);
        Vector(0.f, 0.f, 0.f, vRelativePos);
    }
        return true;
    }

    // MODEL_ARROW_SPARK
    bool Move_MODEL_ARROW_SPARK(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Angle[1] += (35.0f) * FPS_ANIMATION_FACTOR;
        CheckClientArrow(o);
    }
        return true;
    }

    // MODEL_ARROW_RING
    bool Move_MODEL_ARROW_RING(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
    {
        VectorCopy(o->Position, o->EyeLeft);
        Vector(0.0f, 1.0f, 0.1f, o->Light);
        CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 4, NULL, 0);

        Vector(Luminosity * 0.6f, Luminosity * 0.2f, Luminosity * 0.8f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);

        CheckClientArrow(o);
    }
        return true;
    }

    // MODEL_ARROW_TANKER
    bool Move_MODEL_ARROW_TANKER(OBJECT* o, int index, float Luminosity)
    {
    {
        Vector(1.0f, 0.0f, 0.0f, o->Light);
        AddTerrainLight(o->Position[0], o->Position[1], o->Light, 2, PrimaryTerrainLight);
    }
        return true;
    }

    // MODEL_ARROW_BOMB
    bool Move_MODEL_ARROW_BOMB(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
    {
        Vector(1.f, 0.6f, 0.4f, Light);
        CreateSprite(BITMAP_LIGHT, o->Position, 1.f, Light, o, (int)WorldTime * 0.1f);
        CreateSprite(BITMAP_LIGHT, o->Position, 2.f, Light, o, -(int)WorldTime * 0.1f);

        for (int j = 0; j < 4; j++)
        {
            Vector((float)(rand() % 16 - 8), (float)(rand() % 16 - 8), (float)(rand() % 16 - 8), Position);
            VectorAdd(Position, o->Position, Position);
            CreateParticleFpsChecked(BITMAP_BUBBLE, Position, o->Angle, o->Light, 1);
        }
        MoveJump(o);
        if ((int)o->LifeTime == 1)
        {
            CreateBomb(o->Position, true);
            if (o->Owner == &Hero->Object && o->SubType != 99)
                AttackCharacterRange(o->Skill, o->Position, 100.f, o->Weapon, o->PKKey);
        }

        Vector(Luminosity * 0.6f, Luminosity * 0.8f, Luminosity * 0.8f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        CheckClientArrow(o);
    }
        return true;
    }

    // MODEL_ARROW_DARKSTINGER
    bool Move_MODEL_ARROW_DARKSTINGER(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
    {
        CheckClientArrow(o);

        BMD* pModel = &Models[o->Type];

        int iNumCreateFeather = rand() % 2;

        vec3_t vPos;

        Vector(0.6f, 0.7f, 0.9f, Light);
        pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);
        pModel->TransformByObjectBone(vPos, o, 0);

        float Matrix[3][4];
        vec3_t Angle;
        vec3_t Position;
        VectorCopy(o->Angle, Angle);
        AngleMatrix(Angle, Matrix);
        VectorRotate(o->Direction, Matrix, Position);
        VectorAdd(vPos, Position, vPos);

        CreateSprite(BITMAP_LIGHT, vPos, 3.0f, Light, o);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, Light, o);

        if ((int)o->LifeTime == 30 || (int)o->LifeTime == 28 || (int)o->LifeTime == 26 || (int)o->LifeTime == 24)
        {
            int iNumCreateFeather = rand() % 3;
            Vector(0.6f, 0.7f, 0.9f, Light);
            pModel->TransformByObjectBone(vPos, o, 1);
            for (int i = 0; i < iNumCreateFeather; i++)
            {
                CreateEffectFpsChecked(MODEL_FEATHER, vPos, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.6f);
                CreateEffectFpsChecked(MODEL_FEATHER, vPos, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            }
        }

        if ((int)o->LifeTime == 30)
        {
            pModel->TransformByObjectBone(vPos, o, 1);
            Vector(0.4f, 0.4f, 0.9f, Light);
            CreateJointFpsChecked(BITMAP_FLARE + 1, vPos, vPos, o->Angle, 18, o, 90.f, 40, 0, 0, -1, Light);
        }
    }
        return true;
    }

    // MODEL_DUNGEON_STONE01
    bool Move_MODEL_DUNGEON_STONE01(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
        o->Gravity -= (1.f) * FPS_ANIMATION_FACTOR;
        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.4f;
            o->LifeTime -= (4) * FPS_ANIMATION_FACTOR;
            o->Direction[1] -= (2.f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_WARCRAFT
    bool Move_MODEL_WARCRAFT(OBJECT* o, int index, float Luminosity)
    {
        VectorCopy(o->Owner->Position, o->Position);
        return true;
    }

    // BITMAP_FIRECRACKERRISE
    bool Move_BITMAP_FIRECRACKERRISE(OBJECT* o, int index, float Luminosity)
    {
        if (0 == ((int)o->LifeTime % 5) && (0 == (rand() % 3)))
        {
            CreateEffectFpsChecked(BITMAP_FIRECRACKER, o->Position, o->Angle, o->Light);
        }
        return true;
    }

    // BITMAP_FIRECRACKER
    bool Move_BITMAP_FIRECRACKER(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (1 == o->LifeTime)
        {
            int iSubType = rand() % 30;
            Vector((rand() % 3) * .3f + .4f, (rand() % 4) * .1f, .0f, Light);
            //for ( int j = 0; j < 200; ++j)
            for (int j = 0; j < 80 * FPS_ANIMATION_FACTOR; ++j)
            {
                CreateParticleFpsChecked(BITMAP_FIRECRACKER, o->Position, o->Angle, Light, iSubType);
            }
            PlayBuffer(SOUND_FIRECRACKER2, o);
        }
        CreateParticleFpsChecked(BITMAP_FIRECRACKER, o->Position, o->Angle, o->Light, -1);
        Vector(Luminosity * .4f, Luminosity * 0.3f, Luminosity * 0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight);
        return true;
    }

    // BITMAP_FIRECRACKER0001
    bool Move_BITMAP_FIRECRACKER0001(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
    {
        if ((int)o->LifeTime == 1 || (int)o->LifeTime == 9 || (int)o->LifeTime == 17
            || (int)o->LifeTime == 24 || (int)o->LifeTime == 31)
        {
            Vector(o->Position[0] + (rand() % 200 - 100), o->Position[1] + (rand() % 200 - 100),
                o->Position[2], Position);
            CreateJointFpsChecked(BITMAP_JOINT_SPIRIT, Position, Position, o->Angle, 25, o, 1.f, -1, o->SubType);
        }
    }
        return true;
    }

    // BITMAP_FIRECRACKER0002
    bool Move_BITMAP_FIRECRACKER0002(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
    {
        if (rand_fps_check(5))
        {
            vec3_t vLight;
            Vector(0.3f + (rand() % 700) * 0.001f, 0.3f + (rand() % 700) * 0.001f, 0.3f + (rand() % 700) * 0.001f, vLight);
            Vector(o->Position[0] + (rand() % 300 - 150), o->Position[1] + (rand() % 300 - 150),
                o->Position[2], Position);
            float fScale = 0.5f + (rand() % 5) * 0.1f;
            CreateEffectFpsChecked(BITMAP_FIRECRACKER0003, Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, fScale);
        }
    }
        return true;
    }

    // BITMAP_FIRECRACKER0003
    bool Move_BITMAP_FIRECRACKER0003(OBJECT* o, int index, float Luminosity)
    {
    {
        int iFrame;
        if (o->LifeTime > 15 - 8)
        {
            iFrame = (15 - o->LifeTime) / 8.0f * 7;
            if (iFrame >= 7) iFrame = 6;

            if (iFrame == 2)
            {
                // 					CreateSprite(BITMAP_DS_SHOCK, o->Position, o->Scale*1.5f, o->Light, o);
            }
        }
        else
        {
            iFrame = 6;
            o->Position[2] -= (1.0f) * FPS_ANIMATION_FACTOR;
            o->Light[0] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            o->Scale *= pow(1.02f, FPS_ANIMATION_FACTOR);
        }
        CreateSprite(BITMAP_FIRECRACKER0001 + iFrame, o->Position, o->Scale, o->Light, o, o->Angle[2]);
    }
        return true;
    }

    // BITMAP_SWORD_FORCE
    bool Move_BITMAP_SWORD_FORCE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        if (o->LifeTime == 30.f) // at the first frame of the effect
        {
            VectorCopy(o->Position, Position);
            Position[2] += 100.f;
            if (o->SubType == 1)
            {
                vec3_t Light;
                Vector(1.f, 1.f, 1.f, Light);
                CreateJointFpsChecked(BITMAP_JOINT_FORCE, Position, Position, o->HeadAngle, 10, o->Owner, 150.f, o->PKKey, o->Skill, 0, -1, Light);
            }
            else
                if (o->SubType == 0)
                    CreateJointFpsChecked(BITMAP_JOINT_FORCE, Position, Position, o->HeadAngle, 0, o->Owner, 150.f, o->PKKey, o->Skill);
                else
                    CreateJointFpsChecked(BITMAP_JOINT_FORCE, Position, Position, o->HeadAngle, 8, o->Owner, 150.f, o->PKKey, o->Skill);
        }
        return true;
    }

    // BITMAP_BLIZZARD
    bool Move_BITMAP_BLIZZARD(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Height;
        if (o->LifeTime <= 15)
        {
            if (o->SubType == 0)
            {
                o->Position[0] = o->StartPosition[0] + sinf((rand() % 1000) * 0.01f) * 10.f;
                o->Position[1] = o->StartPosition[1] + sinf((rand() % 1000) * 0.01f) * 10.f;
                o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity -= (2.f) * FPS_ANIMATION_FACTOR;

                o->StartPosition[0] -= (10.f) * FPS_ANIMATION_FACTOR;

                CreateParticleFpsChecked(BITMAP_FIRE + 2, o->Position, o->Angle, Light, 7, o->Scale);

                o->Light[0] += (0.1f) * FPS_ANIMATION_FACTOR;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 4 + 4) * 0.2f, o->Light, o, (float)(rand() % 360));
                CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, o, (float)(rand() % 360));
            }
            else if (o->SubType == 1)
            {
                o->Position[0] = o->StartPosition[0];
                o->Position[1] = o->StartPosition[1];
                o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity -= (2.f) * FPS_ANIMATION_FACTOR;

                o->StartPosition[0] -= (10.f) * FPS_ANIMATION_FACTOR;

                CreateParticleFpsChecked(BITMAP_FIRE + 2, o->Position, o->Angle, Light, 11, o->Scale);

                o->Light[0] += (0.1f) * FPS_ANIMATION_FACTOR;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 4 + 4) * 0.2f, o->Light, o, (float)(rand() % 360));
                CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, o, (float)(rand() % 360));

                Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
                if (o->Position[2] < Height)
                {
                    Vector(0.24f, 0.28f, 0.8f, Light);
                    VectorCopy(o->Position, Position);
                    Position[2] += 50.f;
                    CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 11, (float)(rand() % 32 + 80) * 0.025f);

                    if (rand_fps_check(5))
                        CreateEffectFpsChecked(MODEL_ICE_SMALL, Position, o->Angle, o->Light);
                }
            }
        }
        return true;
    }

    // BITMAP_SHOTGUN
    bool Move_BITMAP_SHOTGUN(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
    {
        Vector(Luminosity * 0.5f, Luminosity * 0.5f, Luminosity * 0.8f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);

        Vector(1.f, 1.f, 1.f, Light);
        VectorCopy(o->Angle, Angle);
        Angle[2] += 90.f;

        o->Scale = ((15 - o->LifeTime) / 20.f) * (rand() % 3 + 2);

        AngleMatrix(Angle, Matrix);
        Vector(0.f, 20.f, 0.f, p);
        VectorRotate(p, Matrix, Position);
        VectorAdd(o->Position, Position, o->StartPosition);
        CreateParticleFpsChecked(BITMAP_FIRE + 2, o->StartPosition, o->Angle, Light, 10, o->Scale);

        Vector(0.f, -20.f, 0.f, p);
        VectorRotate(p, Matrix, Position);
        VectorAdd(o->Position, Position, o->StartPosition);
        CreateParticleFpsChecked(BITMAP_FIRE + 2, o->StartPosition, o->Angle, Light, 10, o->Scale);

        if ((int)o->LifeTime == 1)
        {
            CreateBomb2(o->Position, false);
        }
    }
        return true;
    }

    // MODEL_SHINE
    bool Move_MODEL_SHINE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        if (o->SubType == 0 && o->Owner != NULL && o->LifeTime > 10)
        {
            Position[0] = (float)(rand() % 128 - 64) - 50.f;
            Position[1] = Position[0] + 100.f;
            Position[2] = 0.f;
            VectorAdd(Position, o->Owner->Position, Position);
            Position[2] += 360.f;

            float Scale = (rand() % 30 + 50) / 100.f;
            CreateParticleFpsChecked(BITMAP_SHINY, Position, o->Angle, o->Light, 2, Scale);
        }
        return true;
    }

    // MODEL_BLIZZARD
    bool Move_MODEL_BLIZZARD(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Height;
        if (o->SubType == 0)
        {
            o->Position[0] = o->StartPosition[0] + sinf((rand() % 1000) * 0.01f) * 10.f;
            o->Position[1] = o->StartPosition[1] + sinf((rand() % 1000) * 0.01f) * 10.f;
            o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity -= (rand() % 5) * FPS_ANIMATION_FACTOR;

            o->StartPosition[0] -= (10.f) * FPS_ANIMATION_FACTOR;

            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 0, 1.5f);
            if (rand_fps_check(2))
            {
                CreateParticleFpsChecked(BITMAP_ENERGY, o->Position, o->Angle, Light, 1, 0.5f);
            }
            else
            {
                CreateParticleFpsChecked(BITMAP_FIRE + 2, o->Position, o->Angle, Light, 7, o->Scale);
            }

            o->Light[0] += (0.1f) * FPS_ANIMATION_FACTOR;
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
            CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 4 + 4) * 0.2f, o->Light, o, (float)(rand() % 360));
            CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, o, (float)(rand() % 360));

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                Vector(0.24f, 0.28f, 0.8f, Light);
                VectorCopy(o->Position, Position);
                Position[2] += 50.f;
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 11, (float)(rand() % 32 + 80) * 0.025f);

                if (rand_fps_check(5))
                {
                    CreateEffectFpsChecked(MODEL_ICE_SMALL, Position, o->Angle, o->Light);
                }

                CreateEffectFpsChecked(MODEL_BLIZZARD, Position, o->Angle, o->Light, 1, NULL, o->PKKey);

                o->Live = false;
            }
        }
        else if (o->SubType == 1)
        {
            o->BlendMeshLight *= pow(1.0f / (1.1f), FPS_ANIMATION_FACTOR);

            if ((int)o->LifeTime == 18 && o->PKKey == 1)
                PlayBuffer(SOUND_SUDDEN_ICE2);
        }
        else if (o->SubType == 2)
        {
            o->Position[0] = o->StartPosition[0] + sinf((rand() % 1000) * 0.01f) * 50.f;
            o->Position[1] = o->StartPosition[1] + sinf((rand() % 1000) * 0.01f) * 50.f;
            o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity -= (rand() % 5) * FPS_ANIMATION_FACTOR;

            o->StartPosition[0] -= (10.f) * FPS_ANIMATION_FACTOR;

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] > Height)
            {
                if (rand_fps_check(2))
                {
                    CreateParticleFpsChecked(BITMAP_FIRE + 2, o->Position, o->Angle, Light, 7, o->Scale);
                }

                o->Light[0] += (0.1f) * FPS_ANIMATION_FACTOR;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                CreateSprite(BITMAP_SHINY + 1, o->Position, (float)(rand() % 4 + 4) * 0.2f, o->Light, o, (float)(rand() % 360));
                CreateSprite(BITMAP_LIGHT, o->Position, 1.f, o->Light, o, (float)(rand() % 360));
            }
        }
        return true;
    }

    // MODEL_ARROW_DRILL
    bool Move_MODEL_ARROW_DRILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        if (o->SubType == 0 || o->SubType == 2)
        {
            o->Angle[1] += (30.f) * FPS_ANIMATION_FACTOR;

            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 13);
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 13);
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 13);

            if ((int)o->LifeTime == 1)
            {
                CreateBomb(o->Position, true);
            }

            VectorCopy(o->Position, o->EyeLeft);
            VectorCopy(o->Angle, Angle);
            AngleMatrix(Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAdd(o->EyeLeft, Position, o->EyeLeft);

            Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.2f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            CheckClientArrow(o);
        }
        return true;
    }

    // MODEL_COMBO
    bool Move_MODEL_COMBO(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            if (o->LifeTime > 4)
            {
                o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;
            }
            o->BlendMeshLight *= pow(1.0f / (1.4f), FPS_ANIMATION_FACTOR);
        }
        return true;
    }

    // MODEL_WAVES
    bool Move_MODEL_WAVES(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            if (o->LifeTime > 4)
            {
                o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;
            }
            o->BlendMeshLight *= pow(1.0f / (1.4f), FPS_ANIMATION_FACTOR);
        }
        else if (o->SubType == 1)
        {
            o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.07f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 2.f) o->Scale = 2.f;
            o->BlendMeshLight *= pow(1.0f / (1.5f), FPS_ANIMATION_FACTOR);
        }
        else if (o->SubType == 2)
        {
            o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.01f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 1.5f) o->Scale = 1.5f;
            o->BlendMeshLight *= pow(1.0f / (1.5f), FPS_ANIMATION_FACTOR);
        }
        else if (o->SubType == 3)
        {
            o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.005f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 1.5f) o->Scale = 1.5f;
            o->BlendMeshLight *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            o->Angle[1] += (45.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 4)
        {
            o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.002f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 2.5f) o->Scale = 2.5f;
            o->BlendMeshLight *= pow(1.0f / (1.2f), FPS_ANIMATION_FACTOR);
            o->Angle[1] += (45.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 5)
        {
            o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.015f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 2.5f) o->Scale = 2.5f;
            o->BlendMeshLight *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            o->Angle[1] += (45.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 6)
        {
            o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.015f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 2.5f) o->Scale = 2.5f;
            o->BlendMeshLight *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            o->Angle[1] += (45.f) * FPS_ANIMATION_FACTOR;
        }
        return true;
    }

    // MODEL_AIR_FORCE
    bool Move_MODEL_AIR_FORCE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
        if (o->Owner == NULL)
        {
            o->Live = false;
            return false;
        }
        if (o->SubType == 0)
        {
            o->BlendMeshLight *= pow(1.0f / (1.5f), FPS_ANIMATION_FACTOR);
            o->Scale += (0.2f) * FPS_ANIMATION_FACTOR;
        }

        if (o->SubType == 1)
        {
            o->BlendMeshLight = o->LifeTime / 10.f;

            VectorCopy(o->Owner->Position, o->Position);
            VectorCopy(o->Owner->Angle, o->Angle);
            Vector(0.f, -70.f, 0.f, p);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(p, Matrix, Position);
            Position[2] += 100.f;
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
        }
        return true;
    }

    // MODEL_PIERCING2
    bool Move_MODEL_PIERCING2(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        float Matrix[3][4];
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(o->Direction, Matrix, Position);
        //		VectorAdd(o->Position,Position,o->Position);

        o->Direction[1] += (12.f) * FPS_ANIMATION_FACTOR;
        if (o->Direction[1] >= 0)
        {
            o->Direction[1] = 0;
        }

        if (o->SubType == 1) {
            if (o->LifeTime > 1) {
                o->BlendMeshLight *= pow(1.0f / (1.6f), FPS_ANIMATION_FACTOR);
                CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 2, NULL, o->LifeTime);
            }
        }
        else
            if (o->SubType == 2)
            {
                //			o->Direction[1] -= 10.f;
                if (o->LifeTime > 1)
                {
                    o->BlendMeshLight *= pow(1.0f / (1.6f), FPS_ANIMATION_FACTOR);
                    CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 2, NULL, o->LifeTime);
                }
            }
            else {
                if (o->LifeTime > 5) {
                    o->BlendMeshLight *= pow(1.0f / (1.6f), FPS_ANIMATION_FACTOR);
                    CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 2, NULL, o->LifeTime);
                }
            }
        return true;
    }

    // MODEL_DEASULER
    bool Move_MODEL_DEASULER(OBJECT* o, int index, float Luminosity)
    {
    {
        vec3_t	v3RotateAngleRelative;
        o->Visible = true;

        float	fCurrentRate = 1.0f - ((float)o->LifeTime / (float)o->ExtState);

        if (o->m_Interpolates.m_vecInterpolatesAngle.size() > 0)
        {
            o->m_Interpolates.GetAngleCurrent(v3RotateAngleRelative, fCurrentRate);

            o->Angle[0] = v3RotateAngleRelative[0] + o->HeadAngle[0];
            o->Angle[1] = v3RotateAngleRelative[1] + o->HeadAngle[1];
            o->Angle[2] = v3RotateAngleRelative[2] + o->HeadAngle[2];
        }

        if (o->m_Interpolates.m_vecInterpolatesPos.size() > 0)
        {
            o->m_Interpolates.GetPosCurrent(o->Position, fCurrentRate);
        }

        if (o->m_Interpolates.m_vecInterpolatesScale.size() > 0)
        {
            o->m_Interpolates.GetScaleCurrent(o->Scale, fCurrentRate);
        }

        if (o->m_Interpolates.m_vecInterpolatesScale.size() > 0)
        {
            o->m_Interpolates.GetAlphaCurrent(o->Alpha, fCurrentRate);
        }

        float fRateBlurStart, fRateBlurEnd, fRateShadowStart, fRateShadowEnd, fRateJointStart, fRateJointEnd;
        fRateBlurStart = fRateBlurEnd = 0.0f;
        fRateShadowStart = fRateShadowEnd = 0.0f;
        fRateJointStart = fRateJointEnd = 0.0f;
        fRateBlurStart = 0.0f; fRateBlurEnd = 0.90f;

        int		iTYPESWORDFORCE = 0;		// 1: FORCE OF SWORD
        int		iTYPESWORDSHADOW = 0;		// 1: SHADOW SWORD

        iTYPESWORDFORCE = 1;
        iTYPESWORDSHADOW = 0;

        if (iTYPESWORDFORCE == 1)
        {
            if (fCurrentRate > fRateBlurStart && fCurrentRate < fRateBlurEnd)
            {
                BMD* b = &Models[o->Type];
                vec3_t  vLightBlur; Vector(1.0f, 1.0f, 1.0f, vLightBlur);
                float	fPreRate = 1.0f - (float)((o->LifeTime) + 1) / (float)(o->ExtState);
                SETLIMITS(fPreRate, 1.0f, 0.0f);
                if (fPreRate < fCurrentRate)
                {
                    for (int Loop_bk = 0; Loop_bk < 2; Loop_bk++)
                    {
                        float	fStartRate, fEndRate;

                        fStartRate = 1.0f - (float)((o->LifeTime) + 2) / (float)(o->ExtState);
                        fEndRate = 1.0f - (float)((o->LifeTime) + 1) / (float)(o->ExtState);

                        SETLIMITS(fStartRate, 1.0f, 0.0f);
                        SETLIMITS(fEndRate, 1.0f, 0.0f);

                        vec3_t* arrEachBonePos;
                        arrEachBonePos = new vec3_t[b->NumBones];

                        vec3_t v3CurBlurAngle, v3CurBlurPos;
                        int	iAccess = 20;
                        int iBone01, iBone02;
                        int iBlurIdentity, iTypeBlur;
                        float fUnit;
                        float fCurrentRateUnit = fStartRate;
                        float fScale = o->Scale;

                        if (Loop_bk == 0)
                        {
                            iBone01 = 4;
                            iBone02 = 2;
                            iBlurIdentity = 113;
                        }
                        else
                        {
                            iBone01 = 5;
                            iBone02 = 1;
                            iBlurIdentity = 114;
                        }
                        iTypeBlur = 10;

                        fUnit = (fEndRate - fStartRate) / (float)iAccess;

                        for (int i = 0; i < iAccess; i++)
                        {
                            fCurrentRateUnit += fUnit;

                            o->m_Interpolates.GetAngleCurrent(v3CurBlurAngle, fCurrentRateUnit);
                            o->m_Interpolates.GetPosCurrent(v3CurBlurPos, fCurrentRateUnit);

                            VectorAdd(v3CurBlurAngle, o->HeadAngle, v3CurBlurAngle);

                            b->AnimationTransformOnlySelf(
                                arrEachBonePos, v3CurBlurAngle, v3CurBlurPos, fScale);

                            CreateObjectBlur(o,
                                arrEachBonePos[iBone01],
                                arrEachBonePos[iBone02],
                                vLightBlur, iTypeBlur, false, iBlurIdentity, 20);

                            if (rand_fps_check(10))
                            {
                                vec3_t	vAngle, vRandomDir, vRandomDirPosition, vResultRandomPosition;
                                vec34_t	matRandomRotation;
                                vec3_t	vPosition;

                                VectorCopy(arrEachBonePos[1], vPosition);

                                float	fRandDistance = (float)(rand() % 100) + 100;
                                Vector(0.0f, fRandDistance, 0.0f, vRandomDir);

                                CreateParticleFpsChecked(BITMAP_FIRE + 2, vPosition, o->Angle, o->Light, 17, 1.35f);

                                Vector((float)(rand() % 360), 0.f, (float)(rand() % 360), vAngle);
                                AngleMatrix(vAngle, matRandomRotation);
                                VectorRotate(vRandomDir, matRandomRotation, vRandomDirPosition);
                                VectorAdd(vPosition, vRandomDirPosition, vResultRandomPosition);
                                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, vResultRandomPosition, vPosition, vAngle, 3, NULL, 10.f, 10, 10);
                            }
                        }
                        delete[] arrEachBonePos;
                    }
                }	// if( fPreRate < fCurrentRate )
            }
        }
    }
        return true;
    }

    // MODEL_DEATH_SPI_SKILL
    bool Move_MODEL_DEATH_SPI_SKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
    {
        if (o->SubType == 0)
        {
            if (o->Owner != NULL)
            {
                VectorCopy(o->Owner->Position, p);
                VectorAdd(p, o->StartPosition, p);

                float Distance;
                for (int i = 1; i < o->Gravity; ++i)
                {
                    if (rand_fps_check(2))
                    {
                        if (o->Angle[0] < -90)
                            o->Angle[0] += (20.f) * FPS_ANIMATION_FACTOR;
                        else
                            o->Angle[0] -= (20.f) * FPS_ANIMATION_FACTOR;
                    }
                    Distance = MoveHumming(o->Position, o->Angle, p, o->Velocity);
                    o->Velocity += (0.4f) * FPS_ANIMATION_FACTOR;

                    if (o->LifeTime < 10)
                    {
                        o->Velocity += (0.1f) * FPS_ANIMATION_FACTOR;
                        CreateEffectFpsChecked(BITMAP_MAGIC + 1, Position, o->Angle, o->Light, 1, o);
                    }

                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(o->Direction, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

                    CreateEffectFpsChecked(MODEL_TAIL, o->Position, o->Angle, o->Light, 0, o);
                }
                if (Distance < 40 && (int)o->LifeTime == 5)
                {
                    VectorCopy(o->Position, Position);
                    Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                }
                o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;

                PlayBuffer(SOUND_ATTACK_FIRE_BUST_EXP);
            }
        }
        else if (o->SubType == 1)
        {
            if (o->LifeTime < 5)
            {
                o->Alpha *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            }
        }
    }
        return true;
    }

    // MODEL_PIER_PART
    bool Move_MODEL_PIER_PART(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        vec3_t p;
        if (o->SubType == 0)
        {
            if (o->Owner != NULL)
            {
                VectorCopy(o->Owner->Position, p);
                VectorAdd(p, o->StartPosition, p);

                float Distance;
                for (int i = 1; i < o->Gravity; ++i)
                {
                    if (rand_fps_check(2))
                    {
                        if (o->Angle[0] < -90)
                            o->Angle[0] += (20.f) * FPS_ANIMATION_FACTOR;
                        else
                            o->Angle[0] -= (20.f) * FPS_ANIMATION_FACTOR;
                    }
                    Distance = MoveHumming(o->Position, o->Angle, p, o->Velocity);
                    o->Velocity += (0.4f) * FPS_ANIMATION_FACTOR;

                    if (o->LifeTime < 10)
                    {
                        o->Velocity += (0.1f) * FPS_ANIMATION_FACTOR;
                    }

                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(o->Direction, Matrix, Position);
                    VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

                    CreateEffectFpsChecked(MODEL_PIER_PART, o->Position, o->Angle, o->Light, 1, o);
                }
                if (Distance < 40 && (int)o->LifeTime == 5)
                {
                    VectorCopy(o->Position, Position);
                    Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                }
                o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;

                PlayBuffer(SOUND_ATTACK_FIRE_BUST_EXP);
            }
        }
        else if (o->SubType == 2)
        {
            if (o->Owner != NULL)
            {
                VectorCopy(o->Owner->Position, p);

                MoveHumming(o->Position, o->Angle, p, o->Velocity);
                o->Velocity += (2.4f) * FPS_ANIMATION_FACTOR;

                AngleMatrix(o->Angle, Matrix);
                VectorRotate(o->Direction, Matrix, Position);
                VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

                if ((int)o->LifeTime % 3 == 0)
                {
                    Vector(-90.f, 0.f, o->Angle[2], Angle);
                    CreateJointFpsChecked(BITMAP_JOINT_FORCE, o->Position, o->Position, Angle, 2, NULL, 150.f);
                }
            }
        }
        else if (o->SubType == 1)
        {
            if (o->LifeTime < 5)
            {
                o->Alpha *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            }
        }
        return true;
    }

    // BITMAP_FLARE_FORCE
    bool Move_BITMAP_FLARE_FORCE(OBJECT* o, int index, float Luminosity)
    {
        return true;
    }

    // MODEL_DARKLORD_SKILL
    bool Move_MODEL_DARKLORD_SKILL(OBJECT* o, int index, float Luminosity)
    {
        return true;
    }

    // MODEL_GROUND_STONE, MODEL_GROUND_STONE2
    bool Move_MODEL_GROUND_STONE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        if (o->LifeTime > 32 && o->LifeTime < 37)
        {
            Position[0] = o->Position[0] + 60;
            Position[1] = o->Position[1] - 60;
            Position[2] = o->Position[2] + 50.f;

            Vector(1.f, 0.8f, 0.6f, Light);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 11, 2.f, o);
            CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, Position, o->Angle, o->Light, 10);
        }
        //        o->BlendMeshTexCoordU = -(int)WorldTime%2000 * 0.0005f;

        if (o->LifeTime < 8)
        {
            o->Alpha *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            o->BlendMeshLight *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);
        }

        Vector(0.79f, 0.72f, 0.49f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        return true;
    }

    // BITMAP_TWLIGHT
    bool Move_BITMAP_TWLIGHT(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            o->Scale -= (0.1f) * FPS_ANIMATION_FACTOR;
            o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
        }
        else
            if (o->SubType == 1)
            {
                o->Scale -= (0.1f) * FPS_ANIMATION_FACTOR;
                o->Angle[2] += (5.f) * FPS_ANIMATION_FACTOR;
            }
            else
                if (o->SubType == 2)
                {
                    o->Scale -= (0.1f) * FPS_ANIMATION_FACTOR;
                    o->Angle[2] += (15.f) * FPS_ANIMATION_FACTOR;
                }
                else if (o->SubType == 3)
                {
                    VectorCopy(o->Owner->Position, o->Position);
                    o->Scale -= (0.15f) * FPS_ANIMATION_FACTOR;
                    o->Angle[2] += (10.f) * FPS_ANIMATION_FACTOR;
                    if (o->LifeTime >= 20)
                    {
                        o->Alpha += 0.1f * FPS_ANIMATION_FACTOR;
                        o->PKKey += FPS_ANIMATION_FACTOR;
                        o->Light[0] = o->EyeRight[0] * (o->PKKey * 0.1f);
                        o->Light[1] = o->EyeRight[1] * (o->PKKey * 0.1f);
                        o->Light[2] = o->EyeRight[2] * (o->PKKey * 0.1f);
                    }
                    else if (o->LifeTime <= 10)
                    {
                        o->PKKey -= FPS_ANIMATION_FACTOR;
                        o->Alpha -= 0.1f * FPS_ANIMATION_FACTOR;
                        o->Light[0] = o->EyeRight[0] * (o->PKKey * 0.1f);
                        o->Light[1] = o->EyeRight[1] * (o->PKKey * 0.1f);
                        o->Light[2] = o->EyeRight[2] * (o->PKKey * 0.1f);
                    }
                }
    }
        return true;
    }

    // BITMAP_SHOCK_WAVE
    bool Move_BITMAP_SHOCK_WAVE(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Scale -= (1.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 1)
        {
            o->Scale += ((rand() % 5) / 10.f) * FPS_ANIMATION_FACTOR;

            o->Position[0] += (rand() % 8 - 4.f) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (rand() % 8 - 4.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 2)
        {
            o->Scale += ((rand() % 5) / 40.f) * FPS_ANIMATION_FACTOR;

            o->Position[0] += (rand() % 8 - 4.f) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (rand() % 8 - 4.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 3)
        {
            o->Scale += (2.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 4)
        {
            o->Scale += (0.3f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 5)
        {
            o->Scale -= (0.4f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 6)
        {
            if (((int)o->LifeTime % 8) == 0)
            {
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Position, o->Angle, o->Light, 5);
            }
            return true;
        }
        else if (o->SubType == 7)
        {
            o->Scale += (2.5f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 8)
        {
            o->Scale += (1.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 9)
        {
            o->Scale += (0.8f) * FPS_ANIMATION_FACTOR;
            VectorCopy(o->Owner->Position, o->Position);
        }
        else if (o->SubType == 10)
        {
            o->Scale -= (0.02f) * FPS_ANIMATION_FACTOR;
            VectorCopy(o->Owner->Position, o->Position);
        }
        else if (o->SubType == 11)
        {
            o->Scale += ((rand() % 5) / 10.f) * FPS_ANIMATION_FACTOR;

            o->Position[0] += (rand() % 8 - 4.f) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (rand() % 8 - 4.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 12)
        {
            if (o->LifeTime > 4.0f)
                o->Scale += ((o->LifeTime - 4.0f) * 0.25f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 13)
        {
            o->Scale += (0.08f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 14)
        {
            VectorCopy(o->Owner->Position, o->Position);
            o->Scale -= (0.15f) * FPS_ANIMATION_FACTOR;
            if (o->LifeTime >= 20)
            {
                o->Alpha += 0.1f * FPS_ANIMATION_FACTOR;
                o->PKKey += FPS_ANIMATION_FACTOR;
                o->Light[0] = o->EyeRight[0] * (o->PKKey * 0.1f);
                o->Light[1] = o->EyeRight[1] * (o->PKKey * 0.1f);
                o->Light[2] = o->EyeRight[2] * (o->PKKey * 0.1f);
            }
            else if (o->LifeTime <= 10)
            {
                o->PKKey -= FPS_ANIMATION_FACTOR;
                o->Alpha -= 0.1f * FPS_ANIMATION_FACTOR;
                o->Light[0] = o->EyeRight[0] * (o->PKKey * 0.1f);
                o->Light[1] = o->EyeRight[1] * (o->PKKey * 0.1f);
                o->Light[2] = o->EyeRight[2] * (o->PKKey * 0.1f);
            }
            return true;
        }
        if (o->Scale < 0)
        {
            o->Scale = 0;
        }
        if (o->SubType >= 0 && o->SubType <= 3)
        {
            if (o->LifeTime <= 20)
            {
                Luminosity = o->LifeTime / 20.f;
                Vector(Luminosity, Luminosity, Luminosity, o->Light);
            }
            else
            {
                Luminosity = (40 - o->LifeTime) / 20.f;
                Vector(Luminosity, Luminosity, Luminosity, o->Light);
            }
        }
        else
        {
            if (o->LifeTime < 6)
            {
                o->Light[0] *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
            }
        }
        return true;
    }

    // BITMAP_DAMAGE_01_MONO
    bool Move_BITMAP_DAMAGE_01_MONO(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Scale += (5.f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 1)
        {
            o->Scale += (0.5f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 3.5f)
            {
                //o->Scale = 6.0f;

                o->Light[0] *= pow(0.5f, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(0.5f, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(0.5f, FPS_ANIMATION_FACTOR);
            }
        }
        return true;
    }

    // BITMAP_FLARE
    bool Move_BITMAP_FLARE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        if (o->SubType == 1)
        {
            CreateParticleFpsChecked(BITMAP_FLARE, o->Position, o->Angle, Light, 11, 1.0f);
        }
        else if (o->SubType == 2)
        {
            CreateParticleFpsChecked(BITMAP_FLARE_BLUE, o->Position, o->Angle, Light, 1, 1.0f);
        }
        else
            if (o->SubType == 3)
            {
                Vector(0.9f, 0.4f, 0.1f, Light);
                float fRandom = 3.5f + ((float)(rand() % 20 - 10) * 0.1f);
                CreateParticleFpsChecked(BITMAP_LIGHT, o->Position, o->Angle, Light, 5, fRandom);
            }
            else
                if ((int)o->LifeTime % 2 == 0)
                {
                    VectorCopy(o->Position, Position);
                    Position[2] += 100.f;
                    CreateJointFpsChecked(BITMAP_FLARE_BLUE, Position, Position, o->Angle, 14, o, 40.f);
                    CreateJointFpsChecked(BITMAP_FLARE_BLUE, Position, Position, o->Angle, 15, o, 40.f);

                    if (rand_fps_check(2) && o->LifeTime > 5 && o->LifeTime < 15)
                    {
                        Vector(o->Position[0] - 200.f, o->Position[1], o->Position[2] + 700.f, Position);
                        CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Position, o->Position, Angle, 0, o, 20.f);
                    }
                }
        return true;
    }

    // MODEL_CUNDUN_DRAGON_HEAD
    bool Move_MODEL_CUNDUN_DRAGON_HEAD(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
        vec3_t Position;
        o->Angle[2] += (10 + rand() % 10) * FPS_ANIMATION_FACTOR;

        if (o->Position[2] > 300 && o->Position[2] < 600)
        {
            vec3_t Angle;
            Angle[0] = Angle[1] = 0;
            Position[0] = o->Position[0];
            Position[1] = o->Position[1];
            Position[2] = 350;
            if (rand_fps_check(2))
            {
                Angle[2] = (float)(rand() % 24 * 30);
                CreateJointFpsChecked(BITMAP_JOINT_SPIRIT2, Position, Position, Angle, 14, NULL, 100.f, 0, 0);
            }
        }
        return true;
    }

    // MODEL_CUNDUN_PHOENIX
    bool Move_MODEL_CUNDUN_PHOENIX(OBJECT* o, int index, float Luminosity)
    {
        if (o->LifeTime < 5)
        {
            o->Alpha = o->LifeTime * 0.2f;
        }
        return true;
    }

    // MODEL_CUNDUN_GHOST
    bool Move_MODEL_CUNDUN_GHOST(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        if (o->Owner != NULL)
        {
            if (o->Owner->PKKey == 0)
            {
                o->AnimationFrame = 0;
            }
            else
            {
                o->Owner = NULL;
            }
        }
        else if (o->AnimationFrame > 6.0f)
        {
            o->PKKey += (1) * FPS_ANIMATION_FACTOR;
            o->Position[2] += ((o->PKKey * 0.8f)) * FPS_ANIMATION_FACTOR;

            VectorCopy(o->Position, Position);
            Position[0] += rand() % 120 - 60;
            Position[1] += rand() % 120 - 60;
            Position[2] += rand() % 60;
            Vector(1, 1, 1, Light);
            for (int i = 0; i < 3; ++i)
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, Light, 20, 10.f);
        }
        else
        {
            o->Skill += (1) * FPS_ANIMATION_FACTOR;
            if (o->Angle[2] < 45.0f) o->Angle[2] += o->Skill * 0.3f;
            if (o->Angle[2] > 45.0f) o->Angle[2] -= o->Skill * 0.3f;

            if (o->Position[0] < Hero->Object.Position[0] + 300.0f) o->Position[0] += o->Skill * 0.2f;
            if (o->Position[0] > Hero->Object.Position[0] + 300.0f) o->Position[0] -= o->Skill * 0.2f;
            if (o->Position[1] < Hero->Object.Position[1] - 300.0f) o->Position[1] += o->Skill * 0.2f;
            if (o->Position[1] > Hero->Object.Position[1] - 300.0f) o->Position[1] -= o->Skill * 0.2f;

            Vector(1, 1, 1, Light);
            VectorCopy(o->Position, Position);
            Position[0] += rand() % 1200 - 600;
            Position[1] += rand() % 1200 - 600;
            vec3_t Angle;
            Vector(0.f, 0.f, rand() % 10 * 20.f, Angle);
            CreateEffectFpsChecked(MODEL_FIRE, Position, Angle, Light, 0, NULL, 0);

            o->Scale += (0.02f) * FPS_ANIMATION_FACTOR;
            //				o->Position[2] -= 0.5f;
            EarthQuake = (float)(rand() % 8 - 8) * 0.1f;
        }
        return true;
    }

    // MODEL_CUNDUN_SKILL
    bool Move_MODEL_CUNDUN_SKILL(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
        if (o->SubType == 0)
        {
            if ((int)o->LifeTime == 30)
            {
                for (int i = 0; i < 10; ++i)
                {
                    vec3_t Angle;
                    Vector(0, 0, o->Angle[2] + 320 + i * 8, Angle);
                    CreateEffectFpsChecked(MODEL_CUNDUN_PHOENIX, o->Position, Angle, o->Light, 0);
                }
            }
        }
        else if (o->SubType == 1)
        {
            vec3_t Angle;
            Angle[0] = 0;
            Angle[1] = 0;
            if ((int)o->LifeTime == 30)
            {
                for (int i = 0; i < 20; ++i)
                {
                    Angle[2] = (float)(o->PKKey * 30);
                    CreateEffectFpsChecked(MODEL_CUNDUN_DRAGON_HEAD, o->Position, Angle, o->Light);
                }
            }
            if (o->PKKey > 5 && o->PKKey < 10)
            {
                Angle[2] = (float)(o->PKKey * 30);
                CreateEffectFpsChecked(MODEL_CUNDUN_DRAGON_HEAD, o->Position, Angle, o->Light);
            }
            ++o->PKKey;
        }
        else if (o->SubType == 2)
        {
            if (o->PKKey == 0 && o->LifeTime < 10)
            {
                o->PKKey = 1;
            }
        }
        return true;
    }

    // MODEL_BATTLE_GUARD2
    bool Move_MODEL_BATTLE_GUARD2(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        if (o->SubType == 0)
        {
            if ((int)o->LifeTime % 5 == 0)
            {
                VectorCopy(o->Position, Position);

                Position[0] += rand() % 100 - 50.f;
                Position[1] -= rand() % 30 + 15.f;
                CreateEffectFpsChecked(MODEL_FLY_BIG_STONE1, Position, o->Angle, o->Light, 2);
            }
        }
        if (o->LifeTime < 5)
        {
            o->Alpha *= pow(1.0f / (1.3f), FPS_ANIMATION_FACTOR);
        }
        return true;
    }

    // MODEL_ARROW_TANKER_HIT
    bool Move_MODEL_ARROW_TANKER_HIT(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
    {
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);

        if (o->Position[2] < Height + 80.0f)
        {
            if (o->Alpha <= 0.1f) o->LifeTime = 0;
            o->Alpha *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);

            Vector(0.0f, 0.0f, -30.0f, o->Direction);
            Vector(1.f, 0.6f, 0.2f, Light);
            CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, Light);

            if (o->HiddenMesh != 99)
            {
                CreateInferno(o->Position, 1);
                VectorCopy(o->Position, o->StartPosition);
                CreateEffectFpsChecked(BITMAP_CRATER, o->Position, o->Angle, o->Light);
            }

            EarthQuake = (float)(rand() % 8 - 8) * 0.1f;
            EarthQuake *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);

            o->HiddenMesh = 99;
        }

        if (o->HiddenMesh == 99)
        {
            o->Position[0] += ((float)(rand() % 100 - 50)) * FPS_ANIMATION_FACTOR;
            o->Position[1] += ((float)(rand() % 100 - 50)) * FPS_ANIMATION_FACTOR;
            o->Position[2] += ((float)(rand() % 100 - 50)) * FPS_ANIMATION_FACTOR;
            o->Scale = 2.2f;
        }

        if (o->Visible)
        {
            vec3_t p1, p2;
            BMD* b = &Models[o->Type];

            Vector(1.0f, 1.0f, 1.0f, o->Light);
            Vector(0.0f, 1.5f, 0.0f, p1);
            b->TransformPosition(BoneTransform[2], p1, p2);
            CreateParticleFpsChecked(BITMAP_FIRE + 1, o->Position, o->Angle, o->Light, 8, o->Scale - 0.4f, o);
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 38, o->Scale, o);
            Vector(1.0f, 0.4f, 0.0f, o->Light);
            CreateParticleFpsChecked(BITMAP_FIRE + 1, o->Position, o->Angle, o->Light, 9, o->Scale - 0.4f, o);
        }

        if (o->HiddenMesh == 99)
        {
            o->Scale = 1.0f;
            VectorCopy(o->StartPosition, o->Position);
        }

        Vector(1.0f, 0.0f, 0.0f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
    }
        return true;
    }

    // MODEL_FLY_BIG_STONE1
    bool Move_MODEL_FLY_BIG_STONE1(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Matrix[3][4];
        float Height;
    {
        if (o->SubType == 2)
        {
            o->Position[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (1.9f) * FPS_ANIMATION_FACTOR;

            o->Angle[0] += (rand() % 20 + 20.f) * FPS_ANIMATION_FACTOR;
            o->Angle[1] += (rand() % 5) * FPS_ANIMATION_FACTOR;

            float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                o->Position[2] = Height + 50;
                o->Gravity = -o->Gravity * 0.2f;
                o->LifeTime -= (5) * FPS_ANIMATION_FACTOR;
                o->Angle[0] -= (o->Scale * 128.f) * FPS_ANIMATION_FACTOR;

                Vector(0.f, 5.f, 0.f, o->Direction);
            }
            else
                o->Angle[0] -= (o->Scale * 16.f) * FPS_ANIMATION_FACTOR;

            AngleMatrix(o->HeadAngle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

            if (o->LifeTime < 10)
            {
                o->Alpha *= pow(1.0f / (1.5f), FPS_ANIMATION_FACTOR);
            }

            if ((int)o->LifeTime % 3 == 0)
            {
                Vector(1.f, 0.6f, 0.2f, Light);
                CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, Light, 1, 0.2f);
            }
        }
        else
        {
            if (o->SubType <= 1)
            {
                o->Position[0] += (o->Direction[0] * o->Velocity) * FPS_ANIMATION_FACTOR;
                o->Position[1] += (o->Direction[1] * o->Velocity) * FPS_ANIMATION_FACTOR;
                o->StartPosition[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;

                if (o->Position[2] > 1000.f)
                {
                    o->Position[2] = 1000.f;
                    o->HiddenMesh = -2;
                }
                else
                {
                    o->HiddenMesh = -1;
                    Vector(Luminosity * 1.f, Luminosity * 0.5f, Luminosity * 0.1f, Light);
                    CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light, 5, 2.f);
                }

                o->Velocity -= (0.45f) * FPS_ANIMATION_FACTOR;
                if (o->Velocity < 5.f) o->Velocity = 5.f;

                o->Direction[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity -= (0.1f) * FPS_ANIMATION_FACTOR;

                o->Angle[0] += (5.f) * FPS_ANIMATION_FACTOR;
                o->Angle[1] += (5.f) * FPS_ANIMATION_FACTOR;

                if (o->SubType == 0)
                {
                    o->Position[2] = o->StartPosition[2];
                }
                else if (o->Owner == (&Hero->Object))
                {
                    o->Position[2] = o->StartPosition[2];
                    if (o->Position[2] > 800.f)
                    {
                        o->Position[2] = 800.f;
                    }
                    g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
                }
            }

            if (o->SubType == 88 || o->SubType == 99)
            {
                if (o->SubType == 99 && o->Owner == (&Hero->Object))
                {
                    g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
                }

                if (o->LifeTime > o->DamageTime)
                {
                    Vector(1.f, 0.6f, 0.2f, Light);
                    CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, Light);
                    CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light, 2, 2.f);

                    if (o->SubType == 88 && o->Owner != NULL)
                    {
                        battleCastle::CollisionHeroCharacter(o->Position, ((15 - o->LifeTime) * 20) + 350.f, PLAYER_HIGH_SHOCK);
                    }
                    else if (o->SubType == 99)
                    {
                        battleCastle::CollisionTempCharacter(o->Position, ((40 - o->LifeTime) * 7) + 350.f, PLAYER_HIGH_SHOCK);
                    }
                }
            }
            else
            {
                float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
                if (o->LifeTime < 250 && o->Position[2] < Height + 300.f && o->SubType == 1 && o->Kind != 0 && o->Skill != 0)
                {
                    SocketClient->ToGameServer()->SendWeaponExplosionRequest(MAKELONG(o->Skill, o->Kind));
                    o->Kind = 0;
                    o->Skill = 0;
                }

                if (o->Position[2] < Height)
                {
                    o->Position[2] = Height - 5.f;
                    if (o->SubType == 0)
                    {
                        o->LifeTime = 15;
                        o->DamageTime = 0;
                        o->SubType = 88;
                    }
                    else if (o->SubType == 1)
                    {
                        o->LifeTime = 40;
                        o->DamageTime = 25;
                        o->SubType = 99;
                    }
                    if (o->HiddenMesh != 99 && o->Visible)
                    {
                        CreateEffectFpsChecked(BITMAP_CRATER, o->Position, o->Angle, o->Light);
                        PlayBuffer(SOUND_BC_CATAPULT_HIT, o);

                        battleCastle::CollisionEffectToObject(o, 350.f, 250.f, true);

                        if (o->SubType == 88 && o->Owner == NULL)
                        {
                            battleCastle::CollisionHeroCharacter(o->Position, 350.f, PLAYER_HIGH_SHOCK);
                        }
                        else if (o->SubType == 99 || (o->SubType == 88 && o->Owner != NULL))
                        {
                            Vector(1.f, 0.3f, 0.1f, Light);
                            CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 7);
                        }
                    }
                    o->HiddenMesh = 99;

                    if (Height > 150.f)
                    {
                        int CollisionX = (int)(o->Position[0] / TERRAIN_SCALE);
                        int CollisionY = (int)(o->Position[1] / TERRAIN_SCALE);

                        EarthQuake = (float)(rand() % 8 - 8) * 0.1f;
                        if (CollisionY < 104 && o->Visible)
                        {
                            AddTerrainHeight(o->Position[0], o->Position[1], -30.f, 2, BackTerrainHeight);
                            CreateTerrainNormal_Part(CollisionX, CollisionY);
                            CreateTerrainLight_Part(CollisionX, CollisionY);

                            EarthQuake *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);
                        }
                    }
                }
                else
                {
                    bool collision;
                    if (o->SubType == 1)
                    {
                        collision = battleCastle::CollisionEffectToObject(o, 200.f, 250.f, false, true);
                    }
                    else/* if ( o->Visible )*/
                    {
                        collision = battleCastle::CollisionEffectToObject(o, 200.f, 250.f, false);
                    }
                    if (collision && o->Visible)
                    {
                        PlayBuffer(SOUND_BC_CATAPULT_HIT, o);
                    }
                }
            }
            Vector(-0.5f, -0.5f, -0.5f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        }
    }
        return true;
    }

    // MODEL_FLY_BIG_STONE2
    bool Move_MODEL_FLY_BIG_STONE2(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
    {
        if (o->SubType <= 1)
        {
            o->Position[0] += (o->Direction[0] * o->Velocity) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (o->Direction[1] * o->Velocity) * FPS_ANIMATION_FACTOR;
            o->StartPosition[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;

            if (o->Position[2] > 1000.f)
            {
                o->Position[2] = 1000.f;
                o->HiddenMesh = -2;
            }
            else
            {
                o->HiddenMesh = -1;
                Vector(1.f, 1.f, 1.f, Light);

                if (rand_fps_check(2))
                {
                    CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, Light, 1, 2.f);
                }
            }

            o->Velocity -= (0.45f) * FPS_ANIMATION_FACTOR;
            if (o->Velocity < 5.f) o->Velocity = 5.f;

            o->Direction[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity -= (0.1f) * FPS_ANIMATION_FACTOR;

            o->Angle[0] += (5.f) * FPS_ANIMATION_FACTOR;
            o->Angle[1] += (5.f) * FPS_ANIMATION_FACTOR;

            if (o->SubType == 0)
            {
                o->Position[2] = o->StartPosition[2];
            }
            else if (o->Owner == (&Hero->Object))
            {
                o->Position[2] = o->StartPosition[2];
                if (o->Position[2] > 800.f)
                {
                    o->Position[2] = 800.f;
                }
                g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
            }
        }

        Vector(-0.5f, -0.5f, -0.5f, Light);
        if (o->SubType == 88 || o->SubType == 99)
        {
            if (o->SubType == 99 && o->Owner == (&Hero->Object))
            {
                g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
            }

            if (o->LifeTime > o->DamageTime)
            {
                CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, o->Light);

                if (o->SubType == 88 && o->Owner != NULL)
                {
                    battleCastle::CollisionHeroCharacter(o->Position, ((15 - o->LifeTime) * 20) + 350.f, PLAYER_HIGH_SHOCK);
                }
                else if (o->SubType == 99)
                {
                    battleCastle::CollisionTempCharacter(o->Position, ((40 - o->LifeTime) * 7) + 350.f, PLAYER_HIGH_SHOCK);
                }
            }
        }
        else
        {
            float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height + 300.f && o->SubType == 1 && o->Kind != 0 && o->Skill != 0)
            {
                SocketClient->ToGameServer()->SendWeaponExplosionRequest(MAKELONG(o->Skill, o->Kind));
                o->Kind = 0;
                o->Skill = 0;
            }

            if (o->Position[2] < Height)
            {
                o->Position[2] = Height - 5.f;
                if (o->SubType == 0)
                {
                    o->LifeTime = 15;
                    o->DamageTime = 0;
                    o->SubType = 88;
                }
                else if (o->SubType == 1)
                {
                    o->LifeTime = 40;
                    o->DamageTime = 25;
                    o->SubType = 99;
                }
                if (o->HiddenMesh != 99 && o->Visible)
                {
                    CreateEffectFpsChecked(BITMAP_CRATER, o->Position, o->Angle, o->Light);
                    PlayBuffer(SOUND_BC_CATAPULT_HIT, o);

                    if (o->SubType == 88 && o->Owner == NULL)
                    {
                        battleCastle::CollisionHeroCharacter(o->Position, 350.f, PLAYER_HIGH_SHOCK);
                    }
                    else if (o->SubType == 99 || (o->SubType == 88 && o->Owner != NULL))
                    {
                        Vector(1.f, 0.3f, 0.1f, Light);
                        CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 7);
                    }
                }
                o->HiddenMesh = 99;

                if (Height > 150.f)
                {
                    int CollisionX = (int)(o->Position[0] / TERRAIN_SCALE);
                    int CollisionY = (int)(o->Position[1] / TERRAIN_SCALE);

                    EarthQuake = (float)(rand() % 8 - 8) * 0.1f;
                    if (CollisionY < 104 && o->Visible)
                    {
                        AddTerrainHeight(o->Position[0], o->Position[1], -30.f, 2, BackTerrainHeight);
                        CreateTerrainNormal_Part(CollisionX, CollisionY);
                        CreateTerrainLight_Part(CollisionX, CollisionY);

                        EarthQuake *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);
                    }
                }
            }
        }
        AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
    }
        return true;
    }

    // MODEL_BIG_STONE_PART1, MODEL_BIG_STONE_PART2, MODEL_WALL_PART1, MODEL_WALL_PART2, MODEL_GOLEM_STONE
    bool Move_MODEL_BIG_STONE_PART1(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
        if (o->Type == MODEL_BIG_STONE_PART2 && o->SubType == 3)
        {
            o->Direction[2] -= (o->Velocity) * FPS_ANIMATION_FACTOR;
            o->Velocity += (0.3f) * FPS_ANIMATION_FACTOR;

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < Height)
            {
                o->Position[2] = Height;
                Vector(0.f, 0.f, 0.f, o->Direction);
                Vector(0.f, 0.f, 0.f, o->Angle);
                o->HiddenMesh = 0;

                if (rand_fps_check(10))
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 24, 1.25f * o->Scale);
            }
            else
            {
                if (rand_fps_check(2))
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 24, 0.2f);
            }
            return true;
        }
        VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        if (o->Type >= MODEL_WALL_PART1 && o->Type <= MODEL_WALL_PART2)
        {
            VectorScale(o->Direction, 0.9f, o->Direction);
            o->Gravity -= (6.f) * FPS_ANIMATION_FACTOR;
        }
        else
        {
            VectorScale(o->Direction, 0.9f, o->Direction);
            o->Gravity -= (3.f) * FPS_ANIMATION_FACTOR;
        }
        o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.2f;
            o->LifeTime -= (5) * FPS_ANIMATION_FACTOR;
            o->Angle[0] -= (o->Scale * 128.f) * FPS_ANIMATION_FACTOR;
        }
        else
        {
            o->Angle[0] -= (o->Scale * 16.f) * FPS_ANIMATION_FACTOR;
        }

        o->Alpha = o->LifeTime / 10.f;
        if (o->Type == MODEL_GOLEM_STONE)
        {
            if (rand_fps_check(4)) {
                CreateParticleFpsChecked(BITMAP_TRUE_FIRE, o->Position, o->Angle, Light, 5, 2.8f);
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, 1.8f);
            }
            if (rand_fps_check(10)) {
                CreateParticleFpsChecked(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
            }
        }
        else if (o->Type == MODEL_BIG_STONE_PART1 && o->SubType == 2)
        {
            if (rand_fps_check(10)) {
                Vector(0.2f, 0.5f, 0.35f, Light);
                o->Position[2] = Height;
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 11, 2.f);
            }
        }
        return true;
    }

    // MODEL_GATE_PART1, MODEL_GATE_PART2, MODEL_GATE_PART3
    bool Move_MODEL_GATE_PART1(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Height;
        VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        VectorScale(o->Direction, 0.9f, o->Direction);
        o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;

        if (o->SubType == 1)
        {
            VectorScale(o->Direction, 1.1f, o->Direction);
        }

        o->Gravity -= (4.f) * FPS_ANIMATION_FACTOR;
        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.5f;
            o->LifeTime -= (5) * FPS_ANIMATION_FACTOR;
            o->Angle[0] -= (o->Scale * 128.f) * FPS_ANIMATION_FACTOR;
        }
        else
            o->Angle[0] -= (o->Scale * 16.f) * FPS_ANIMATION_FACTOR;

        if (rand_fps_check(10))
        {
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, Position, o->Angle, Light);
        }
        return true;
    }

    // MODEL_AURORA
    bool Move_MODEL_AURORA(OBJECT* o, int index, float Luminosity)
    {
        if (o->Owner != NULL && o->Owner->Live == true)
        {
            if (battleCastle::IsBattleCastleStart() == false)
            {
                o->HiddenMesh = -2;
            }
            else
            {
                o->HiddenMesh = -1;
            }
            o->LifeTime = 2;
            o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.1f + 0.2f;
        }
        else
        {
            o->Live = false;
        }
        o->BlendMeshTexCoordU = WorldTime * 0.0005f;
        return true;
    }

    // MODEL_FENRIR_THUNDER
    bool Move_MODEL_FENRIR_THUNDER(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            if (o->Live)
            {
                if (o->m_iAnimation == 0)
                {
                    o->Alpha += (0.3f) * FPS_ANIMATION_FACTOR;
                    if (o->Alpha >= 1.0f)
                    {
                        o->m_iAnimation = 1;
                        o->Alpha = 1.0f;
                    }

                    if (o->Alpha < 0.0f)
                    {
                        o->Alpha = 1.0f;
                    }
                }
                else if (o->m_iAnimation == 1)
                {
                    o->Alpha -= (0.3f) * FPS_ANIMATION_FACTOR;
                    if (o->Alpha <= 0.0f)
                    {
                        o->Alpha = 0.0f;
                        o->Live = false;
                    }
                }
                o->Angle[0] += (0.15f) * FPS_ANIMATION_FACTOR;
                o->Angle[1] += (0.15f) * FPS_ANIMATION_FACTOR;
                o->Angle[2] += (0.15f) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 1)
        {
            if (o->Live)
            {
                if (o->m_iAnimation == 0)
                {
                    o->Alpha += (0.3f) * FPS_ANIMATION_FACTOR;
                    if (o->Alpha >= 1.0f)
                    {
                        o->m_iAnimation = 1;
                        o->Alpha = 1.0f;
                    }

                    if (o->Alpha < 0.0f)
                    {
                        o->Alpha = 1.0f;
                    }
                }
                else if (o->m_iAnimation == 1)
                {
                    o->Alpha -= (0.3f) * FPS_ANIMATION_FACTOR;
                    if (o->Alpha <= 0.0f)
                    {
                        o->Alpha = 0.0f;
                        o->Live = false;
                    }
                }
                o->Angle[0] += (0.15f) * FPS_ANIMATION_FACTOR;
                o->Angle[1] += (0.15f) * FPS_ANIMATION_FACTOR;
                o->Angle[2] += (0.15f) * FPS_ANIMATION_FACTOR;
            }
        }
        return true;
    }

    // MODEL_FALL_STONE_EFFECT
    bool Move_MODEL_FALL_STONE_EFFECT(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0 || o->SubType == 1)
        {
            o->Position[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;
            o->Angle[0] += (0.5f) * FPS_ANIMATION_FACTOR;
            o->Angle[1] += (0.5f) * FPS_ANIMATION_FACTOR;
            o->Angle[2] += (0.5f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 2)
        {
            o->Position[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity += (0.1f) * FPS_ANIMATION_FACTOR;

            float fHeight = RequestTerrainHeight(o->Position[0], o->Position[1]);
            if (o->Position[2] < fHeight)
            {
                vec3_t vLight;
                Vector(0.5f, 0.5f, 0.5f, vLight);
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 11, (float)(rand() % 20 + 30) * 0.020f);

                int iRand = rand() % 2 + 2;
                for (int i = 0; i < iRand; ++i)
                {
                    float fScale = 0.03f + (rand() % 10) / 40.0f + o->Scale * 0.3f;
                    CreateEffectFpsChecked(MODEL_FALL_STONE_EFFECT, o->Position, o->Angle, o->Light, 3, NULL, -1, 0, 0, 0, fScale);
                }

                o->Live = false;
            }
        }
        else if (o->SubType == 3)
        {
            o->HeadAngle[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;

            o->Position[0] += (o->HeadAngle[0]) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (o->HeadAngle[1]) * FPS_ANIMATION_FACTOR;
            o->Position[2] += (o->HeadAngle[2]) * FPS_ANIMATION_FACTOR;

            float fHeight = RequestTerrainHeight(o->Position[0], o->Position[1]);
            o->Angle[0] += (0.5f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            o->Angle[1] += (0.5f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            if (o->Position[2] + o->Direction[2] <= fHeight)
            {
                o->Position[2] = fHeight;
                o->HeadAngle[0] *= pow(0.6f, FPS_ANIMATION_FACTOR);
                o->HeadAngle[1] *= pow(0.6f, FPS_ANIMATION_FACTOR);
                o->HeadAngle[2] += (1.0f * o->LifeTime) * FPS_ANIMATION_FACTOR;
                if (o->HeadAngle[2] < 0.5f)
                    o->HeadAngle[2] = 0;

                o->Alpha -= (0.15f) * FPS_ANIMATION_FACTOR;

                o->Light[0] *= pow(1.0f / (1.08f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.08f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.08f), FPS_ANIMATION_FACTOR);
            }
        }
    }
        return true;
    }

    // MODEL_FENRIR_FOOT_THUNDER
    bool Move_MODEL_FENRIR_FOOT_THUNDER(OBJECT* o, int index, float Luminosity)
    {
        if (o->Live == true)
        {
            o->Angle[0] += (0.1f) * FPS_ANIMATION_FACTOR;

            if (o->SubType == 1)
            {
                o->Light[1] -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[2] -= (0.05f) * FPS_ANIMATION_FACTOR;
            }
            else if (o->SubType == 2)
            {
                o->Light[0] -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[1] -= (0.05f) * FPS_ANIMATION_FACTOR;
            }
            else if (o->SubType == 3)
            {
                o->Light[0] -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[2] -= (0.05f) * FPS_ANIMATION_FACTOR;
            }
            else if (o->SubType == 4)
            {
                o->Light[2] -= (0.05f) * FPS_ANIMATION_FACTOR;
            }
            o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
            if (o->Alpha <= 0.0f)
                o->Live = false;

            if (timeGetTime() - o->m_dwTime > 200)
            {
                o->m_iAnimation++;
                if (o->m_iAnimation > 4)
                    o->Live = false;
                o->m_dwTime = timeGetTime();
            }
        }
        return true;
    }

    // MODEL_TWINTAIL_EFFECT
    bool Move_MODEL_TWINTAIL_EFFECT(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            o->Alpha -= (0.01f) * FPS_ANIMATION_FACTOR;
            if (o->Alpha <= 0.0f)
            {
                o->Live = false;
            }

            o->Light[0] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);

            if (timeGetTime() - o->m_dwTime > 1000)
            {
                if (o->m_iAnimation == 0)
                {
                    o->m_iAnimation = 1;
                }
                else
                {
                    o->m_iAnimation = 0;
                }

                o->m_dwTime = timeGetTime();
            }

            if (o->m_iAnimation == 0)
            {
                o->Scale -= (0.015f) * FPS_ANIMATION_FACTOR;
                if (o->Scale <= 0.0f)
                {
                    o->Scale = 0.0f;
                }
            }
            else
            {
                o->Scale += (0.02f) * FPS_ANIMATION_FACTOR;
                if (o->Scale >= 1.2f)
                {
                    o->Scale = 1.2f;
                }
            }
        }
        else if (o->SubType == 1 || o->SubType == 2)
        {
            o->Alpha -= (0.01f) * FPS_ANIMATION_FACTOR;
            if (o->Alpha <= 0.0f)
            {
                o->Live = false;
            }

            o->Light[0] *= pow(1.0f / (1.04f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.04f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.04f), FPS_ANIMATION_FACTOR);

            if (o->SubType == 1)
                o->Angle[0] = -(WorldTime * 0.3f);
            else if (o->SubType == 2)
                o->Angle[0] = -(WorldTime * 0.1f);

            o->Scale -= (0.02f) * FPS_ANIMATION_FACTOR;
        }
    }
        return true;
    }

    // MODEL_TOWER_GATE_PLANE
    bool Move_MODEL_TOWER_GATE_PLANE(OBJECT* o, int index, float Luminosity)
    {
        o->LifeTime = 100;
        if (o->Owner != NULL && o->Live == true)
        {
            o->Position[2] = o->StartPosition[2] + sinf(WorldTime * 0.001f) * 200.f + 200.f;
        }
        return true;
    }

    // BITMAP_CRATER
    bool Move_BITMAP_CRATER(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        if (o->LifeTime < 10)
        {
            o->Light[0] = o->LifeTime / 10.f;
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
            o->Alpha = o->Light[0];
        }
        Vector(-0.5f, -0.5f, -0.5f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, (int)(o->StartPosition[0] - 1), PrimaryTerrainLight);
        return true;
    }

    // BITMAP_CHROME_ENERGY2
    bool Move_BITMAP_CHROME_ENERGY2(OBJECT* o, int index, float Luminosity)
    {
        if (o->LifeTime < 10)
        {
            o->Light[0] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
        }
        return true;
    }

    // MODEL_STUN_STONE
    bool Move_MODEL_STUN_STONE(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        if (o->SubType == 0)
        {
            o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
            o->Gravity -= (15.f) * FPS_ANIMATION_FACTOR;

            Height = RequestTerrainHeight(o->Position[0], o->Position[1]) + 50;
            if (o->Position[2] <= Height)
            {
                o->Position[2] = Height;
                if (o->ExtState == 0)
                {
                    CreateEffectFpsChecked(BITMAP_CRATER, o->Position, o->Angle, o->Light, 1);
                    o->ExtState = 1;
                }
            }
            else
            {
                VectorAdd(o->Position, o->StartPosition, o->Position);
                VectorScale(o->StartPosition, 0.9f, o->StartPosition);

                o->HeadAngle[0] -= (o->Scale * 32.f) * FPS_ANIMATION_FACTOR;

                if ((int)o->LifeTime % 3 == 0)
                {
                    CreateParticleFpsChecked(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, o->Light, 1, 1.f);
                }
            }
            o->Light[0] = o->LifeTime / 10.f;
            o->Light[1] = o->Light[0];
            o->Light[2] = o->Light[0];
            o->Alpha = o->Light[0];
        }
        else if (o->SubType == 1)
        {
            if ((int)o->LifeTime % 3 == 0)
            {
                CreateEffectFpsChecked(MODEL_STUN_STONE, o->Position, o->Angle, o->Light);
            }
            AddTerrainLight(o->Position[0], o->Position[1], o->Light, 1, PrimaryTerrainLight);
        }
        return true;
    }

    // MODEL_SKIN_SHELL
    bool Move_MODEL_SKIN_SHELL(OBJECT* o, int index, float Luminosity)
    {
        float Height;
        VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
        VectorScale(o->Direction, 0.9f, o->Direction);
        o->Position[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;

        VectorScale(o->Direction, 1.1f, o->Direction);

        o->Gravity -= (3.f) * FPS_ANIMATION_FACTOR;
        Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.2f;
            o->LifeTime -= (5) * FPS_ANIMATION_FACTOR;
            o->Angle[0] -= (o->Scale * 128.f) * FPS_ANIMATION_FACTOR;
        }
        else
            o->Angle[0] -= (o->Scale * 16.f) * FPS_ANIMATION_FACTOR;

        o->Alpha = o->LifeTime / 10.f;
        return true;
    }

    // MODEL_MANA_RUNE
    bool Move_MODEL_MANA_RUNE(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            if (o->LifeTime > 43)
            {
                o->HiddenMesh = -2;
            }
            else if (o->LifeTime > 40)
            {
                o->HiddenMesh = 0;
            }
            else if (o->LifeTime > 30)
            {
                o->Scale += (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity += (0.15f) * FPS_ANIMATION_FACTOR;
                if (o->Scale > 1.f)
                {
                    o->Scale = 1.f;
                    o->Gravity = 0.01f;

                    CreateEffectFpsChecked(MODEL_MANA_RUNE, o->Position, o->Angle, o->Light, 1);
                }
            }
            else if (o->LifeTime < 15)
            {
                o->Scale -= (o->Gravity) * FPS_ANIMATION_FACTOR;
                o->Gravity += (0.2f) * FPS_ANIMATION_FACTOR;
                if (o->Scale < 0.f)
                {
                    o->Scale = 0.f;
                }
                o->Alpha = (o->LifeTime / 20.f);
                o->Position[2] -= (10.f) * FPS_ANIMATION_FACTOR;
            }
        }
        else if (o->SubType == 1)
        {
            o->Scale -= (0.02f) * FPS_ANIMATION_FACTOR;
            if (o->Scale < 1.f)
            {
                o->Scale = 1.f;
            }
            o->Alpha = (o->LifeTime / 20.f);
        }
        return true;
    }

    // MODEL_SKILL_JAVELIN
    bool Move_MODEL_SKILL_JAVELIN(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Position;
        float Matrix[3][4];
        float Height;
        if (o->Owner != NULL)
        {
            VectorCopy(o->Owner->Position, o->StartPosition);
            o->StartPosition[2] += (150.f) * FPS_ANIMATION_FACTOR;
        }

        o->Scale += (0.015f) * FPS_ANIMATION_FACTOR;
        if (o->LifeTime < 25)
        {
            float Distance;
            if (o->LifeTime < 20)
            {
                Distance = MoveHumming(o->Position, o->HeadAngle, o->StartPosition, o->Velocity);
                if (Distance < 100.f)
                {
                    VectorCopy(o->StartPosition, o->Position);
                    o->Scale += (0.04f) * FPS_ANIMATION_FACTOR;

                    if ((int)o->LifeTime % 3 == 0)
                    {
                        CreateParticleFpsChecked(BITMAP_POUNDING_BALL, o->Position, o->Angle, o->Light, 1);
                    }
                }
                else
                {
                    CreateParticleFpsChecked(BITMAP_POUNDING_BALL, o->Position, o->Angle, o->Light, 1);
                }
            }
            o->Velocity += (1.5f) * FPS_ANIMATION_FACTOR;

            AngleMatrix(o->HeadAngle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);

            o->Gravity = rand() % 30 + 30.f;

            if (o->Direction[1] > -50.f)
            {
                o->Direction[1] -= (8.f) * FPS_ANIMATION_FACTOR;
            }
        }
        else
        {
            o->Gravity += (10.f) * FPS_ANIMATION_FACTOR;
            AngleMatrix(o->HeadAngle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);
            VectorAddScaled(o->Position, Position, o->Position, FPS_ANIMATION_FACTOR);
        }
        o->Angle[2] += (o->Gravity) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = o->LifeTime / 10.f;
        o->Alpha = o->BlendMeshLight;

        Height = sinf(o->LifeTime * 0.1f) * 30.f;
        if (o->SubType == 1)
        {
            o->Position[2] = o->StartPosition[2] + Height;
        }
        else if (o->SubType == 2)
        {
            o->Position[2] = o->StartPosition[2] - Height;
        }

        Vector(1.f, 0.6f, 0.3f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
        return true;
    }

    // MODEL_ARROW_IMPACT
    bool Move_MODEL_ARROW_IMPACT(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Position;
        o->Angle[0] -= (5.f) * FPS_ANIMATION_FACTOR;
        o->Direction[1] -= (8.f) * FPS_ANIMATION_FACTOR;

        if (o->LifeTime < 2 && o->SubType == 0 && o->Owner != NULL)
        {
            o->Angle[0] = 90.f;
            o->SubType = 1;

            VectorCopy(o->Owner->Position, Position);
            Position[0] += rand() % 100 - 50.f;
            Position[1] += rand() % 100 - 50.f;
            Position[2] += 1200.f;
            CreateJointFpsChecked(BITMAP_FLASH, Position, Position, o->Angle, 2, o, 50.f);
            CreateJointFpsChecked(BITMAP_FLASH, Position, Position, o->Angle, 3, o, 50.f);
        }
        else if (o->SubType == 1)
        {
            o->Live = false;
        }
        return true;
    }

    // MODEL_SWORD_FORCE
    bool Move_MODEL_SWORD_FORCE(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        if (o->SubType == 0 || o->SubType == 2)
        {
            if (o->LifeTime > 12)
            {
                o->Scale += (0.9f) * FPS_ANIMATION_FACTOR;

                o->Direction[1] -= (2.f) * FPS_ANIMATION_FACTOR;
                if (o->SubType == 2)
                {
                    CreateEffectFpsChecked(MODEL_SWORD_FORCE, o->Position, o->Angle, o->Light, 3, o);
                }
                else
                    CreateEffectFpsChecked(MODEL_SWORD_FORCE, o->Position, o->Angle, o->Light, 1, o);
            }
            else
            {
                o->Scale -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->BlendMeshLight = (float)o->LifeTime / 18.f;
                o->Alpha = o->BlendMeshLight;
                o->Light[0] = o->Alpha;
                o->Light[1] = o->Alpha;
                o->Light[2] = o->Alpha;

                o->Direction[1] -= (2.f) * FPS_ANIMATION_FACTOR;

                VectorCopy(o->Position, Position);
                Position[0] += rand() % 30 - 15.f;
                Position[1] += rand() % 30 - 15.f;
                Position[2] -= 100.f;
                for (int i = 0; i < 4; i++)
                {
                    Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2], Angle);
                    CreateJointFpsChecked(BITMAP_JOINT_SPARK, Position, Position, Angle);
                    if (o->SubType == 2)
                    {
                        CreateParticleFpsChecked(BITMAP_FIRE, Position, Angle, o->Light, 18, 1.5f);
                    }
                    else
                        CreateParticleFpsChecked(BITMAP_FIRE, Position, Angle, o->Light, 2, 1.5f);
                }
            }
            Vector(1.f, 0.8f, 0.6f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight);
        }
        else if (o->SubType == 1 || o->SubType == 3)
        {
            o->BlendMeshLight = (float)o->LifeTime / 10.f;
            o->Alpha = o->BlendMeshLight;
            o->Light[0] = o->Alpha;
            o->Light[1] = o->Alpha;
            o->Light[2] = o->Alpha;
        }
        return true;
    }

    // MODEL_MOVE_TARGETPOSITION_EFFECT
    bool Move_MODEL_MOVE_TARGETPOSITION_EFFECT(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            //float fScale = 0.f;
            vec3_t vLight, vPos, vAngle, vRelativePos;
            Vector(1.0f, 0.7f, 0.3f, vLight);
            Vector(0.0f, 0.0f, 0.0f, vAngle);
            Vector(0.0f, 0.0f, 0.0f, vRelativePos);
            VectorCopy(o->Position, vPos);
            vPos[2] += 110.f;

            {
                CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 24, 1.0f, o);
            }

            if ((int)o->LifeTime % 15 == 0)
            {
                CreateEffectFpsChecked(BITMAP_TARGET_POSITION_EFFECT1, o->Position, vAngle, vLight, 0);//, NULL, -1, 0, 0, 0, fScale );
            }

            if (o->LifeTime <= 10)
            {
                o->BlendMeshLight -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[0] *= pow(o->BlendMeshLight, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(o->BlendMeshLight, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(o->BlendMeshLight, FPS_ANIMATION_FACTOR);
            }
        }
    }
        return true;
    }

    // BITMAP_TARGET_POSITION_EFFECT1
    bool Move_BITMAP_TARGET_POSITION_EFFECT1(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            o->Scale -= (0.04f) * FPS_ANIMATION_FACTOR;
            if (o->Scale <= 0.2f)
            {
                o->Live = false;
            }
            if (o->LifeTime <= 10)
            {
                o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[0] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
            }
        }
    }
        return true;
    }

    // BITMAP_TARGET_POSITION_EFFECT2
    bool Move_BITMAP_TARGET_POSITION_EFFECT2(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            if (o->Scale >= 1.8f)
            {
                o->m_iAnimation = 0;
            }
            else if (o->Scale <= 0.8f)
            {
                o->m_iAnimation = 1;
            }

            if (o->m_iAnimation == 0)
            {
                o->Scale -= (0.15f) * FPS_ANIMATION_FACTOR;
            }
            else if (o->m_iAnimation == 1)
            {
                o->Scale += (0.15f) * FPS_ANIMATION_FACTOR;
            }

            if (o->LifeTime <= 10)
            {
                o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
                o->Light[0] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(o->Alpha, FPS_ANIMATION_FACTOR);
            }
        }
    }
        return true;
    }

    // MODEL_EFFECT_SAPITRES_ATTACK
    bool Move_MODEL_EFFECT_SAPITRES_ATTACK(OBJECT* o, int index, float Luminosity)
    {
    {
        if ((int)o->LifeTime % 6 == 0)
        {
            o->Position[0] += ((float)((rand() % 120) - 60)) * FPS_ANIMATION_FACTOR;
            o->Position[1] += ((float)((rand() % 120) - 60)) * FPS_ANIMATION_FACTOR;

            CreateEffectFpsChecked(MODEL_EFFECT_SAPITRES_ATTACK_1, o->Position, o->Angle, o->Light, 0, o->Owner);
        }
    }
        return true;
    }

    // MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1
    bool Move_MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 0)
        {
            vec3_t vLight, vLightFlare, vPos;
            Vector(0.4f, 0.7f, 1.0f, vLight);
            Vector(0.1f, 0.2f, 0.8f, vLightFlare);
            VectorCopy(o->Position, vPos);
            if (o->LifeTime >= 4)
            {
                o->Scale = 0.8f + ((float)(rand() % 10) * 0.1f);
                vPos[0] += (3.0f * ((float)(rand() % 40) - 20));
                vPos[1] += (3.0f * ((float)(rand() % 40) - 20));
                vPos[2] += (3.0f * ((float)(rand() % 40) - 20));
                CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, o->Angle, vLight, 0, o->Scale);
            }
            CreateSprite(BITMAP_LIGHT, o->Position, 8.0f, vLightFlare, o);
            CreateSprite(BITMAP_LIGHT, o->Position, 8.0f, vLightFlare, o);
        }
        else if (o->SubType == 1)
        {
            const int iOffsetPos = 200;
            vec3_t vLight, vLightFlare, vPos;

            Vector(0.4f, 0.7f, 1.0f, vLight);
            Vector(0.1f, 0.2f, 0.8f, vLightFlare);
            VectorCopy(o->Position, vPos);

            vPos[0] = vPos[0] + ((rand() % iOffsetPos) - (iOffsetPos * 0.5f));
            vPos[1] = vPos[1] + ((rand() % iOffsetPos) - (iOffsetPos * 0.5f));
            vPos[2] = vPos[2] + ((rand() % (iOffsetPos / 2)) - ((iOffsetPos * 0.5f) * 0.5f));
            if (o->LifeTime >= 4)
            {
                o->Scale = (0.8f + ((float)(rand() % 10) * 0.1f));
                vPos[0] += (3.0f * ((float)(rand() % 40) - 20));
                vPos[1] += (3.0f * ((float)(rand() % 40) - 20));
                vPos[2] += (3.0f * ((float)(rand() % 40) - 20));
                CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, o->Angle, vLight, 0, o->Scale);
            }

            VectorCopy(o->Position, vPos);

            vPos[0] = vPos[0] + ((rand() % iOffsetPos) - (iOffsetPos * 0.5f));
            vPos[1] = vPos[1] + ((rand() % iOffsetPos) - (iOffsetPos * 0.5f));
            vPos[2] = vPos[2] + ((rand() % (iOffsetPos / 2)) - ((iOffsetPos * 0.5f) * 0.5f));
            CreateSprite(BITMAP_LIGHT, vPos, 8.0f, vLightFlare, o);
            CreateSprite(BITMAP_LIGHT, vPos, 8.0f, vLightFlare, o);
        }
    }
        return true;
    }

    // MODEL_EFFECT_SKURA_ITEM
    bool Move_MODEL_EFFECT_SKURA_ITEM(OBJECT* o, int index, float Luminosity)
    {
    {
        if ((o->SubType == 0) || (o->SubType == 1))
        {
            if (o->Owner->Live && o->Live)
            {
                vec3_t vRelativePos, vtaWorldPos, vLight1, vLight2;
                BMD* b = &Models[MODEL_EFFECT_SKURA_ITEM];

                VectorCopy(o->Owner->Position, o->Position);
                VectorCopy(o->Position, b->BodyOrigin);
                Vector(0.f, 0.f, 0.f, vRelativePos);
                Vector(1.f, 0.6f, 0.8f, o->Light);
                Vector(0.3f, 0.3f, 0.3f, vLight1);
                Vector(1.f, 0.6f, 0.8f, vLight2);

                b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                b->TransformPosition(BoneTransform[1], vRelativePos, vtaWorldPos, false);
                CreateSprite(BITMAP_SHINY + 1, vtaWorldPos, 1.8f, o->Light, o, +WorldTime * 0.08f);
                for (int i = 0; i < 7; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_SHINY + 1, vtaWorldPos, o->Angle, o->Light, 5, 0.8f, o);
                    CreateParticleFpsChecked(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand() % 7 == 3 ? vLight2 : vLight1, 0, 0.5f);
                }

                b->TransformPosition(BoneTransform[2], vRelativePos, vtaWorldPos, false);
                CreateSprite(BITMAP_SHINY + 1, vtaWorldPos, 1.8f, o->Light, o, -WorldTime * 0.08f);

                for (int i = 0; i < 7; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_SHINY + 1, vtaWorldPos, o->Angle, o->Light, 5, 0.8f, o);
                    CreateParticleFpsChecked(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand() % 7 == 3 ? vLight2 : vLight1, 0, 0.5f);
                }

                if ((int)o->LifeTime == 30 || (int)o->LifeTime == 15 || (int)o->LifeTime == 4)
                {
                    vec3_t vAngle;

                    BMD* characterBMD = &Models[o->Owner->Type];
                    VectorCopy(o->Position, characterBMD->BodyOrigin);

                    characterBMD->Animation(o->Owner->BoneTransform, o->Owner->AnimationFrame,
                        o->Owner->PriorAnimationFrame, o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle);
                    characterBMD->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, vtaWorldPos, false);

                    if (rand_fps_check(2)) {
                        vtaWorldPos[0] += rand() % 40 + 10;
                    }
                    else {
                        vtaWorldPos[0] -= rand() % 30 + 10;
                    }

                    vtaWorldPos[2] += rand() % 110 + 50;

                    Vector(0.7f, 0.71f, 1.0f, vLight1);
                    Vector(0.8f, 0.85f, 1.0f, vLight2);

                    for (int k = 0; k < 70; ++k)
                    {
                        if (rand_fps_check(3)) {
                            CreateParticleFpsChecked(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand() % 7 == 3 ? vLight2 : vLight1, 0, 0.4f);
                        }
                        CreateParticleFpsChecked(BITMAP_CHERRYBLOSSOM_EVENT_FLOWER, vtaWorldPos, o->Angle, rand() % 4 == 3 ? vLight2 : vLight1, 0, 0.4f);
                    }

                    Vector(0.f, 1.f, 0.f, vAngle);
                    Vector(1.f, 0.6f, 0.8f, vLight2);
                    CreateParticleFpsChecked(BITMAP_SHOCK_WAVE, vtaWorldPos, vAngle, vLight2, 4, 0.005f);

                    characterBMD->Animation(o->Owner->BoneTransform, o->Owner->AnimationFrame,
                        o->Owner->PriorAnimationFrame, o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle);
                    characterBMD->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, vtaWorldPos, false);

                    vtaWorldPos[2] += 70;
                    Vector(1.f, 0.3f, 0.6f, vLight2);
                    CreateSprite(BITMAP_SHINY + 1, vtaWorldPos, 1.8f, o->Light, o, -WorldTime * 0.08f);

                    StopBuffer(SOUND_CHERRYBLOSSOM_EFFECT1, true);
                    PlayBuffer(SOUND_CHERRYBLOSSOM_EFFECT1, o->Owner);
                }

                b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity / 3.f, o->Position, o->Angle);
            }
        }
    }
        return true;
    }

    // MODEL_BLOW_OF_DESTRUCTION
    bool Move_MODEL_BLOW_OF_DESTRUCTION(OBJECT* o, int index, float Luminosity)
    {
        float Matrix[3][4];
    {
        if (o->SubType == 0)
        {
            vec3_t vLight;
            if (o->LifeTime <= 24)
            {
                if (o->LifeTime >= 15.f)
                {
                    EarthQuake = (float)(rand() % 8 - 4) * 0.1f;
                }
                else
                {
                    EarthQuake = 0.f;
                }

                if ((int)o->LifeTime == 23)
                {
                    Vector(0.3f, 0.3f, 1.0f, vLight);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);
                    CreateEffectFpsChecked(MODEL_KNIGHT_PLANCRACK_A, o->Position, o->Angle, vLight, 0, o, 0, 0, 0, 0, 1.2f);

                    vec3_t vDir, vPos, vAngle;
                    VectorSubtract(o->StartPosition, o->Position, vDir);
                    float fLength = VectorLength(vDir);
                    VectorNormalize(vDir);
                    int iNum = (int)(fLength / 100) + 1;
                    for (int i = 0; i < iNum; ++i)
                    {
                        VectorScale(vDir, 55.f * i, vPos);
                        VectorAdd(o->Position, vPos, vPos);
                        VectorCopy(o->Owner->Angle, vAngle);
                        if (i % 2 == 0)
                        {
                            vAngle[2] += (rand() % 20 + 10);
                        }
                        else
                        {
                            vAngle[2] -= (rand() % 20 + 10);
                        }
                        CreateEffectFpsChecked(MODEL_KNIGHT_PLANCRACK_B, vPos, vAngle, vLight, 0, o, 0, 0, 0, 0, 1.0f);
                    }
                }

                o->Light[0] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 1)
        {
            if (o->LifeTime <= 24)
            {
                if (o->LifeTime >= 15)
                {
                    vec3_t vPos, vLight;
                    for (int i = 0; i < 15; ++i)
                    {
                        VectorCopy(o->Position, vPos);
                        vPos[0] += rand() % 300 - 150;
                        vPos[1] += rand() % 300 - 150;
                        vPos[2] += rand() % 300 - 150;
                        float fScale = 1.6f + rand() % 10 * 0.1f;
                        Vector(0.5f, 0.5f, 1.0f, vLight);
                        int index = (rand_fps_check(2)) ? BITMAP_WATERFALL_5 : BITMAP_WATERFALL_3;
                        CreateParticleFpsChecked(index, vPos, o->Angle, vLight, 8, fScale);
                        Vector(1.0f, 1.0f, 1.0f, vLight);
                        if (rand_fps_check(2))
                        {
                            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 55, 1.f);
                        }
                    }
                }

                if ((int)o->LifeTime == 23)
                {
                    vec3_t vLight;
                    Vector(0.5f, 0.5f, 1.f, vLight);

                    Vector(0.3f, 0.3f, 1.0f, vLight);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 2.f);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 1.f);
                    CreateEffectFpsChecked(MODEL_RAKLION_BOSS_CRACKEFFECT, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 0.2f);

                    vec3_t vPos, vResult;
                    vec3_t vAngle;
                    float Matrix[3][4];
                    for (int i = 0; i < (5 + rand() % 3); ++i)
                    {
                        Vector(0.f, (float)(rand() % 150), 0.f, vPos);
                        Vector(0.f, 0.f, (float)(rand() % 360), vAngle);
                        AngleMatrix(vAngle, Matrix);
                        VectorRotate(vPos, Matrix, vResult);
                        VectorAdd(vResult, o->Position, vResult);

                        CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, vResult, o->Angle, o->Light, 13);
                    }
                }

                o->Light[0] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 2)
        {
            vec3_t vLight;
            if (o->LifeTime <= 35.0f && o->LifeTime >= 15.0f)
            {
                EarthQuake = (float)(rand() % 8 - 4) * 0.1f;
            }
            else
            {
                EarthQuake = 0.f;
            }

            if ((int)o->LifeTime == 30)
            {
                Vector(1.0f, 1.0f, 1.0f, vLight);
                vec3_t vDir, vPos, vAngle;
                VectorSubtract(o->StartPosition, o->Position, vDir);
                VectorLength(vDir);
                VectorNormalize(vDir);
                VectorScale(vDir, 55.f, vPos);
                VectorAdd(o->Position, vPos, vPos);
                VectorCopy(o->Owner->Angle, vAngle);
                vAngle[2] += (rand() % 40 + 140);
                CreateEffectFpsChecked(MODEL_DRAGON_LOWER_DUMMY, vPos, vAngle, vLight, 0, o, 0, 0, 0, 0, 0.8f);

                Vector(1.0f, 1.0f, 1.0f, vLight);
                CreateEffectFpsChecked(BITMAP_LIGHT_RED, vPos, vAngle, vLight, 4, o, -1, 0, 0, 0, 3.0f);
            }
        }
    }
        return true;
    }

    // MODEL_NIGHTWATER_01
    bool Move_MODEL_NIGHTWATER_01(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Alpha -= (0.04f) * FPS_ANIMATION_FACTOR;
    }
        return true;
    }

    // MODEL_KNIGHT_PLANCRACK_A
    bool Move_MODEL_KNIGHT_PLANCRACK_A(OBJECT* o, int index, float Luminosity)
    {
        if (o->SubType == 0)
        {
            o->Alpha -= (0.04f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 1)
        {
            //if (o->LifeTime < 10)
            {
                o->Alpha *= pow(0.9f, FPS_ANIMATION_FACTOR);
            }
        }
        return true;
    }

    // MODEL_KNIGHT_PLANCRACK_B
    bool Move_MODEL_KNIGHT_PLANCRACK_B(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Alpha -= (0.04f) * FPS_ANIMATION_FACTOR;
    }
        return true;
    }

    // MODEL_1_STREAMBREATHFIRE
    bool Move_MODEL_1_STREAMBREATHFIRE(OBJECT* o, int index, float Luminosity)
    {
    {
        vec3_t	v3Pos;
        VectorCopy(o->Position, v3Pos);

        CreateParticleFpsChecked(BITMAP_WATERFALL_3, v3Pos, o->Angle, o->Light, 11, 0.6f);
        // 연기
        CreateParticleFpsChecked(BITMAP_SMOKE, v3Pos, o->Angle, o->Light, 52, 0.6f);

        if (15 == o->LifeTime)
        {
            const float fForce = 20.0f;

            float matRotation[3][4];
            vec3_t vPos;
            vec3_t vDir, vDir_;

            Vector(0.0f, -fForce, 0.0f, vDir);
            AngleMatrix(o->Angle, matRotation);

            VectorRotate(vDir, matRotation, vDir_);

            VectorCopy(o->Position, vPos);
            //vPos[2] += 100.f;

            CreateEffectFpsChecked(MODEL_MOONHARVEST_MOON, vPos, vDir_, o->Light,
                1,
                NULL, -1, 0, 0, 0, 0.3f);
        }
    }
        return true;
    }

    // MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD, MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD
    bool Move_MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        float Height;
    {
        o->HeadAngle[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
        VectorCopy(o->Light, Light);

        o->Position[0] += (o->HeadAngle[0]) * FPS_ANIMATION_FACTOR;
        o->Position[1] += (o->HeadAngle[1]) * FPS_ANIMATION_FACTOR;
        o->Position[2] += (o->HeadAngle[2]) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]) + 20;

        if (o->Position[2] + o->Direction[2] <= Height)
        {
            o->Position[2] = Height;
            o->HeadAngle[0] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[1] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[2] += (0.6f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            if (o->HeadAngle[2] < 5.0f)
                o->HeadAngle[2] = 0;

            o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;
        }
        else
        {
            if (o->SubType == 0)
            {
                o->Angle[0] += (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
                o->Angle[1] += (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Angle[0] -= (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
                o->Angle[1] -= (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            }
        }

        vec3_t vLook, vPosition, vLight;
        float matRotate[3][4];
        Vector(1.f, 1.f, 1.f, vLight);
        Vector(0.0f, 0.0f, 0.0f, vPosition);
        AngleMatrix(o->Angle, matRotate);
        VectorRotate(vPosition, matRotate, vLook);
        VectorAdd(o->Position, vLook, vPosition);
        if (o->Type == MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD)
        {
            CreateEffectFpsChecked(BITMAP_FIRE_CURSEDLICH, vPosition, o->Angle, vLight, 4, o);
        }
        else
        {
            CreateEffectFpsChecked(BITMAP_FIRE_CURSEDLICH, vPosition, o->Angle, vLight, 3, o);
        }
    }
        return true;
    }

    // MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY, MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY
    bool Move_MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY(OBJECT* o, int index, float Luminosity)
    {
    {
        o->m_iAnimation++;

        if (o->LifeTime < 20)
        {
            o->Alpha = 0.1f * o->LifeTime;
        }
    }
        return true;
    }

    // MODEL_LAVAGIANT_FOOTPRINT_R, MODEL_LAVAGIANT_FOOTPRINT_V
    bool Move_MODEL_LAVAGIANT_FOOTPRINT_R(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Angle[0] += (0.1f) * FPS_ANIMATION_FACTOR;

        o->Alpha -= (0.01f) * FPS_ANIMATION_FACTOR;
        if (o->Alpha <= 0.0f)
            o->Live = false;

        o->Light[0] *= pow(0.93f, FPS_ANIMATION_FACTOR);
        o->Light[1] *= pow(0.93f, FPS_ANIMATION_FACTOR);
        o->Light[2] *= pow(0.93f, FPS_ANIMATION_FACTOR);
    }
        return true;
    }

    // MODEL_PROJECTILE
    bool Move_MODEL_PROJECTILE(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->LifeTime > 38)
        {
            BMD* b = &Models[o->Owner->Type];
            b->Animation(BoneTransform, o->Owner->AnimationFrame, o->Owner->PriorAnimationFrame, o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle, false, false);
            vec3_t vRelativePos, vWorldPos;
            Vector(0, 0, 0, vRelativePos);
            b->TransformPosition(BoneTransform[9], vRelativePos, vWorldPos, false);

            o->Position[0] = vWorldPos[0] + o->Owner->Position[0];
            o->Position[1] = vWorldPos[1] + o->Owner->Position[1];
            o->Position[2] = vWorldPos[2] + o->Owner->Position[2];
        }
        else
        {
            if (o->Gravity == 0.0f)
            {
                o->Gravity = 0.1f;

                o->Direction[0] = o->Position[0] - o->StartPosition[0];
                o->Direction[1] = o->Position[1] - o->StartPosition[1];
                o->Direction[2] = o->Position[2] - o->StartPosition[2];
                VectorNormalize(o->Direction);
            }
            o->Angle[0] += (0.5f) * FPS_ANIMATION_FACTOR;
            o->Velocity += (0.2f) * FPS_ANIMATION_FACTOR;

            o->Position[0] += (o->Direction[0] * o->Velocity) * FPS_ANIMATION_FACTOR;
            o->Position[1] += (o->Direction[1] * o->Velocity) * FPS_ANIMATION_FACTOR;
            o->Position[2] += (o->Direction[2]) * FPS_ANIMATION_FACTOR;
        }
    }
        return true;
    }

    // MODEL_DOOR_CRUSH_EFFECT_PIECE01, MODEL_DOOR_CRUSH_EFFECT_PIECE02, MODEL_DOOR_CRUSH_EFFECT_PIECE03, MODEL_DOOR_CRUSH_EFFECT_PIECE04, MODEL_DOOR_CRUSH_EFFECT_PIECE05, MODEL_DOOR_CRUSH_EFFECT_PIECE06, MODEL_DOOR_CRUSH_EFFECT_PIECE07, MODEL_DOOR_CRUSH_EFFECT_PIECE08, MODEL_DOOR_CRUSH_EFFECT_PIECE09, MODEL_DOOR_CRUSH_EFFECT_PIECE11, MODEL_DOOR_CRUSH_EFFECT_PIECE12, MODEL_DOOR_CRUSH_EFFECT_PIECE13, MODEL_STATUE_CRUSH_EFFECT_PIECE01, MODEL_STATUE_CRUSH_EFFECT_PIECE02, MODEL_STATUE_CRUSH_EFFECT_PIECE03
    bool Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
        float Matrix[3][4];
        float Height;
        vec3_t p;
    {
        o->HeadAngle[2] -= (o->Gravity) * FPS_ANIMATION_FACTOR;
        VectorCopy(o->Light, Light);

        o->Position[0] += (o->HeadAngle[0]) * FPS_ANIMATION_FACTOR;
        o->Position[1] += (o->HeadAngle[1]) * FPS_ANIMATION_FACTOR;
        o->Position[2] += (o->HeadAngle[2]) * FPS_ANIMATION_FACTOR;

        Height = RequestTerrainHeight(o->Position[0], o->Position[1]) + 20;

        if (o->Position[2] + o->Direction[2] <= Height)
        {
            o->Position[2] = Height;
            o->HeadAngle[0] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[1] *= pow(0.8f, FPS_ANIMATION_FACTOR);
            o->HeadAngle[2] += (0.6f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            if (o->HeadAngle[2] < 5.0f)
                o->HeadAngle[2] = 0;

            o->Alpha -= (0.05f) * FPS_ANIMATION_FACTOR;

            if (rand_fps_check(4))
            {
                Vector(0.f, (float)(rand() % 80) + 50.f, 0.f, p);
                Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, Position);

                CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, Position, o->Angle, o->Light, 0);
            }
        }
        else
        {
            if (o->SubType == 0)
            {
                o->Angle[0] += (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
                o->Angle[1] += (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            }
            else
            {
                o->Angle[0] -= (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
                o->Angle[1] -= (0.15f * o->LifeTime) * FPS_ANIMATION_FACTOR;
            }
        }

        if (rand_fps_check(3))
        {
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, o->Position, o->Angle, o->Light, 6);
        }
    }
        return true;
    }

    // MODEL_STATUE_CRUSH_EFFECT_PIECE04, MODEL_DOOR_CRUSH_EFFECT_PIECE10
    bool Move_MODEL_STATUE_CRUSH_EFFECT_PIECE04(OBJECT* o, int index, float Luminosity)
    {
    {
        o->LifeTime -= (1) * FPS_ANIMATION_FACTOR;

        if (o->LifeTime < 40)
        {
            o->Alpha -= (0.1f) * FPS_ANIMATION_FACTOR;
        }
    }
        return true;
    }

    // MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION
    bool Move_MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION(OBJECT* o, int index, float Luminosity)
    {
        float Matrix[3][4];
    {
        if (o->SubType == 0)
        {
            vec3_t vLight;
            if (o->LifeTime <= 24)
            {
                if (o->LifeTime >= 15.f)
                {
                    EarthQuake = (float)(rand() % 8 - 4) * 0.1f;
                }
                else
                {
                    EarthQuake = 0.f;
                }

                if ((int)o->LifeTime == 23)
                {
                    Vector(0.3f, 0.3f, 1.0f, vLight);
                    //CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);
                    //CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);

                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, o->PKKey, o->Kind, o->Skill, o->m_bySkillSerialNum, o->Scale * 1.0f);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, o->PKKey, o->Kind, o->Skill, o->m_bySkillSerialNum, o->Scale * 1.0f);

                    CreateEffectFpsChecked(MODEL_KNIGHT_PLANCRACK_A, o->Position, o->Angle, vLight, 0, o, 0, 0, 0, 0, o->Scale * 0.2f);

                    vec3_t vDir, vPos, vAngle;
                    VectorSubtract(o->StartPosition, o->Position, vDir);
                    float fLength = VectorLength(vDir);
                    VectorNormalize(vDir);
                    int iNum = (int)(fLength / 100) + 1;
                    for (int i = 0; i < iNum; ++i)
                    {
                        VectorScale(vDir, 55.f * i, vPos);
                        VectorAdd(o->Position, vPos, vPos);
                        VectorCopy(o->Owner->Angle, vAngle);
                        if (i % 2 == 0)
                        {
                            vAngle[2] += (rand() % 20 + 10);
                        }
                        else
                        {
                            vAngle[2] -= (rand() % 20 + 10);
                        }
                        CreateEffectFpsChecked(MODEL_KNIGHT_PLANCRACK_B,
                            vPos, vAngle, vLight, 0, o, 0, 0, 0, 0, o->Scale);
                    }
                }

                o->Light[0] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            }
        }
        else if (o->SubType == 1)
        {
            if (o->LifeTime <= 24)
            {
                if (o->LifeTime >= 15)
                {
                    vec3_t vPos, vLight;
                    for (int i = 0; i < 15; ++i)
                    {
                        VectorCopy(o->Position, vPos);
                        vPos[0] += rand() % 400 - 200;
                        vPos[1] += rand() % 400 - 200;
                        vPos[2] += rand() % 400 - 200;
                        float fScale = (o->Scale * 1.6f) + rand() % 10 * 0.1f;
                        Vector(0.3f, 0.3f, 1.0f, vLight);
                        int index = (rand_fps_check(2)) ? BITMAP_WATERFALL_5 : BITMAP_WATERFALL_3;
                        CreateParticleFpsChecked(index, vPos, o->Angle, vLight, 8, fScale);
                        Vector(1.0f, 1.0f, 1.0f, vLight);
                        if (rand_fps_check(2))
                        {
                            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 55, o->Scale * 0.9f);
                        }
                    }
                }

                if ((int)o->LifeTime == 23)
                {
                    vec3_t vLight;
                    Vector(0.5f, 0.5f, 1.f, vLight);

                    Vector(0.3f, 0.3f, 1.0f, vLight);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, o->Scale * 1.1f);
                    CreateEffectFpsChecked(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, o->Scale * 1.2f);
                    CreateEffectFpsChecked(MODEL_RAKLION_BOSS_CRACKEFFECT, o->Position, o->Angle, vLight,
                        0, o, -1, 0, 0, 0, o->Scale * 0.4f);

                    vec3_t vPos, vResult;
                    vec3_t vAngle;
                    float Matrix[3][4];
                    for (int i = 0; i < (5 + rand() % 3); ++i)
                    {
                        Vector(0.f, (float)(rand() % 150), 0.f, vPos);
                        Vector(0.f, 0.f, (float)(rand() % 360), vAngle);
                        AngleMatrix(vAngle, Matrix);
                        VectorRotate(vPos, Matrix, vResult);
                        VectorAdd(vResult, o->Position, vResult);

                        CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, vResult, o->Angle, o->Light, 13);
                    }
                }

                o->Light[0] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[1] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
                o->Light[2] *= pow(1.0f / (1.05f), FPS_ANIMATION_FACTOR);
            }
        }
    }
        return true;
    }

    // MODEL_EFFECT_SD_AURA
    bool Move_MODEL_EFFECT_SD_AURA(OBJECT* o, int index, float Luminosity)
    {
    {
        vec3_t Temp_Pos;
        BMD* b = &Models[o->Owner->Type];
        b->TransformByObjectBone(Temp_Pos, o->Owner, 18);
        Temp_Pos[2] -= 30.0f;
        VectorCopy(Temp_Pos, o->Position);
        o->LifeTime = 100;
    }
        return true;
    }

    // MODEL_WOLF_HEAD_EFFECT
    bool Move_MODEL_WOLF_HEAD_EFFECT(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t p;
    {
        if (o->SubType == 0)
        {
            o->BlendMeshLight = (float)o->LifeTime / 20.0f;
            o->Alpha = o->BlendMeshLight;
            vec3_t Angle;
            if (o->LifeTime >= 3)
            {
                VectorCopy(o->Owner->Angle, Angle);
                int nRandom = rand() % 6;
                float _Value = 7.0f + rand() % 3;
                if (nRandom / 3)
                    o->Owner->Angle[2] = Angle[2] + (_Value * (nRandom % 3 * 2 + 1));
                else
                    o->Owner->Angle[2] = Angle[2] - (_Value * (nRandom % 3 * 2 + 1));

                int _Temp = rand() % 3;
                if (_Temp == 1)
                    o->Owner->Angle[0] = Angle[0] + (_Value * (nRandom % 3 * 2 + 1));
                else if (_Temp == 2)
                    o->Owner->Angle[0] = Angle[0] - (_Value * (nRandom % 3 * 2 + 1));

                for (int i = 0; i < 3; i++)
                {
                    BMD* b = &Models[o->Owner->Type];
                    b->TransformByObjectBone(p, o->Owner, 27);
                    VectorCopy(p, o->Position);
                    o->Scale = 1.5f;
                    Vector(0.2f, 0.3f, 1.0f, Light);
                    p[1] += rand() % 20 - 10;
                    p[2] += rand() % 28 + 6;
                    CreateJointFpsChecked(BITMAP_PIN_LIGHT, p, p, o->Owner->Angle, 0, o->Owner, 5.0f);
                }
                CreateEffectFpsChecked(MODEL_WOLF_HEAD_EFFECT, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 1, o->Owner);
                CreateEffectFpsChecked(MODEL_WOLF_HEAD_EFFECT, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 2, o->Owner);
                o->Owner->Angle[2] = Angle[2];
                o->Owner->Angle[0] = Angle[0];
            }
        }
        else if (o->SubType == 1)
        {
            o->Owner->Velocity += (2.5f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 2)
        {
            o->Owner->Velocity += (5.5f) * FPS_ANIMATION_FACTOR;
        }
    }
        return true;
    }

    // BITMAP_SBUMB
    bool Move_BITMAP_SBUMB(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->LifeTime < 17)
        {
            vec3_t vLight, vPosition;
            Vector(1.0f, 1.0f, 1.0f, vLight);
            if (o->Owner->m_sTargetIndex < 0)
                return true;

            VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
            Vector(vPosition[0] + rand() % 80 - 40, vPosition[1] + rand() % 80 - 40, vPosition[2] + rand() % 120 + 30, vPosition);
            float Scale = 1.5f + (rand() % 10 * 0.02f);
            CreateParticleFpsChecked(BITMAP_SBUMB, vPosition, o->Angle, vLight, 0, Scale * o->Scale, o->Owner);
        }
        if ((int)o->LifeTime == 15 || (int)o->LifeTime == 9 || (int)o->LifeTime == 6)
        {
            StopBuffer(SOUND_RAGESKILL_THRUST_ATTACK, true);
            PlayBuffer(SOUND_RAGESKILL_THRUST_ATTACK);
        }
    }
        return true;
    }

    // MODEL_DOWN_ATTACK_DUMMY_L
    bool Move_MODEL_DOWN_ATTACK_DUMMY_L(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
    {
        if (o->Owner->Live)
        {
            BMD* pModel = &Models[o->Type];
            pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);

            if (o->AnimationFrame >= 8.0f || !o->Owner->Live)
                o->Live = false;

            if (o->AnimationFrame >= 5.0f && o->LifeTime > 50)
            {
                if (o->Owner->m_sTargetIndex < 0)
                    return true;
                CreateBomb3(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, 3, 0.9f);
                StopBuffer(SOUND_RAGESKILL_STAMP_ATTACK, true);
                PlayBuffer(SOUND_RAGESKILL_STAMP_ATTACK);
                o->LifeTime = 40;
            }

            vec3_t _StartPos, _EndPos;
            pModel->TransformByObjectBone(_StartPos, o, 0);
            CreateJointFpsChecked(BITMAP_FORCEPILLAR, _StartPos, _StartPos, o->Angle, 0, o, 50.0f);

            VectorCopy(o->Owner->Angle, o->Angle);
            VectorCopy(o->Owner->Position, o->Position);

            vec3_t Light;
            Vector(0.47f, 0.36f, 0.24f, Light);
            pModel->TransformByObjectBone(_EndPos, o, 0);
            CreateSprite(BITMAP_FLARE, _EndPos, 5.0f, Light, o);
        }
        else
        {
            o->Live = false;
        }
    }
        return true;
    }

    // MODEL_DOWN_ATTACK_DUMMY_R
    bool Move_MODEL_DOWN_ATTACK_DUMMY_R(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
    {
        if (o->Owner->Live)
        {
            BMD* pModel = &Models[o->Type];
            pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);

            if (o->AnimationFrame >= 6.0f || !o->Owner->Live)
            {
                o->Live = false;
                CreateEffectFpsChecked(MODEL_DOWN_ATTACK_DUMMY_L, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 0, o->Owner);
            }

            if (o->AnimationFrame >= 3.0f && o->LifeTime > 50)
            {
                if (o->Owner->m_sTargetIndex < 0)
                    return true;
                CreateBomb3(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, 3, 0.9f);
                StopBuffer(SOUND_RAGESKILL_STAMP_ATTACK, true);
                PlayBuffer(SOUND_RAGESKILL_STAMP_ATTACK);
                o->LifeTime = 40;
            }

            vec3_t _StartPos, _EndPos;
            pModel->TransformByObjectBone(_StartPos, o, 0);
            CreateJointFpsChecked(BITMAP_FORCEPILLAR, _StartPos, _StartPos, o->Angle, 1, o, 50.0f);

            VectorCopy(o->Owner->Angle, o->Angle);
            VectorCopy(o->Owner->Position, o->Position);

            vec3_t Light;
            Vector(0.47f, 0.36f, 0.24f, Light);
            pModel->TransformByObjectBone(_EndPos, o, 0);
            CreateSprite(BITMAP_FLARE, _EndPos, 5.0f, Light, o);
        }
        else
        {
            o->Live = false;
        }
    }
        return true;
    }

    // BITMAP_SWORDEFF
    bool Move_BITMAP_SWORDEFF(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
    {
        if (o->Owner->AnimationFrame > Models[o->Owner->Type].Actions[PLAYER_SKILL_GIANTSWING].NumAnimationKeys)
            o->Live = false;

        BMD* b = &Models[o->Owner->Type];
        vec3_t _StartPos, _EndPos;
        b->TransformByObjectBone(_StartPos, o->Owner, 5);
        b->TransformByObjectBone(_EndPos, o->Owner, 4);
        CreateJointFpsChecked(BITMAP_SWORDEFF, _StartPos, _EndPos, o->Owner->Angle, 0, o->Owner, 100.0f);

        if (o->Owner->AnimationFrame > 6 && o->LifeTime > 50)
        {
            vec3_t Light, Angle;
            Vector(0.4f, 0.5f, 1.0f, Light);
            VectorCopy(o->Owner->Angle, Angle);

            vec3_t position, Relative;
            Vector(-180.0f, 100.0f, 0.0f, Relative);
            b->TransformByObjectBone(position, o->Owner, 0, Relative);

            Vector(60.0f, 0.0f, Angle[2] - 180, Angle);
            CreateEffectFpsChecked(MODEL_SHOCKWAVE01, position, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 1.0f);
            CreateEffectFpsChecked(MODEL_WOLF_HEAD_EFFECT2, o->Position, o->Angle, o->Light, 3, o->Owner);

            if (o->Owner->m_sTargetIndex < 0)
                return true;

            CreateEffectFpsChecked(BITMAP_DAMAGE1, CharactersClient[o->Owner->m_sTargetIndex].Object.Position, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 1.4f);
            StopBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK, true);
            PlayBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK);
            o->LifeTime = 45;
        }

        if (o->Owner->AnimationFrame > 11 && o->LifeTime > 20)
        {
            vec3_t Light, Angle, vPosition;
            Vector(0.5f, 0.5f, 1.0f, Light);
            VectorCopy(o->Owner->Angle, Angle);
            vec3_t position, Relative;
            Vector(-80.0f, 120.0f, 0.0f, Relative);
            b->TransformByObjectBone(position, o->Owner, 4, Relative);
            Vector(90.f, 0.0f, Angle[2] - 180, Angle);
            CreateEffectFpsChecked(MODEL_SHOCKWAVE02, position, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 0.5f);
            CreateEffectFpsChecked(MODEL_WOLF_HEAD_EFFECT2, o->Position, o->Angle, o->Light, 5, o->Owner);

            Vector(1.0f, 1.0f, 1.0f, Light);
            Vector(-80.0f, 20.0f, 0.0f, vPosition);
            b->TransformByObjectBone(vPosition, o->Owner, 4, vPosition);
            Vector(90.0f, Angle[1], Angle[2], Angle);
            CreateEffectFpsChecked(MODEL_SHOCKWAVE_SPIN01, vPosition, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 0.5f);
            CreateEffectFpsChecked(MODEL_SHOCKWAVE_SPIN01, vPosition, Angle, Light, 1, o->Owner, -1, 0, 0, 0, 0.9f);

            if (o->Owner->m_sTargetIndex < 0)
                return true;

            VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
            CreateEffectFpsChecked(BITMAP_SHINY + 4, vPosition, o->Angle, Light, 0, o->Owner, -1, 0, 0, 0, 1.9f);
            StopBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK, true);
            PlayBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK);
            o->LifeTime = 20;
        }
    }
        return true;
    }

    // MODEL_SHOCKWAVE01
    bool Move_MODEL_SHOCKWAVE01(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 1 || o->SubType == 2)
        {
            o->Alpha = 1.0f;
            o->Scale *= pow(1.24f, FPS_ANIMATION_FACTOR);
            o->Position[2] *= pow(1.19f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.45f, o->Light);
            if (o->Light[0] < 0.001f)
            {
                Vector(0.0f, 0.0f, 0.0f, o->Light);
            }
        }
        else if (o->SubType == 3)
        {
            o->Scale *= pow(1.15f, FPS_ANIMATION_FACTOR);
            VectorScale(o->StartPosition, 0.99f, o->StartPosition);
            VectorSubtract(o->Position, o->StartPosition, o->Position);
            VectorScale(o->Light, 0.7f, o->Light);
            if (o->Light[0] > 0.6f)
            {
                o->SubType = 5;
            }
            o->Position[2] = 180.0f + o->Scale * 80.0f;
        }
        else if (o->SubType == 4)
        {
            o->Scale *= pow(1.15f, FPS_ANIMATION_FACTOR);
            VectorScale(o->StartPosition, 1.04f, o->StartPosition);
            VectorSubtract(o->Position, o->StartPosition, o->Position);
            VectorScale(o->Light, 1.5f, o->Light);
            if (o->Light[0] > 0.6f)
            {
                o->SubType = 5;
            }
            o->Position[2] = 160.0f + o->Scale * 10.0f;
        }
        else if (o->SubType == 5)
        {
            o->Scale *= pow(1.15f, FPS_ANIMATION_FACTOR);
            VectorScale(o->StartPosition, 1.04f, o->StartPosition);
            VectorSubtract(o->Position, o->StartPosition, o->Position);
            VectorScale(o->Light, 0.5f, o->Light);
            o->Position[2] = 160.0f + o->Scale * 10.0f;
        }
        else
        {
            o->Alpha = 1.0f;
            o->Scale += (0.2f) * FPS_ANIMATION_FACTOR;
            VectorScale(o->StartPosition, 0.99f, o->StartPosition);
            VectorSubtract(o->Position, o->StartPosition, o->Position);
            VectorScale(o->Light, 0.65f, o->Light);
            o->Position[2] = 220.0f + o->Scale * 80.0f;
        }
    }
        return true;
    }

    // MODEL_SHOCKWAVE02
    bool Move_MODEL_SHOCKWAVE02(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Alpha = 1.0f;
        VectorScale(o->Light, 0.8f, o->Light);
        o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
        VectorScale(o->StartPosition, 0.95f, o->StartPosition);
        VectorSubtract(o->Position, o->StartPosition, o->Position);
    }
        return true;
    }

    // BITMAP_DAMAGE1
    bool Move_BITMAP_DAMAGE1(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->LifeTime < 17)
        {
            if ((int)o->LifeTime % 4 != 0)
                return true;

            vec3_t vLight, vPosition;
            Vector(0.6f, 0.94f, 1.0f, vLight);

            if (o->Owner->m_sTargetIndex < 0)
                return true;

            VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
            Vector(vPosition[0] + rand() % 90 - 40, vPosition[1] + rand() % 90 - 40, vPosition[2] + rand() % 100 + 50, vPosition);
            float Scale = 1.5f + (rand() % 5 * 0.01f);
            CreateParticleFpsChecked(BITMAP_DAMAGE1, vPosition, o->Angle, vLight, 0, Scale * o->Scale, o->Owner);
        }
    }
        return true;
    }

    // MODEL_SHOCKWAVE_SPIN01
    bool Move_MODEL_SHOCKWAVE_SPIN01(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Scale *= pow(1.01f, FPS_ANIMATION_FACTOR);
        o->Angle[1] -= (10.0f) * FPS_ANIMATION_FACTOR;
        if (o->SubType == 1)
        {
            o->Angle[1] += (20.0f) * FPS_ANIMATION_FACTOR;
        }
        VectorScale(o->Light, 0.9f, o->Light);
        o->Alpha = 1.0f;
    }
        return true;
    }

    // BITMAP_EVENT_CLOUD
    bool Move_BITMAP_EVENT_CLOUD(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 1)
        {
            o->Scale *= pow(1.08f, FPS_ANIMATION_FACTOR);
            if (o->Scale > 5)
            {
                o->Scale = 5.0f;
            }
            o->Angle[0] = -(int)WorldTime * 0.5f;
            VectorScale(o->Light, 0.9f, o->Light);
        }
        else
        {
            o->Scale *= pow(1.08f, FPS_ANIMATION_FACTOR);
            if (o->Scale > 3)
            {
                o->Scale = 3.0f;
            }
            o->Angle[0] = -(int)WorldTime * 0.5f;
            VectorScale(o->Light, 0.9f, o->Light);
        }
    }
        return true;
    }

    // MODEL_WINDFOCE
    bool Move_MODEL_WINDFOCE(OBJECT* o, int index, float Luminosity)
    {
    {
        o->Angle[2] = -(int)WorldTime * 0.3f;

        if (o->SubType == 0 || o->SubType == 4 || o->SubType == 5)
        {
            o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 3)
            {
                o->Scale = 3;
            }
            o->Alpha *= pow(0.92f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.9f, o->Light);
            if ((int)o->LifeTime == 40 || (int)o->LifeTime == 30)
            {
                vec3_t vLight;
                if (o->SubType == 0)
                {
                    Vector(0.5f, 0.55f, 1.0f, vLight);
                }
                else if (o->SubType == 4)
                {
                    Vector(0.85f, 0.2f, 1.0f, vLight);
                }
                else if (o->SubType == 5)
                {
                    Vector(1.0f, 0.2f, 0.0f, vLight);
                }
                CreateEffectFpsChecked(MODEL_WINDFOCE, o->Position, o->Angle, vLight, 3, o, -1, 0, 0, 0, 1.0f);
            }
        }
        else if (o->SubType == 1)		//지속적인거
        {
            if (o->Owner != NULL && o->Owner->Live == true
                && (g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces)
                    || g_isCharacterBuff(o->Owner, eBuff_Hp_up_Ourforces)
                    || g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces)))
            {
                o->LifeTime = 10;
            }
            else
            {
                o->LifeTime = 0;
            }

            if (g_isCharacterBuff(o->Owner, eBuff_Cloaking))
                return true;

            Vector(0.6f, 0.6f, 1.0f, o->Light);
            auto fFadeInOut = (float)(sinf(WorldTime * 0.001f) + 1.0f);
            VectorScale(o->Light, fFadeInOut * 0.25f + 0.2f, o->Light);
            o->Scale = 1.0f;
            VectorCopy(o->Owner->Position, o->Position);
        }
        else if (o->SubType == 2)
        {
            o->Angle[2] = -(int)WorldTime * 0.9f;
            o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 3)
            {
                o->Scale = 3;
            }
            o->Alpha *= pow(0.92f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.85f, o->Light);

            if ((int)o->LifeTime == 60 || (int)o->LifeTime == 50)
            {
                vec3_t vLight;
                Vector(1.0f, 0.5f, 0.0f, vLight);
                CreateEffectFpsChecked(MODEL_WINDFOCE, o->Position, o->Angle, vLight, 3, o, -1, 0, 0, 0, 1.0f);
            }
        }
        else if (o->SubType == 3)
        {
            o->Angle[2] = -(int)WorldTime * 0.9f;
            o->Scale += (0.1f) * FPS_ANIMATION_FACTOR;
            if (o->Scale > 3)
            {
                o->Scale = 3;
            }
            o->Alpha *= pow(0.92f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.85f, o->Light);
        }
    }
        return true;
    }

    // BITMAP_LIGHT_RED
    bool Move_BITMAP_LIGHT_RED(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        vec3_t Angle;
        vec3_t Position;
    {
        if (o->Owner != NULL && o->Owner->Live == true
            && (g_isCharacterBuff(o->Owner, eBuff_Hp_up_Ourforces)
                || g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces)
                || g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces)))
        {
            o->LifeTime = 10;

            if (g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces))
            {
                if (rand() % 70 == 7)
                {
                    if (!SearchEffect(MODEL_WINDFOCE_MIRROR, o->Owner))
                    {
                        vec3_t vLight;
                        Vector(0.6f, 0.6f, 1.0f, vLight);
                        CreateEffectFpsChecked(MODEL_WINDFOCE_MIRROR, o->Owner->Position, o->Angle, vLight, 0, o->Owner, -1, 0, 0, 0, 1.0f);
                    }
                }
            }
        }
        else if (o->SubType == 3 || o->SubType == 4)
        {
            vec3_t Light;
            float Luminosity = (float)(rand() % 5) * 0.1f;
            Vector(Luminosity + 0.4f, Luminosity + 0.0f, Luminosity + 0.0f, Light);
            int range = 2;
            if (o->SubType == 4)
            {
                range = 3;
            }
            AddTerrainLight(o->Position[0], o->Position[1], Light, range, PrimaryTerrainLight);
        }
        else
        {
            o->LifeTime = 0;
        }

        if (g_isCharacterBuff(o->Owner, eBuff_Cloaking) && (o->SubType != 3))
            return true;

        if (g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces) || o->SubType == 1)
        {
            vec3_t Light, Angle;
            Vector(0.f, 0.f, 0.f, Angle);
            BMD* b = &Models[o->Owner->Type];
            vec3_t Position;

            if (g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces))
            {
                if ((int)o->LifeTime % 2 == 0)
                {
                    VectorCopy(o->Position, Position);
                    Vector(1.0f, 0.12f, 0.0f, Light);
                    b->TransformByObjectBone(Position, o->Owner, (rand_fps_check(2)) ? 26 : 35);
                    CreateParticleFpsChecked(BITMAP_SMOKELINE1, Position, o->Angle, Light, 5, 0.005f, o->Owner);
                }
            }
        }
    }
        return true;
    }

    // MODEL_WINDFOCE_MIRROR
    bool Move_MODEL_WINDFOCE_MIRROR(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->Owner == NULL || o->Owner->Live == false
            || !g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces))
        {
            o->Live = false;
            return true;
        }
        o->Angle[2] = (int)WorldTime * 0.4f;
        o->Scale *= pow(1.015f, FPS_ANIMATION_FACTOR);

        if (o->Scale > 1.5f)
        {
            o->Scale = 1.5f;
        }
        VectorScale(o->Light, 0.9f, o->Light);
        VectorCopy(o->Owner->Position, o->Position);
    }
        return true;
    }

    // BITMAP_SWORD_EFFECT_MONO
    bool Move_BITMAP_SWORD_EFFECT_MONO(OBJECT* o, int index, float Luminosity)
    {
    {
        vec3_t vLight;
        VectorCopy(o->Light, vLight);
        VectorScale(vLight, 0.1f, vLight);
        if ((int)o->LifeTime % 3 != 0)
            CreateParticleFpsChecked(BITMAP_SWORD_EFFECT_MONO, o->Position, o->Angle, o->Light, 0, o->Scale, o);

        if ((int)o->LifeTime == 14)
        {
            Vector(0.4f, 0.4f, 1.0f, vLight);
            if (o->SubType == 0)
            {
                Vector(0.4f, 0.4f, 1.0f, vLight);
            }
            else if (o->SubType == 1)
            {
                Vector(0.7f, 0.5f, 1.0f, vLight);
            }
            else if (o->SubType == 2)
            {
                Vector(1.0f, 0.12f, 0.0f, vLight);
            }
            CreateEffectFpsChecked(MODEL_SHOCKWAVE01, o->Position, o->Angle, vLight, 2, o->Owner, -1, 0, 0, 0, 1.0f);
        }
    }
        return true;
    }

    // MODEL_WOLF_HEAD_EFFECT2
    bool Move_MODEL_WOLF_HEAD_EFFECT2(OBJECT* o, int index, float Luminosity)
    {
        vec3_t Angle;
    {
        if (o->SubType == 4 || o->SubType == 6)
        {
            o->Owner->Velocity += (2.5f) * FPS_ANIMATION_FACTOR;
        }
        else if (o->SubType == 3)
        {
            o->BlendMeshLight = (float)o->LifeTime / 20.0f;
            o->Alpha = o->BlendMeshLight;

            VectorCopy(o->Owner->Angle, o->Angle);

            vec3_t Angle;
            if (o->LifeTime >= 3)
            {
                VectorCopy(o->Owner->Angle, Angle);
                int nRandom = rand() % 6;
                float _Value = 2.0f + rand() % 3;
                if (nRandom / 3)
                    o->Owner->Angle[2] = Angle[2] + (_Value * (nRandom % 3 * 2 + 1));
                else
                    o->Owner->Angle[2] = Angle[2] - (_Value * (nRandom % 3 * 2 + 1));

                int _Temp = rand() % 3;
                if (_Temp == 1)
                    o->Owner->Angle[0] = Angle[0] + (_Value * (nRandom % 3 * 2 + 1));
                else if (_Temp == 2)
                    o->Owner->Angle[0] = Angle[0] - (_Value * (nRandom % 3 * 2 + 1));

                vec3_t pos;
                BMD* b = &Models[o->Owner->Type];
                Vector(0.0f, 0.0f, 0.0f, pos);
                if (o->SubType == 5)
                    b->TransformByObjectBone(pos, o->Owner, 27, pos);
                else
                    b->TransformByObjectBone(pos, o->Owner, 4, pos);

                VectorCopy(pos, o->Position);

                CreateEffectFpsChecked(MODEL_WOLF_HEAD_EFFECT2, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 4, o->Owner);
                CreateEffectFpsChecked(MODEL_WOLF_HEAD_EFFECT2, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 6, o->Owner);
                o->Owner->Angle[2] = Angle[2];
                o->Owner->Angle[0] = Angle[0];
            }
        }
        else if (o->SubType == 4)
        {
            VectorCopy(o->Owner->Angle, o->Angle);
        }
        else if (o->SubType == 5)
        {
            vec3_t pos;
            BMD* b = &Models[o->Owner->Type];
            Vector(0.0f, 0.0f, 0.0f, pos);
            b->TransformByObjectBone(pos, o->Owner, 27, pos);
            VectorCopy(pos, o->Position);
            VectorCopy(o->Owner->Angle, o->Angle);
        }
    }
        return true;
    }

    // MODEL_SHOCKWAVE_GROUND01
    bool Move_MODEL_SHOCKWAVE_GROUND01(OBJECT* o, int index, float Luminosity)
    {
    {
        if (o->SubType == 1)
        {
            o->Scale *= pow(1.2f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.9f, o->Light);
        }
        else if (o->SubType == 2)
        {
            o->Scale *= pow(1.2f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.7f, o->Light);
        }
        else
        {
            o->Scale *= pow(1.2f, FPS_ANIMATION_FACTOR);
            VectorScale(o->Light, 0.8f, o->Light);
        }
    }
        return true;
    }

    // MODEL_DRAGON_LOWER_DUMMY
    bool Move_MODEL_DRAGON_LOWER_DUMMY(OBJECT* o, int index, float Luminosity)
    {
    {
        BMD* pModel = &Models[o->Type];
        pModel->CurrentAction = o->CurrentAction;
        pModel->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity, o->Position, o->Angle);

        if (o->AnimationFrame > 1 && o->AnimationFrame < 6)
        {
            vec3_t temp;
            VectorCopy(o->Position, temp);
            temp[2] = 250.0f;
            CreateParticleFpsChecked(BITMAP_SPARK, temp, o->Angle, o->Light, 12, 1.5f);
        }

        if (o->AnimationFrame > 1 && o->LifeTime > 80)
        {
            vec3_t vLight;
            Vector(1.0f, 1.0f, 1.0f, vLight);
            CreateEffectFpsChecked(BITMAP_LIGHT_RED, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 15.0f);
            o->LifeTime = 80;
        }
        o->Alpha *= pow(0.96f, FPS_ANIMATION_FACTOR);
    }
        return true;
    }

    const std::vector<std::pair<int, MoveHandler>>& ExtractedMoveHandlers()
    {
        static const std::vector<std::pair<int, MoveHandler>> handlers = {
            { MODEL_ARROW_AUTOLOAD, &Move_MODEL_ARROW_AUTOLOAD },
            { MODEL_INFINITY_ARROW, &Move_MODEL_INFINITY_ARROW },
            { MODEL_INFINITY_ARROW1, &Move_MODEL_INFINITY_ARROW1 },
            { MODEL_INFINITY_ARROW2, &Move_MODEL_INFINITY_ARROW1 },
            { MODEL_INFINITY_ARROW3, &Move_MODEL_INFINITY_ARROW1 },
            { MODEL_SHIELD_CRASH, &Move_MODEL_SHIELD_CRASH },
            { MODEL_SHIELD_CRASH2, &Move_MODEL_SHIELD_CRASH2 },
            { MODEL_IRON_RIDER_ARROW, &Move_MODEL_IRON_RIDER_ARROW },
            { MODEL_MULTI_SHOT3, &Move_MODEL_MULTI_SHOT3 },
            { MODEL_MULTI_SHOT1, &Move_MODEL_MULTI_SHOT1 },
            { MODEL_MULTI_SHOT2, &Move_MODEL_MULTI_SHOT2 },
            { MODEL_KENTAUROS_ARROW, &Move_MODEL_KENTAUROS_ARROW },
            { MODEL_WARP3, &Move_MODEL_WARP3 },
            { MODEL_WARP2, &Move_MODEL_WARP3 },
            { MODEL_WARP, &Move_MODEL_WARP3 },
            { MODEL_WARP6, &Move_MODEL_WARP3 },
            { MODEL_WARP5, &Move_MODEL_WARP3 },
            { MODEL_WARP4, &Move_MODEL_WARP3 },
            { MODEL_GHOST, &Move_MODEL_GHOST },
            { MODEL_TREE_ATTACK, &Move_MODEL_TREE_ATTACK },
            { MODEL_BUTTERFLY01, &Move_MODEL_BUTTERFLY01 },
            { MODEL__SPEAR, &Move_MODEL__SPEAR },
            { MODEL_HALLOWEEN_CANDY_BLUE, &Move_MODEL_HALLOWEEN_CANDY_BLUE },
            { MODEL_HALLOWEEN_CANDY_ORANGE, &Move_MODEL_HALLOWEEN_CANDY_BLUE },
            { MODEL_HALLOWEEN_CANDY_YELLOW, &Move_MODEL_HALLOWEEN_CANDY_BLUE },
            { MODEL_HALLOWEEN_CANDY_RED, &Move_MODEL_HALLOWEEN_CANDY_BLUE },
            { MODEL_HALLOWEEN_CANDY_HOBAK, &Move_MODEL_HALLOWEEN_CANDY_BLUE },
            { MODEL_HALLOWEEN_CANDY_STAR, &Move_MODEL_HALLOWEEN_CANDY_BLUE },
            { MODEL_HALLOWEEN_EX, &Move_MODEL_HALLOWEEN_EX },
            { MODEL_XMAS_EVENT_BOX, &Move_MODEL_XMAS_EVENT_BOX },
            { MODEL_XMAS_EVENT_CANDY, &Move_MODEL_XMAS_EVENT_BOX },
            { MODEL_XMAS_EVENT_TREE, &Move_MODEL_XMAS_EVENT_BOX },
            { MODEL_XMAS_EVENT_SOCKS, &Move_MODEL_XMAS_EVENT_BOX },
            { MODEL_XMAS_EVENT_ICEHEART, &Move_MODEL_XMAS_EVENT_ICEHEART },
            { MODEL_NEWYEARSDAY_EVENT_BEKSULKI, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_NEWYEARSDAY_EVENT_CANDY, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_NEWYEARSDAY_EVENT_MONEY, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_NEWYEARSDAY_EVENT_PIG, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_NEWYEARSDAY_EVENT_YUT, &Move_MODEL_NEWYEARSDAY_EVENT_BEKSULKI },
            { MODEL_MOONHARVEST_MOON, &Move_MODEL_MOONHARVEST_MOON },
            { MODEL_MOONHARVEST_GAM, &Move_MODEL_MOONHARVEST_GAM },
            { MODEL_MOONHARVEST_SONGPUEN1, &Move_MODEL_MOONHARVEST_GAM },
            { MODEL_MOONHARVEST_SONGPUEN2, &Move_MODEL_MOONHARVEST_GAM },
            { MODEL_SPEARSKILL, &Move_MODEL_SPEARSKILL },
            { MODEL_SUMMONER_WRISTRING_EFFECT, &Move_MODEL_SUMMONER_WRISTRING_EFFECT },
            { MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, &Move_MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT },
            { MODEL_SUMMONER_EQUIP_HEAD_NEIL, &Move_MODEL_SUMMONER_EQUIP_HEAD_NEIL },
            { MODEL_SUMMONER_SUMMON_SAHAMUTT, &Move_MODEL_SUMMONER_SUMMON_SAHAMUTT },
            { MODEL_SUMMONER_SUMMON_NEIL, &Move_MODEL_SUMMONER_SUMMON_NEIL },
            { MODEL_SUMMONER_SUMMON_LAGUL, &Move_MODEL_SUMMONER_SUMMON_LAGUL },
            { BITMAP_MAGIC, &Move_BITMAP_MAGIC },
            { BITMAP_OUR_INFLUENCE_GROUND, &Move_BITMAP_OUR_INFLUENCE_GROUND },
            { BITMAP_ENEMY_INFLUENCE_GROUND, &Move_BITMAP_OUR_INFLUENCE_GROUND },
            { BITMAP_ORORA, &Move_BITMAP_ORORA },
            { BITMAP_JOINT_THUNDER, &Move_BITMAP_JOINT_THUNDER },
            { BITMAP_IMPACT, &Move_BITMAP_IMPACT },
            { BITMAP_FLAME, &Move_BITMAP_FLAME },
            { MODEL_RAKLION_BOSS_CRACKEFFECT, &Move_MODEL_RAKLION_BOSS_CRACKEFFECT },
            { MODEL_RAKLION_BOSS_MAGIC, &Move_MODEL_RAKLION_BOSS_MAGIC },
            { BITMAP_FIRE_HIK2_MONO, &Move_BITMAP_FIRE_HIK2_MONO },
            { BITMAP_CLOUD, &Move_BITMAP_CLOUD },
            { MODEL_ALICE_BUFFSKILL_EFFECT, &Move_MODEL_ALICE_BUFFSKILL_EFFECT },
            { MODEL_ALICE_BUFFSKILL_EFFECT2, &Move_MODEL_ALICE_BUFFSKILL_EFFECT },
            { MODEL_LIGHTNING_SHOCK, &Move_MODEL_LIGHTNING_SHOCK },
            { MODEL_SKILL_BLAST, &Move_MODEL_SKILL_BLAST },
            { MODEL_WAVE, &Move_MODEL_WAVE },
            { MODEL_TAIL, &Move_MODEL_TAIL },
            { MODEL_WAVE_FORCE, &Move_MODEL_WAVE_FORCE },
            { MODEL_SKILL_INFERNO, &Move_MODEL_SKILL_INFERNO },
            { MODEL_MAGIC_CIRCLE1, &Move_MODEL_MAGIC_CIRCLE1 },
            { MODEL_PROTECT, &Move_MODEL_PROTECT },
            { MODEL_POISON, &Move_MODEL_POISON },
            { MODEL_SAW, &Move_MODEL_SAW },
            { MODEL_LASER, &Move_MODEL_LASER },
            { MODEL_SKILL_WHEEL1, &Move_MODEL_SKILL_WHEEL1 },
            { MODEL_SKILL_FISSURE, &Move_MODEL_SKILL_FISSURE },
            { MODEL_FISSURE, &Move_MODEL_FISSURE },
            { MODEL_SKILL_FURY_STRIKE, &Move_MODEL_SKILL_FURY_STRIKE },
            { MODEL_BALGAS_SKILL, &Move_MODEL_BALGAS_SKILL },
            { MODEL_CHANGE_UP_EFF, &Move_MODEL_CHANGE_UP_EFF },
            { MODEL_CHANGE_UP_NASA, &Move_MODEL_CHANGE_UP_NASA },
            { MODEL_CHANGE_UP_CYLINDER, &Move_MODEL_CHANGE_UP_CYLINDER },
            { MODEL_DARK_ELF_SKILL, &Move_MODEL_DARK_ELF_SKILL },
            { MODEL_MAGIC2, &Move_MODEL_MAGIC2 },
            { MODEL_SUMMON, &Move_MODEL_SUMMON },
            { MODEL_STORM2, &Move_MODEL_STORM2 },
            { MODEL_STORM3, &Move_MODEL_STORM3 },
            { MODEL_MAYASTONE1, &Move_MODEL_MAYASTONE1 },
            { MODEL_MAYASTONE2, &Move_MODEL_MAYASTONE1 },
            { MODEL_MAYASTONE3, &Move_MODEL_MAYASTONE1 },
            { MODEL_MAYASTONE4, &Move_MODEL_MAYASTONE4 },
            { MODEL_MAYASTONE5, &Move_MODEL_MAYASTONE4 },
            { MODEL_MAYASTONEFIRE, &Move_MODEL_MAYASTONEFIRE },
            { MODEL_MAYAHANDSKILL, &Move_MODEL_MAYAHANDSKILL },
            { MODEL_CIRCLE, &Move_MODEL_CIRCLE },
            { MODEL_CIRCLE_LIGHT, &Move_MODEL_CIRCLE_LIGHT },
            { MODEL_ICE_SMALL, &Move_MODEL_ICE_SMALL },
            { MODEL_METEO1, &Move_MODEL_ICE_SMALL },
            { MODEL_METEO2, &Move_MODEL_ICE_SMALL },
            { MODEL_BOSS_ATTACK, &Move_MODEL_ICE_SMALL },
            { MODEL_EFFECT_SAPITRES_ATTACK_2, &Move_MODEL_ICE_SMALL },
            { MODEL_SKULL, &Move_MODEL_SKULL },
            { MODEL_CURSEDTEMPLE_STATUE_PART1, &Move_MODEL_CURSEDTEMPLE_STATUE_PART1 },
            { MODEL_CURSEDTEMPLE_STATUE_PART2, &Move_MODEL_CURSEDTEMPLE_STATUE_PART1 },
            { MODEL_XMAS2008_SNOWMAN_HEAD, &Move_MODEL_XMAS2008_SNOWMAN_HEAD },
            { MODEL_XMAS2008_SNOWMAN_BODY, &Move_MODEL_XMAS2008_SNOWMAN_BODY },
            { MODEL_DOPPELGANGER_SLIME_CHIP, &Move_MODEL_DOPPELGANGER_SLIME_CHIP },
            { MODEL_WATER_WAVE, &Move_MODEL_WATER_WAVE },
            { MODEL_STAFF_OF_DESTRUCTION, &Move_MODEL_STAFF_OF_DESTRUCTION },
            { MODEL_PIERCING, &Move_MODEL_PIERCING },
            { MODEL_ARROW_BEST_CROSSBOW, &Move_MODEL_ARROW_BEST_CROSSBOW },
            { MODEL_ARROW_DOUBLE, &Move_MODEL_ARROW_DOUBLE },
            { MODEL_ARROW_HOLY, &Move_MODEL_ARROW_HOLY },
            { MODEL_ARROW, &Move_MODEL_ARROW },
            { MODEL_ARROW_STEEL, &Move_MODEL_ARROW_STEEL },
            { MODEL_ARROW_THUNDER, &Move_MODEL_ARROW_STEEL },
            { MODEL_ARROW_LASER, &Move_MODEL_ARROW_STEEL },
            { MODEL_ARROW_V, &Move_MODEL_ARROW_STEEL },
            { MODEL_ARROW_SAW, &Move_MODEL_ARROW_STEEL },
            { MODEL_ARROW_NATURE, &Move_MODEL_ARROW_STEEL },
            { MODEL_ARROW_WING, &Move_MODEL_ARROW_STEEL },
            { MODEL_LACEARROW, &Move_MODEL_ARROW_STEEL },
            { MODEL_DARK_SCREAM_FIRE, &Move_MODEL_DARK_SCREAM_FIRE },
            { MODEL_DARK_SCREAM, &Move_MODEL_DARK_SCREAM_FIRE },
            { MODEL_CURSEDTEMPLE_HOLYITEM, &Move_MODEL_CURSEDTEMPLE_HOLYITEM },
            { MODEL_CURSEDTEMPLE_PRODECTION_SKILL, &Move_MODEL_CURSEDTEMPLE_PRODECTION_SKILL },
            { MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, &Move_MODEL_CURSEDTEMPLE_RESTRAINT_SKILL },
            { MODEL_ARROW_SPARK, &Move_MODEL_ARROW_SPARK },
            { MODEL_ARROW_RING, &Move_MODEL_ARROW_RING },
            { MODEL_ARROW_TANKER, &Move_MODEL_ARROW_TANKER },
            { MODEL_ARROW_BOMB, &Move_MODEL_ARROW_BOMB },
            { MODEL_ARROW_DARKSTINGER, &Move_MODEL_ARROW_DARKSTINGER },
            { MODEL_DUNGEON_STONE01, &Move_MODEL_DUNGEON_STONE01 },
            { MODEL_WARCRAFT, &Move_MODEL_WARCRAFT },
            { BITMAP_FIRECRACKERRISE, &Move_BITMAP_FIRECRACKERRISE },
            { BITMAP_FIRECRACKER, &Move_BITMAP_FIRECRACKER },
            { BITMAP_FIRECRACKER0001, &Move_BITMAP_FIRECRACKER0001 },
            { BITMAP_FIRECRACKER0002, &Move_BITMAP_FIRECRACKER0002 },
            { BITMAP_FIRECRACKER0003, &Move_BITMAP_FIRECRACKER0003 },
            { BITMAP_SWORD_FORCE, &Move_BITMAP_SWORD_FORCE },
            { BITMAP_BLIZZARD, &Move_BITMAP_BLIZZARD },
            { BITMAP_SHOTGUN, &Move_BITMAP_SHOTGUN },
            { MODEL_SHINE, &Move_MODEL_SHINE },
            { MODEL_BLIZZARD, &Move_MODEL_BLIZZARD },
            { MODEL_ARROW_DRILL, &Move_MODEL_ARROW_DRILL },
            { MODEL_COMBO, &Move_MODEL_COMBO },
            { MODEL_WAVES, &Move_MODEL_WAVES },
            { MODEL_AIR_FORCE, &Move_MODEL_AIR_FORCE },
            { MODEL_PIERCING2, &Move_MODEL_PIERCING2 },
            { MODEL_DEASULER, &Move_MODEL_DEASULER },
            { MODEL_DEATH_SPI_SKILL, &Move_MODEL_DEATH_SPI_SKILL },
            { MODEL_PIER_PART, &Move_MODEL_PIER_PART },
            { BITMAP_FLARE_FORCE, &Move_BITMAP_FLARE_FORCE },
            { MODEL_DARKLORD_SKILL, &Move_MODEL_DARKLORD_SKILL },
            { MODEL_GROUND_STONE, &Move_MODEL_GROUND_STONE },
            { MODEL_GROUND_STONE2, &Move_MODEL_GROUND_STONE },
            { BITMAP_TWLIGHT, &Move_BITMAP_TWLIGHT },
            { BITMAP_SHOCK_WAVE, &Move_BITMAP_SHOCK_WAVE },
            { BITMAP_DAMAGE_01_MONO, &Move_BITMAP_DAMAGE_01_MONO },
            { BITMAP_FLARE, &Move_BITMAP_FLARE },
            { MODEL_CUNDUN_DRAGON_HEAD, &Move_MODEL_CUNDUN_DRAGON_HEAD },
            { MODEL_CUNDUN_PHOENIX, &Move_MODEL_CUNDUN_PHOENIX },
            { MODEL_CUNDUN_GHOST, &Move_MODEL_CUNDUN_GHOST },
            { MODEL_CUNDUN_SKILL, &Move_MODEL_CUNDUN_SKILL },
            { MODEL_BATTLE_GUARD2, &Move_MODEL_BATTLE_GUARD2 },
            { MODEL_ARROW_TANKER_HIT, &Move_MODEL_ARROW_TANKER_HIT },
            { MODEL_FLY_BIG_STONE1, &Move_MODEL_FLY_BIG_STONE1 },
            { MODEL_FLY_BIG_STONE2, &Move_MODEL_FLY_BIG_STONE2 },
            { MODEL_BIG_STONE_PART1, &Move_MODEL_BIG_STONE_PART1 },
            { MODEL_BIG_STONE_PART2, &Move_MODEL_BIG_STONE_PART1 },
            { MODEL_WALL_PART1, &Move_MODEL_BIG_STONE_PART1 },
            { MODEL_WALL_PART2, &Move_MODEL_BIG_STONE_PART1 },
            { MODEL_GOLEM_STONE, &Move_MODEL_BIG_STONE_PART1 },
            { MODEL_GATE_PART1, &Move_MODEL_GATE_PART1 },
            { MODEL_GATE_PART2, &Move_MODEL_GATE_PART1 },
            { MODEL_GATE_PART3, &Move_MODEL_GATE_PART1 },
            { MODEL_AURORA, &Move_MODEL_AURORA },
            { MODEL_FENRIR_THUNDER, &Move_MODEL_FENRIR_THUNDER },
            { MODEL_FALL_STONE_EFFECT, &Move_MODEL_FALL_STONE_EFFECT },
            { MODEL_FENRIR_FOOT_THUNDER, &Move_MODEL_FENRIR_FOOT_THUNDER },
            { MODEL_TWINTAIL_EFFECT, &Move_MODEL_TWINTAIL_EFFECT },
            { MODEL_TOWER_GATE_PLANE, &Move_MODEL_TOWER_GATE_PLANE },
            { BITMAP_CRATER, &Move_BITMAP_CRATER },
            { BITMAP_CHROME_ENERGY2, &Move_BITMAP_CHROME_ENERGY2 },
            { MODEL_STUN_STONE, &Move_MODEL_STUN_STONE },
            { MODEL_SKIN_SHELL, &Move_MODEL_SKIN_SHELL },
            { MODEL_MANA_RUNE, &Move_MODEL_MANA_RUNE },
            { MODEL_SKILL_JAVELIN, &Move_MODEL_SKILL_JAVELIN },
            { MODEL_ARROW_IMPACT, &Move_MODEL_ARROW_IMPACT },
            { MODEL_SWORD_FORCE, &Move_MODEL_SWORD_FORCE },
            { MODEL_MOVE_TARGETPOSITION_EFFECT, &Move_MODEL_MOVE_TARGETPOSITION_EFFECT },
            { BITMAP_TARGET_POSITION_EFFECT1, &Move_BITMAP_TARGET_POSITION_EFFECT1 },
            { BITMAP_TARGET_POSITION_EFFECT2, &Move_BITMAP_TARGET_POSITION_EFFECT2 },
            { MODEL_EFFECT_SAPITRES_ATTACK, &Move_MODEL_EFFECT_SAPITRES_ATTACK },
            { MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1, &Move_MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1 },
            { MODEL_EFFECT_SKURA_ITEM, &Move_MODEL_EFFECT_SKURA_ITEM },
            { MODEL_BLOW_OF_DESTRUCTION, &Move_MODEL_BLOW_OF_DESTRUCTION },
            { MODEL_NIGHTWATER_01, &Move_MODEL_NIGHTWATER_01 },
            { MODEL_KNIGHT_PLANCRACK_A, &Move_MODEL_KNIGHT_PLANCRACK_A },
            { MODEL_KNIGHT_PLANCRACK_B, &Move_MODEL_KNIGHT_PLANCRACK_B },
            { MODEL_1_STREAMBREATHFIRE, &Move_MODEL_1_STREAMBREATHFIRE },
            { MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD, &Move_MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD },
            { MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD, &Move_MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD },
            { MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY, &Move_MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY },
            { MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY, &Move_MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY },
            { MODEL_LAVAGIANT_FOOTPRINT_R, &Move_MODEL_LAVAGIANT_FOOTPRINT_R },
            { MODEL_LAVAGIANT_FOOTPRINT_V, &Move_MODEL_LAVAGIANT_FOOTPRINT_R },
            { MODEL_PROJECTILE, &Move_MODEL_PROJECTILE },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE01, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE02, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE03, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE04, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE05, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE06, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE07, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE08, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE09, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE11, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE12, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE13, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_STATUE_CRUSH_EFFECT_PIECE01, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_STATUE_CRUSH_EFFECT_PIECE02, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_STATUE_CRUSH_EFFECT_PIECE03, &Move_MODEL_DOOR_CRUSH_EFFECT_PIECE01 },
            { MODEL_STATUE_CRUSH_EFFECT_PIECE04, &Move_MODEL_STATUE_CRUSH_EFFECT_PIECE04 },
            { MODEL_DOOR_CRUSH_EFFECT_PIECE10, &Move_MODEL_STATUE_CRUSH_EFFECT_PIECE04 },
            { MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION, &Move_MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION },
            { MODEL_EFFECT_SD_AURA, &Move_MODEL_EFFECT_SD_AURA },
            { MODEL_WOLF_HEAD_EFFECT, &Move_MODEL_WOLF_HEAD_EFFECT },
            { BITMAP_SBUMB, &Move_BITMAP_SBUMB },
            { MODEL_DOWN_ATTACK_DUMMY_L, &Move_MODEL_DOWN_ATTACK_DUMMY_L },
            { MODEL_DOWN_ATTACK_DUMMY_R, &Move_MODEL_DOWN_ATTACK_DUMMY_R },
            { BITMAP_SWORDEFF, &Move_BITMAP_SWORDEFF },
            { MODEL_SHOCKWAVE01, &Move_MODEL_SHOCKWAVE01 },
            { MODEL_SHOCKWAVE02, &Move_MODEL_SHOCKWAVE02 },
            { BITMAP_DAMAGE1, &Move_BITMAP_DAMAGE1 },
            { MODEL_SHOCKWAVE_SPIN01, &Move_MODEL_SHOCKWAVE_SPIN01 },
            { BITMAP_EVENT_CLOUD, &Move_BITMAP_EVENT_CLOUD },
            { MODEL_WINDFOCE, &Move_MODEL_WINDFOCE },
            { BITMAP_LIGHT_RED, &Move_BITMAP_LIGHT_RED },
            { MODEL_WINDFOCE_MIRROR, &Move_MODEL_WINDFOCE_MIRROR },
            { BITMAP_SWORD_EFFECT_MONO, &Move_BITMAP_SWORD_EFFECT_MONO },
            { MODEL_WOLF_HEAD_EFFECT2, &Move_MODEL_WOLF_HEAD_EFFECT2 },
            { MODEL_SHOCKWAVE_GROUND01, &Move_MODEL_SHOCKWAVE_GROUND01 },
            { MODEL_DRAGON_LOWER_DUMMY, &Move_MODEL_DRAGON_LOWER_DUMMY },
        };
        return handlers;
    }
}
