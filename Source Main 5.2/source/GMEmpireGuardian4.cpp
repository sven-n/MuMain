// GMEmpireGuardian4.cpp: implementation of the GMEmpireGuardian4 class.
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
#include "GOBoid.h"

#include "LoadData.h"



GMEmpireGuardian4Ptr GMEmpireGuardian4::Make()
{
    GMEmpireGuardian4Ptr doppelganger(new GMEmpireGuardian4);
    doppelganger->Init();
    return doppelganger;
}

GMEmpireGuardian4::GMEmpireGuardian4()
{
}

GMEmpireGuardian4::~GMEmpireGuardian4()
{
    Destroy();
}

void GMEmpireGuardian4::Init()
{
}

void GMEmpireGuardian4::Destroy()
{
}

bool GMEmpireGuardian4::CreateObject(OBJECT* o)
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
    case 10:
    {
        o->SubType = rand() % 50;
    }
    return true;
    }

    return false;
}

CHARACTER* GMEmpireGuardian4::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = g_EmpireGuardian1.CreateMonster(iType, PosX, PosY, Key);

    if (NULL != pCharacter)
    {
        return pCharacter;
    }

    switch (iType)
    {
    case MONSTER_GAYION_THE_GLADIATOR:
    {
        OpenMonsterModel(MONSTER_MODEL_GAYION);
        pCharacter = CreateCharacter(Key, MODEL_GAYION, PosX, PosY);
        memset(pCharacter->ID, 0, sizeof(pCharacter->ID));
        std::wstring(L"Gayion The Gladiator").copy(pCharacter->ID, 19);

        gLoadData.AccessModel(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, L"Data\\Monster\\", L"Boss_Karane_sword_left01");
        gLoadData.AccessModel(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_, L"Data\\Monster\\", L"Boss_Karane_sword_left02");
        gLoadData.AccessModel(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_, L"Data\\Monster\\", L"Boss_Karane_sword_right01");
        gLoadData.AccessModel(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_, L"Data\\Monster\\", L"Boss_Karane_sword_right02");
        gLoadData.AccessModel(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, L"Data\\Monster\\", L"Boss_Karane_sword_main01");

        gLoadData.OpenTexture(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, L"Monster\\");
        gLoadData.OpenTexture(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_, L"Monster\\");
        gLoadData.OpenTexture(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_, L"Monster\\");
        gLoadData.OpenTexture(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_, L"Monster\\");
        gLoadData.OpenTexture(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, L"Monster\\");

        pCharacter->Object.Scale = 1.40f;

        m_bCurrentIsRage_BossGaion = false;
    }
    break;
    case MONSTER_JERRY:
    case MONSTER_ADVISER_JERINTEU:
    {
        OpenMonsterModel(MONSTER_MODEL_JERRY);
        pCharacter = CreateCharacter(Key, MODEL_JERRY, PosX, PosY);
        memset(pCharacter->ID, 0, sizeof(pCharacter->ID));
        std::wstring(L"Jerry The Adviseru").copy(pCharacter->ID, 19);
        pCharacter->Object.Scale = 1.45f;

        m_bCurrentIsRage_Jerint = false;
    }
    break;
    case MONSTER_STAR_GATE:
    {
        OpenMonsterModel(MONSTER_MODEL_STAR_GATE);
        pCharacter = CreateCharacter(Key, MODEL_STAR_GATE, PosX, PosY);
        memset(pCharacter->ID, 0, sizeof(pCharacter->ID));
        std::wstring(L"Star Gate").copy(pCharacter->ID, 10);
        pCharacter->Object.m_bRenderShadow = false;
        pCharacter->Object.Scale = 1.25f;
    }
    break;

    case MONSTER_RUSH_GATE:
    {
        OpenMonsterModel(MONSTER_MODEL_RUSH_GATE);
        pCharacter = CreateCharacter(Key, MODEL_RUSH_GATE, PosX, PosY);
        memset(pCharacter->ID, 0, sizeof(pCharacter->ID));
        std::wstring(L"Rush Gate").copy(pCharacter->ID, 10);
        pCharacter->Object.m_bRenderShadow = false;
        pCharacter->Object.LifeTime = 100;
        pCharacter->Object.Scale = 1.25f;
    }
    break;

    default: return pCharacter;
    }

    return pCharacter;
}

bool GMEmpireGuardian4::MoveObject(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

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
    case 10:
    {
        if (o->SubType > 0)
        {
            o->SubType -= 1;
            o->AnimationFrame = 0.0f;
            o->PriorAnimationFrame = 0.0f;
        }
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

bool GMEmpireGuardian4::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

    if (true == g_EmpireGuardian1.MoveMonsterVisual(o, b))
    {
        return true;
    }

    switch (o->Type)
    {
    case MODEL_RUSH_GATE:
    {
        if (o->CurrentAction == MONSTER01_DIE)
        {
            if ((int)o->LifeTime == 100)
            {
                o->LifeTime = 90;

                vec3_t vPos, vRelativePos;
                Vector(200.0f, 0.0f, 0.0f, vRelativePos);
                b->TransformPosition(o->BoneTransform[0], vRelativePos, vPos, true);
                CreateEffect(MODEL_DOOR_CRUSH_EFFECT, vPos, o->Angle, o->Light, 1, o, 0, 0);
            }
        }
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian4::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

    if (true == g_EmpireGuardian1.MoveMonsterVisual(c, o, b))
    {
        return true;
    }

    switch (o->Type)
    {
    case MODEL_GAYION:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_ATTACK1:
        {
            CreateEffect(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);

            if (o->AnimationFrame >= 0.0f && o->AnimationFrame < 0.5f)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t Angle, AngleRelative = { 0.0f, 0.0f, 0.0f };
                vec3_t v3PosTo = { to->Position[0], to->Position[1], to->Position[2] };
                vec3_t v3RelativePos, v3StartPos;

                Vector(o->Angle[0] + AngleRelative[0], o->Angle[1] + AngleRelative[1], o->Angle[2] + AngleRelative[2], Angle);

                Vector(0.f, 0.f, 0.f, v3RelativePos);

                b->TransformPosition(o->BoneTransform[2], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_, v3StartPos, Angle, v3PosTo, 1, o, -1, 0, 0, 0, o->Scale);

                b->TransformPosition(o->BoneTransform[10], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_, v3StartPos, Angle, v3PosTo, 1, o, -1, 0, 0, 0, o->Scale);
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            CreateEffect(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);

            if (o->AnimationFrame >= 0.0f && o->AnimationFrame < 0.7f)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t Angle, AngleRelative = { 0.0f, 0.0f, 0.0f };
                vec3_t v3PosTo = { to->Position[0], to->Position[1], to->Position[2] };
                vec3_t v3RelativePos, v3StartPos;

                Vector(o->Angle[0] + AngleRelative[0], o->Angle[1] + AngleRelative[1], o->Angle[2] + AngleRelative[2], Angle);
                Vector(0.f, 0.f, 0.f, v3RelativePos);

                b->TransformPosition(o->BoneTransform[4], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, v3StartPos, Angle, v3PosTo, 1, o, -1, 0, 0, 0, o->Scale);

                b->TransformPosition(o->BoneTransform[8], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_,
                    v3StartPos, Angle, v3PosTo, 1, o, -1, 0, 0, 0, o->Scale);
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (o->AnimationFrame >= 0.0f && o->AnimationFrame < 0.5f)
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

                Vector(0.f, 0.f, 0.f, v3RelativePos);

                b->TransformPosition(o->BoneTransform[4], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_,
                    v3StartPos, Angle, v3PosTo, 3, o, -1, 0, 0, 0, o->Scale);

                b->TransformPosition(o->BoneTransform[8], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_,
                    v3StartPos, Angle, v3PosTo, 3, o, -1, 0, 0, 0, o->Scale);

                b->TransformPosition(o->BoneTransform[2], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_,
                    v3StartPos, Angle, v3PosTo, 3, o, -1, 0, 0, 0, o->Scale);

                b->TransformPosition(o->BoneTransform[10], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_,
                    v3StartPos, Angle, v3PosTo, 3, o, -1, 0, 0, 0, o->Scale);

                b->TransformPosition(o->BoneTransform[6], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_,
                    v3StartPos, Angle, v3PosTo, 3, o, -1, 0, 0, 0, o->Scale);
            }
        }
        break;
        case MONSTER01_ATTACK4:
        {
            CreateEffect(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_,
                o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);

            if (o->AnimationFrame >= 0.0f && o->AnimationFrame < 0.5f)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t Angle, AngleRelative = { 0.0f, 0.0f, 0.0f };
                vec3_t v3PosTo = { to->Position[0], to->Position[1], to->Position[2] };
                vec3_t v3RelativePos, v3StartPos;

                Vector(o->Angle[0] + AngleRelative[0], o->Angle[1] + AngleRelative[1], o->Angle[2] + AngleRelative[2], Angle);

                Vector(0.f, 0.f, 0.f, v3RelativePos);

                b->TransformPosition(o->BoneTransform[6], v3RelativePos, v3StartPos, true);
                CreateEffect(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, v3StartPos, Angle, v3PosTo, 1, o, -1, 0, 0, 0, o->Scale);
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            CreateEffect(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            {
                vec3_t		Light;

                if (m_bCurrentIsRage_BossGaion == true)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateInferno(o->Position);
                    CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    m_bCurrentIsRage_BossGaion = false;
                }
            }
        }
        break;
        default:
        {
            CreateEffect(MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
            CreateEffect(MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, o->Scale);
        }
        break;
        }
    }
    return true;
    case MODEL_DEATH_ANGEL_3:
    case MODEL_JERRY:
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
            if (7.5f <= o->AnimationFrame && o->AnimationFrame < 10.8f)
            {
                vec3_t  Light;
                Vector(1.0f, 0.5f, 0.2f, Light);

                vec3_t vPosBlur01, vPosBlurRelative01;
                vec3_t vPosBlur02, vPosBlurRelative02;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;

                int iSwordForceType = 0;

                for (int i = 0; i < 14; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative01);
                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative02);

                    b->TransformPosition(BoneTransform[61], vPosBlurRelative01, vPosBlur01, false);
                    b->TransformPosition(BoneTransform[51], vPosBlurRelative02, vPosBlur02, false);

                    CreateObjectBlur(o, vPosBlur01, vPosBlur02, Light, iSwordForceType, true, 1, 30);
                    CreateObjectBlur(o, vPosBlur01, vPosBlur02, Light, iSwordForceType, true, 2, 30);

                    b->TransformPosition(BoneTransform[52], vPosBlurRelative01, vPosBlur01, false);
                    b->TransformPosition(BoneTransform[60], vPosBlurRelative02, vPosBlur02, false);

                    CreateObjectBlur(o, vPosBlur01, vPosBlur02, Light, iSwordForceType, true, 11, 30);
                    CreateObjectBlur(o, vPosBlur01, vPosBlur02, Light, iSwordForceType, true, 12, 30);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (o->AnimationFrame >= 3.0f && o->AnimationFrame <= 5.0f)
            {
                CreateEffect(BITMAP_GATHERING, o->Position, o->Angle, o->Light, 1, o);
            }

            if (8.0f <= o->AnimationFrame && o->AnimationFrame < 10.1f)
            {
                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;

                vec3_t	vRelative, vPosition, vRelative2;

                vec3_t	vAngle, vRandomDir, vRandomDirPosition, vResultRandomPosition;
                vec34_t	matRandomRotation;
                Vector(0.0f, 0.0f, 0.0f, vAngle);

                Vector(0.0f, 0.0f, 0.0f, vRandomDirPosition);

                Vector(0.0f, 0.0f, 0.0f, vRelative);
                Vector(0.0f, 0.0f, 0.0f, vRelative2);
                for (int i = 0; i < 100; i++)
                {
                    float	fRandDistance = (float)(rand() % 100) + 100;
                    Vector(0.0f, fRandDistance, 0.0f, vRandomDir);

                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[61], vRelative, vPosition, false);
                    CreateParticleFpsChecked(BITMAP_FIRE, vPosition, o->Angle, o->Light, 0);

                    Vector((float)(rand() % 360), 0.f, (float)(rand() % 360), vAngle);
                    AngleMatrix(vAngle, matRandomRotation);
                    VectorRotate(vRandomDir, matRandomRotation, vRandomDirPosition);
                    VectorAdd(vPosition, vRandomDirPosition, vResultRandomPosition);
                    CreateJointFpsChecked(BITMAP_JOINT_THUNDER, vResultRandomPosition, vPosition, vAngle, 3, NULL, 10.f, 10, 10);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }

            if (6.0f <= o->AnimationFrame && o->AnimationFrame < 10.1f)
            {
                vec3_t  Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

                vec3_t vPosBlur01, vPosBlurRelative01;
                vec3_t vPosBlur02, vPosBlurRelative02;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;

                for (int i = 0; i < 40; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative01);
                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative02);

                    b->TransformPosition(BoneTransform[61], vPosBlurRelative01, vPosBlur01, false);
                    b->TransformPosition(BoneTransform[51], vPosBlurRelative02, vPosBlur02, false);

                    CreateBlur(c, vPosBlur01, vPosBlur02, Light, 2);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (o->AnimationFrame >= 0.2f && o->AnimationFrame <= 1.0f)
            {
                CHARACTER* chT = &CharactersClient[c->TargetCharacter];
                vec3_t& v3TargetPos = chT->Object.Position;

                Vector(v3TargetPos[0], v3TargetPos[1], v3TargetPos[2], o->Light);
                CreateEffect(MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, 1.0f);
            }

            if (9.8f <= o->AnimationFrame && o->AnimationFrame < 14.0f)
            {
                vec3_t  Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

                vec3_t vPosBlur03, vPosBlurRelative03;
                vec3_t vPosBlur04, vPosBlurRelative04;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;

                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 10; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative03);
                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative04);

                    b->TransformPosition(BoneTransform[52], vPosBlurRelative04, vPosBlur04, false);
                    b->TransformPosition(BoneTransform[58], vPosBlurRelative04, vPosBlur03, false);

                    CreateObjectBlur(o, vPosBlur03, vPosBlur04, Light, 10, false, 0);
                    CreateObjectBlur(o, vPosBlur03, vPosBlur04, Light, 10, false, 1);
                    CreateObjectBlur(o, vPosBlur03, vPosBlur04, Light, 10, false, 2);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            {
                vec3_t		Light;

                if (m_bCurrentIsRage_Jerint == true)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateInferno(o->Position);

                    CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                    m_bCurrentIsRage_Jerint = false;
                }
            }
        }
        break;
        }
    }
    return true;
    }
    return false;
}

void GMEmpireGuardian4::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_RAYMOND:
    case MODEL_DEFENDER:
    case MODEL_FORSAKER:
    case MODEL_OCELOT:
    case MODEL_ERIC:
    {
        g_EmpireGuardian1.MoveBlurEffect(c, o, b);
    }
    break;
    }
}

bool GMEmpireGuardian4::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

    if (true == g_EmpireGuardian1.RenderObjectMesh(o, b, ExtraMon))
    {
        return true;
    }

    switch (o->Type)
    {
    case MODEL_GAYION:
    case MODEL_JERRY:
    case MODEL_LUCAS:
    {
        RenderMonster(o, b, ExtraMon);

        return true;
    }
    case 96:
    case 97:
    case 100:
    {
        Vector(0.170382, 0.170382, 0.170382, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    case MODEL_STAR_GATE:
    {
        int tileX = int(o->Position[0] / 100);
        int tileY = int(o->Position[1] / 100);

        if ((49 <= tileX && tileX <= 51) && (68 <= tileY && tileY <= 70))
        {
            o->Scale = 1.0f;
            o->Position[0] = 5080;	o->Position[1] = 6920;
        }
        else if ((51 <= tileX && tileX <= 53) && (190 <= tileY && tileY <= 192))
        {
            o->Scale = 1.0f;
            o->Position[0] = 5270;	o->Position[1] = 19120;
        }
        else if ((196 <= tileX && tileX <= 198) && (131 <= tileY && tileY <= 133))
        {
            o->Scale = 1.0f;
            o->Position[0] = 19750;	o->Position[1] = 13220;
        }

        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;

    case MODEL_RUSH_GATE:
    {
        if (o->CurrentAction != MONSTER01_DIE)
        {
            int tileX = int(o->Position[0] / 100);
            int tileY = int(o->Position[1] / 100);

            if ((80 <= tileX && tileX <= 82) && (68 <= tileY && tileY <= 70))
            {
                o->Scale = 0.8f;
                o->Position[0] = 8115;	o->Position[1] = 6880;
            }
            else if ((31 <= tileX && tileX <= 33) && (89 <= tileY && tileY <= 91))
            {
                o->Scale = 0.9f;
                o->Position[0] = 3250;	o->Position[1] = 9000;
            }
            else if ((33 <= tileX && tileX <= 35) && (175 <= tileY && tileY <= 177))
            {
                o->Scale = 0.8f;
                o->Position[0] = 3470;	o->Position[1] = 17600;
            }
            else if ((68 <= tileX && tileX <= 70) && (165 <= tileY && tileY <= 167))
            {
                o->Scale = 0.9f;
                o->Position[0] = 6915;	o->Position[1] = 16650;
            }
            else if ((155 <= tileX && tileX <= 157) && (131 <= tileY && tileY <= 133))
            {
                o->Scale = 0.9f;
                o->Position[0] = 15710;	o->Position[1] = 13250;
            }
            else if ((223 <= tileX && tileX <= 225) && (158 <= tileY && tileY <= 160))
            {
                o->Scale = 0.8f;
                o->Position[0] = 22500;	o->Position[1] = 16000;
            }
            else if ((213 <= tileX && tileX <= 215) && (23 <= tileY && tileY <= 25))
            {
                o->Scale = 0.9f;
                o->Position[0] = 21480;	o->Position[1] = 2430;
            }

            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian4::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

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
#ifdef LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.5f, vLight2, o);
        CreateSprite(BITMAP_SHINY + 6, vPos, fScale, vLight1, o);
        Vector(3.f, -3.f, -3.5f, vRelativePos);
        b->TransformPosition(BoneTransform[3], vRelativePos, vPos);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.5f, vLight2, o);
        CreateSprite(BITMAP_SHINY + 6, vPos, fScale, vLight1, o);
#else // LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE
        CreateSprite(BITMAP_SHINY + 5, vPos, 0.5f, vLight2, o);
        CreateSprite(BITMAP_SHINY + 5, vPos, fScale, vLight1, o);
        Vector(3.f, -3.f, -3.5f, vRelativePos);
        b->TransformPosition(BoneTransform[3], vRelativePos, vPos);
        CreateSprite(BITMAP_SHINY + 5, vPos, 0.5f, vLight2, o);
        CreateSprite(BITMAP_SHINY + 5, vPos, fScale, vLight1, o);
#endif // LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE
    }
    return true;

    case 20:
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
        else if (o->AnimationFrame > 15.4f && o->AnimationFrame < 16.5f)
        {
            vec3_t	Angle;
            for (int i = 0; i < 4; ++i)
            {
                Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2], Angle);
                CreateJointFpsChecked(BITMAP_JOINT_SPARK, o->Position, o->Position, Angle, 5, o);
            }
            CreateParticleFpsChecked(BITMAP_SPARK, o->Position, Angle, o->Light, 11);
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

    case 157:
    {
        if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE)
            return true;

        vec3_t vPos, vRelativePos, vLightFire01, vLightFire02, vLightFlareFire, vLightSmoke, vAngle;
        Vector(0.f, 0.f, 0.f, vPos);

        Vector(4.f, 0.f, 0.0f, vRelativePos);
        Vector(0.0f, 0.0f, 0.0f, vAngle);
        Vector(0.9f, 0.5f, 0.0f, vLightFire01);
        Vector(0.75f, 0.3f, 0.0f, vLightFire02);

        int	arriCandleFire[] = { 22, 23, 25 };
        int	arriCandleSmoke[] = { 26, 24, 21 };

        Vector(0.65f, 0.45f, 0.02f, vLightFlareFire);

        {
            for (int i = 0; i < 3; ++i)
            {
                int iCurBoneIdx = arriCandleFire[i];
                b->TransformPosition(BoneTransform[iCurBoneIdx], vRelativePos, vPos);

                for (int j = 0; j < 5; ++j)
                {
                    CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, vPos, vAngle, vLightFire01, 3, o->Scale * 0.2f);
                    CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, vPos, vAngle, vLightFire02, 3, o->Scale * 0.1f);
                }
            }
        }

        Vector(4.f, 0.f, 0.0f, vRelativePos);
        Vector(1.f, 1.f, 1.f, vLightSmoke);

        {
            for (int i = 0; i < 3; i++)
            {
                int iCurBoneIdx = arriCandleSmoke[i];
                b->TransformPosition(BoneTransform[iCurBoneIdx], vRelativePos, vPos);

                for (int j = 0; j < 4; ++j)
                {
                    CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLightSmoke, 65, o->Scale * 0.1, o);
                }
            }
        }
    }
    return true;
    case 158:
    {
        for (int i_ = 0; i_ < 1; ++i_)
        {
            CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 64, o->Scale, o);
        }
    }
    return true;
    }

    return false;
}

bool GMEmpireGuardian4::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_GAYION:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    break;
    case MODEL_JERRY:
    case MODEL_DEATH_ANGEL_3:
    {
        vec3_t		v3LightBackup;

        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        Vector(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2], v3LightBackup);
        Vector(0.3f, 0.3f, 0.3f, b->BodyLight);

        b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        VectorCopy(v3LightBackup, b->BodyLight);
    }
    return true;
    }
    return false;
}

bool GMEmpireGuardian4::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (g_EmpireGuardian1.RenderMonsterVisual(c, o, b))
    {
        return true;
    }

    vec3_t vPos, vRelative, vLight;

    switch (o->Type)
    {
    case MODEL_GAYION:
    {
        VectorCopy(o->Position, b->BodyOrigin);
        Vector(0.0f, 0.0f, 0.0f, vRelative);

        float fLumi1 = (sinf(WorldTime * 0.004f) + 1.f) * 0.25f;
        float fLumi2 = (sinf(WorldTime * 0.004f) + 1.f) * 0.2f;

        Vector(0.9f + fLumi1, 0.3f + fLumi1, 0.2f + fLumi1, vLight);
        Vector(0.0f, 5.0f, 0.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 57, vRelative);

        CreateSprite(BITMAP_LIGHT_RED, vPos, 1.5f + fLumi2, vLight, o);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f + fLumi2, vLight, o);

        Vector(0.0f, -10.0f, 0.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 57, vRelative);
        Vector(1.0f, 0.0f, 0.0f, vLight);
        CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 10, 2.0f);

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
    break;
    case MODEL_JERRY:
    case MODEL_DEATH_ANGEL_3:
    {
        VectorCopy(o->Position, b->BodyOrigin);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        float fLumi1 = (sinf(WorldTime * 0.004f) + 1.f) * 0.05f;
        float fLumi2 = (sinf(WorldTime * 0.004f) + 1.f) * 0.2f;

        float fSize = 1.6f;
        Vector(0.6f + fLumi1, 0.2f + fLumi1, 0.1f + fLumi1, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);

        b->TransformByObjectBone(vPos, o, 50, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 54, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 59, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 60, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 53, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 55, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 56, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        b->TransformByObjectBone(vPos, o, 57, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi2, vLight, o);

        float fLumi3 = (cosf(WorldTime * 0.004f) + 1.f) * 0.1f;
        float fLumi4 = (cosf(WorldTime * 0.004f) + 1.f) * 0.2f;

        fSize = 1.6f;

        Vector(0.9f + fLumi3, 0.1f + fLumi3, 0.6f + fLumi3, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);

        b->TransformByObjectBone(vPos, o, 29, vRelative);		// node_body01
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 30, vRelative);		// node_body02
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 31, vRelative);		// node_body03
        CreateSprite(BITMAP_LIGHT, vPos, (fSize * 0.6f) + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 67, vRelative);		// node_body04
        CreateSprite(BITMAP_LIGHT, vPos, (fSize * 0.6f) + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 66, vRelative);		// node_body05
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 65, vRelative);		// node_body06
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 9, vRelative);		// node_body07
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 28, vRelative);		// node_body08
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 64, vRelative);		// node_body09
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        fLumi3 = (cosf(WorldTime * 0.004f) + 1.f) * 0.25f;
        fLumi4 = (cosf(WorldTime * 0.004f) + 1.f) * 0.2f;

        fSize = 1.3f;

        Vector(0.9f + fLumi3, 0.1f + fLumi3, 0.4f + fLumi3, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);

        b->TransformByObjectBone(vPos, o, 7, vRelative);		// Bip01 Head
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 11, vRelative);		// Bip01 L UpperArm
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 33, vRelative);		// Bip01 R UpperArm
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        fSize = 1.3f;

        Vector(0.9f + fLumi3, 0.1f + fLumi3, 0.4f + fLumi3, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);

        b->TransformByObjectBone(vPos, o, 7, vRelative);		// Bip01 Head
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 11, vRelative);		// Bip01 L UpperArm
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        b->TransformByObjectBone(vPos, o, 33, vRelative);		// Bip01 R UpperArm
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        fSize = 2.3f;

        Vector(0.9f + fLumi3, 0.1f + fLumi3, 0.4f + fLumi3, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);

        b->TransformByObjectBone(vPos, o, 5, vRelative);		// Bip01 Spine2
        CreateSprite(BITMAP_LIGHT, vPos, fSize + fLumi4, vLight, o);

        if (g_isNotCharacterBuff(o) == true && g_isCharacterBuff(o, eBuff_Berserker) == true)
        {
            vec3_t vLightRage, vPosRage;

            int iBoneCount = b->NumBones;
            Vector(1.0f, 0.1f, 0.1f, vLightRage);
            for (int i = 0; i < iBoneCount; ++i)
            {
                if (i % 5 == 0 && rand_fps_check(5))
                {
                    b->TransformByObjectBone(vPosRage, o, i);
                    CreateParticle(BITMAP_SMOKE, vPosRage, o->Angle, vLightRage, 50, 1.0f);
                    CreateParticle(BITMAP_SMOKELINE1 + rand() % 3, vPosRage, o->Angle, vLightRage, 0, 1.0f);
                }
            }
        }
    }
    return true;
    }
    return false;
}

void GMEmpireGuardian4::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return;

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

void GMEmpireGuardian4::SetWeather(int weather)
{
    g_EmpireGuardian1.SetWeather(weather);
}

void GMEmpireGuardian4::RenderFrontSideVisual()
{
    g_EmpireGuardian1.RenderFrontSideVisual();
}

bool GMEmpireGuardian4::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

    if (true == g_EmpireGuardian1.SetCurrentActionMonster(c, o))
    {
        return true;
    }

    switch (c->MonsterIndex)
    {
    case MONSTER_GAYION_THE_GLADIATOR:
    {
        if (m_bCurrentIsRage_BossGaion == true)
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            return true;
        }

        switch (c->MonsterSkill)
        {
        case ATMON_SKILL_EMPIREGUARDIAN_GAION_01_GENERALATTACK:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_GAION_02_BLOODATTACK:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_GAION_03_GIGANTIKSTORM:
        {
            SetAction(o, MONSTER01_ATTACK3);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_GAION_04_FLAMEATTACK:
        {
            SetAction(o, MONSTER01_ATTACK4);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            m_bCurrentIsRage_BossGaion = true;
        }
        break;
        default:
        {
            SetAction(o, MONSTER01_ATTACK1);
            c->MonsterSkill = -1;
        }
        break;
        }

        return true;
    }
    return true;
    case MONSTER_JERRY:
    {
        if (m_bCurrentIsRage_Jerint == true)
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
            return true;
        }

        switch (c->MonsterSkill)
        {
        case 55:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 61:
        {
            SetAction(o, MONSTER01_ATTACK3);
            c->MonsterSkill = -1;
        }
        break;
        case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;

            m_bCurrentIsRage_Jerint = true;
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

bool GMEmpireGuardian4::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

    return false;
}

bool GMEmpireGuardian4::PlayMonsterSound(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian4() == false)
        return false;

    if (true == g_EmpireGuardian1.PlayMonsterSound(o))
    {
        return true;
    }

    float fDis_x, fDis_y;
    fDis_x = o->Position[0] - Hero->Object.Position[0];
    fDis_y = o->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 500.0f)
        return true;

    switch (o->Type)
    {
    case MODEL_GAYION:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_MOVE);
        }
        return true;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_DEATH);
        }
        return true;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_RAGE);
        }
        return true;
        }
    }
    return true;
    case MODEL_JERRY:
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
        return true;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
        }
        return true;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_BLOODATTACK);
        }
        return true;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK03);
            PlayBuffer(SOUND_SKILL_BLOWOFDESTRUCTION);
        }
        return true;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
        }
        return true;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE);
        }
        return true;
        }
    }
    return true;
    }

    return false;
}

void GMEmpireGuardian4::PlayObjectSound(OBJECT* o)
{
    g_EmpireGuardian1.PlayObjectSound(o);
}

void GMEmpireGuardian4::PlayBGM()
{
    if (gMapManager.IsEmpireGuardian4())
    {
        PlayMp3(MUSIC_EMPIREGUARDIAN4);
    }
    else
    {
        StopMp3(MUSIC_EMPIREGUARDIAN4);
    }
}