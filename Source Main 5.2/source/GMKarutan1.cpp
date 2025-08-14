//*****************************************************************************
// File: GMKarutan1.cpp
//*****************************************************************************

#include "stdafx.h"

#ifdef ASG_ADD_MAP_KARUTAN

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
#include "NewUISystem.h"



CGMKarutan1::CGMKarutan1()
{
}

CGMKarutan1::~CGMKarutan1()
{
}

CGMKarutan1Ptr CGMKarutan1::Make()
{
    CGMKarutan1Ptr karutan(new CGMKarutan1);
    return karutan;
}

bool CGMKarutan1::CreateObject(OBJECT* o)
{
    return false;
}

bool CGMKarutan1::MoveObject(OBJECT* o)
{
    if (!IsKarutanMap())
        return false;

    switch (o->Type)
    {
    case 113:
    {
        vec3_t vLight;
        float fLuminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(fLuminosity, fLuminosity * 0.6f, fLuminosity * 0.2f, vLight);
        AddTerrainLight(o->Position[0], o->Position[1], vLight, 3, PrimaryTerrainLight);
    }
    case 114:
    case 115:
    case 116:
    case 118:
        o->HiddenMesh = -2;
        return true;
    }

    return false;
}

bool CGMKarutan1::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (!IsKarutanMap())
        return false;

    switch (o->Type)
    {
    case 66:
    {
        vec3_t vPos, vRelativePos, vLight0;
        Vector(0.f, 0.f, 0.f, vPos);
        float flumi = absf(sinf(WorldTime * 0.0004)) * 0.4f;//+0.1f;
        Vector(flumi * 1.9f, flumi * 1.1f, flumi * 1.1f, vLight0);

        Vector(10.f, 0.f, -6.f, vRelativePos);
        b->TransformPosition(BoneTransform[13], vRelativePos, vPos);
        CreateSprite(BITMAP_SHINY + 5, vPos, 1.2f, vLight0, o);
        Vector(10.f, 0.f, 6.f, vRelativePos);
        b->TransformPosition(BoneTransform[14], vRelativePos, vPos);
        CreateSprite(BITMAP_SHINY + 5, vPos, 1.2f, vLight0, o);
    }
    return true;
    case 72:
    {
        vec3_t vPos, vRelativePos, vLight0, vLight1;
        Vector(0.f, 0.f, 0.f, vPos);
        Vector(20.f, 0.f, 0.f, vRelativePos);
        float flumi = absf(sinf(WorldTime * 0.0004)) * 0.4f;//+0.1f;
        Vector(flumi * 1.3f, flumi * 1.3f, flumi * 1.9f, vLight0);
        Vector(0.15f, 0.15f, 0.15f, vLight1);

        b->TransformPosition(BoneTransform[11], vRelativePos, vPos);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight0, o);
        CreateSprite(BITMAP_SPARK + 1, vPos, 1.5f, vLight1, o);
        b->TransformPosition(BoneTransform[7], vRelativePos, vPos);
        CreateSprite(BITMAP_SPARK + 1, vPos, 4.0f, vLight0, o);
        CreateSprite(BITMAP_SPARK + 1, vPos, 1.5f, vLight1, o);
    }
    return true;
    case 113:
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
    case 114:
        if (rand_fps_check(3))
        {
            vec3_t vLight;
            Vector(0.2f, 0.2f, 0.2f, vLight);
            CreateParticle(BITMAP_WATERFALL_3, o->Position, o->Angle, vLight, 16, o->Scale);
        }
        return true;
    case 115:
        if (o->HiddenMesh != -2)
        {
            vec3_t vLight;
            Vector(0.04f, 0.04f, 0.04f, vLight);
            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, vLight, 0, o->Scale, o);
        }
        return true;
    case 116:
        if (rand_fps_check(8))
        {
            vec3_t vLight;
            Vector(0.5f, 0.9f, 0.5f, vLight);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 69, o->Scale);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 13, o->Scale * 2.0f, o);
        }
        return true;
    case 118:
        if (o->HiddenMesh != -2)
        {
            vec3_t vLight;
            Vector(0.27f, 0.2f, 0.1f, vLight);
            for (int i = 0; i < 4; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, vLight, 0, o->Scale, o);
        }
        return true;
    }

    return false;
}

bool CGMKarutan1::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!IsKarutanMap())
        return false;

    switch (o->Type)
    {
    case 1:
    case 3:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 62:
    case 63:
    case 119:
        o->m_bRenderAfterCharacter = true;
        return true;
    case 66:
        if (o->AnimationFrame >= 19)
        {
            SetAction(o, rand_fps_check(10) ? 1 : 0);
        }
        o->m_bRenderAfterCharacter = true;
        return true;

#ifdef ASG_ADD_KARUTAN_MONSTERS
    case MODEL_BONE_SCORPION:
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

        if (o->CurrentAction != MONSTER01_DIE)
        {
            float fLumi = sinf(WorldTime * 0.002f) + 0.5f;
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BONE_SCORPION_SKIN_EFFECT);
        }
        return true;
    case MODEL_CRYPTA:
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

        if (o->CurrentAction != MONSTER01_DIE)
        {
            float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_KRYPTA_BALL_EFFECT);
        }
        return true;
    case MODEL_CRYPOS:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return true;
    case MODEL_CONDRA:
        if (o->CurrentAction != MONSTER01_DIE)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CONDRA_SKIN_EFFECT);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CONDRA_SKIN_EFFECT2);

            b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 3, o->BlendMeshLight * 0.9f,
                WorldTime * 0.0010f, o->BlendMeshTexCoordV);
            Vector(0.4f, 0.95f, 1.0f, b->BodyLight);
            b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, o->BlendMeshLight * 0.3f,
                WorldTime * 0.0015f, o->BlendMeshTexCoordV);
        }
        return true;
    case MODEL_NACONDRA:
        if (o->CurrentAction != MONSTER01_DIE)
        {
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
            b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_NARCONDRA_SKIN_EFFECT);
            b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 3, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_NARCONDRA_SKIN_EFFECT2);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fLumi,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_NARCONDRA_SKIN_EFFECT3);

            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight,
                o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            Vector(1.0f, 0.1f, 1.0f, b->BodyLight);
            b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 4, o->BlendMeshLight * 0.9f,
                WorldTime * 0.0010f, o->BlendMeshTexCoordV);
            Vector(0.7f, 0.4f, 1.0f, b->BodyLight);
            b->RenderMesh(5, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 5, o->BlendMeshLight * 0.7f,
                WorldTime * 0.0015f, o->BlendMeshTexCoordV);
        }
        return true;
#endif	// ASG_ADD_KARUTAN_MONSTERS
    }

    return false;
}

void CGMKarutan1::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!IsKarutanMap())
        return;

    switch (o->Type)
    {
    case 1:
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        break;
    case 3:
    case 56:
    case 62:
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        break;
    case 54:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 0.5f) * 0.5f + 1.0f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi);
    }
    break;
    case 55:
    case 57:
        b->StreamMesh = 0;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime % 10000 * 0.0001f);
        b->StreamMesh = -1;
        break;
    case 58:
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.f) * 0.5f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fLumi);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
    }
    break;
    case 63:
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.f) * 0.5f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, fLumi);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
    }
    break;
    case 66:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        break;
    case 119:
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.f) * 0.5f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fLumi);
    }
    break;
    }
}

void CGMKarutan1::PlayObjectSound(OBJECT* o)
{
    switch (o->Type)
    {
    case 58:
    case 66:
        PlayBuffer(SOUND_KARUTAN_INSECT_ENV, o, false);
        break;
    }
}

#ifdef ASG_ADD_KARUTAN_MONSTERS
CHARACTER* CGMKarutan1::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case MONSTER_VENOMOUS_CHAIN_SCORPION:
        OpenMonsterModel(MONSTER_MODEL_VENOMOUS_CHAIN_SCORPION);
        pCharacter = CreateCharacter(Key, MODEL_VENOMOUS_CHAIN_SCORPION, PosX, PosY);
        pCharacter->Object.Scale = 1.0f;
        break;
    case MONSTER_BONE_SCORPION:
        OpenMonsterModel(MONSTER_MODEL_BONE_SCORPION);
        pCharacter = CreateCharacter(Key, MODEL_BONE_SCORPION, PosX, PosY);
        pCharacter->Object.Scale = 0.58f;
        break;
    case MONSTER_ORCUS:
        OpenMonsterModel(MONSTER_MODEL_ORCUS);
        pCharacter = CreateCharacter(Key, MODEL_ORCUS, PosX, PosY);
        pCharacter->Object.Scale = 0.64f;
        break;
    case MONSTER_GOLLOCK:
        OpenMonsterModel(MONSTER_MODEL_GOLLOCK);
        pCharacter = CreateCharacter(Key, MODEL_GOLLOCK, PosX, PosY);
        pCharacter->Object.Scale = 1.5f;
        break;
    case MONSTER_CRYPTA:
        OpenMonsterModel(MONSTER_MODEL_CRYPTA);
        pCharacter = CreateCharacter(Key, MODEL_CRYPTA, PosX, PosY);
        pCharacter->Object.Scale = 1.5f;
        break;
    case MONSTER_CRYPOS:
        OpenMonsterModel(MONSTER_MODEL_CRYPOS);
        pCharacter = CreateCharacter(Key, MODEL_CRYPOS, PosX, PosY);
        pCharacter->Object.Scale = 1.25f;
        break;
    case MONSTER_CONDRA:
        OpenMonsterModel(MONSTER_MODEL_CONDRA);
        pCharacter = CreateCharacter(Key, MODEL_CONDRA, PosX, PosY);
        pCharacter->Object.Scale = 1.45f;
        pCharacter->Object.LifeTime = 100;
        break;
    case MONSTER_NARCONDRA:
        OpenMonsterModel(MONSTER_MODEL_NACONDRA);
        pCharacter = CreateCharacter(Key, MODEL_NACONDRA, PosX, PosY);
        pCharacter->Object.Scale = 1.55f;
        pCharacter->Object.LifeTime = 100;

        OBJECT* o = &pCharacter->Object;

        vec3_t vColor = { 1.5f, 0.1f, 0.5f };
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 24, o, 10.f, -1, 0, 0, -1, vColor);
        break;
    }

    return pCharacter;
}

bool CGMKarutan1::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (!IsKarutanMap())
        return false;

    vec3_t p, Position;
    vec3_t Light;
    float Luminosity = (float)(rand() % 8 + 2) * 0.1f;;

    switch (o->Type)
    {
    case MODEL_VENOMOUS_CHAIN_SCORPION:
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(o->BoneTransform[15], p, Position, true);
        Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.2f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        return true;
    case MODEL_BONE_SCORPION:
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(o->BoneTransform[8], p, Position, true);
        Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.2f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);
        return true;
    case MODEL_GOLLOCK:
        if (o->CurrentAction == MONSTER01_WALK)
        {
            vec3_t Position;
            Vector(o->Position[0] + rand() % 200 - 100, o->Position[1] + rand() % 200 - 100, o->Position[2], Position);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, Position, o->Angle, o->Light);
        }
        return true;
    case MODEL_CONDRA:
        if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->AnimationFrame < 1.0f)
            {
                vec3_t vPos, vLight;
                Vector(1.0f, 1.0f, 1.0f, vLight);
                b->TransformByObjectBone(vPos, o, 6);
                for (int j = 0; j < 6; ++j)
                    CreateEffect(MODEL_CONDRA_STONE + rand() % 6, vPos, o->Angle, vLight, 0);
            }
            if ((int)o->LifeTime == 100)
            {
                o->LifeTime = 90;
                o->m_bRenderShadow = false;

                vec3_t vRelativePos, vWorldPos, Light;
                Vector(1.0f, 1.0f, 1.0f, Light);
                Vector(0.f, 0.f, 0.f, vRelativePos);

                b->TransformPosition(o->BoneTransform[12], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_ARM_L, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[13], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_ARM_L2, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[33], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_SHOULDER, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[33], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_ARM_R, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[34], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_ARM_R2, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[11], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_CONE_L, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[32], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_CONE_R, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[0], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_PELVIS, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[4], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_STOMACH, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[6], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_CONDRA_NECK, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[5], vRelativePos, vWorldPos, true);
                VectorCopy(vWorldPos, vRelativePos);

                int i;
                Vector(0.5f, 0.5f, 0.5f, Light);
                for (i = 0; i < 20; ++i)
                {
                    vWorldPos[0] = vRelativePos[0] + rand() % 160 - 80;
                    vWorldPos[1] = vRelativePos[1] + rand() % 160 - 80;
                    vWorldPos[2] = vRelativePos[2] + (rand() % 150) - 50;
                    CreateParticle(BITMAP_SMOKE, vWorldPos, o->Angle, Light, 48, 1.0f);
                }
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->AnimationFrame >= 12.5f && o->AnimationFrame < 13.0f && rand_fps_check(1))
            {
                vec3_t Light;
                vec3_t EndPos, EndRelative;

                Vector(0.0f, 0.0f, 0.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[14], EndRelative, EndPos, true);
                CreateEffect(BITMAP_CRATER, EndPos, o->Angle, o->Light, 2);
                Vector(1.f, 0.6f, 0.2f, Light);
                CreateParticle(BITMAP_FIRE, EndPos, o->Angle, Light, 2, 2.f);
                CreateParticle(BITMAP_ADV_SMOKE + 1, EndPos, o->Angle, o->Light);
                CreateParticle(BITMAP_ADV_SMOKE + 1, EndPos, o->Angle, o->Light, 1, 1.8f);
                for (int i = 0; i < 10; ++i)
                    CreateEffect(MODEL_STONE2, EndPos, o->Angle, o->Light);
            }
        }
        return true;
    case MODEL_NACONDRA:
        if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->AnimationFrame < 1.0f)
            {
                vec3_t vPos, vLight;
                Vector(1.0f, 1.0f, 1.0f, vLight);
                b->TransformByObjectBone(vPos, o, 6);
                for (int j = 0; j < 4; ++j)
                    CreateEffect(MODEL_NARCONDRA_STONE + rand() % 4, vPos, o->Angle, vLight, 0);
            }
            if ((int)o->LifeTime == 100)
            {
                o->LifeTime = 90;
                o->m_bRenderShadow = false;

                vec3_t vRelativePos, vWorldPos, Light;
                Vector(1.0f, 1.0f, 1.0f, Light);
                Vector(0.f, 0.f, 0.f, vRelativePos);

                b->TransformPosition(o->BoneTransform[12], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_ARM_L, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[12], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_ARM_L2, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[11], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_SHOULDER_L, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[34], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_SHOULDER_R, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[35], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_ARM_R, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[35], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_ARM_R2, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[36], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_ARM_R3, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[82], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_CONE_1, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[80], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_CONE_2, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[78], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_CONE_3, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[76], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_CONE_4, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[74], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_CONE_5, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[72], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_CONE_6, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[0], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_PELVIS, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[4], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_STOMACH, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[6], vRelativePos, vWorldPos, true);
                CreateEffect(MODEL_NARCONDRA_NECK, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                b->TransformPosition(o->BoneTransform[5], vRelativePos, vWorldPos, true);
                VectorCopy(vWorldPos, vRelativePos);

                int i;
                Vector(0.5f, 0.5f, 0.5f, Light);
                for (i = 0; i < 20; ++i)
                {
                    vWorldPos[0] = vRelativePos[0] + rand() % 160 - 80;
                    vWorldPos[1] = vRelativePos[1] + rand() % 160 - 80;
                    vWorldPos[2] = vRelativePos[2] + (rand() % 150) - 50;
                    CreateParticle(BITMAP_SMOKE, vWorldPos, o->Angle, Light, 48, 1.0f);
                }
            }
        }
        else
        {
            MoveEye(o, b, 9, 9);
            Vector(0.f, 0.f, 0.f, p);
            Vector(Luminosity * 1.0f, Luminosity * 0.4f, Luminosity * 1.0f, Light);
            b->TransformPosition(o->BoneTransform[9], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 0.7f, Light, o);
            b->TransformPosition(o->BoneTransform[33], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 1.0f, Light, o);
            b->TransformPosition(o->BoneTransform[34], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 1.0f, Light, o);
            b->TransformPosition(o->BoneTransform[58], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 1.0f, Light, o);
            b->TransformPosition(o->BoneTransform[59], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 1.0f, Light, o);
            b->TransformPosition(o->BoneTransform[88], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 0.7f, Light, o);
            b->TransformPosition(o->BoneTransform[89], p, Position, true);
            CreateSprite(BITMAP_SHINY + 5, Position, 0.7f, Light, o);
        }
        return true;
    }

    return false;
}

void CGMKarutan1::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_ORCUS:
    {
        float Start_Frame = 0.f;
        float End_Frame = 9.0f;
        if ((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame
            && pObject->CurrentAction == MONSTER01_ATTACK1)
            || (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame
                && pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 1.2f, 1.2f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; ++i)
            {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                pModel->TransformPosition(BoneTransform[56], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[57], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_CRYPOS:
    {
        float Start_Frame = 0.f;//3.5f;
        float End_Frame = 6.0f;//6.7f;
        if ((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame
            && pObject->CurrentAction == MONSTER01_ATTACK1))
        {
            vec3_t  Light;
            Vector(1.f, 0.05f, 0.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            int i;
            for (i = 0; i < 10; ++i)
            {
                pModel->Animation(BoneTransform, fAnimationFrame,
                    pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle,
                    pObject->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                Vector(2.f, 2.f, 2.f, pModel->BodyLight);

                pModel->TransformPosition(
                    BoneTransform[156], StartRelative, StartPos, false);
                pModel->TransformPosition(
                    BoneTransform[153], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 1, false, 0);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 1, false, 2);

                pModel->TransformPosition(
                    BoneTransform[149], StartRelative, StartPos, false);
                pModel->TransformPosition(
                    BoneTransform[146], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 1, false, 1);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 1, false, 3);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}

bool CGMKarutan1::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    return false;
}

bool CGMKarutan1::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    return false;
}

bool CGMKarutan1::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!IsKarutanMap())
        return false;

    switch (o->Type)
    {
    case MODEL_CRYPOS:
        if (o->CurrentAction == MONSTER01_ATTACK2
            && (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 4.5f)
            && rand_fps_check(1))
        {
            CHARACTER* tc = &CharactersClient[c->TargetCharacter];
            OBJECT* to = &tc->Object;
            vec3_t vLight;
            int i;

            Vector(0.4f, 0.9f, 0.6f, vLight);
            for (i = 0; i < 5; ++i)
                CreateParticle(BITMAP_SMOKE, to->Position, to->Angle, vLight, 1);

            Vector(0.4f, 1.0f, 0.6f, vLight);
            for (i = 0; i < 2; ++i)
                CreateParticle(BITMAP_TWINTAIL_WATER, to->Position, to->Angle, vLight, 0);
        }
        return true;
    }
    return false;
}

bool CGMKarutan1::PlayMonsterSound(OBJECT* o)
{
    if (!IsKarutanMap())
        return false;

    float fDis_x, fDis_y;
    fDis_x = o->Position[0] - Hero->Object.Position[0];
    fDis_y = o->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 600.0f)
        return true;

    switch (o->Type)
    {
    case MODEL_VENOMOUS_CHAIN_SCORPION:
        if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_TCSCORPION_ATTACK);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_TCSCORPION_DEATH);
        else if (MONSTER01_SHOCK == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_TCSCORPION_HIT);
        return true;

    case MODEL_BONE_SCORPION:
        if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_BONESCORPION_ATTACK);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_BONESCORPION_DEATH);
        else if (MONSTER01_SHOCK == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_BONESCORPION_HIT);
        return true;

    case MODEL_ORCUS:
        if (MONSTER01_WALK == o->CurrentAction)
        {
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
                PlayBuffer(SOUND_KARUTAN_ORCUS_MOVE1);
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                PlayBuffer(SOUND_KARUTAN_ORCUS_MOVE2);
        }
        else if (MONSTER01_ATTACK1 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_ORCUS_ATTACK1);
        else if (MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_ORCUS_ATTACK2);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_ORCUS_DEATH);
        return true;

    case MODEL_GOLLOCK:
        if (MONSTER01_WALK == o->CurrentAction)
        {
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
                PlayBuffer(SOUND_KARUTAN_GOLOCH_MOVE1);
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                PlayBuffer(SOUND_KARUTAN_GOLOCH_MOVE2);
        }
        else if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_GOLOCH_ATTACK);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_GOLOCH_DEATH);
        return true;

    case MODEL_CRYPTA:
        if (MONSTER01_WALK == o->CurrentAction)
        {
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
                PlayBuffer(SOUND_KARUTAN_CRYPTA_MOVE1);
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                PlayBuffer(SOUND_KARUTAN_CRYPTA_MOVE2);
        }
        else if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CRYPTA_ATTACK);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CRYPTA_DEATH);
        return true;

    case MODEL_CRYPOS:
        if (MONSTER01_WALK == o->CurrentAction)
        {
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
                PlayBuffer(SOUND_KARUTAN_CRYPOS_MOVE1);
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                PlayBuffer(SOUND_KARUTAN_CRYPOS_MOVE2);
        }
        else if (MONSTER01_ATTACK1 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CRYPOS_ATTACK1);
        else if (MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CRYPOS_ATTACK2);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CRYPTA_DEATH);
        return true;

    case MODEL_CONDRA:
        if (MONSTER01_WALK == o->CurrentAction)
        {
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
                PlayBuffer(SOUND_KARUTAN_CONDRA_MOVE1);
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                PlayBuffer(SOUND_KARUTAN_CONDRA_MOVE2);
        }
        else if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CONDRA_ATTACK);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CONDRA_DEATH);
        return true;

    case MODEL_NACONDRA:
        if (MONSTER01_WALK == o->CurrentAction)
        {
            if (1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f)
                PlayBuffer(SOUND_KARUTAN_CONDRA_MOVE1);
            if (7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f)
                PlayBuffer(SOUND_KARUTAN_CONDRA_MOVE2);
        }
        else if (MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_NARCONDRA_ATTACK);
        else if (MONSTER01_DIE == o->CurrentAction)
            PlayBuffer(SOUND_KARUTAN_CONDRA_DEATH);
        return true;
    }

    return false;
}
#endif	// ASG_ADD_KARUTAN_MONSTERS

void CGMKarutan1::PlayBGM()
{
    if (gMapManager.WorldActive == WD_80KARUTAN1)
        PlayMp3(MUSIC_KARUTAN1);
    else
        StopMp3(MUSIC_KARUTAN1);

    if (gMapManager.WorldActive == WD_81KARUTAN2)
        PlayMp3(MUSIC_KARUTAN2);
    else
        StopMp3(MUSIC_KARUTAN2);
}

bool IsKarutanMap()
{
    return gMapManager.WorldActive == WD_80KARUTAN1 || gMapManager.WorldActive == WD_81KARUTAN2;
}

#endif	// ASG_ADD_MAP_KARUTAN