// GMEmpireGuardian1.cpp: implementation of the GMEmpireGuardian1 class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "ZzzLodTerrain.h"
#include "BoneManager.h"
#include "PhysicsManager.h"
#include "MapManager.h"
#include "GOBoid.h"



GMEmpireGuardian1Ptr GMEmpireGuardian1::Make()
{
    GMEmpireGuardian1Ptr empire(new GMEmpireGuardian1);
    empire->Init();
    return empire;
}

GMEmpireGuardian1::GMEmpireGuardian1()
{
    m_iWeather = (int)WEATHER_SUN;
}

GMEmpireGuardian1::~GMEmpireGuardian1()
{
    Destroy();
}

void GMEmpireGuardian1::Init()
{
}

void GMEmpireGuardian1::Destroy()
{
}

bool GMEmpireGuardian1::CreateObject(OBJECT* o)
{
    switch (o->Type)
    {
    case 129:
    case 130:
    case 131:
    case 132:
    {
        o->Angle[2] = (float)((int)o->Angle[2] % 360);
        VectorCopy(o->Angle, o->HeadAngle);
        VectorCopy(o->Position, o->HeadTargetAngle);
    }
    return true;

    case 115:
    case 117:
    {
        o->SubType = 100;
    }
    return true;
    }

    return false;
}

CHARACTER* GMEmpireGuardian1::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case MONSTER_RAYMOND:
    {
        OpenMonsterModel(MONSTER_MODEL_RAYMOND);
        pCharacter = CreateCharacter(Key, MODEL_RAYMOND, PosX, PosY);

        pCharacter->Object.Scale = 1.45f;

        m_bCurrentIsRage_Raymond = false;
    }
    break;
    case MONSTER_LUCAS:
    {
        OpenMonsterModel(MONSTER_MODEL_LUCAS);
        pCharacter = CreateCharacter(Key, MODEL_LUCAS, PosX, PosY);

        pCharacter->Object.Scale = 1.25f;

        m_bCurrentIsRage_Ercanne = false;
    }
    break;
    case MONSTER_FRED:
    {
        OpenMonsterModel(MONSTER_MODEL_FRED);
        pCharacter = CreateCharacter(Key, MODEL_FRED, PosX, PosY);
        pCharacter->Object.Scale = 1.55f;

        BoneManager::RegisterBone(pCharacter, L"node_eyes01", 14);
        BoneManager::RegisterBone(pCharacter, L"node_eyes02", 15);
        BoneManager::RegisterBone(pCharacter, L"node_blade01", 71);
        BoneManager::RegisterBone(pCharacter, L"node_blade05", 72);
        BoneManager::RegisterBone(pCharacter, L"node_blade04", 73);
        BoneManager::RegisterBone(pCharacter, L"node_blade02", 74);
        //			BoneManager::RegisterBone(pCharacter, L"node_blade03" ,75);

        OBJECT* o = &pCharacter->Object;
        BMD* b = &Models[o->Type];

        MoveEye(o, b, 14, 15, 71, 72, 73, 74);
        vec3_t vColor = { 0.7f, 0.7f, 1.0f };
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 24, o, 10.f, -1, 0, 0, -1, vColor);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 25, o, 10.f, -1, 0, 0, -1, vColor);
        Vector(0.7f, 0.7f, 1.0f, vColor);
        float Sca = 100.f;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 54, o, Sca, 0, 0, 0, -1, vColor);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 54, o, Sca, 1, 0, 0, -1, vColor);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 54, o, Sca, 2, 0, 0, -1, vColor);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 54, o, Sca, 3, 0, 0, -1, vColor);

        m_bCurrentIsRage_Daesuler = false;
    }
    break;
    case MONSTER_DEVIL_LORD:
    {
        OpenMonsterModel(MONSTER_MODEL_DEVIL_LORD);
        pCharacter = CreateCharacter(Key, MODEL_DEVIL_LORD, PosX, PosY);
        pCharacter->Object.Scale = 1.35f;

        m_bCurrentIsRage_Gallia = false;
    }
    break;
    case MONSTER_QUARTER_MASTER:
    {
        OpenMonsterModel(MONSTER_MODEL_QUARTER_MASTER);
        pCharacter = CreateCharacter(Key, MODEL_QUARTER_MASTER, PosX, PosY);
        pCharacter->Object.Scale = 1.27f;
    }
    break;
    case MONSTER_COMBAT_INSTRUCTOR:
    {
        OpenMonsterModel(MONSTER_MODEL_COMBAT_INSTRUCTOR);
        pCharacter = CreateCharacter(Key, MODEL_COMBAT_INSTRUCTOR, PosX, PosY);
        pCharacter->Object.Scale = 1.25f;
    }
    break;
    case MONSTER_DEFENDER:
    {
        OpenMonsterModel(MONSTER_MODEL_DEFENDER);
        pCharacter = CreateCharacter(Key, MODEL_DEFENDER, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;

        Vector(0.0f, 0.0f, 0.0f, pCharacter->Object.EyeRight3);
        Vector(0.0f, 0.0f, 0.0f, pCharacter->Object.EyeLeft3);
    }
    break;
    case MONSTER_FORSAKER:
    {
        OpenMonsterModel(MONSTER_MODEL_FORSAKER);
        pCharacter = CreateCharacter(Key, MODEL_FORSAKER, PosX, PosY);
        pCharacter->Object.Scale = 0.9f;
    }
    break;
    case MONSTER_OCELOT_THE_LORD:
    {
        OpenMonsterModel(MONSTER_MODEL_OCELOT);
        pCharacter = CreateCharacter(Key, MODEL_OCELOT, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
    }
    break;
    case MONSTER_ERIC_THE_GUARD:
    {
        OpenMonsterModel(MONSTER_MODEL_ERIC);
        pCharacter = CreateCharacter(Key, MODEL_ERIC, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
    }
    break;
    case MONSTER_EVIL_GATE:
    {
        OpenMonsterModel(MONSTER_MODEL_EVIL_GATE);
        pCharacter = CreateCharacter(Key, MODEL_EVIL_GATE, PosX, PosY);
        pCharacter->Object.m_bRenderShadow = false;
        pCharacter->Object.Scale = 1.25f;
    }
    break;
    case MONSTER_LION_GATE:
    {
        OpenMonsterModel(MONSTER_MODEL_LION_GATE);
        pCharacter = CreateCharacter(Key, MODEL_LION_GATE, PosX, PosY);
        pCharacter->Object.m_bRenderShadow = false;
        pCharacter->Object.LifeTime = 100;
        pCharacter->Object.Scale = 1.25f;
    }
    break;
    case MONSTER_STATUE:
    {
        OpenMonsterModel(MONSTER_MODEL_STATUE);
        pCharacter = CreateCharacter(Key, MODEL_STATUE, PosX, PosY);
        pCharacter->Object.m_bRenderShadow = false;
        pCharacter->Object.Scale = 0.6f;
        pCharacter->Object.LifeTime = 100;
    }
    break;

    default: return pCharacter;
    }

    return pCharacter;
}

bool GMEmpireGuardian1::MoveObject(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian1() == false) return false;

    Alpha(o);
    if (o->Alpha < 0.01f) return false;

    BMD* b = &Models[o->Type];
    float fSpeed = o->Velocity;
    switch (o->Type)
    {
    case 20:
    {
        fSpeed *= 2.0f;
    }
    break;

    case 122:
    case 123:
    case 124:
    {
        fSpeed *= 3.0f;
    }
    break;

    case 128:
    {
        fSpeed *= 6.0f;
    }
    break;
    }

    b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, fSpeed, o->Position, o->Angle);

    switch (o->Type)
    {
    case 20:
    {
        static float fAniFrame = 0;

        if (fAniFrame - o->AnimationFrame > 10 || fAniFrame < o->AnimationFrame)
            fAniFrame = o->AnimationFrame;
        else
            o->AnimationFrame = fAniFrame;
    }
    return true;
    case 64:
    {
        o->Velocity = 0.64f;
    }
    return true;
    case 79:
    case 80:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 129:
    case 130:
    case 131:
    case 132:
    {
        o->HiddenMesh = -2;
    }
    return true;
    case 81:
    {
        o->BlendMeshTexCoordV += 0.015f;
    }
    return true;
    case 36:
    {
        o->Velocity = 0.02f;
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian1::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return false;
    }

    vec3_t vPos, vRelativePos, Light;

    switch (o->Type)
    {
    case MODEL_LION_GATE:
    {
        if (o->CurrentAction == MONSTER01_DIE)
        {
            if ((int)o->LifeTime == 100)
            {
                o->LifeTime = 90;

                Vector(0, 0, 200.0f, vRelativePos);
                b->TransformPosition(o->BoneTransform[0], vRelativePos, vPos, true);
                CreateEffect(MODEL_DOOR_CRUSH_EFFECT, vPos, o->Angle, o->Light, 0, o, 0, 0);
            }
        }
    }
    return true;
    case MODEL_STATUE:
    {
        if (o->CurrentAction == MONSTER01_DIE)
        {
            if ((int)o->LifeTime == 100)
            {
                o->LifeTime = 90;

                Vector(0, 0, 0.0f, vRelativePos);
                b->TransformPosition(o->BoneTransform[1], vRelativePos, vPos, true);
                CreateEffect(MODEL_STATUE_CRUSH_EFFECT, vPos, o->Angle, o->Light, 0, o, 0, 0);

                CreateEffect(MODEL_STATUE_CRUSH_EFFECT_PIECE04, o->Position, o->Angle, o->Light, 0);

                vec3_t Angle;
                Vector(0.f, 0.f, (float)(rand() % 360), Angle);

                CreateEffect(MODEL_STONE1 + rand() % 2, vPos, Angle, o->Light, 0);
                CreateEffect(MODEL_STONE1 + rand() % 2, vPos, Angle, o->Light, 0);
                CreateEffect(MODEL_STONE1 + rand() % 2, vPos, Angle, o->Light, 0);
            }
        }
        else
        {
            float Luminosity = sinf(WorldTime * 0.003f) * 0.2f + 0.6f;
            Vector(1.0f * Luminosity, 0.8f * Luminosity, 0.2f * Luminosity, Light);

            b->TransformByObjectBone(vPos, o, 3);
            CreateSprite(BITMAP_LIGHT_RED, vPos, 1.0f, Light, o);
            b->TransformByObjectBone(vPos, o, 4);
            CreateSprite(BITMAP_LIGHT_RED, vPos, 1.0f, Light, o);
        }
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian1::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return false;
    }

    vec3_t vPos, Light;

    switch (o->Type)
    {
    case MODEL_RAYMOND:
    {
        if (!rand_fps_check(1))
        {
            break;
        }

        float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;
        vec3_t EndPos, EndRelative, Light, vPos;
        Vector(1.0f, 1.0f, 1.0f, Light);

        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
        case MONSTER01_ATTACK1:
        case MONSTER01_APEAR:
        {
            {
                vec3_t		Light;

                if (m_bCurrentIsRage_Raymond == true)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateInferno(o->Position);

                    CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    m_bCurrentIsRage_Raymond = false;
                }
            }
        }
        break;
        case MONSTER01_DIE:
            break;
        case MONSTER01_WALK:
        {
            Vector(0.9f, 0.2f, 0.1f, Light);
            if (7.5f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                b->TransformByObjectBone(vPos, o, 54);
                CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, Light, 0, 0.1f);
            }
            if (0.0f <= o->AnimationFrame && o->AnimationFrame < 1.0f)
            {
                b->TransformByObjectBone(vPos, o, 59);
                CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, Light, 0, 0.1f);
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (o->AnimationFrame >= 2.5f && o->AnimationFrame < (2.5f + _fActSpdTemp))
            {
                Vector(0.0f, 0.0f, 100.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[31], EndRelative, EndPos, true);
                CreateEffect(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 0, 0);
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (o->AnimationFrame >= 7.0f && o->AnimationFrame < (7.0f + _fActSpdTemp))
            {
                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, o->Light, 0, 0, -1, 0, 0, 0);
                CreateEffect(MODEL_CIRCLE_LIGHT, o->Position, o->Angle, o->Light, 0, 0, -1, 0, 0, 0);
            }
            Vector(o->Position[0] + rand() % 1024 - 512, o->Position[1] + rand() % 1024 - 512, o->Position[2], EndPos);
            CreateEffect(MODEL_FIRE, EndPos, o->Angle, o->Light);
        }
        break;
        }
    }
    return true;
    case MODEL_LUCAS:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
            break;
        case MONSTER01_WALK:
            break;
        case MONSTER01_DIE:
        {
            // 					float Scale = 0.3f;
            // 					b->TransformByObjectBone( vPos, o, 30 );
            // 					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
            //
            // 					b->TransformByObjectBone( vPos, o, 17 );
            // 					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
        }
        break;
        case MONSTER01_ATTACK1:
        {
            if (2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f)
            {
                vec3_t  Light;
                //Vector(0.3f, 0.3f, 0.3f, Light);
                Vector(0.3f, 0.8f, 0.4f, Light);

                vec3_t StartPos, StartRelative;
                vec3_t EndPos, EndRelative;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 25; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 4.4f)
            {
                int SkillIndex = 9;
                OBJECT* pObj = o;
                vec3_t ap, P, dp;

                VectorCopy(pObj->Position, ap);
                CreateEffectFpsChecked(MODEL_DARK_SCREAM, pObj->Position, pObj->Angle, pObj->Light, 1, pObj, pObj->PKKey, SkillIndex);
                CreateEffectFpsChecked(MODEL_DARK_SCREAM_FIRE, pObj->Position, pObj->Angle, pObj->Light, 1, pObj, pObj->PKKey, SkillIndex);

                Vector(80.f, 0.f, 0.f, P);

                pObj->Angle[2] += 10.f;

                AngleMatrix(pObj->Angle, pObj->Matrix);
                VectorRotate(P, pObj->Matrix, dp);
                VectorAdd(dp, pObj->Position, pObj->Position);
                CreateEffectFpsChecked(MODEL_DARK_SCREAM, pObj->Position, pObj->Angle, pObj->Light, 1, pObj, pObj->PKKey, SkillIndex);
                CreateEffectFpsChecked(MODEL_DARK_SCREAM_FIRE, pObj->Position, pObj->Angle, pObj->Light, 1, pObj, pObj->PKKey, SkillIndex);

                VectorCopy(ap, pObj->Position);
                VectorCopy(pObj->Position, ap);
                Vector(-80.f, 0.f, 0.f, P);
                pObj->Angle[2] -= 20.f;

                AngleMatrix(pObj->Angle, pObj->Matrix);
                VectorRotate(P, pObj->Matrix, dp);
                VectorAdd(dp, pObj->Position, pObj->Position);
                CreateEffectFpsChecked(MODEL_DARK_SCREAM, pObj->Position, pObj->Angle, pObj->Light, 1, pObj, pObj->PKKey, SkillIndex);
                CreateEffectFpsChecked(MODEL_DARK_SCREAM_FIRE, pObj->Position, pObj->Angle, pObj->Light, 1, pObj, pObj->PKKey, SkillIndex);
                VectorCopy(ap, pObj->Position);
            }

            if (2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f)
            {
                vec3_t  Light;
                //Vector(0.3f, 0.3f, 0.3f, Light);
                Vector(0.3f, 0.8f, 0.4f, Light);

                vec3_t StartPos, StartRelative;
                vec3_t EndPos, EndRelative;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 25; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (o->AnimationFrame >= 3.4f && o->AnimationFrame <= 9.0f)
            {
                RenderSkillEarthQuake(c, o, b, 12);
            }

            if (2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f)
            {
                vec3_t  Light;
                //Vector(0.3f, 0.3f, 0.3f, Light);
                Vector(0.3f, 0.8f, 0.4f, Light);

                vec3_t StartPos, StartRelative;
                vec3_t EndPos, EndRelative;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 25; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            {
                vec3_t		Light;

                if (m_bCurrentIsRage_Ercanne == true)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateInferno(o->Position);

                    CreateEffectFpsChecked(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    CreateEffectFpsChecked(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    m_bCurrentIsRage_Ercanne = false;
                }
            }
        }
        break;
        }
    }
    return true;
    case MODEL_FRED:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
            break;
        case MONSTER01_WALK:
            break;
        case MONSTER01_DIE:
        {
        }
        break;
        case MONSTER01_ATTACK1:
        {
        }
        break;
        case MONSTER01_ATTACK2:
        {
            float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;
            if (o->AnimationFrame >= 4.5f && o->AnimationFrame < (4.5f + _fActSpdTemp))
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t Angle, AngleRelative = { 0.0f, 0.0f, 0.0f };
                vec3_t v3PosTo = { to->Position[0], to->Position[1], to->Position[2] };
                vec3_t v3RelativePos, v3StartPos;

                Vector(o->Angle[0] + AngleRelative[0],
                    o->Angle[1] + AngleRelative[1],
                    o->Angle[2] + AngleRelative[2],
                    Angle);

                Vector(0.0f, 0.0f, 0.0f, v3RelativePos);
                b->TransformPosition(o->BoneTransform[20], v3RelativePos, v3StartPos, true);
                CreateEffectFpsChecked(MODEL_DEASULER, v3StartPos, Angle, v3PosTo, 0, o, -1, 0, 0, 0, 1.8f);
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            vec3_t Angle, p, Position;
            float Matrix[3][4];
            Vector(0.f, -500.f, 0.f, p);
            for (int j = 0; j < 3; j++)
            {
                Vector((float)(rand() % 90), 0.f, (float)(rand() % 360), Angle);
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorSubtract(o->Position, Position, Position);
                Position[2] += 120.f;
                CreateJointFpsChecked(BITMAP_JOINT_HEALING, Position, o->Position, Angle, 17, o, 10.f);
            }
            vec3_t Light;
            Vector(1.f, 0.1f, 0.f, Light);
            Vector(0.f, 0.f, 0.f, p);
            for (int i = 0; i < 10; i++)
            {
                b->TransformPosition(o->BoneTransform[rand() % 62], p, Position, true);
                CreateParticleFpsChecked(BITMAP_LIGHT, Position, o->Angle, Light, 5, 0.5f + (rand() % 100) / 50.f);

                b->TransformPosition(o->BoneTransform[50], p, Position, true);
                CreateParticleFpsChecked(BITMAP_LIGHT, Position, o->Angle, Light, 5, 0.5f + (rand() % 100) / 50.f);
            }

            float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;

            if (o->AnimationFrame >= 4.5f && o->AnimationFrame < (4.5f + _fActSpdTemp))

            {
                vec3_t EndPos, EndRelative;

                Vector(0.0f, 0.0f, 0.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[11], EndRelative, EndPos, true);
                CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 0);
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            {
                vec3_t		Light;

                if (m_bCurrentIsRage_Daesuler == true)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateInferno(o->Position);

                    CreateEffectFpsChecked(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    CreateEffectFpsChecked(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    m_bCurrentIsRage_Daesuler = false;
                }
            }
        }
        break;
        }
    }
    break;
    case MODEL_DEVIL_LORD:
    {
        //wing node 1,2,3
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 81);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.5f, Light, o);
        b->TransformByObjectBone(vPos, o, 82);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 1.3f, Light, o);
        b->TransformByObjectBone(vPos, o, 83);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

        //wing node 4,5,6
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 105);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.5f, Light, o);
        b->TransformByObjectBone(vPos, o, 104);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 1.3f, Light, o);
        b->TransformByObjectBone(vPos, o, 103);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

        //head
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 10);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 1.5f, Light, o);

        //neak
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 9);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

        //spine
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 6);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 4.0f, Light, o);

        //L arm
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 13);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);
        b->TransformByObjectBone(vPos, o, 30);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

        //R arm
        Vector(1.0f, 0.8f, 0.2f, Light);
        b->TransformByObjectBone(vPos, o, 32);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);
        b->TransformByObjectBone(vPos, o, 75);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

        //sword
        float Luminosity = sinf(WorldTime * 0.003f) * 0.2f + 0.6f;
        Vector(1.0f * Luminosity, 0.5f * Luminosity, 0.4f * Luminosity, Light);

        int temp[] = { 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
                        59, 60, 61, 62, 63, 64, 65, 66, 67, 71, 72 };

        for (int i = 0; i < 21; i++)
        {
            b->TransformByObjectBone(vPos, o, temp[i]);
            CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.6f, Light, o);
        }
        b->TransformByObjectBone(vPos, o, 68);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.3f, Light, o);
        b->TransformByObjectBone(vPos, o, 69);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.2f, Light, o);
        b->TransformByObjectBone(vPos, o, 70);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.3f, Light, o);

        // Action 정의
        float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;

        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
            break;
        case MONSTER01_WALK:
            break;
        case MONSTER01_DIE:
        {
        }
        break;
        case MONSTER01_ATTACK1:
        {
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (o->AnimationFrame >= 2.5f && o->AnimationFrame < (2.5f + _fActSpdTemp))
            {
                vec3_t EndPos, EndRelative;

                Vector(0.0f, 0.0f, 100.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[11], EndRelative, EndPos, true);
                CreateEffectFpsChecked(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 0);
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (o->AnimationFrame >= 3.4f && o->AnimationFrame <= 9.0f)
            {
                RenderSkillEarthQuake(c, o, b, 12);
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            {
                vec3_t		Light;

                if (m_bCurrentIsRage_Gallia == true)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateInferno(o->Position);

                    CreateEffectFpsChecked(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    CreateEffectFpsChecked(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    m_bCurrentIsRage_Gallia = false;
                }
            }
        }
        break;
        }
    }
    return true;

    case MODEL_COMBAT_INSTRUCTOR:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            Vector(0.5f, 0.2f, 0.1f, o->Light);

            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                b->TransformByObjectBone(vPos, o, 53);
                vPos[2] += 25.0f;
                CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                b->TransformByObjectBone(vPos, o, 48);
                vPos[2] += 25.0f;
                CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
        }
        break;
        case MONSTER01_ATTACK2:
        {
        }
        break;
        case MONSTER01_APEAR:
        {
            if (o->AnimationFrame >= 5.0f && o->AnimationFrame <= 5.8f)
            {
                CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 1);
                CreateEffectFpsChecked(MODEL_WAVES, o->Position, o->Angle, o->Light, 1);
                CreateEffectFpsChecked(MODEL_PIERCING2, o->Position, o->Angle, o->Light);
                PlayBuffer(SOUND_ATTACK_SPEAR);
            }
        }
        break;
        }
    }
    return true;
    case MODEL_DEFENDER:
    {
        // HeadTargetAngle
        switch (o->CurrentAction)
        {
        case MONSTER01_ATTACK1:
        {
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 5.0f)
            {
                vec3_t vLook, vPosition, vLight;
                float matRotate[3][4];
                Vector(0.0f, -100.0f, 100.0f, vPosition);
                AngleMatrix(o->Angle, matRotate);
                VectorRotate(vPosition, matRotate, vLook);
                VectorAdd(o->Position, vLook, vPosition);

                Vector(0.8f, 0.8f, 1.0f, vLight);

                CreateEffectFpsChecked(MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2, vPosition, o->Angle, vLight, 0, o);
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            VectorCopy(o->EyeRight3, o->Angle);
        }
        break;
        };
        VectorCopy(o->Angle, o->EyeRight3);
    }
    return true;
    case MODEL_FORSAKER:
    {
        vec3_t vRelative, vLight, vPos;
        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
        case MONSTER01_WALK:
        {
            Vector(10.0f, 0.0f, 0.0f, vRelative);
            b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
            float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.3f + 0.4f;
            Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, vLight);
            CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.5f, vLight, o);
            Vector(0.5f, 0.5f, 0.5f, vLight);

            CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f, vLight, o, -WorldTime * 0.1f);
        }
        break;
        case MONSTER01_DIE:
            break;
        case MONSTER01_ATTACK1:
        {
            Vector(10.0f, 0.0f, 0.0f, vRelative);
            b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
            float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.3f + 0.4f;
            float fScale = (sinf(WorldTime * 0.001f) + 1.0f) * 0.5f + 1.0f;
            Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, vLight);
            CreateSprite(BITMAP_FLARE_RED, vPos, 1.5f, vLight, o);
            Vector(0.5f, 0.5f, 0.5f, vLight);
            CreateSprite(BITMAP_FLARE_RED, vPos, 1.2f * fScale * o->AnimationFrame * 0.15f, vLight, o, -WorldTime * 0.1f);

            CHARACTER* tc = &CharactersClient[c->TargetCharacter];
            OBJECT* to = &tc->Object;

            if (o->AnimationFrame > 10)
            {
                vec3_t p, Angle;
                Vector(0.0f, 0.0f, 0.0f, p);
                Vector(0.88f, 0.12f, 0.08f, vLight);
                b->TransformPosition(o->BoneTransform[55], p, vPos, true);
                Vector(-60.0f, 0.0f, o->Angle[2], Angle);
                Vector(-60.0f, 0.0f, o->Angle[2], Angle);
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, vPos, to->Position, Angle, 27, to, 50.0f, -1, 0, 0, 0, vLight);
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, vPos, to->Position, Angle, 27, to, 10.0f, -1, 0, 0, 0, vLight);
                CreateParticleFpsChecked(BITMAP_ENERGY, vPos, o->Angle, vLight);
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            Vector(10.0f, 0.0f, 0.0f, vRelative);
            b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
            float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.3f + 0.4f;
            float fScale = (sinf(WorldTime * 0.001f) + 1.0f) * 0.5f + 1.0f;
            Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, vLight);
            CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.5f, vLight, o);
            Vector(0.5f, 0.5f, 0.5f, vLight);
            //CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f, vLight, o, -WorldTime*0.1f);
            CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f * fScale * o->AnimationFrame * 0.15f, vLight, o, -WorldTime * 0.1f);

            CHARACTER* tc = &CharactersClient[c->TargetCharacter];
            OBJECT* to = &tc->Object;

            if (o->AnimationFrame > 4.5f && o->AnimationFrame <= 4.8f)
                CreateEffectFpsChecked(BITMAP_MAGIC + 1, to->Position, to->Angle, to->Light, 1, to);
        }
        break;
        }
    }
    return true;
    } //switch end

    return false;
}

void GMEmpireGuardian1::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    vec3_t  Light;
    vec3_t StartPos, StartRelative;
    vec3_t EndPos, EndRelative;
    Vector(1.0f, 1.0f, 1.0f, Light);
    Vector(0.0f, 0.0f, 0.0f, StartRelative);
    Vector(0.0f, 0.0f, 0.0f, EndRelative);

    float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
    float fSpeedPerFrame = fActionSpeed / 10.f;
    float fAnimationFrame = o->AnimationFrame - fActionSpeed;

    switch (o->Type)
    {
    case MODEL_RAYMOND:
    {
        float Start_Frame = 0.0f;
        float End_Frame = 0.0f;

        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
        case MONSTER01_WALK:
        case MONSTER01_DIE:
            break;
        case MONSTER01_ATTACK1:
        {
            Start_Frame = 4.0f;
            End_Frame = 11.0f;
        }
        break;
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK3:
        {
            Start_Frame = 5.0f;
            End_Frame = 10.0f;
        }
        break;
        }

        if ((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame)
            && ((o->CurrentAction == MONSTER01_ATTACK1) || (o->CurrentAction == MONSTER01_ATTACK2) || (o->CurrentAction == MONSTER01_ATTACK3))
            )
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

                Vector(0.9f, 0.2f, 0.1f, Light);
                b->TransformPosition(BoneTransform[32], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[33], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_DEVIL_LORD:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_ATTACK1:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 6.0f)
            {
                Vector(1.0f, 0.2f, 0.0f, Light);
                Vector(0.0f, 0.0f, 0.0f, StartRelative);
                Vector(0.0f, 0.0f, 0.0f, EndRelative);

                for (int i = 0; i < 28; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[69], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[49], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 7);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 7.5f)
            {
                Vector(1.0f, 1.0f, 1.0f, Light);
                Vector(0.0f, 0.0f, 0.0f, StartRelative);
                Vector(0.0f, 0.0f, 0.0f, EndRelative);

                for (int i = 0; i < 28; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[69], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[49], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (4.5f <= o->AnimationFrame && o->AnimationFrame < 7.8f)
            {
                Vector(1.0f, 0.4f, 0.0f, Light);
                Vector(0.0f, 0.0f, 0.0f, StartRelative);
                Vector(0.0f, 0.0f, 0.0f, EndRelative);

                for (int i = 0; i < 28; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[69], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[49], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 8);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        } //attack switch end
    }
    break;

    case MODEL_QUARTER_MASTER:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  vLight;
            Vector(0.5f, 0.5f, 0.7f, vLight);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, vLight, 0, false, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;

    case MODEL_COMBAT_INSTRUCTOR:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_ATTACK1:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 11.7f)
            {
                Vector(1.0f, 0.2f, 0.0f, Light);
                Vector(0.0f, 0.0f, 0.0f, StartRelative);
                Vector(0.0f, 0.0f, 0.0f, EndRelative);

                for (int i = 0; i < 18; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[36], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 7);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 6.840f)
            {
                Vector(1.0f, 1.0f, 1.0f, Light);
                Vector(0.0f, 0.0f, 0.0f, StartRelative);
                Vector(0.0f, 0.0f, 0.0f, EndRelative);

                for (int i = 0; i < 16; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[36], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 1);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            if (4.0f <= o->AnimationFrame && o->AnimationFrame < 6.0f)
            {
                Vector(1.0f, 0.2f, 0.0f, Light);
                Vector(0.0f, 0.0f, 0.0f, StartRelative);
                Vector(0.0f, 0.0f, 0.0f, EndRelative);

                for (int i = 0; i < 28; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[36], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 7);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        } // attack switch end
    }
    break;
    case MODEL_OCELOT:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  vLight;
            Vector(0.5f, 0.5f, 0.5f, vLight);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[28], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[29], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, vLight, 0, false, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_ERIC:
    {
        float Start_Frame = 0.f;
        float End_Frame = 10.f;
        if ((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(0.5f, 0.5f, 0.5f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 20.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 20; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(BoneTransform[37], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    } //switch end
}

bool GMEmpireGuardian1::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return false;
    }

    switch (o->Type)
    {
    case 0:
    case 1:
    case 3:
    case 44:
    case 81:
        o->m_bRenderAfterCharacter = true;
        return true;

    case 37:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 41:// sos_bobpu
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        float Luminosity = sinf(WorldTime * 0.002f) * 0.5f + 0.6f;
        Vector(Luminosity * b->BodyLight[0], Luminosity * b->BodyLight[1], Luminosity * b->BodyLight[2], b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 47:// Gateflag
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.2f, 1.2f, 1.2f, b->BodyLight);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 48:// Gateflag2
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.1f, 1.1f, 1.1f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 49:
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 55:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;

    case 64:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.0f, 0.0f, 0.0f, b->BodyLight);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 70:
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;

    case 115:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        o->BlendMeshTexCoordV = (int)WorldTime % 25000 * 0.0004f;
        b->StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        o->BlendMeshTexCoordV = (int)WorldTime % 25000 * -0.0004f;
        b->StreamMesh = 3;
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->StreamMesh = -1;

        vec3_t vRelativePos, vWorldPos, Light, Angle;
        Vector(0, 0, 0, vRelativePos);
        Vector(1.f, 1.f, 1.f, Light);
        Vector(0.f, 0.f, 0.f, Angle);
        b->TransformPosition(BoneTransform[9], vRelativePos, vWorldPos, false);

        if (0.0f < o->AnimationFrame && o->AnimationFrame < 0.4f)
        {
            if (o->SubType == 100)
            {
                CreateEffect(MODEL_PROJECTILE, vWorldPos, Angle, Light, 0, o);
                o->SubType = 101;
            }
        }
        else if (7.0f < o->AnimationFrame && o->AnimationFrame < 7.5f)
        {
            if (o->SubType == 101)
            {
                o->SubType = 100;
            }
        }
    }
    return true;

    case 117:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        o->BlendMeshTexCoordV = (int)WorldTime % 25000 * 0.0004f;
        b->StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        o->BlendMeshTexCoordV = (int)WorldTime % 25000 * -0.0004f;
        b->StreamMesh = 4;
        b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->StreamMesh = -1;

        vec3_t vRelativePos, vWorldPos, Light, Angle;
        Vector(0, 0, 0, vRelativePos);
        Vector(1.f, 1.f, 1.f, Light);
        Vector(0.f, 0.f, 0.f, Angle);
        b->TransformPosition(BoneTransform[9], vRelativePos, vWorldPos, false);

        if (0.0f < o->AnimationFrame && o->AnimationFrame < 0.4f)
        {
            if (o->SubType == 100)
            {
                CreateEffect(MODEL_PROJECTILE, vWorldPos, Angle, Light, 0, o);
                o->SubType = 101;
            }
        }
        else if (7.0f < o->AnimationFrame && o->AnimationFrame < 7.5f)
        {
            if (o->SubType == 101)
            {
                o->SubType = 100;
            }
        }
    }
    return true;

    case MODEL_PROJECTILE:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    case MODEL_DOOR_CRUSH_EFFECT_PIECE01:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE02:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE03:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE04:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE05:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE06:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE07:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE08:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE09:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE10:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE11:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE12:
    case MODEL_DOOR_CRUSH_EFFECT_PIECE13:
    case MODEL_STATUE_CRUSH_EFFECT_PIECE01:
    case MODEL_STATUE_CRUSH_EFFECT_PIECE02:
    case MODEL_STATUE_CRUSH_EFFECT_PIECE03:
    case MODEL_STATUE_CRUSH_EFFECT_PIECE04:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;

    case MODEL_RAYMOND:
    case MODEL_LUCAS:
    case MODEL_EVIL_GATE:
    case MODEL_LION_GATE:
    case MODEL_STATUE:
    case MODEL_DEVIL_LORD:
    case MODEL_QUARTER_MASTER:
    case MODEL_DEFENDER:
    {
        RenderMonster(o, b, ExtraMon);
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian1::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian1() == false) return false;

    vec3_t p, Position, Light;
    Vector(0.f, 30.f, 0.f, Position);
    Vector(0.f, 0.f, 0.f, p);

    switch (o->Type)
    {
    case 12:
    {
        vec3_t vPos, vRelativePos, vLight1, vLight2;
        float flumi = absf(sinf(WorldTime * 0.0008)) * 0.9f + 0.1f;
        float fScale = o->Scale * 0.3f * flumi;
        Vector(0.f, 0.f, 0.f, vPos);
        Vector(8.f, -3.f, -3.f, vRelativePos);
        Vector(flumi, flumi, flumi, vLight1);
        Vector(0.9f, 0.1f, 0.1f, vLight2);
        b->TransformPosition(BoneTransform[2], vRelativePos, vPos);
        CreateSprite(BITMAP_SHINY + 5, vPos, 0.5f, vLight2, o);
        CreateSprite(BITMAP_SHINY + 5, vPos, fScale, vLight1, o);
        Vector(3.f, -3.f, -3.5f, vRelativePos);
        b->TransformPosition(BoneTransform[3], vRelativePos, vPos);
        CreateSprite(BITMAP_SHINY + 5, vPos, 0.5f, vLight2, o);
        CreateSprite(BITMAP_SHINY + 5, vPos, fScale, vLight1, o);
    }
    return true;

    case 20:
    {
        if (o->AnimationFrame > 5.4f && o->AnimationFrame < 6.5f && rand_fps_check(1))
        {
            vec3_t	Angle;
            for (int i = 0; i < 4; ++i)
            {
                Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2] + 180, Angle);
                CreateJoint(BITMAP_JOINT_SPARK, o->Position, o->Position, Angle, 5, o);
            }
            CreateParticle(BITMAP_SPARK, o->Position, Angle, o->Light, 11);
        }
        else if (o->AnimationFrame > 15.4f && o->AnimationFrame < 16.5f && rand_fps_check(1))
        {
            vec3_t	Angle;
            for (int i = 0; i < 4; ++i)
            {
                Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2], Angle);
                CreateJoint(BITMAP_JOINT_SPARK, o->Position, o->Position, Angle, 5, o);
            }
            CreateParticle(BITMAP_SPARK, o->Position, Angle, o->Light, 11);
        }
    }
    return true;

    case 37:
    {
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position);

        float fLumi;
        fLumi = (sinf(WorldTime * 0.039f) + 1.0f) * 0.2f + 0.6f;
        vec3_t vLightFire;
        Vector(fLumi * 0.7f, fLumi * 0.7f, fLumi * 0.7f, vLightFire);
        CreateSprite(BITMAP_FLARE, Position, 4.0f * o->Scale, vLightFire, o);
    }
    return true;

    case 50:
    {
        vec3_t vPos, vRelativePos, vLight1, vLight2, vAngle;
        Vector(0.f, 0.f, 0.f, vPos);
        Vector(0.f, 0.f, 5.f, vRelativePos);
        Vector(0.0f, -1.0f, 0.0f, vAngle);
        Vector(0.05f, 0.1f, 0.3f, vLight1);
        Vector(1.f, 1.f, 1.f, vLight2);

        for (int i = 2; i <= 7; i++)
        {
            b->TransformPosition(BoneTransform[i], vRelativePos, vPos);
            CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, vPos, vAngle, vLight1, 4, o->Scale * 0.6f);
            CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, vPos, vAngle, vLight2, 4, o->Scale * 0.3f);
        }
    }
    return true;

    case 51:
    {
        if (o->AnimationFrame > 5.4f && o->AnimationFrame < 6.5f)
        {
            vec3_t	Angle;
            for (int i = 0; i < 4; ++i)
            {
                Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2] + 180, Angle);
                CreateJointFpsChecked(BITMAP_JOINT_SPARK, o->Position, o->Position, Angle, 5, o);
            }
            CreateParticleFpsChecked(BITMAP_SPARK, o->Position, Angle, o->Light, 11);
        }
    }
    return true;

    case 64:
    {
        if ((o->AnimationFrame > 9.5f && o->AnimationFrame < 11.5f) ||
            (o->AnimationFrame > 23.5f && o->AnimationFrame < 25.5f))
        {
            float Matrix[3][4];
            vec3_t vAngle, vDirection, vPosition;
            Vector(0.f, 0.f, o->Angle[2] + 90, vAngle);
            AngleMatrix(vAngle, Matrix);
            Vector(0.f, 30.0f, 0.f, vDirection);
            VectorRotate(vDirection, Matrix, vPosition);
            VectorAdd(vPosition, o->Position, Position);

            Vector(0.04f, 0.03f, 0.02f, Light);
            for (int i = 0; i < 3; ++i)
            {
                CreateParticleFpsChecked(BITMAP_CLOUD, Position, o->Angle, Light, 22, o->Scale, o);
            }
        }
    }
    return true;

    case 79:
    {
        vec3_t vLightFire;
        Vector(1.0f, 0.2f, 0.0f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 2.0f * o->Scale, vLightFire, o);

        vec3_t vLight;
        Vector(1.0f, 1.0f, 1.0f, vLight);

        switch (rand() % 3)
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

    case 80:
    {
        float fLumi;
        fLumi = (sinf(WorldTime * 0.04f) + 1.0f) * 0.3f + 0.4f;
        vec3_t vLightFire;
        Vector(fLumi * 0.1f, fLumi * 0.1f, fLumi * 0.5f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 8.0f * o->Scale, vLightFire, o);
    }
    return true;

    case 82:
    {
        CreateParticleFpsChecked(BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 9, o->Scale);
    }
    return true;

    case 83:
    {
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_WATERFALL_3, o->Position, o->Angle, Light, 14, o->Scale);
    }
    return true;

    case 84:
    {
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(8))
        {
            CreateParticleFpsChecked(BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 4, o->Scale);
        }
    }
    return true;

    case 85:
    {
        vec3_t vLight;
        Vector(0.5f, 0.5f, 0.5f, vLight);
        int iWorldTime = (int)WorldTime;
        int iRemainder = iWorldTime % 200;

        if (iRemainder >= 0 && iRemainder <= 1)
        {
            int iRand = rand() % 4 + 4;
            for (int i = 0; i < iRand; ++i)
            {
                CreateEffectFpsChecked(BITMAP_FLAME, o->Position, o->Angle, vLight, 6, NULL, -1, 0, o->Scale);
            }
        }
    }
    return true;

    case 86:
    {
        if (rand_fps_check(6))
        {
            Vector(0.05f, 0.02f, 0.01f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 21, o->Scale, o);
        }
    }
    return true;

    case 129:
    {
        if (rand_fps_check(6))
        {
            Vector(0.01f, 0.02f, 0.05f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 21, o->Scale, o);
        }
    }
    return true;

    case 130:
    {
        if (rand_fps_check(6))
        {
            Vector(0.01f, 0.05f, 0.02f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 21, o->Scale, o);
        }
    }
    return true;

    case 131:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 22, o->Scale);

            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale * 2.0f, o);
        }
    }
    return true;

    case 132:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 60, o->Scale, o);

            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 60, o->Scale, o);

            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale * 2.0f, o);
        }
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian1::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_RAYMOND:
    {
        float fBlendMeshLight = (sinf(WorldTime * 0.003f) + 1.0f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        vec3_t _temp;
        VectorCopy(b->BodyLight, _temp);
        Vector(b->BodyLight[0] * 3.0f, b->BodyLight[0] * 3.0f, b->BodyLight[0] * 3.0f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAYMOND_SWORD);
        VectorCopy(_temp, b->BodyLight);
        b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    case MODEL_LUCAS:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        //b->RenderMesh ( 4, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
        //b->RenderMesh ( 4, RENDER_BRIGHT|RENDER_CHROME6, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
    }
    return true;
    case MODEL_DEVIL_LORD:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_CHROME4, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    case MODEL_QUARTER_MASTER:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, 0.3f, 2, 0.3f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    case MODEL_DEFENDER:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    case MODEL_EVIL_GATE:
    {
        int tileX = int(o->Position[0] / 100);
        int tileY = int(o->Position[1] / 100);

        switch (gMapManager.WorldActive)
        {
        case WD_69EMPIREGUARDIAN1:
        {
            if (tileX == 233 && tileY == 55)
            {
                o->Position[0] = 23350;	o->Position[1] = 5520;
            }
            else if ((165 <= tileX && tileX <= 167) && (25 <= tileY && tileY <= 27))
            {
                o->Position[0] = 16710;	o->Position[1] = 2620;
            }
        }break;
        case WD_70EMPIREGUARDIAN2:
        {
            if ((49 <= tileX && tileX <= 51) && (64 <= tileY && tileY <= 66))
            {
                o->Position[0] = 5075;	o->Position[1] = 6490;
            }
            else if ((40 <= tileX && tileX <= 42) && (116 <= tileY && tileY <= 118))
            {
                o->Position[0] = 4200;	o->Position[1] = 11680;
            }
        }break;
        case WD_71EMPIREGUARDIAN3:
        {
            if ((118 <= tileX && tileX <= 120) && (191 <= tileY && tileY <= 193))
            {
                o->Scale = 0.9f;
                o->Position[0] = 11985;	o->Position[1] = 19250;
            }
            else if ((221 <= tileX && tileX <= 223) && (159 <= tileY && tileY <= 161))
            {
                o->Scale = 1.08f;
                o->Position[0] = 22300;	o->Position[1] = 16000;
            }
        }break;
        }

        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;

    case MODEL_LION_GATE:
    {
        int tileX = int(o->Position[0] / 100);
        int tileY = int(o->Position[1] / 100);
        switch (gMapManager.WorldActive)
        {
        case WD_69EMPIREGUARDIAN1:
        {
            if (tileX == 234 && tileY == 28)
            {
                o->Position[0] = 23450;	o->Position[1] = 2820;
            }
            else if (tileX == 216 && tileY == 80)
            {
                o->Position[0] = 21650;	o->Position[1] = 8000;
            }
            else if (tileX == 194 && tileY == 25)
            {
                o->Position[0] = 19450;	o->Position[1] = 2530;
            }
            else if ((153 <= tileX && tileX <= 155) && (52 <= tileY && tileY <= 54))
            {
                o->Scale = 1.15f;
                o->Position[0] = 15510;	o->Position[1] = 5360;
            }
            else if (tileX == 180 && tileY == 79)
            {
                o->Position[0] = 18070;	o->Position[1] = 7950;
            }
        }break;
        case WD_70EMPIREGUARDIAN2:
        {
            if ((74 <= tileX && tileX <= 76) && (66 <= tileY && tileY <= 68))
            {
                o->Scale = 1.17f;
                o->Position[0] = 7620;	o->Position[1] = 6740;
            }
            else if ((18 <= tileX && tileX <= 20) && (64 <= tileY && tileY <= 66))
            {
                o->Position[0] = 1950;	o->Position[1] = 6500;
            }
            else if ((36 <= tileX && tileX <= 38) && (92 <= tileY && tileY <= 94))
            {
                o->Scale = 1.1f;
                o->Position[0] = 3770;	o->Position[1] = 9250;
            }
            else if ((54 <= tileX && tileX <= 56) && (153 <= tileY && tileY <= 155))
            {
                o->Scale = 1.15f;
                o->Position[0] = 5515;	o->Position[1] = 15350;
            }
            else if ((106 <= tileX && tileX <= 108) && (111 <= tileY && tileY <= 113))
            {
                o->Scale = 1.05f;
                o->Position[0] = 10830;	o->Position[1] = 11180;
            }
        }break;
        case WD_71EMPIREGUARDIAN3:
        {
            if ((145 <= tileX && tileX <= 147) && (190 <= tileY && tileY <= 192))
            {
                o->Scale = 1.28f;
                o->Position[0] = 14700;	o->Position[1] = 19140;
            }
            else if ((88 <= tileX && tileX <= 90) && (194 <= tileY && tileY <= 196))
            {
                o->Scale = 1.10f;
                o->Position[0] = 9010;	o->Position[1] = 19580;
            }
            else if ((221 <= tileX && tileX <= 223) && (133 <= tileY && tileY <= 135))
            {
                o->Scale = 1.1f;
                o->Position[0] = 22300;	o->Position[1] = 13360;
            }
            else if ((222 <= tileX && tileX <= 224) && (192 <= tileY && tileY <= 194))
            {
                o->Scale = 1.1f;
                o->Position[0] = 22305;	o->Position[1] = 19280;
            }
            else if ((166 <= tileX && tileX <= 168) && (216 <= tileY && tileY <= 218))
            {
                o->Scale = 1.23f;
                o->Position[0] = 16720;	o->Position[1] = 21750;
            }
        }break;
        }

        if (o->CurrentAction != MONSTER01_DIE)
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    return true;
    case MODEL_STATUE:
    {
        if (o->CurrentAction != MONSTER01_DIE)
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian1::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    vec3_t vLight, vPos;

    switch (o->Type)
    {
    case MODEL_RAYMOND:
    {
        if (g_isNotCharacterBuff(o) == true && g_isCharacterBuff(o, eBuff_Berserker) == true)
        {
            vec3_t vLightRage, vPosRage;

            int iBoneCount = b->NumBones;
            Vector(1.0f, 0.1f, 0.1f, vLightRage);
            for (int i = 0; i < iBoneCount; ++i)
            {
                if (rand() % 6 > 0) continue;
                if (i % 5 == 0)
                {
                    b->TransformByObjectBone(vPosRage, o, i);
                    CreateParticleFpsChecked(BITMAP_SMOKE, vPosRage, o->Angle, vLightRage, 50, 1.0f);
                    CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPosRage, o->Angle, vLightRage, 0, 1.0f);
                }
            }
        }
    }
    return true;
    case MODEL_LUCAS:
    {
        vec3_t vRelative;
        Vector(0.0f, 0.0f, 0.0f, vRelative)
            float fLumi1 = (sinf(WorldTime * 0.004f) + 0.9f) * 0.25f;

        Vector(0.05f + fLumi1, 0.75f + fLumi1, 0.35f + fLumi1, vLight);

        Vector(0.0f, 0.0f, -10.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 41, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 1.0f + fLumi1, vLight, o);

        Vector(0.0f, 0.0f, 10.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 41, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 1.0f + fLumi1, vLight, o);

        Vector(0.0f, 0.0f, -10.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 42, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 1.0f + fLumi1, vLight, o);

        Vector(0.0f, 0.0f, 10.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 42, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 1.0f + fLumi1, vLight, o);

        float fScale = 1.2f;
        for (int i = 50; i <= 55; ++i)
        {
            int iCurrentBoneIndex = i;

            Vector(0.0f, 6.0f, 0.0f, vRelative);
            b->TransformByObjectBone(vPos, o, iCurrentBoneIndex, vRelative);
            CreateSprite(BITMAP_LIGHT, vPos, fScale + fLumi1, vLight, o);

            fScale = fScale - 0.15f;
        }

        if (g_isNotCharacterBuff(o) == true && g_isCharacterBuff(o, eBuff_Berserker) == true)
        {
            vec3_t vLightRage, vPosRage;

            int iBoneCount = b->NumBones;
            Vector(1.0f, 0.1f, 0.1f, vLightRage);
            for (int i = 0; i < iBoneCount; ++i)
            {
                if (rand() % 6 > 0) continue;
                if (i % 5 == 0)
                {
                    b->TransformByObjectBone(vPosRage, o, i);
                    CreateParticleFpsChecked(BITMAP_SMOKE, vPosRage, o->Angle, vLightRage, 50, 1.0f);
                    CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPosRage, o->Angle, vLightRage, 0, 1.0f);
                }
            }
        }
    }
    return true;
    case MODEL_FRED:
    {
        if (g_isNotCharacterBuff(o) == true && g_isCharacterBuff(o, eBuff_Berserker) == true)
        {
            vec3_t vLightRage, vPosRage;

            int iBoneCount = b->NumBones;
            Vector(1.0f, 0.1f, 0.1f, vLightRage);
            for (int i = 0; i < iBoneCount; ++i)
            {
                if (rand() % 6 > 0) continue;
                if (i % 5 == 0)
                {
                    b->TransformByObjectBone(vPosRage, o, i);
                    CreateParticleFpsChecked(BITMAP_SMOKE, vPosRage, o->Angle, vLightRage, 50, 1.0f);
                    CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPosRage, o->Angle, vLightRage, 0, 1.0f);
                }
            }
        }
    }
    return true;
    case MODEL_DEVIL_LORD:
    {
        if (g_isNotCharacterBuff(o) == true && g_isCharacterBuff(o, eBuff_Berserker) == true)
        {
            vec3_t vLightRage, vPosRage;

            int iBoneCount = b->NumBones;
            Vector(1.0f, 0.1f, 0.1f, vLightRage);
            for (int i = 0; i < iBoneCount; ++i)
            {
                if (rand() % 6 > 0) continue;
                if (i % 5 == 0)
                {
                    b->TransformByObjectBone(vPosRage, o, i);
                    CreateParticleFpsChecked(BITMAP_SMOKE, vPosRage, o->Angle, vLightRage, 50, 1.0f);
                    CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPosRage, o->Angle, vLightRage, 0, 1.0f);
                }
            }
        }
    }
    return true;
    case MODEL_QUARTER_MASTER:
    {
        int i;
        Vector(0.5f, 0.5f, 0.8f, vLight);
        int iBoneNumbers[] = { 10, 8, 12, 25 };
        for (i = 1; i < 4; ++i)
        {
            b->TransformByObjectBone(vPos, o, iBoneNumbers[i]);
            CreateSprite(BITMAP_LIGHT, vPos, 3.1f, vLight, o);
        }

        if (o->AnimationFrame > 2 && o->AnimationFrame < 3)
        {
            o->m_dwTime = 0;
            b->TransformByObjectBone(vPos, o, 11);

            Vector(1.0f, 1.0f, 1.0f, vLight);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 4, 1.0f);
        }
    }
    return true;
    case MODEL_DEFENDER:
    {
    }
    return true;
    case MODEL_FORSAKER:
    {
        if (o->CurrentAction != MONSTER01_ATTACK1 && o->CurrentAction != MONSTER01_ATTACK2)
        {
            vec3_t vRelative;
            Vector(10.0f, 0.0f, 0.0f, vRelative);
            b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
            float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.3f + 0.4f;
            Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, vLight);
            CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.5f, vLight, o);
            Vector(0.5f, 0.5f, 0.5f, vLight);
            CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f, vLight, o, -WorldTime * 0.1f);
        }
    }
    return true;
    }

    return false;
}

void GMEmpireGuardian1::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian1() == false) return;

    switch (o->Type)
    {
    case 0:
    case 1:
    case 3:
    case 44:
    {
        float fLumi;
        fLumi = (sinf(WorldTime * 0.0015f) + 1.0f) * 0.4f + 0.2f;

        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    break;

    case 81:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    break;
    }
}

bool GMEmpireGuardian1::CreateRain(PARTICLE* o)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return false;
    }

    if (m_iWeather == (int)WEATHER_RAIN || m_iWeather == (int)WEATHER_STORM)
    {
        {
            o->Type = BITMAP_RAIN;
            Vector(Hero->Object.Position[0] + (float)(rand() % 800 - 300),
                Hero->Object.Position[1] + (float)(rand() % 800 - 400),
                Hero->Object.Position[2] + (float)(rand() % 200 + 400),
                o->Position);
            Vector(-30.f, 0.f, 0.f, o->Angle);
            vec3_t Velocity;
            Vector(0.f, 0.f, -(float)(rand() % 24 + 30), Velocity);
            float Matrix[3][4];
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(Velocity, Matrix, o->Velocity);
        }

        VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (rand_fps_check(2))
        {
            o->Live = false;
            o->Position[2] = Height + 10.f;
            if (rand_fps_check(4))
                CreateParticle(BITMAP_RAIN_CIRCLE, o->Position, o->Angle, o->Light);
            else
                CreateParticle(BITMAP_RAIN_CIRCLE + 1, o->Position, o->Angle, o->Light);
        }
    }

    return true;
}

void GMEmpireGuardian1::RenderFrontSideVisual()
{
    switch (m_iWeather)
    {
    case WEATHER_FOG:
    {
        EnableAlphaBlend();
        glColor3f(0.6f, 0.6f, 0.9f);
        float WindX2 = (float)((int)WorldTime % 100000) * 0.00005f;
        float WindY2 = (float)((int)WorldTime % 100000) * 0.00008f;
        RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, WindY2, 2.0f, 2.0f);
        float WindX = -(float)((int)WorldTime % 100000) * 0.00005f;
        RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX, 0.f, 0.3f, 0.3f);
    }
    break;
    case WEATHER_STORM:
    {
        if (rand_fps_check(20))
        {
            EnableAlphaBlend();
            glColor3f(0.7f, 0.7f, 0.9f);
            float WindX2 = (float)((int)WorldTime % 100000) * 0.0006f;
            float WindY2 = -(float)((int)WorldTime % 100000) * 0.0006f;
            RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, WindY2, 3.0f, 2.0f);
            WindX2 = -(float)((int)WorldTime % 100000) * 0.0006f;
            WindY2 = (float)((int)WorldTime % 100000) * 0.0006f;
            RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, WindY2, 3.0f, 2.0f);
        }
    }
    break;
    }
}

bool GMEmpireGuardian1::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return false;
    }

    switch (c->MonsterIndex)
    {
    case MONSTER_RAYMOND:
    {
        if (m_bCurrentIsRage_Raymond == true)
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            return true;
        }

        switch (c->MonsterSkill)
        {
        case 60:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 52:
        {
            SetAction(o, MONSTER01_ATTACK3);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;

            m_bCurrentIsRage_Raymond = true;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    case MONSTER_LUCAS:
    {
        if (m_bCurrentIsRage_Ercanne == true)
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            return true;
        }

        switch (c->MonsterSkill)
        {
        case 62:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 63:
        {
            SetAction(o, MONSTER01_ATTACK3);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;

            m_bCurrentIsRage_Ercanne = true;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    case MONSTER_FRED:
    {
        if (m_bCurrentIsRage_Daesuler == true)
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            return true;
        }

        switch (c->MonsterSkill)
        {
        case 57:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 58:
        {
            SetAction(o, MONSTER01_ATTACK3);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;

            m_bCurrentIsRage_Daesuler = true;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    case MONSTER_DEVIL_LORD:
    {
        if (m_bCurrentIsRage_Gallia == true)
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            return true;
        }

        switch (c->MonsterSkill)
        {
        case 58:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 60:
        {
            SetAction(o, MONSTER01_ATTACK3);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;

            m_bCurrentIsRage_Gallia = true;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    case MONSTER_COMBAT_INSTRUCTOR:
    {
        switch (c->MonsterSkill)
        {
        case 47:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 49:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    case MONSTER_DEFENDER:
    {
        switch (c->MonsterSkill)
        {
        case 44:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
        }
        break;
        case 45:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    case MONSTER_FORSAKER:
    {
        switch (c->MonsterSkill)
        {
        case 46:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }
    }
    return true;
    }
    return false;
}

bool GMEmpireGuardian1::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian1() == false)
        return false;

    //  switch(c->MonsterIndex)
    //  {
    //  }

    return false;
}

bool GMEmpireGuardian1::PlayMonsterSound(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return false;
    }

    float fDis_x, fDis_y;
    fDis_x = o->Position[0] - Hero->Object.Position[0];
    fDis_y = o->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 500.0f)
        return true;

    switch (o->Type)
    {
    case MODEL_RAYMOND:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_ATTACK02);// Raymond_attack2.wav
            // 					PlayBuffer(SOUND_SKILL_BLOWOFDESTRUCTION);
        }
        break;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_ATTACK02);// Raymond_attack2.wav
            // 					PlayBuffer(SOUND_METEORITE01);
            // 					PlayBuffer(SOUND_EXPLOTION01);
        }
        break;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE);// Raymond_rage.wav
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
        }
        break;
        }
    }
    return true;
    case MODEL_LUCAS:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (rand_fps_check(2))
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
            }
            else
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);	// 공격1 사운드
        }
        break;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK03);	// 공격2 사운드
        }
        break;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_ERCANNE_MONSTER_ATTACK03);	// 공격3 사운드
        }
        break;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE);	// 광폭화 사운드
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);	// 죽음 사운드
        }
        break;
        }
    }
    return true;
    case MODEL_FRED:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK02);// 1Deasuler_attack2.wav
        }
        break;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK03);// 1Deasuler_attack3.wav
        }
        break;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE);
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
        }
        break;
        }
    }
    return true;

    case MODEL_DEVIL_LORD:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_3CORP_CATO_MOVE);
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_4CORP_GALLIA_ATTACK02);// 4Gallia_attack2.wav
        }
        break;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE);
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_DEATH);
        }
        break;
        }
    }
    return true;

    case MODEL_QUARTER_MASTER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_QUATERMASTER_ATTACK02);// QuaterMaster_attack2.wav
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
        }
        break;
        }
    }
    return true;

    case MODEL_COMBAT_INSTRUCTOR:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);// CombatMaster_attack1.wav
        }break;

        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02);// CombatMaster_attack2.wav
        }break;

        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03);// CombatMaster_attack3.wav
        }break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
        }
        break;
        }
    }
    return true;
    case MODEL_DEFENDER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);// CombatMaster_attack1.wav
        }break;

        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02);// CombatMaster_attack2.wav
        }break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_DEFENDER_ATTACK02);
        }
        break;
        }
    }
    return true;

    case MODEL_FORSAKER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_PRIEST_STOP);
        }break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_THUNDERS01);
        }break;

        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_RAKLION_SERUFAN_CURE);
        }break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_DARKLORD_DEAD);
        }
        break;
        }
    }
    return true;
    }

    return false;
}

void GMEmpireGuardian1::PlayObjectSound(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        return;
    }

    if (gMapManager.IsEmpireGuardian4())
    {
        PlayBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, NULL, false);
        return;
    }

    switch (m_iWeather)
    {
    case WEATHER_SUN:
        break;
    case WEATHER_RAIN:
        PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, NULL, false);
        break;
    case WEATHER_FOG:
        PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, NULL, false);
        break;
    case WEATHER_STORM:
        PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, NULL, false);
        break;
    }
}

void GMEmpireGuardian1::PlayBGM()
{
    if (gMapManager.IsEmpireGuardian1())
    {
        PlayMp3(MUSIC_EMPIREGUARDIAN1);
    }
    else
    {
        StopMp3(MUSIC_EMPIREGUARDIAN1);
    }
}