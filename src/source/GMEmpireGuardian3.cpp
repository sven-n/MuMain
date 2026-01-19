// GMEmpireGuardian3.cpp: implementation of the GMEmpireGuardian3 class.
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



GMEmpireGuardian3Ptr GMEmpireGuardian3::Make()
{
    GMEmpireGuardian3Ptr empire(new GMEmpireGuardian3);
    empire->Init();
    return empire;
}

GMEmpireGuardian3::GMEmpireGuardian3()
{
    // 	m_iWeather = WEATHER_TYPE::WEATHER_SUN;
}

GMEmpireGuardian3::~GMEmpireGuardian3()
{
    Destroy();
}

void GMEmpireGuardian3::Init()
{
}

void GMEmpireGuardian3::Destroy()
{
}

bool GMEmpireGuardian3::CreateObject(OBJECT* o)
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

CHARACTER* GMEmpireGuardian3::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = g_EmpireGuardian1.CreateMonster(iType, PosX, PosY, Key);

    if (NULL != pCharacter)
    {
        return pCharacter;
    }

    switch (iType)
    {
    case MONSTER_DUAL_BERSERKER:
    {
        OpenMonsterModel(MONSTER_MODEL_DUAL_BERSERKER);
        pCharacter = CreateCharacter(Key, MODEL_DUAL_BERSERKER, PosX, PosY);
        pCharacter->Object.Scale = 1.35f;

        m_bCurrentIsRage_Kato = false;
    }
    break;
    case MONSTER_BANSHEE:
    {
        OpenMonsterModel(MONSTER_MODEL_BANSHEE);
        pCharacter = CreateCharacter(Key, MODEL_BANSHEE, PosX, PosY);
        pCharacter->Object.Scale = 1.55f;
    }
    break;
    case MONSTER_HEAD_MOUNTER:
    {
        OpenMonsterModel(MONSTER_MODEL_HEAD_MOUNTER);
        pCharacter = CreateCharacter(Key, MODEL_HEAD_MOUNTER, PosX, PosY);
        pCharacter->Object.Scale = 1.25f;
    }
    break;

    default: return pCharacter;
    }

    return pCharacter;
}

bool GMEmpireGuardian3::MoveObject(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian3() == false)
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
        o->Velocity = 0.44f;
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

bool GMEmpireGuardian3::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (true == g_EmpireGuardian1.MoveMonsterVisual(o, b))
    {
        return true;
    }

    return false;
}

bool GMEmpireGuardian3::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    //  	if(IsEmpireGuardian3() == false)
      //		return false;

    if (true == g_EmpireGuardian1.MoveMonsterVisual(c, o, b))
    {
        return true;
    }

    vec3_t vPos;

    switch (o->Type)
    {
    case MODEL_DUAL_BERSERKER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        case MONSTER01_ATTACK1:
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK3:
        case MONSTER01_DIE:
            break;
        case MONSTER01_APEAR:
        {
            vec3_t		Light;

            if (m_bCurrentIsRage_Kato == true)
            {
                Vector(1.0f, 1.0f, 1.0f, Light);
                CreateInferno(o->Position);

                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                m_bCurrentIsRage_Kato = false;
            }
        }
        break;
        }
    }
    return true;
    case MODEL_BANSHEE:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
            break;
        case MONSTER01_ATTACK1:
        {
            if (2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f)
            {
                vec3_t  Light;
                //						Vector(0.3f, 0.3f, 0.3f, Light);
                Vector(0.3f, 0.8f, 0.4f, Light);

                vec3_t StartPos, StartRelative;
                vec3_t EndPos, EndRelative;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 10.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 10; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[49], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[51], EndRelative, EndPos, false);
                    CreateObjectBlur(o, StartPos, EndPos, Light, 3, false, o->m_iAnimation + 1);
                    //							CreateObjectBlur(o, StartPos, EndPos, Light, 2, false, o->m_iAnimation + 1);
                    //							CreateObjectBlur(o, StartPos, EndPos, Light, 3, false, o->m_iAnimation + 1);
                    //							CreateObjectBlur(o, StartPos, EndPos, Light, 2);
                    //							CreateObjectBlur(o, StartPos, EndPos, Light, 3);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (c->AttackTime >= 1 && c->AttackTime <= 2)
            {
                vec3_t Angle;
                Vector(1.f, 0.f, 0.f, Angle);
                CreateEffect(BITMAP_GATHERING, o->Position, o->Angle, o->Light, 1, o);
            }

            if (4.5f <= o->AnimationFrame && o->AnimationFrame < 4.6f)
            {
                vec3_t  Light;
                Vector(1.0f, 0.5f, 0.2f, Light);

                vec3_t vPosBlur01, vPosBlurRelative01;
                vec3_t vPosBlur02, vPosBlurRelative02;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 6.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 8; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative01);
                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative02);

                    b->TransformPosition(BoneTransform[49], vPosBlurRelative01, vPosBlur01, false);
                    b->TransformPosition(BoneTransform[51], vPosBlurRelative02, vPosBlur02, false);

                    CreateBlur(c, vPosBlur01, vPosBlur02, Light, 2);

                    fAnimationFrame += fSpeedPerFrame;
                }
                Vector(0.1f, 0.6f, 0.3f, Light);
                CreateJoint(BITMAP_JOINT_FORCE, vPosBlur01, vPosBlur02, o->Angle, 10, o, 150.f, o->PKKey, o->Skill, 1, -1, Light);

                for (int iLoop = 0; iLoop < 3; iLoop++)
                {
                    vPosBlur01[2] -= 30.f;
                    vPosBlur02[2] -= 30.f;
                    CreateJoint(BITMAP_JOINT_FORCE, vPosBlur01, vPosBlur02, o->Angle, 10, o, 150.f, o->PKKey, o->Skill, 1, -1, Light);
                }

                //						c->AttackTime = 15;
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (c->AttackTime >= 1 && c->AttackTime <= 2)
            {
                vec3_t Angle;
                Vector(1.f, 0.f, 0.f, Angle);
                CreateEffect(BITMAP_GATHERING, o->Position, o->Angle, o->Light, 1, o);
            }

            if (4.5f <= o->AnimationFrame && o->AnimationFrame < 4.6f)
            {
                vec3_t  Light;
                Vector(1.0f, 0.5f, 0.2f, Light);

                vec3_t vPosBlur01, vPosBlurRelative01;
                vec3_t vPosBlur02, vPosBlurRelative02;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 6.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 8; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative01);
                    Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative02);

                    b->TransformPosition(BoneTransform[49], vPosBlurRelative01, vPosBlur01, false);
                    b->TransformPosition(BoneTransform[51], vPosBlurRelative02, vPosBlur02, false);

                    CreateBlur(c, vPosBlur01, vPosBlur02, Light, 2);

                    fAnimationFrame += fSpeedPerFrame;
                }
                Vector(1.f, 1.f, 1.f, Light);
                CreateJoint(BITMAP_JOINT_FORCE, vPosBlur01, vPosBlur02, o->Angle, 10, o, 150.f, o->PKKey, o->Skill, 1, -1, Light);

                for (int iLoop = 0; iLoop < 3; iLoop++)
                {
                    vPosBlur01[2] -= 30.f;
                    vPosBlur02[2] -= 30.f;
                    CreateJoint(BITMAP_JOINT_FORCE, vPosBlur01, vPosBlur02, o->Angle, 10, o, 150.f, o->PKKey, o->Skill, 1, -1, Light);
                }

                //						c->AttackTime = 15;
            }
        }
        break;
        case MONSTER01_DIE:
            break;
        case MONSTER01_APEAR:
        {
        }
        break;
        }
    }
    return true;
    case MODEL_HEAD_MOUNTER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
        {
            if (4.5f <= o->AnimationFrame && o->AnimationFrame < 10.0f)
            {
                Vector(0.8f, 0.4f, 0.1f, o->Light);

                b->TransformByObjectBone(vPos, o, 15);
                CreateParticleFpsChecked(BITMAP_CLUD64, vPos, o->Angle, o->Light, 3, 0.2f);
                CreateParticleFpsChecked(BITMAP_WATERFALL_3, vPos, o->Angle, o->Light, 13, 1.0f);
            }
        }
        break;
        case MONSTER01_WALK:
        {
            Vector(0.5f, 0.2f, 0.1f, o->Light);

            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                b->TransformByObjectBone(vPos, o, 32);
                vPos[2] += 25.0f;
                CreateParticleFpsChecked(BITMAP_ADV_SMOKE, vPos, o->Angle, o->Light, 3, 2.0f);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                b->TransformByObjectBone(vPos, o, 19);
                vPos[2] += 25.0f;
                CreateParticleFpsChecked(BITMAP_ADV_SMOKE, vPos, o->Angle, o->Light, 3, 2.0f);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        {
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (o->AnimationFrame >= 3.8f && o->AnimationFrame <= 9.4f)
            {
                RenderSkillEarthQuake(c, o, b, 14);
            }
            else
            {
                o->WeaponLevel = 0;
            }
        }
        break;
        case MONSTER01_APEAR:
        {
        }
        break;
        case MONSTER01_ATTACK3:
        {
        }
        break;
        case MONSTER01_DIE:
        {
            float Scale = 0.3f;
            b->TransformByObjectBone(vPos, o, 30);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);

            b->TransformByObjectBone(vPos, o, 17);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);

            b->TransformByObjectBone(vPos, o, 2);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);

            b->TransformByObjectBone(vPos, o, 6);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);

            b->TransformByObjectBone(vPos, o, 98);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);

            b->TransformByObjectBone(vPos, o, 91);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);

            b->TransformByObjectBone(vPos, o, 2);
            CreateParticleFpsChecked(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);

            b->TransformByObjectBone(vPos, o, 3);
            CreateParticleFpsChecked(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
        }
        break;
        } //switch end
    }
    return true;
    } //switch end

    return false;
}

void GMEmpireGuardian3::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
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
    case MODEL_DUAL_BERSERKER:
    {
        vec3_t  Light;
        vec3_t StartPos, StartRelative;
        vec3_t EndPos, EndRelative;
        float fActionSpeed, fSpeedPerFrame, fAnimationFrame;

        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (4.2f <= o->AnimationFrame && o->AnimationFrame < 6.6f)
            {
                Vector(0.9f, 0.5f, 0.4f, Light);

                fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                fSpeedPerFrame = fActionSpeed / 5.f;
                fAnimationFrame = o->AnimationFrame - fActionSpeed;

                for (int i = 0; i < 3; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2, false, 1);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
            else if (10.2f <= o->AnimationFrame && o->AnimationFrame < 12.6f)
            {
                Vector(0.9f, 0.5f, 0.4f, Light);

                fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);;
                fSpeedPerFrame = fActionSpeed / 5.f;
                fAnimationFrame = o->AnimationFrame - fActionSpeed;

                for (int i = 0; i < 3; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[44], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2, false, 0);
                    //b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
                    //b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
                    //CreateBlur(c, StartPos, EndPos, Light, 2, false, 1);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (6.2f <= o->AnimationFrame && o->AnimationFrame < 8.2f)
            {
                Vector(0.9f, 0.5f, 0.4f, Light);

                fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);;
                fSpeedPerFrame = fActionSpeed / 5.f;
                fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 3; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[44], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2, false, 0);
                    b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2, false, 1);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            vec3_t	vRelative, vRelative2, vPosition;
            Vector(0.0f, 0.0f, 0.0f, vRelative);
            Vector(0.0f, 0.0f, 0.0f, vRelative2);

            if (6.2f <= o->AnimationFrame && o->AnimationFrame < 8.2f)
            {
                Vector(0.9f, 0.5f, 0.4f, Light);

                fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);;
                fSpeedPerFrame = fActionSpeed / 10.f;
                fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 16; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[44], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2, false, 10);
                    b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 2, false, 21);

                    fAnimationFrame += fSpeedPerFrame;

                    if (7.4f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                    {
                        int iOffset = 80;

                        for (int i_ = 0; i_ < 8; i_++)
                        {
                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[24], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_FIRE, vPosition, o->Angle, o->Light, 0);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[25], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_FIRE, vPosition, o->Angle, o->Light, 0);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[43], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_FIRE, vPosition, o->Angle, o->Light, 0);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[44], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_FIRE, vPosition, o->Angle, o->Light, 0);

                            vec3_t	vLight__;
                            Vector(0.9f, 0.9f, 0.9f, vLight__);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[24], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_SMOKELINE2, vPosition, o->Angle, vLight__, 3);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[25], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_SMOKELINE2, vPosition, o->Angle, vLight__, 3);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[43], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_SMOKELINE2, vPosition, o->Angle, vLight__, 3);

                            vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
                            vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
                            b->TransformPosition(BoneTransform[44], vRelative, vPosition, false);
                            CreateParticleFpsChecked(BITMAP_SMOKELINE2, vPosition, o->Angle, vLight__, 3);
                        }
                    } // 검기
                }
            }
        }
    }
    break;
    case MODEL_HEAD_MOUNTER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_ATTACK1:
        {
            if (4.2f <= o->AnimationFrame && o->AnimationFrame < 8.9f)
            {
                vec3_t  Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

                vec3_t StartPos, StartRelative;
                vec3_t EndPos, EndRelative;

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
                float fSpeedPerFrame = fActionSpeed / 5.f;
                float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                for (int i = 0; i < 5; i++)
                {
                    b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    Vector(0.0f, 0.0f, 0.0f, StartRelative);
                    Vector(0.0f, 0.0f, 0.0f, EndRelative);

                    b->TransformPosition(BoneTransform[35], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[36], EndRelative, EndPos, false);
                    //CreateBlur(c, StartPos, EndPos, Light, 6);
                    CreateBlur(c, EndPos, StartPos, Light, 6);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        } //switch end
    }
    break;
    } //switch end
}

bool GMEmpireGuardian3::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian3() == false)
        return false;

    if (true == g_EmpireGuardian1.RenderObjectMesh(o, b, ExtraMon))
    {
        return true;
    }

    return false;
}

bool GMEmpireGuardian3::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian3() == false)
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
            CreateParticle(BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 4, o->Scale);
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

bool GMEmpireGuardian3::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
    return false;
}

bool GMEmpireGuardian3::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (g_EmpireGuardian1.RenderMonsterVisual(c, o, b))
    {
        return true;
    }

    switch (o->Type)
    {
    case MODEL_DUAL_BERSERKER:
    {
        vec3_t Light, Position;

        Vector(0.4f, 0.4f, 0.4f, Light);
        b->TransformByObjectBone(Position, o, 9);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 10);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 28);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 29);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 47);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 48);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 123);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 124);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 119);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 120);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 125);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
        b->TransformByObjectBone(Position, o, 126);
        CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);

        Vector(0.9f, 0.9f, 0.9f, Light);
        if (rand_fps_check(3))
        {
            b->TransformByObjectBone(Position, o, 54);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 58);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 64);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 65);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 69);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 70);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 77);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 78);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 83);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 87);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 91);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
            b->TransformByObjectBone(Position, o, 92);
            CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
        }

        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 0.5f, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

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
    case MODEL_BANSHEE:
    {
    }
    return true;
    }
    return false;
}

void GMEmpireGuardian3::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian3() == false)
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

void GMEmpireGuardian3::SetWeather(int weather)
{
    g_EmpireGuardian1.SetWeather(weather);
}

bool GMEmpireGuardian3::CreateRain(PARTICLE* o)
{
    return g_EmpireGuardian1.CreateRain(o);
}

void GMEmpireGuardian3::RenderFrontSideVisual()
{
    g_EmpireGuardian1.RenderFrontSideVisual();
}

bool GMEmpireGuardian3::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian3() == false)
        return false;

    if (true == g_EmpireGuardian1.SetCurrentActionMonster(c, o))
    {
        return true;
    }

    switch (c->MonsterIndex)
    {
    case MONSTER_DUAL_BERSERKER:
    {
        if (m_bCurrentIsRage_Kato == true)
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

            m_bCurrentIsRage_Kato = true;
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
    case MONSTER_HEAD_MOUNTER:
    {
        switch (c->MonsterSkill)
        {
        case 54:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 55:
        {
            SetAction(o, MONSTER01_APEAR);
            c->MonsterSkill = -1;
        }
        break;
        case 56:
        {
            SetAction(o, MONSTER01_ATTACK3);
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
    case MONSTER_BANSHEE:
    {
        switch (c->MonsterSkill)
        {
        case 47:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 53:
        {
            SetAction(o, MONSTER01_ATTACK3);
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

bool GMEmpireGuardian3::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian3() == false)
        return false;
    return false;
}

bool GMEmpireGuardian3::PlayMonsterSound(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian3() == false)
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
    case MODEL_DUAL_BERSERKER:
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
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_3CORP_CATO_ATTACK02);
        }
        break;
        case MONSTER01_APEAR:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE);
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

    case MODEL_BANSHEE:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_MOVE);
        }
        break;
        case MONSTER01_ATTACK1:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02);
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_ASSASSINMASTER_DEATH);
        }
        break;
        }
    }
    return true;

    case MODEL_HEAD_MOUNTER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_WALK:
        {
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE01);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE02);
            }
        }
        break;
        case MONSTER01_ATTACK1:
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        case MONSTER01_ATTACK4:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK02);
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_RAKLION_GIANT_MAMUD_DEATH);
        }
        break;
        }
    }
    return true;
    }

    return false;
}

void GMEmpireGuardian3::PlayObjectSound(OBJECT* o)
{
    g_EmpireGuardian1.PlayObjectSound(o);
}

void GMEmpireGuardian3::PlayBGM()
{
    if (gMapManager.IsEmpireGuardian3())
    {
        PlayMp3(MUSIC_EMPIREGUARDIAN3);
    }
    else
    {
        StopMp3(MUSIC_EMPIREGUARDIAN3);
    }
}