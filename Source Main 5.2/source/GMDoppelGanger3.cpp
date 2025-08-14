// GMDoppelGanger3.cpp: implementation of the CGMDoppelGanger3 class.
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



CGMDoppelGanger3Ptr CGMDoppelGanger3::Make()
{
    CGMDoppelGanger3Ptr doppelganger(new CGMDoppelGanger3);
    doppelganger->Init();
    return doppelganger;
}

CGMDoppelGanger3::CGMDoppelGanger3()
{
}

CGMDoppelGanger3::~CGMDoppelGanger3()
{
    Destroy();
}

void CGMDoppelGanger3::Init()
{
}

void CGMDoppelGanger3::Destroy()
{
}

bool CGMDoppelGanger3::CreateObject(OBJECT* o)
{
    // 	switch(o->Type)
    // 	{
    // 	}

    return false;
}

CHARACTER* CGMDoppelGanger3::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    // 	switch (iType)
    // 	{
    // 	}

    return pCharacter;
}

bool CGMDoppelGanger3::MoveObject(OBJECT* o)
{
    if (IsDoppelGanger3() == false)
        return false;

    switch (o->Type)
    {
    case 22:
        o->HiddenMesh = -2;
        o->Timer += 0.1f * FPS_ANIMATION_FACTOR;
        if (o->Timer > 10.f)
            o->Timer = 0.f;
        if (o->Timer > 5.f)
            CreateParticleFpsChecked(BITMAP_BUBBLE, o->Position, o->Angle, o->Light);
        return true;
    case 23:
        o->BlendMesh = 0;
        //o->BlendMeshLight = o->Light[1]+1.f;
        o->BlendMeshLight = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
        return true;
    case 32:
    case 34:
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.5f + 0.5f;
        return true;
    case 38:
        o->BlendMesh = 0;
        //o->BlendMeshLight = sinf(WorldTime*0.004f)*0.3f+0.7f;
        return true;
    case 40:
        o->BlendMesh = 0;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.5f;
        o->Velocity = 0.05f;
        return true;
    case 47:
    case 48:
        o->HiddenMesh = -2;
        return true;
    }

    return false;
}

bool CGMDoppelGanger3::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (IsDoppelGanger3() == false)
        return false;

    switch (o->Type)
    {
    case MODEL_ICE_WALKER:
    {
        switch (o->CurrentAction)
        {
        case MONSTER01_ATTACK2:
            if (o->AnimationFrame > 4.4f && o->AnimationFrame < 4.7f)
            {
                const float ANG_REVISION = 20.0f;

                vec3_t v3Pos, v3Ang_, v3BasisPos;

                b->TransformByObjectBone(v3BasisPos, o, 8);

                VectorCopy(v3BasisPos, v3Pos);
                VectorCopy(o->Angle, v3Ang_);
                v3Ang_[0] = v3Ang_[0] + ANG_REVISION;
                CreateEffect(MODEL_STREAMOFICEBREATH, v3Pos, o->Angle, o->Light, 0, 0, -1, 0, 0, 0, 0.2f, -1);
            }
            break;
        case MONSTER01_DIE:
        {
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);

            float Scale = 3.5f;
            Vector(1.f, 1.f, 1.f, o->Light);
            b->TransformByObjectBone(vPos, o, 6);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->Light, 3, Scale);
            b->TransformByObjectBone(vPos, o, 79);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, o->Light, 53, Scale);
        }
        break;
        }
    }
    return true;
    case MODEL_MAD_BUTCHER:
    case MODEL_TERRIBLE_BUTCHER:
    {
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2 || o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
            o->CurrentAction = MONSTER01_WALK;
    }
    return true;
    case MODEL_DOPPELGANGER:
        if (o->CurrentAction == MONSTER01_APEAR)
        {
            if (o->AnimationFrame > 18.0f)
            {
                if (o->m_bActionStart == FALSE)
                {
                    o->m_bActionStart = TRUE;

                    vec3_t vPos, vLight;
                    int i;

                    for (i = 0; i < 6; ++i)
                    {
                        VectorCopy(o->Position, vPos);
                        vPos[0] += (float)(rand() % 140 - 70);
                        vPos[1] += (float)(rand() % 140 - 70);
                        Vector(0.2f, 1.0f, 0.3f, vLight);
                        CreateEffectFpsChecked(BITMAP_CLOUD, vPos, o->Angle, vLight, 0, NULL, -1, 0, 0, 0, 2.0f);
                    }

                    Vector(0.4f, 1.0f, 0.6f, vLight);
                    for (i = 0; i < 3; ++i)
                    {
                        vPos[0] = o->Position[0] + (rand() % 100 - 50) * 1.0f;
                        vPos[1] = o->Position[1] + (rand() % 100 - 50) * 1.0f;
                        vPos[2] = o->Position[2] + 10.f + (rand() % 20) * 10.0f;
                        CreateParticleFpsChecked(BITMAP_EXPLOTION_MONO, vPos, o->Angle, vLight, 0, (rand() % 8 + 7) * 0.1f);
                    }

                    Vector(0.0f, 0.5f, 0.0f, vLight);
                    for (i = 0; i < 15; ++i)
                    {
                        vPos[0] = o->Position[0] + (rand() % 200 - 100) * 1.0f;
                        vPos[1] = o->Position[1] + (rand() % 200 - 100) * 1.0f;
                        vPos[2] = o->Position[2] + (rand() % 10) * 10.0f;
                        CreateEffectFpsChecked(MODEL_DOPPELGANGER_SLIME_CHIP, vPos, o->Angle, vLight, 0, o, 0, 0);
                    }

                    Vector(0.2f, 0.9f, 0.3f, vLight);
                    for (i = 0; i < 30; ++i)
                    {
                        vPos[0] = o->Position[0] + (rand() % 300 - 150) * 1.0f;
                        vPos[1] = o->Position[1] + (rand() % 300 - 150) * 1.0f;
                        vPos[2] = o->Position[2] + 20.0f + (rand() % 10) * 10.0f;
                        CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 31);
                    }

                    Vector(0.8f, 1.0f, 0.8f, vLight);
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, vLight, 54, 2.8f);

                    SetAction(o, MONSTER01_ATTACK1);
                }
            }
        }
        return true;
    }

    return false;
}

void CGMDoppelGanger3::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_MAD_BUTCHER:
    case MODEL_TERRIBLE_BUTCHER:
    {
        if (!(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2))
            break;

        vec3_t  vLight;
        Vector(0.6f, 0.4f, 0.2f, vLight);

        vec3_t StartPos, StartRelative;
        vec3_t EndPos, EndRelative;

        float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
        float fSpeedPerFrame = fActionSpeed / 10.f;
        float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
        for (int i = 0; i < 10; i++)
        {
            pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

            Vector(0.f, 0.f, 0.f, StartRelative);
            Vector(0.f, 0.f, 0.f, EndRelative);
            pModel->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
            pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
            CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 0);

            fAnimationFrame += fSpeedPerFrame;
        }
    }
    break;
    }
}

bool CGMDoppelGanger3::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (IsDoppelGanger3() == false)
        return false;

    float fBlendMeshLight = 0;

    switch (o->Type)
    {
    case 19:
    case 20:
    case 31:
    case 33:
        o->m_bRenderAfterCharacter = true;
        return true;
    case 38:
    {
        o->m_bRenderAfterCharacter = true;
    }
    return true;
    case 43:
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    case MODEL_ICE_WALKER:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_ICEWALKER0);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_ICEWALKER1);
        return true;
        break;
    case MODEL_LARVA:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_SNAKE01);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return true;
    case MODEL_MAD_BUTCHER:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        fBlendMeshLight = (sinf(WorldTime * 0.003f) + 1.0f) * 0.5f * 0.8f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BUGBEAR_R);
        return true;
    case MODEL_TERRIBLE_BUTCHER:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        fBlendMeshLight = (sinf(WorldTime * 0.003f) + 1.0f) * 0.5f * 0.8f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fBlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BUGBEAR_R);
        return true;
    case MODEL_DOPPELGANGER:
        return true;
    }

    return false;
}

bool CGMDoppelGanger3::RenderObjectVisual(OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case 47:
    {
        vec3_t vLight;
        Vector(0.1f, 0.4f, 1.0f, vLight);

        switch (rand() % 3)
        {
        case 0:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK1_MONO, o->Position, o->Angle, vLight, 2, o->Scale);
            break;
        case 1:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK2_MONO, o->Position, o->Angle, vLight, 6, o->Scale);
            break;
        case 2:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, o->Position, o->Angle, vLight, 2, o->Scale);
            break;
        }
        CreateParticleFpsChecked(BITMAP_FIRE_HIK1_MONO, o->Position, o->Angle, vLight, 2, o->Scale);
    }
    return true;
    case 48:
        if (rand_fps_check(3))
        {
            vec3_t Light, vPos;
            Vector(0.6f, 0.8f, 1.0f, Light);
            VectorCopy(o->Position, vPos);
            int iScale = o->Scale * 60;
            vPos[0] += rand() % iScale - iScale / 2;
            vPos[1] += rand() % iScale - iScale / 2;
            CreateParticle(BITMAP_LIGHT, vPos, o->Angle, Light, 15, o->Scale, o);
        }
        return true;
    }
    return false;
}

bool CGMDoppelGanger3::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    vec3_t vPos, vLight;

    switch (o->Type)
    {
    case MODEL_MAD_BUTCHER:
    case MODEL_TERRIBLE_BUTCHER:
        if (rand_fps_check(4))
        {
            b->TransformByObjectBone(vPos, o, 6);
            vPos[1] += 50.0f;

            Vector(1.0f, 1.0f, 1.0f, vLight);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 61);
        }
        if (c->Dead == 0 && rand_fps_check(4))
        {
            Vector(o->Position[0] + (float)(rand() % 64 - 32),
                o->Position[1] + (float)(rand() % 64 - 32),
                o->Position[2] + (float)(rand() % 32 - 16), vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, vLight, 0);
        }

        if (o->Type == MODEL_TERRIBLE_BUTCHER)
        {
            Vector(1.0f, 0.2f, 0.1f, vLight);
            for (int j = 0; j < 50; ++j)
            {
                if (j >= 0 && j <= 1) continue;
                if (j >= 12 && j <= 20) continue;
                if (j >= 24 && j <= 32) continue;
                if (j >= 35 && j <= 50) continue;
                b->TransformByObjectBone(vPos, o, j);
                CreateSprite(BITMAP_LIGHT, vPos, 3.1f, vLight, o);
            }

            float fScale = 0.8f;
            Vector(1.0f, 1.0f, 1.0f, vLight);
            for (int i = 0; i < 30; ++i)
            {
                b->TransformByObjectBone(vPos, o, rand() % 50);
                vPos[0] += rand() % 10 - 20;
                vPos[1] += rand() % 10 - 20;
                vPos[2] += rand() % 10 - 20;

                switch (rand() % 3)
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
        return true;
    case MODEL_DOPPELGANGER:
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 0.3f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return true;
        break;
    }

    return true;
}

void CGMDoppelGanger3::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!IsDoppelGanger3())
        return;

    switch (o->Type)
    {
    case 19:
    case 20:
    case 31:
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        break;
    case 33:
        b->StreamMesh = 0;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime % 10000 * 0.0001f);
        b->StreamMesh = -1;
        break;
    case 38:
    {
        b->BodyLight[0] = min(b->BodyLight[0] * 2.0f, 1.0f);
        b->BodyLight[1] = min(b->BodyLight[1] * 2.0f, 1.0f);
        b->BodyLight[2] = min(b->BodyLight[2] * 2.0f, 1.0f);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    break;
    default:
        break;
    }
}

bool IsDoppelGanger3()
{
    if (gMapManager.WorldActive == WD_67DOPPLEGANGER3)
    {
        return true;
    }

    return false;
}

bool CGMDoppelGanger3::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (IsDoppelGanger3() == false)
        return false;

    return g_DoppelGanger1.SetCurrentActionMonster(c, o);

    // 	switch(c->MonsterIndex)
    // 	{
    // 	}

    return false;
}

bool CGMDoppelGanger3::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (IsDoppelGanger3() == false)
        return false;

    return g_DoppelGanger1.AttackEffectMonster(c, o, b);

    // 	switch(c->MonsterIndex)
    // 	{
    // 	}

    return false;
}

bool CGMDoppelGanger3::PlayMonsterSound(OBJECT* o)
{
    if (IsDoppelGanger3() == false)
        return false;

    return g_DoppelGanger1.PlayMonsterSound(o);

    return false;
}

void CGMDoppelGanger3::PlayObjectSound(OBJECT* o)
{
}