// GMEmpireGuardian2.cpp: implementation of the GMEmpireGuardian2 class.
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



GMEmpireGuardian2Ptr GMEmpireGuardian2::Make()
{
    GMEmpireGuardian2Ptr doppelganger(new GMEmpireGuardian2);
    doppelganger->Init();
    return doppelganger;
}

GMEmpireGuardian2::GMEmpireGuardian2()
{
}

GMEmpireGuardian2::~GMEmpireGuardian2()
{
    Destroy();
}

void GMEmpireGuardian2::Init()
{
}

void GMEmpireGuardian2::Destroy()
{
}

bool GMEmpireGuardian2::CreateObject(OBJECT* o)
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

CHARACTER* GMEmpireGuardian2::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = g_EmpireGuardian1.CreateMonster(iType, PosX, PosY, Key);

    if (NULL != pCharacter)
    {
        return pCharacter;
    }

    switch (iType)
    {
    case MONSTER_HAMMERIZE:
    {
        OpenMonsterModel(MONSTER_MODEL_HAMMERIZE);
        pCharacter = CreateCharacter(Key, MODEL_HAMMERIZE, PosX, PosY);

        pCharacter->Object.Scale = 1.3f;

        m_bCurrentIsRage_Bermont = false;
    }
    break;
    case MONSTER_ATICLES_HEAD:
    {
        OpenMonsterModel(MONSTER_MODEL_ATICLES_HEAD);
        pCharacter = CreateCharacter(Key, MODEL_ATICLES_HEAD, PosX, PosY);

        pCharacter->Object.Scale = 1.35f;
    }
    break;
    case MONSTER_DARK_GHOST:
    {
        OpenMonsterModel(MONSTER_MODEL_DARK_GHOST);
        pCharacter = CreateCharacter(Key, MODEL_DARK_GHOST, PosX, PosY);

        OBJECT* pObject = &pCharacter->Object;
        pObject->Scale = 1.3f;

        MoveEye(pObject, &Models[pObject->Type], 79, 33);
        CreateJoint(BITMAP_JOINT_ENERGY, pObject->Position, pObject->Position, pObject->Angle, 22, pObject, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, pObject->Position, pObject->Position, pObject->Angle, 23, pObject, 30.f);
    }
    break;

    default: return pCharacter;
    }

    return pCharacter;
}

bool GMEmpireGuardian2::MoveObject(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian2() == false)
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
    break;
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

bool GMEmpireGuardian2::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian2() == false)
        return false;

    if (true == g_EmpireGuardian1.MoveMonsterVisual(o, b))
    {
        return true;
    }

    return false;
}

bool GMEmpireGuardian2::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian2() == false)
        return false;

    if (true == g_EmpireGuardian1.MoveMonsterVisual(c, o, b))
    {
        return true;
    }

    switch (o->Type)
    {
    case MODEL_HAMMERIZE:
    {
        vec3_t vPos;
        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
            break;
        case MONSTER01_WALK:
        {
            int		iTypeSubType = 0;

            Vector(0.4f, 0.4f, 0.4f, o->Light);

            if (6.8f <= o->AnimationFrame && o->AnimationFrame < 7.5f)
            {
                b->TransformByObjectBone(vPos, o, 42);
                vPos[2] += 25.0f;
                vPos[1] += 0.0f;
                //vPos[0] += 100.0f;
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 1.0f);
            }
            if (0.8f <= o->AnimationFrame && o->AnimationFrame < 1.5f)
            {
                b->TransformByObjectBone(vPos, o, 49);
                vPos[2] += 25.0f;
                vPos[1] += 0.0f;
                //vPos[0] += 100.0f;
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 1.0f);
            }
        }
        break;
        case MONSTER01_DIE:
        {
        }
        break;
        case MONSTER01_ATTACK1:
        {
            if (6.0f <= o->AnimationFrame && o->AnimationFrame < 12.0f)
            {
                vec3_t  Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

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

                    b->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK2:
        {
            if (o->AnimationFrame >= 5.4f && o->AnimationFrame <= 11.0f)
            {
                RenderSkillEarthQuake(c, o, b, 14);
            }

            if (6.0f <= o->AnimationFrame && o->AnimationFrame < 10.0f)
            {
                vec3_t  Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

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

                    b->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_ATTACK3:
        {
            if (o->AnimationFrame >= 1.7f && o->AnimationFrame <= 2.3f)
            {
                CHARACTER* chT = &CharactersClient[c->TargetCharacter];
                vec3_t& v3TargetPos = chT->Object.Position;

                Vector(v3TargetPos[0], v3TargetPos[1], v3TargetPos[2], o->Light);
                CreateEffect(MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION,
                    o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, 1.0f);
            }

            if (6.0f <= o->AnimationFrame && o->AnimationFrame < 12.0f)
            {
                vec3_t  Light;
                Vector(1.0f, 1.0f, 1.0f, Light);

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

                    b->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
                    b->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                    CreateBlur(c, StartPos, EndPos, Light, 0);

                    fAnimationFrame += fSpeedPerFrame;
                }
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            vec3_t		Light;

            if (m_bCurrentIsRage_Bermont == true)
            {
                Vector(1.0f, 1.0f, 1.0f, Light);
                CreateInferno(o->Position);

                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                m_bCurrentIsRage_Bermont = false;
            }
        }
        break;
        }
    }
    return true;
    case MODEL_ATICLES_HEAD:
    {
        vec3_t vPos, vRelative, vLight, v3Temp;

        float fLumi1 = (sinf(WorldTime * 0.004f) + 1.f) * 0.25f;

        Vector(0.7f + fLumi1, 0.7f + fLumi1, 0.7f + fLumi1, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 21, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f + fLumi1, vLight, o);

        b->TransformByObjectBone(vPos, o, 11, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f + fLumi1, vLight, o);

        Vector(0.4f, 0.5f, 0.7f, vLight);
        b->TransformPosition(o->BoneTransform[8], v3Temp, vPos, true);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 2.0f, vLight, o);

        switch (o->CurrentAction)
        {
        case MONSTER01_STOP1:
        case MONSTER01_STOP2:
        {
        }
        break;
        case MONSTER01_WALK:
        {
            int		iTypeSubType = 0;

            Vector(0.7f, 0.7f, 0.7f, o->Light);

            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
            {
                b->TransformByObjectBone(vPos, o, 42);
                vPos[2] += 25.0f;
                vPos[1] += 0.0f;
                //vPos[0] += 100.0f;
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 2.0f);
            }
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
            {
                b->TransformByObjectBone(vPos, o, 47);
                vPos[2] += 25.0f;
                vPos[1] += 0.0f;
                //vPos[0] += 100.0f;
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 2.0f);
            }
        }
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
            // 						if( o->AnimationFrame >= 6.6f && o->AnimationFrame <= 7.4f )
            // 						{
            // 							CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
            // 							CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
            // 							CreateEffect ( MODEL_PIERCING2, o->Position, o->Angle, o->Light );
            // 							PlayBuffer ( SOUND_ATTACK_SPEAR );
            // 						}
        }
        break;
        case MONSTER01_ATTACK2:
        {
            vec3_t	v3PosPiercing, v3AnglePiercing;

            if (o->AnimationFrame >= 6.6f && o->AnimationFrame <= 7.4f)
            {
                b->TransformByObjectBone(v3PosPiercing, o, 23);
                v3PosPiercing[2] = v3PosPiercing[2] - 150.0f;

                VectorCopy(o->Angle, v3AnglePiercing);
                v3AnglePiercing[2] = v3AnglePiercing[2] + 5.0f;

                //CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 2 );
                //CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
                CreateEffect(MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light, 0);

                PlayBuffer(SOUND_ATTACK_SPEAR);
            }
        }
        break;
        case MONSTER01_APEAR:
        {
            vec3_t	v3PosPiercing, v3AnglePiercing;
            if (o->AnimationFrame >= 6.6f && o->AnimationFrame <= 7.4f)
            {
                b->TransformByObjectBone(v3PosPiercing, o, 23);
                v3PosPiercing[2] = v3PosPiercing[2] - 150.0f;

                VectorCopy(o->Angle, v3AnglePiercing);
                v3AnglePiercing[2] = v3AnglePiercing[2] + 5.0f;

                CreateEffect(MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1);
                CreateEffect(MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1);
                CreateEffect(MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1);
                CreateEffect(MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1);
                CreateEffect(MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1);
                CreateEffect(MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1);

                CreateEffect(MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light);
                CreateEffect(MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light);
                CreateEffect(MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light);
                CreateEffect(MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light);
                CreateEffect(MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light);

                PlayBuffer(SOUND_ATTACK_SPEAR);
            }
        }
        break;
        }
    }
    return true;
    }
    return false;
}

void GMEmpireGuardian2::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_LUCAS:
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

bool GMEmpireGuardian2::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian2() == false)
        return false;

    if (true == g_EmpireGuardian1.RenderObjectMesh(o, b, ExtraMon))
    {
        return true;
    }

    switch (o->Type)
    {
    case MODEL_LUCAS:
    case MODEL_HAMMERIZE:
    case MODEL_ATICLES_HEAD:
    case MODEL_DARK_GHOST:
    {
        RenderMonster(o, b, ExtraMon);

        return true;
    }
    }

    return false;
}

bool GMEmpireGuardian2::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian2() == false)
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

bool GMEmpireGuardian2::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_HAMMERIZE:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    case MODEL_ATICLES_HEAD:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    case MODEL_DARK_GHOST:
    {
        if (o->CurrentAction == MONSTER01_DIE)
        {
            Vector(0.3f, 1.0f, 0.2f, b->BodyLight);
        }
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 3, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    }
    return false;
}

bool GMEmpireGuardian2::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (g_EmpireGuardian1.RenderMonsterVisual(c, o, b))
    {
        return true;
    }

    vec3_t vPos, vLight;

    switch (o->Type)
    {
    case MODEL_HAMMERIZE:
    {
        vec3_t vPos, vRelative, vLight;
        float fLumi1 = (sinf(WorldTime * 0.004f) + 1.f) * 0.25f;

        Vector(0.1f + fLumi1, 0.4f + fLumi1, 0.8f + fLumi1, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        b->TransformByObjectBone(vPos, o, 21, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 1.5f + fLumi1, vLight, o);

        b->TransformByObjectBone(vPos, o, 38, vRelative);
        CreateSprite(BITMAP_LIGHT, vPos, 1.5f + fLumi1, vLight, o);

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
                    CreateParticle(BITMAP_SMOKE, vPosRage, o->Angle, vLightRage, 50, 1.0f);	// 연기
                    CreateParticle(BITMAP_SMOKELINE1 + rand() % 3, vPosRage, o->Angle, vLightRage, 0, 1.0f);	// 3종 연기
                }
            }
        }
    }
    return true;
    case MODEL_DARK_GHOST:
    {
        int i;
        float fLumi = (sinf(WorldTime * 0.08f) + 1.0f) * 0.5f * 0.3f + 0.7f;
        Vector(0.1f * fLumi, 0.6f * fLumi, 1.0f * fLumi, vLight);

        int iBlueLights[] = { 93, 100, 47, 54 };
        for (i = 0; i < 4; ++i)
        {
            b->TransformByObjectBone(vPos, o, iBlueLights[i]);
            CreateSprite(BITMAP_LIGHT, vPos, 3.0f, vLight, o);
        }

        Vector(0.1f * fLumi, 1.0f * fLumi, 0.2f * fLumi, vLight);
        int iGreenLights[] = { 92, 99, 46, 53, 95, 88, 42, 49 };
        for (i = 0; i < 8; ++i)
        {
            b->TransformByObjectBone(vPos, o, iGreenLights[i]);
            CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLight, o);
        }

        int iBigGreenLights[] = { 80, 34 };
        for (i = 0; i < 2; ++i)
        {
            b->TransformByObjectBone(vPos, o, iBigGreenLights[i]);
            Vector(0.1f * fLumi, 1.0f * fLumi, 0.3f * fLumi, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 3.0f, vLight, o);
            Vector(0.5f, 0.5f, 0.5f, vLight);
            CreateParticleFpsChecked(BITMAP_CHROME2, vPos, o->Angle, vLight, 0, 0.9f, o);
        }

        MoveEye(o, b, 80, 34);

        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            for (i = 0; i < 2; ++i)
            {
                b->TransformByObjectBone(vPos, o, iBigGreenLights[i]);
                Vector(0.3f, 1.0f, 0.8f, vLight);
                CreateParticleFpsChecked(BITMAP_WATERFALL_4, vPos, o->Angle, vLight, 15, 2.0f);
                Vector(0.0f, 0.4f, 0.0f, vLight);
                CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 13, 1.0f, o);
                CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 13, 1.0f, o);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            o->BlendMesh = -2;
            o->m_bRenderShadow = false;
            if (o->AnimationFrame <= 3.0f)
            {
                Vector(0.1f, 1.0f, 0.2f, vLight);
                for (int i = 0; i < 5; i++)
                {
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 39);
                }

                CreateEffectFpsChecked(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 9, o);

                if (o->AnimationFrame <= 0.2f)
                {
                    Vector(0.4f, 1.0f, 0.6f, vLight);
                    CreateEffectFpsChecked(MODEL_TWINTAIL_EFFECT, o->Position, o->Angle, vLight, 1, o);
                    CreateEffectFpsChecked(MODEL_TWINTAIL_EFFECT, o->Position, o->Angle, vLight, 2, o);
                }
            }
        }
    }
    return true;
    default:
    {
    }
    return true;
    }
    return true;
}

void GMEmpireGuardian2::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (gMapManager.IsEmpireGuardian2() == false)
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

void GMEmpireGuardian2::SetWeather(int weather)
{
    g_EmpireGuardian1.SetWeather(weather);
}

bool GMEmpireGuardian2::CreateRain(PARTICLE* o)
{
    return g_EmpireGuardian1.CreateRain(o);
}

void GMEmpireGuardian2::RenderFrontSideVisual()
{
    g_EmpireGuardian1.RenderFrontSideVisual();
}

bool GMEmpireGuardian2::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian2() == false)
        return false;

    if (true == g_EmpireGuardian1.SetCurrentActionMonster(c, o))
    {
        return true;
    }

    switch (c->MonsterIndex)
    {
    case MONSTER_HAMMERIZE:
    {
        if (m_bCurrentIsRage_Bermont == true)
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

            m_bCurrentIsRage_Bermont = true;
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
    case MONSTER_ATICLES_HEAD:
    {
        switch (c->MonsterSkill)
        {
        case 47:
        {
            SetAction(o, MONSTER01_ATTACK2);
            c->MonsterSkill = -1;
        }
        break;
        case 50:
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
    case MONSTER_DARK_GHOST:
    {
        switch (c->MonsterSkill)
        {
        case 51:
        {
            SetAction(o, MONSTER01_ATTACK2);
        }
        break;
        case 52:
        {
            SetAction(o, MONSTER01_ATTACK2);
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
    case MONSTER_DEFENDER:
    {
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

bool GMEmpireGuardian2::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (gMapManager.IsEmpireGuardian2() == false)
        return false;

    switch (c->MonsterIndex)
    {
    case MONSTER_DARK_GHOST:
    {
        vec3_t vPos;

        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* pTargetCharacter = &CharactersClient[c->TargetCharacter];
                CreateEffect(MODEL_FIRE, pTargetCharacter->Object.Position, o->Angle, o->Light);
                PlayBuffer(SOUND_METEORITE01);
                c->TargetCharacter = -1;
                c->MonsterSkill = -1;
            }
        }
        else if (c->MonsterSkill == 51)
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* pTargetCharacter = &CharactersClient[c->TargetCharacter];
                OBJECT* pTargetObject = &pTargetCharacter->Object;
                CreateEffect(BITMAP_FLAME, pTargetObject->Position, pTargetObject->Angle, pTargetObject->Light, 0, pTargetObject);
                PlayBuffer(SOUND_FLAME);
                c->MonsterSkill = -1;
            }
        }
        else if (c->MonsterSkill == 52)
        {
            Vector(o->Position[0] + rand() % 1024 - 512, o->Position[1] + rand() % 1024 - 512, o->Position[2], vPos);
            CreateEffect(MODEL_FIRE, vPos, o->Angle, o->Light);
            PlayBuffer(SOUND_METEORITE01);
        }
    }
    return true;
    }
    return false;
}

bool GMEmpireGuardian2::PlayMonsterSound(OBJECT* o)
{
    if (gMapManager.IsEmpireGuardian2() == false)
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
    case MODEL_HAMMERIZE:
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
            PlayBuffer(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK01);
        }
        break;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK02);
        }
        break;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK01);
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
    case MODEL_ATICLES_HEAD:
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
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03);
        }
        break;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03);
        }
        break;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03);
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
    case MODEL_DARK_GHOST:
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
            PlayBuffer(SOUND_METEORITE01);
            PlayBuffer(SOUND_EXPLOTION01);
        }
        break;
        case MONSTER01_ATTACK2:
        {
            PlayBuffer(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR);
        }
        break;
        case MONSTER01_ATTACK3:
        {
            PlayBuffer(SOUND_EXPLOTION01);
        }
        break;
        case MONSTER01_DIE:
        {
            PlayBuffer(SOUND_EMPIREGUARDIAN_GRANDWIZARD_DEATH);
        }
        break;
        }
    }
    return true;
    }

    return false;
}

void GMEmpireGuardian2::PlayObjectSound(OBJECT* o)
{
    g_EmpireGuardian1.PlayObjectSound(o);
}

void GMEmpireGuardian2::PlayBGM()
{
    if (gMapManager.IsEmpireGuardian2())
    {
        PlayMp3(MUSIC_EMPIREGUARDIAN2);
    }
    else
    {
        StopMp3(MUSIC_EMPIREGUARDIAN2);
    }
}