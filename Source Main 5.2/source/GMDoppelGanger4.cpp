// GMDoppelGanger4.cpp: implementation of the CGMDoppelGanger4 class.
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
#include "MapManager.h"



CGMDoppelGanger4Ptr CGMDoppelGanger4::Make()
{
    CGMDoppelGanger4Ptr doppelganger(new CGMDoppelGanger4);
    doppelganger->Init();
    return doppelganger;
}

CGMDoppelGanger4::CGMDoppelGanger4()
{
}

CGMDoppelGanger4::~CGMDoppelGanger4()
{
    Destroy();
}

void CGMDoppelGanger4::Init()
{
}

void CGMDoppelGanger4::Destroy()
{
}

bool CGMDoppelGanger4::CreateObject(OBJECT* o)
{
    // 	switch(o->Type)
    // 	{
    // 	}

    return false;
}

CHARACTER* CGMDoppelGanger4::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    // 	switch (iType)
    // 	{
    // 	}

    return pCharacter;
}

bool CGMDoppelGanger4::MoveObject(OBJECT* o)
{
    if (IsDoppelGanger4() == false)
        return false;

    vec3_t Light;
    float Luminosity;

    switch (o->Type)
    {
    case 47:
    case 48:
    case 59:
    case 62:
    case 81:
    case 82:
    case 83:
    case 107:
    case 108:
        o->HiddenMesh = -2;
        return true;
    case 44:
        o->Velocity = 0.02f;
        return true;
    case 46:
        o->Velocity = 0.01f;
        o->BlendMeshLight = (float)sinf(WorldTime * 0.0015f) * 0.8f + 1.0f;
        return true;
    case 60:
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity * 0.9f, Luminosity * 0.2f, Luminosity * 0.1f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        o->HiddenMesh = -2;
        return true;
    case 61:
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        o->HiddenMesh = -2;
        return true;
    case 70:
        o->Velocity = 0.04f;
        Luminosity = (float)sinf(WorldTime * 0.002f) * 0.45f + 0.55f;
        Vector(Luminosity * 1.4f, Luminosity * 0.7f, Luminosity * 0.4f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight);
        return true;
    case 76:
        o->Alpha = 0.5f;
        return true;
    case 77:
        o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
        return true;
    case 90:
        o->Velocity = 0.04f;
        return true;
    case 96:
        o->Alpha = 0.5f;
        return true;
    case 97:
        o->HiddenMesh = -2;
        o->Timer += 0.1f * FPS_ANIMATION_FACTOR;
        if (o->Timer > 10.f)
            o->Timer = 0.f;
        if (o->Timer > 5.f)
            CreateParticleFpsChecked(BITMAP_BUBBLE, o->Position, o->Angle, o->Light, 5);
        return true;
    case 102:
        o->BlendMeshLight = (float)sinf(WorldTime * 0.0010f) + 1.0f;
        return true;
    }

    return false;
}

bool CGMDoppelGanger4::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (IsDoppelGanger4() == false)
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

void CGMDoppelGanger4::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
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

bool CGMDoppelGanger4::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (IsDoppelGanger4() == false)
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
    case 103:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        o->HiddenMesh = 1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    case 76:
    case 77:
    case 91:
    case 92:
    case 95:
    case 105:
        o->m_bRenderAfterCharacter = true;
        return true;
    case 142:
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return true;
    case MODEL_ICE_WALKER:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_ICEWALKER0);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_ICEWALKER1);
        return true;
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

bool CGMDoppelGanger4::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (IsDoppelGanger4() == false)
        return false;

    vec3_t Light, p, Position;
    float Luminosity;

    switch (o->Type)
    {
    case 37:
    {
        int time = timeGetTime() % 1024;
        if (time >= 0 && time < 10) {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_BUTTERFLY01, o->Position, o->Angle, Light, 3, o);
        }
        o->HiddenMesh = -2;
    }
    return true;
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
    case 59:
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle,
                Light, 13, o->Scale);
        }
        return true;
    case 61:
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, o->Position, o->Angle,
                Light, 0, o->Scale);
        }
        return true;
    case 62:
        if (o->HiddenMesh != -2)
        {
            vec3_t Light;
            Vector(0.04f, 0.04f, 0.04f, Light);

            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 0, o->Scale, o);
        }
        return true;
    case 70:
        Vector(0.0f, 0.0f, 0.0f, p);
        b->TransformPosition(BoneTransform[6], p, Position, false);
        Luminosity = (float)sinf(WorldTime * 0.002f) + 1.8f;
        Vector(0.8f, 0.4f, 0.2f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, Luminosity * 7.0f, Light, o);
        Vector(0.65f, 0.65f, 0.65f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, Luminosity * 4.0f, Light, o);
        return true;
    case 81:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(2))
            CreateParticle(BITMAP_WATERFALL_1, o->Position, o->Angle, Light, 2, o->Scale);
        return true;
    case 82:
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_WATERFALL_3, o->Position, o->Angle, Light, 3, o->Scale);
        return true;
    case 83:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(3))
            CreateParticle(BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 1, o->Scale);
        return true;
    case 85:
        b->BeginRender(o->Alpha);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_CHROME | RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        b->EndRender();
        return true;
    case 92:
    {
        vec3_t EndRelative, StartPos, EndPos;
        Vector(0.f, 0.f, 0.f, EndRelative);
        b->TransformPosition(BoneTransform[1], EndRelative, StartPos, false);
        b->TransformPosition(BoneTransform[2], EndRelative, EndPos, false);
        if (rand() % 30 == 1)
            CreateJointFpsChecked(BITMAP_JOINT_THUNDER, StartPos, EndPos, o->Angle, 8, NULL, 50.f);
    }
    return true;
    case 96:
        b->StreamMesh = 0;
        glAlphaFunc(GL_GREATER, 0.0f);
        b->RenderMesh(
            0, RENDER_TEXTURE, 1.0f, o->BlendMesh,
            o->BlendMeshLight, o->BlendMeshTexCoordU,
            -(int)WorldTime % 20000 * 0.00005f);
        glAlphaFunc(GL_GREATER, 0.25f);
        b->StreamMesh = -1;
        return true;
    case 98:
    {
        if (rand_fps_check(3))
        {
            vec3_t vPos;
            Vector(0.0f, 0.0f, 0.0f, vPos);
            b->TransformPosition(BoneTransform[1], vPos, Position, false);
            Vector(0.5f, 0.6f, 0.1f, Light);
            CreateParticle(BITMAP_TWINTAIL_WATER, Position, o->Angle, Light, 2);
        }
    }
    return true;
    case 105:
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.5f;
        vec3_t vPos;
        Vector(0.0f, 0.0f, 0.0f, vPos);
        Vector(fLumi, fLumi, fLumi, Light);
        b->TransformPosition(BoneTransform[4], vPos, Position, false);
        CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light, 0, 1.5f);
        CreateSprite(BITMAP_SPARK + 1, Position, 10.0f, Light, o);
        vec3_t StartPos, EndPos;
        VectorCopy(Position, StartPos);
        VectorCopy(Position, EndPos);
        StartPos[0] -= 50.f;
        StartPos[0] -= rand() % 100;
        EndPos[0] += rand() % 80;
        StartPos[1] -= rand() % 50;
        EndPos[1] += rand() % 50;
        StartPos[2] += 10.0f;
        EndPos[2] += 10.f;
        if (rand_fps_check(20))
        {
            CreateJoint(BITMAP_JOINT_THUNDER, StartPos, EndPos, o->Angle, 8, NULL, 40.f);
        }
    }
    return true;
    case 107:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.06f, 0.06f, 0.06f, Light);
            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 2, o->Scale, o);
        }
        return true;
    case 108:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.2f, 0.2f, 0.2f, Light);
            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 7, o->Scale, o);
        }
        return true;
    case 110:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.5f) * 0.5f;
        Vector(fLumi * 0.6f, fLumi * 1.0f, fLumi * 0.8f, Light);
        vec3_t vPos;
        Vector(0.0f, 0.0f, 0.0f, vPos);
        b->TransformPosition(BoneTransform[1], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, 1.1f, Light, o);
    }
    return true;
    }

    return false;
}

bool CGMDoppelGanger4::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
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

void CGMDoppelGanger4::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!IsDoppelGanger4())
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
    case 76:
    {
        b->BodyLight[0] = 0.52f;
        b->BodyLight[1] = 0.52f;
        b->BodyLight[2] = 0.52f;
        b->StreamMesh = 0;
        b->RenderMesh(
            0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh,
            o->BlendMeshLight, -(int)WorldTime % 100000 * 0.00001f,
            o->BlendMeshTexCoordV);
        b->StreamMesh = -1;
    }
    break;
    case 95:
    {
        b->BeginRender(o->Alpha);

        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME | RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        b->EndRender();

        vec3_t p, Position, Light;
        for (int i = 0; i < 10; ++i)
        {
            Vector(0.0f, 0.0f, 0.0f, p);
            b->TransformPosition(BoneTransform[i], p, Position, false);
            Vector(0.1f, 0.1f, 0.3f, Light);
            CreateSprite(BITMAP_SPARK + 1, Position, 7.5f, Light, o);
        }
    }
    break;
    case 105:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
    }
    break;
    case 77:
    case 91:
    case 92:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
            o->BlendMeshLight, o->BlendMeshTexCoordU,
            o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    break;
    }
}

bool IsDoppelGanger4()
{
    if (gMapManager.WorldActive == WD_68DOPPLEGANGER4)
    {
        return true;
    }

    return false;
}

bool CGMDoppelGanger4::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (IsDoppelGanger4() == false)
        return false;

    return g_DoppelGanger1.SetCurrentActionMonster(c, o);

    // 	switch(c->MonsterIndex)
    // 	{
    // 	}

    return false;
}

bool CGMDoppelGanger4::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (IsDoppelGanger4() == false)
        return false;

    return g_DoppelGanger1.AttackEffectMonster(c, o, b);

    // 	switch(c->MonsterIndex)
    // 	{
    // 	}

    return false;
}

// 몬스터 사운드
bool CGMDoppelGanger4::PlayMonsterSound(OBJECT* o)
{
    if (IsDoppelGanger4() == false)
        return false;

    return g_DoppelGanger1.PlayMonsterSound(o);

    // 	float fDis_x, fDis_y;
    // 	fDis_x = o->Position[0] - Hero->Object.Position[0];
    // 	fDis_y = o->Position[1] - Hero->Object.Position[1];
    // 	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
    //
    // 	if (fDistance > 500.0f)
    // 		return true;
    //
    // 	switch(o->Type)
    // 	{
    // 	}

    return false;
}

void CGMDoppelGanger4::PlayObjectSound(OBJECT* o)
{
}