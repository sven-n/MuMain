//*****************************************************************************
// File: GM_kanturu_1st.cpp
//*****************************************************************************
#include "stdafx.h"
#include "Core/Utilities/Random.h"
#include "Render/Renderer/MuRenderer.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/AI/ZzzAI.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Effects/ZzzEffect.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "Render/Models/BoneManager.h"
#include "Audio/DSPlaySound.h"
#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "Render/Effects/EffectDraw.h"

bool M40GMArea::IsGmArea()
{
    return (gMapManager.WorldActive == WD_40AREA_FOR_GM) ? true : false;
}

using namespace M37Kanturu1st;

bool M37Kanturu1st::IsKanturu1st()
{
    return (gMapManager.WorldActive == WD_37KANTURU_1ST) ? true : false;
}

bool M37Kanturu1st::CreateKanturu1stObject(OBJECT* pObject)
{
    if (!IsKanturu1st())
        return false;

    return true;
}

bool M37Kanturu1st::MoveKanturu1stObject(OBJECT* pObject)
{
    if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
        return false;

    vec3_t Light;
    float Luminosity;

    switch (pObject->Type)
    {
    case 59:
    case 62:
    case 81:
    case 82:
    case 83:
    case 107:
    case 108:
        pObject->HiddenMesh = -2;
        break;
    case 44:
        pObject->Velocity = 0.02f;
        break;
    case 46:
        pObject->Velocity = 0.01f;
        pObject->BlendMeshLight
            = (float)sinf(WorldTime * 0.0015f) * 0.8f + 1.0f;
        break;
    case 60:
        Luminosity = (float)(Random::Rand() % 4 + 3) * 0.1f;
        Vector(Luminosity * 0.9f, Luminosity * 0.2f, Luminosity * 0.1f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3,
            PrimaryTerrainLight);
        pObject->HiddenMesh = -2;
        break;
    case 61:
        Luminosity = (float)(Random::Rand() % 4 + 3) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3,
            PrimaryTerrainLight);
        pObject->HiddenMesh = -2;
        break;
    case 70:
        pObject->Velocity = 0.04f;
        Luminosity = (float)sinf(WorldTime * 0.002f) * 0.45f + 0.55f;
        Vector(Luminosity * 1.4f, Luminosity * 0.7f, Luminosity * 0.4f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 4,
            PrimaryTerrainLight);
        break;
    case 76:
        pObject->Alpha = 0.5f;
        break;
    case 77:
        pObject->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
        break;
    case 90:
        pObject->Velocity = 0.04f;
        break;
    case 96:
        pObject->Alpha = 0.5f;
        break;
    case 97:
        pObject->HiddenMesh = -2;
        pObject->Timer += 0.1f * FPS_ANIMATION_FACTOR;
        if (pObject->Timer > 10.f)
            pObject->Timer = 0.f;
        if (pObject->Timer > 5.f)
            CreateParticleFpsChecked(BITMAP_BUBBLE, pObject->Position, pObject->Angle, pObject->Light, 5);
        break;
    case 102:
        pObject->BlendMeshLight = (float)sinf(WorldTime * 0.0010f) + 1.0f;
        break;
    }

    return true;
}

bool M37Kanturu1st::UpdateKanturu1stObjectEffects(OBJECT* pObject, BMD* pModel)
{
    if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
        return false;

    vec3_t Light, p, Position;

    switch (pObject->Type)
    {
    case 95:
    {
        vec3_t sparkP, sparkPosition, sparkLight;
        Vector(0.0f, 0.0f, 0.0f, sparkP);
        Vector(0.1f, 0.1f, 0.3f, sparkLight);
        for (int i = 0; i <= 9; ++i)
        {
            pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[i], sparkP, sparkPosition, false);
            CreateSprite(BITMAP_SPARK + 1, sparkPosition, 7.5f, sparkLight, pObject);
        }
    }
    break;
    case 37:
    {
        int time = timeGetTime() % 1024;
        if (time >= 0 && time < 10)
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_BUTTERFLY01, pObject->Position,
                pObject->Angle, Light, 3, pObject);
        }
        pObject->HiddenMesh = -2;	// Hide Object
    }
    break;
    case 59:
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle,
                Light, 21, pObject->Scale);
        }
        break;
    case 61:
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle,
                Light, 0, pObject->Scale);
        }
        break;
    case 62:
        if (pObject->HiddenMesh != -2)
        {
            Vector(0.04f, 0.04f, 0.04f, Light);
            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle,
                    Light, 20, pObject->Scale, pObject);
        }
        break;
    case 70:
        float Luminosity;
        Vector(0.0f, 0.0f, 0.0f, p);
        pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[6], p, Position, false);
        Luminosity = (float)sinf(WorldTime * 0.002f) + 1.8f;
        Vector(0.8f, 0.4f, 0.2f, Light);
        CreateSprite(
            BITMAP_SPARK + 1, Position, Luminosity * 7.0f, Light, pObject);
        Vector(0.65f, 0.65f, 0.65f, Light);
        CreateSprite(
            BITMAP_SPARK + 1, Position, Luminosity * 4.0f, Light, pObject);
        break;
    case 81:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(2))
            CreateParticle(BITMAP_WATERFALL_1, pObject->Position,
                pObject->Angle, Light, 2, pObject->Scale);
        break;
    case 82:
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_WATERFALL_3, pObject->Position,
            pObject->Angle, Light, 4, pObject->Scale);
        break;
    case 83:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(3))
            CreateParticle(BITMAP_WATERFALL_2, pObject->Position,
                pObject->Angle, Light, 2, pObject->Scale);
        break;
    case 92:
    {
        vec3_t EndRelative, StartPos, EndPos;
        Vector(0.f, 0.f, 0.f, EndRelative);
        pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[1], EndRelative, StartPos, false);
        pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[2], EndRelative, EndPos, false);
        if (Random::Rand() % 30 == 1)
            CreateJoint(BITMAP_JOINT_THUNDER, StartPos, EndPos,
                pObject->Angle, 18, NULL, 50.f);
    }
    break;
    case 98:
        if (rand_fps_check(3))
        {
            vec3_t vPos;
            Vector(0.0f, 0.0f, 0.0f, vPos);
            pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[1], vPos, Position, false);
            Vector(0.5f, 0.6f, 0.1f, Light);
            CreateParticle(
                BITMAP_TWINTAIL_WATER, Position, pObject->Angle, Light, 2);
        }
        break;
    case 105:
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.5f;
        vec3_t vPos;
        Vector(0.0f, 0.0f, 0.0f, vPos);
        Vector(fLumi, fLumi, fLumi, Light);
        pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[4], vPos, Position, false);
        CreateParticleFpsChecked(
            BITMAP_ENERGY, Position, pObject->Angle, Light, 0, 1.5f);
        CreateSprite(BITMAP_SPARK + 1, Position, 10.0f, Light, pObject);
        vec3_t StartPos, EndPos;
        VectorCopy(Position, StartPos);
        VectorCopy(Position, EndPos);
        StartPos[0] -= 50.f;
        StartPos[0] -= Random::Rand() % 100;
        EndPos[0] += Random::Rand() % 80;
        StartPos[1] -= Random::Rand() % 50;
        EndPos[1] += Random::Rand() % 50;
        StartPos[2] += 10.0f;
        EndPos[2] += 10.f;
        if (rand_fps_check(20))
            CreateJoint(BITMAP_JOINT_THUNDER, StartPos, EndPos,
                pObject->Angle, 8, NULL, 40.f);
    }
    break;
    case 107:
        if (pObject->HiddenMesh != -2)
        {
            vec3_t Light;
            Vector(0.06f, 0.06f, 0.06f, Light);
            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle,
                    Light, 2, pObject->Scale, pObject);
        }
        break;
    case 108:
        if (pObject->HiddenMesh != -2)
        {
            vec3_t Light;
            Vector(0.2f, 0.2f, 0.2f, Light);
            for (int i = 0; i < 20; ++i)
                CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle,
                    Light, 7, pObject->Scale, pObject);
        }
        break;
    case 110:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.5f) * 0.5f;
        Vector(fLumi * 0.6f, fLumi * 1.0f, fLumi * 0.8f, Light);
        vec3_t vPos;
        Vector(0.0f, 0.0f, 0.0f, vPos);
        pModel->TransformPosition(Engine::Object::Pose::World(pObject, pModel)[1], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, 1.1f, Light, pObject);
    }
    break;
    }

    return true;
}

bool M37Kanturu1st::RenderKanturu1stObjectVisual(const OBJECT* pObject, DrawLook& look, BMD* pModel)
{
    if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
        return false;

    switch (pObject->Type)
    {
    case 85:
        pModel->BeginRender(look.alpha);
        pModel->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                           look.blendMeshU, look.blendMeshV);
        pModel->RenderMesh(3, RENDER_CHROME | RENDER_DARK, look.alpha, look.blendMesh, look.blendMeshLight,
                           look.blendMeshU, look.blendMeshV, BITMAP_CHROME);
        pModel->EndRender();
        break;
    case 96:
        pModel->StreamMesh = 0;
        SetAlphaFuncRef(0.f);
        pModel->RenderMesh(0, RENDER_TEXTURE, 1.0f, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                           -(int)WorldTime % 20000 * 0.00005f);
        SetAlphaFuncRef(0.25f);
        pModel->StreamMesh = -1;
        break;
    }

    return true;
}

bool M37Kanturu1st::RenderKanturu1stObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon)
{
    if (IsKanturu1st() || M40GMArea::IsGmArea())
    {
        switch (o->Type)
        {
        case 103:
        {
            float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
            look.hiddenMesh = 1;
            b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, look.hiddenMesh);
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, 1, fLumi, look.blendMeshU, look.blendMeshV);

            return true;
        }

        case 76:
        case 77:
        case 91:
        case 92:
        case 95:
        case 105:
            SendAfterCharacter(o);
            return true;
        }
    }

    return RenderKanturu1stMonsterObjectMesh(o, look, b, ExtraMon);
}

void M37Kanturu1st::RenderKanturu1stAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b)
{
    if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
        return;

    switch (o->Type)
    {
    case 76:
        b->BodyLight[0] = 0.52f;
        b->BodyLight[1] = 0.52f;
        b->BodyLight[2] = 0.52f;

        b->StreamMesh = 0;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      -(int)WorldTime % 100000 * 0.00001f, look.blendMeshV);
        b->StreamMesh = -1;
        break;

    case 95:
    {
        b->BeginRender(look.alpha);

        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);
        b->RenderMesh(0, RENDER_CHROME7 | RENDER_DARK, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV, BITMAP_CHROME);
        b->EndRender();
    }
    break;

    case 105:
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV, BITMAP_CHROME);
        break;

    case 77:
    case 91:
    case 92:
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        break;
    }
}

CHARACTER* M37Kanturu1st::CreateKanturu1stMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case MONSTER_BERSERK:
    case MONSTER_BERSERKER:
    {
        OpenMonsterModel(MONSTER_MODEL_BERSERK);
        pCharacter = CreateCharacter(Key, MODEL_BERSERK, PosX, PosY);
        pCharacter->Object.Scale = 0.95f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"BERSERK_MOUTH", 9);
    }
    break;
    case MONSTER_GIGANTIS2:
    case MONSTER_GIGANTIS:
    {
        OpenMonsterModel(MONSTER_MODEL_GIGANTIS);
        pCharacter = CreateCharacter(Key, MODEL_GIGANTIS, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_GENOCIDER:
    {
        OpenMonsterModel(MONSTER_MODEL_GENOCIDER);
        pCharacter = CreateCharacter(Key, MODEL_GENOCIDER, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"GENO_WP", 47);
    }
    break;
    case MONSTER_SPLINTER_WOLF:
    {
        OpenMonsterModel(MONSTER_MODEL_SPLINTER_WOLF);
        pCharacter = CreateCharacter(Key, MODEL_SPLINTER_WOLF, PosX, PosY);
        pCharacter->Object.Scale = 0.8f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        BoneManager::RegisterBone(pCharacter, L"SPL_WOLF_EYE_26", 16);
        BoneManager::RegisterBone(pCharacter, L"SPL_WOLF_EYE_25", 17);

        OBJECT* o = &pCharacter->Object;
        BMD* b = &Models[o->Type];

        MoveEye(o, b, 16, 17);
        vec3_t vColor = { 1.5f, 0.01f, 0.0f };
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 24, o, 10.f, -1, 0, 0, -1, vColor);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 25, o, 10.f, -1, 0, 0, -1, vColor);
    }
    break;

    case MONSTER_IRON_RIDER:
    {
        OpenMonsterModel(MONSTER_MODEL_IRON_RIDER);
        pCharacter = CreateCharacter(Key, MODEL_IRON_RIDER, PosX, PosY);
        pCharacter->Object.Scale = 1.3f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        BoneManager::RegisterBone(pCharacter, L"IRON_RIDER_BIP01", 2);
        BoneManager::RegisterBone(pCharacter, L"IRON_RIDER_BOW_6", 42);
        BoneManager::RegisterBone(pCharacter, L"IRON_RIDER_BOW_15", 52);
        BoneManager::RegisterBone(pCharacter, L"IRON_RIDER_BOW_16", 47);
    }
    break;
    case MONSTER_BLADE_HUNTER:
    {
        OpenMonsterModel(MONSTER_MODEL_BLADE_HUNTER);
        pCharacter = CreateCharacter(Key, MODEL_BLADE_HUNTER, PosX, PosY);
        pCharacter->Object.Scale = 1.3f;
        pCharacter->Object.Gravity = 0.0f;
        pCharacter->Object.Distance = (float)(Random::Rand() % 20) / 10.0f;
        pCharacter->Object.Angle[0] = 0.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        BoneManager::RegisterBone(pCharacter, L"BLADE_L_HAND", 12);
        BoneManager::RegisterBone(pCharacter, L"BOX1", 54);
        BoneManager::RegisterBone(pCharacter, L"BOX2", 55);
    }
    break;
    case MONSTER_SATYROS:
    {
        OpenMonsterModel(MONSTER_MODEL_SATYROS);
        pCharacter = CreateCharacter(Key, MODEL_SATYROS, PosX, PosY);
        pCharacter->Object.Scale = 1.3f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_KENTAUROS:
    {
        OpenMonsterModel(MONSTER_MODEL_KENTAUROS);
        pCharacter = CreateCharacter(Key, MODEL_KENTAUROS, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_23", 27);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_24", 28);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_25", 29);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_26", 30);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_18", 34);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_19", 35);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_20", 36);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_21", 37);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_TAIL", 81);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_TAIL_1", 82);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_TAIL_2", 83);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_SPAIN_1", 4);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_SPAIN_2", 5);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_SPAIN_3", 6);
    }
    break;
    case MONSTER_BERSERKER_WARRIOR:
    {
        OpenMonsterModel(MONSTER_MODEL_BERSERKER_WARRIOR);
        pCharacter = CreateCharacter(Key, MODEL_BERSERKER_WARRIOR, PosX, PosY);
        //pCharacter->Object.Scale = 0.95f;
        pCharacter->Object.Scale = 1.15f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        BoneManager::RegisterBone(pCharacter, L"BERSERK_MOUTH", 9);
    }
    break;
    case MONSTER_KENTAUROS_WARRIOR:
    {
        OpenMonsterModel(MONSTER_MODEL_KENTAUROS_WARRIOR);
        pCharacter = CreateCharacter(Key, MODEL_KENTAUROS_WARRIOR, PosX, PosY);
        //pCharacter->Object.Scale = 1.1f;
        pCharacter->Object.Scale = 1.3f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_23", 27);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_24", 28);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_25", 29);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_26", 30);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_18", 34);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_19", 35);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_20", 36);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_21", 37);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_TAIL", 81);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_TAIL_1", 82);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_TAIL_2", 83);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_SPAIN_1", 4);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_SPAIN_2", 5);
        BoneManager::RegisterBone(pCharacter, L"KENTAUROS_BIP_SPAIN_3", 6);
    }
    break;
    case MONSTER_GIGANTIS_WARRIOR:
    {
        OpenMonsterModel(MONSTER_MODEL_GIGANTIS_WARRIOR);
        pCharacter = CreateCharacter(Key, MODEL_GIGANTIS_WARRIOR, PosX, PosY);
        //pCharacter->Object.Scale = 1.2f;
        pCharacter->Object.Scale = 1.5f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_GENOCIDER_WARRIOR:
    {
        OpenMonsterModel(MONSTER_MODEL_SOCCERBALL);
        pCharacter = CreateCharacter(Key, MODEL_SOCCERBALL, PosX, PosY);
        //pCharacter->Object.Scale = 1.2f;
        pCharacter->Object.Scale = 1.35f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"GENO_WP", 47);
    }
    break;
    }

    return pCharacter;
}

bool M37Kanturu1st::AttackEffectKanturu1stMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!IsKanturu1st())
        return false;

    switch (o->Type)
    {
    case MODEL_BERSERK:
    {
        return true;
    }
    break;
    case MODEL_GIGANTIS:
    {
        return true;
    }
    break;
    case MODEL_GENOCIDER:
    {
        return true;
    }
    break;
    case MODEL_SPLINTER_WOLF:
    {
        return true;
    }
    break;
    case MODEL_IRON_RIDER:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t vPos, vRelative;
            vec3_t vLight = { 0.8f, 1.0f, 0.8f };
            Vector(0.f, 0.f, 0.f, vRelative);
            vRelative[0] = (float)(4 - Random::Rand() % 5);
            vRelative[1] = (float)(4 - Random::Rand() % 5);
            vRelative[2] = (float)(4 - Random::Rand() % 5);
            BoneManager::GetBonePosition(o, L"IRON_RIDER_BOW_6", vRelative, vPos);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 10, 4.0f);
        }

        if (c->CheckAttackTime(10))
        {
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);
            BoneManager::GetBonePosition(o, L"IRON_RIDER_BOW_6", vRelative, vPos);
            CreateEffect(MODEL_IRON_RIDER_ARROW, vPos, o->Angle, o->Light, 0);
            c->SetLastAttackEffectTime();
        }

        return true;
    }
    break;
    case MODEL_BLADE_HUNTER:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (c->CheckAttackTime(14))
                //				if(o->AnimationFrame >= StartAction && o->AnimationFrame < (StartAction + fActionSpeed))
            {
                vec3_t vPos, vRelative, Light;
                Vector(140.f, 0.f, -30.f, vRelative);
                Vector(0.2f, 0.2f, 1.f, Light);
                BoneManager::GetBonePosition(o, L"BLADE_L_HAND", vRelative, vPos);

                CreateEffect(MODEL_BLADE_SKILL, vPos, o->Angle, Light, 0);

                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 2.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.8f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 2.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.8f);
                c->SetLastAttackEffectTime();
            }
        }

        return true;
    }
    break;
    case MODEL_KENTAUROS:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t vLight = { 0.3f, 0.5f, 0.7f };
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);
            int index = 31;
            float Width = 2.f;
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_23", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_24", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_25", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_26", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_18", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_19", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_20", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_21", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
        }
        if (c->CheckAttackTime(14))
        {
            vec3_t vPos, vRelative;

            if (o->CurrentAction == MONSTER01_ATTACK1)
            {
                Vector(30.f, -30.f, 0.f, vRelative);
                BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_25", vRelative, vPos);
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
            }
            else
            {
                Vector(60.f, -30.f, 50.f, vRelative);
                BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_23", vRelative, vPos);
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
                o->Angle[2] += 15.f;
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
                o->Angle[2] -= 30.f;
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
                o->Angle[2] += 15.f;
            }

            c->SetLastAttackEffectTime();
        }

        return true;
    }
    break;
    case MODEL_BERSERKER_WARRIOR:
    {
        return true;
    }
    break;
    case MODEL_KENTAUROS_WARRIOR:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t vLight = { 0.3f, 0.5f, 0.7f };
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);
            int index = 31;
            float Width = 2.f;
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_23", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_24", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_25", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_26", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_18", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_19", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_20", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_21", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
        }
        if (c->CheckAttackTime(14))
        {
            vec3_t vPos, vRelative;

            if (o->CurrentAction == MONSTER01_ATTACK1)
            {
                Vector(30.f, -30.f, 0.f, vRelative);
                BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_25", vRelative, vPos);
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
            }
            else
            {
                Vector(60.f, -30.f, 50.f, vRelative);
                BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_23", vRelative, vPos);
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
                o->Angle[2] += 15.f;
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
                o->Angle[2] -= 30.f;
                CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
                o->Angle[2] += 15.f;
            }

            c->SetLastAttackEffectTime();
        }

        return true;
    }
    break;
    case MODEL_GIGANTIS_WARRIOR:
    {
        return true;
    }
    break;
    case MODEL_SOCCERBALL:
    {
        return true;
    }
    break;
    }

    return false;
}

bool M37Kanturu1st::MoveKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_BLADE_HUNTER:
        // A dying Blade Hunter fades without a shadow.
        if (o->CurrentAction == MONSTER01_DIE)
            o->m_bRenderShadow = false;
        break;
    case MODEL_BERSERK:
        break;
    case MODEL_GIGANTIS:
        break;
    case MODEL_GENOCIDER:
        break;
    case MODEL_KENTAUROS:
        break;
    case MODEL_BERSERKER_WARRIOR:
    {
    }
    break;
    case MODEL_KENTAUROS_WARRIOR:
    {
    }
    break;
    case MODEL_GIGANTIS_WARRIOR:
    {
    }
    break;
    case MODEL_SOCCERBALL:
    {
    }
    break;
    }

    return false;
}

bool M37Kanturu1st::RenderKanturu1stMonsterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, int ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_BERSERK:
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);

        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV,
                      BITMAP_BERSERK_EFFECT);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, fLumi, look.blendMeshU, look.blendMeshV,
                      BITMAP_BERSERK_WP_EFFECT);

        return true;
    }
    break;
    case MODEL_GIGANTIS:
    {
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);

        if (o->CurrentAction != MONSTER01_DIE)
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);

        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);

        return true;
    }
    break;
    case MODEL_GENOCIDER:
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        return true;
    }
    break;
    case MODEL_SPLINTER_WOLF:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE);
        b->RenderMesh(1, RENDER_TEXTURE);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 2, fLumi);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 3, fLumi);
        b->RenderMesh(4, RENDER_TEXTURE);
        return true;
    }
    break;

    case MODEL_IRON_RIDER:
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        return true;
    }
    break;
    case MODEL_BLADE_HUNTER:
    {
        if (o->CurrentAction != MONSTER01_DIE)
        {
            float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
            b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, look.hiddenMesh);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV,
                          BITMAP_BLADEHUNTER_EFFECT);
        }
        else
        {
            if (look.alpha < 0.f)
                look.alpha = 0.f;

            if (look.alpha > 0.8f)
            {
                b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                              look.blendMeshV, look.hiddenMesh);
            }
            else
                b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                              look.blendMeshU, look.blendMeshV, BITMAP_CHROME);
            b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV, BITMAP_CHROME);
        }
        return true;
    }
    break;
    case MODEL_SATYROS:
    {
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        b->BeginRender(1.f);
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        b->StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 0, look.blendMeshLight, look.blendMeshU,
                      WorldTime * 0.0005f);
        b->RenderMesh(3, RENDER_CHROME | RENDER_BRIGHT, 1.f, 0, look.blendMeshLight, look.blendMeshU,
                      WorldTime * 0.001f, BITMAP_CHROME);

        float Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
        Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
        b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, look.blendMeshLight, WorldTime * 0.0001f,
                      -WorldTime * 0.0005f);
        b->EndRender();
        return true;
    }
    break;
    case MODEL_KENTAUROS:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, fLumi);
        b->RenderMesh(1, RENDER_CHROME2 | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);
        return true;
    }
    break;
    case MODEL_BERSERKER_WARRIOR:
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);

        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi, look.blendMeshU, look.blendMeshV,
                      BITMAP_BERSERK_EFFECT);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, fLumi, look.blendMeshU, look.blendMeshV,
                      BITMAP_BERSERK_WP_EFFECT);

        return true;
    }
    break;
    case MODEL_KENTAUROS_WARRIOR:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, fLumi);
        b->RenderMesh(1, RENDER_CHROME2 | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);
        return true;
    }
    break;
    case MODEL_GIGANTIS_WARRIOR:
    {
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);

        if (o->CurrentAction != MONSTER01_DIE)
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);

        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);

        return true;
    }
    break;
    case MODEL_SOCCERBALL:
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        return true;
    }
    break;
    }

    return false;
}

bool M37Kanturu1st::UpdateKanturu1stMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_BERSERK:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);

        if (o->CurrentAction == MONSTER01_DIE
            || o->CurrentAction == MONSTER01_ATTACK1
            || o->CurrentAction == MONSTER01_ATTACK2)
            return true;

        vec3_t vPos, vRelative;
        Vector(0.f, 0.f, 0.f, vRelative);
        BoneManager::GetBonePosition(o, L"BERSERK_MOUTH", vRelative, vPos);
        CreateParticle(
            BITMAP_SMOKE, vPos, o->Angle, o->Light, 42, o->Scale);

        return true;
    }
    break;

    case MODEL_GIGANTIS:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);

        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        vec3_t EndRelative, EndPos;
        Vector(19.f, -2.f, 0.f, EndRelative);

        b->TransformPosition(
            o->BoneTransform[7], EndRelative, EndPos, true);

        Vector(0.4f, 0.6f, 0.8f, o->Light);
        CreateSprite(BITMAP_LIGHT, EndPos, 3.0f, o->Light, o, 0.5f);

        float Luminosity;
        Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
        Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f,
            o->Light);
        CreateSprite(BITMAP_LIGHT, EndPos, 1.0f, o->Light, o);

        return true;
    }
    break;

    case MODEL_GENOCIDER:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);

        if (o->CurrentAction == MONSTER01_WALK && rand_fps_check(1))
        {
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);
            BoneManager::GetBonePosition(o, L"GENO_WP", vRelative, vPos);
            CreateParticle(
                BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, o->Scale);

            CreateEffect(MODEL_STONE1 + Random::Rand() % 2, vPos, o->Angle, o->Light);
        }
        return true;
    }
    break;
    case MODEL_SPLINTER_WOLF:
    {
        MoveEye(o, b, 16, 17);

        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);
        return true;
    }
    break;
    case MODEL_IRON_RIDER:
    {
        if (o->CurrentAction != MONSTER01_DIE)
        {
            vec3_t vPos, vRelative;
            vec3_t vLight = { 0.8f, 1.0f, 0.8f };
            Vector(0.f, 0.f, 0.f, vRelative);
            vRelative[0] = (float)(4 - Random::Rand() % 5);
            vRelative[1] = (float)(4 - Random::Rand() % 5);
            vRelative[2] = (float)(4 - Random::Rand() % 5);
            BoneManager::GetBonePosition(o, L"IRON_RIDER_BOW_15", vRelative, vPos);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 10, 4.0f);
            BoneManager::GetBonePosition(o, L"IRON_RIDER_BOW_16", vRelative, vPos);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 10, 4.0f);

            GameLogic::Monsters::SoundEvents::PlayDrawn(*c);
        }
        else
        {
            vec3_t vLight = { 1.0f, 1.0f, 1.0f };
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);
            BoneManager::GetBonePosition(o, L"IRON_RIDER_BIP01", vRelative, vPos);
            CreateParticleFpsChecked(BITMAP_SMOKE + 3, vPos, o->Angle, vLight, 3, 2.0f);
            CreateParticleFpsChecked(BITMAP_SMOKE + 3, vPos, o->Angle, vLight, 4, 1.0f);
            GameLogic::Monsters::SoundEvents::PlayDrawn(*c);
        }
        return true;
    }
    break;
    case MODEL_SATYROS:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);
    }
    break;
    case MODEL_BLADE_HUNTER:
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
        {
            o->Distance += 0.05f * FPS_ANIMATION_FACTOR;
            if (o->Distance >= 3.0f)
                o->Distance = 0.1f;
        }
        bool played = false;
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c, &played);
        // The walk's smoke comes with its step sound (Q-MN92).
        if (o->CurrentAction == MONSTER01_WALK && played && gMapManager.WorldActive != WD_39KANTURU_3RD)
            CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, o->Light);
    }
    break;
    case MODEL_KENTAUROS:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);
        vec3_t vPos, vRelative;
        Vector(0.f, 0.f, 0.f, vRelative);
        if (o->CurrentAction == MONSTER01_DIE && rand_fps_check(1))
        {
            float Scale = 0.3f;
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_TAIL", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_TAIL_1", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_TAIL_2", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_SPAIN_1", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_SPAIN_2", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_SPAIN_3", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);
        }
        else
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, o->Light);
                }
            }
    }
    break;
    case MODEL_BERSERKER_WARRIOR:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);

        if (o->CurrentAction == MONSTER01_DIE
            || o->CurrentAction == MONSTER01_ATTACK1
            || o->CurrentAction == MONSTER01_ATTACK2)
            return true;

        vec3_t vPos, vRelative;
        Vector(0.f, 0.f, 0.f, vRelative);
        BoneManager::GetBonePosition(o, L"BERSERK_MOUTH", vRelative, vPos);
        CreateParticleFpsChecked(
            BITMAP_SMOKE, vPos, o->Angle, o->Light, 42, o->Scale);
    }
    return true;
    case MODEL_KENTAUROS_WARRIOR:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);
        vec3_t vPos, vRelative;
        Vector(0.f, 0.f, 0.f, vRelative);
        if (o->CurrentAction == MONSTER01_DIE && rand_fps_check(1))
        {
            float Scale = 0.3f;
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_TAIL", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_TAIL_1", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_TAIL_2", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_SPAIN_1", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_SPAIN_2", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 3, vPos, o->Angle, o->Light, 3, Scale);
            BoneManager::GetBonePosition(o, L"KENTAUROS_BIP_SPAIN_3", vRelative, vPos);
            CreateParticle(BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, Scale);
        }
        else
        {
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, o->Light);
                }
            }
        }
    }
    return true;
    case MODEL_GIGANTIS_WARRIOR:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);

        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        vec3_t EndRelative, EndPos;
        Vector(19.f, -2.f, 0.f, EndRelative);

        b->TransformPosition(
            o->BoneTransform[7], EndRelative, EndPos, true);

        Vector(0.4f, 0.6f, 0.8f, o->Light);
        CreateSprite(BITMAP_LIGHT, EndPos, 3.0f, o->Light, o, 0.5f);

        float Luminosity;
        Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
        Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f,
            o->Light);
        CreateSprite(BITMAP_LIGHT, EndPos, 1.0f, o->Light, o);
    }
    return true;
    case MODEL_SOCCERBALL:
    {
        GameLogic::Monsters::SoundEvents::PlayDrawn(*c);

        if (o->CurrentAction == MONSTER01_WALK && rand_fps_check(1))
        {
            vec3_t vPos, vRelative;
            Vector(0.f, 0.f, 0.f, vRelative);
            BoneManager::GetBonePosition(o, L"GENO_WP", vRelative, vPos);
            CreateParticle(
                BITMAP_SMOKE + 1, vPos, o->Angle, o->Light, 1, o->Scale);

            CreateEffect(MODEL_STONE1 + Random::Rand() % 2, vPos, o->Angle, o->Light);
        }
    }
    return true;
    }
    return false;
}

bool M37Kanturu1st::RenderKanturu1stMonsterVisual(const CHARACTER* c, const OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_BLADE_HUNTER:
        if (gMapManager.WorldActive == WD_39KANTURU_3RD &&
            g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
        {
            vec3_t Light;
            const float Scale = o->Distance;
            const float Angle = (timeGetTime() % 9000) / 10.0f;

            Light[0] = 0.6f - (Scale / 5.0f);
            Light[1] = 0.6f - (Scale / 5.0f);
            Light[2] = 0.6f - (Scale / 5.0f);

            EnableAlphaBlend();
            Render::EffectDrawing::ActiveEffectDrawer().DrawDecal(Render::EffectDrawing::Decal(
                BITMAP_ENERGY_RING, o->Position[0], o->Position[1], Scale, Scale, Light, Angle));

            Vector(0.5f, 0.5f, 0.5f, Light);
            Render::EffectDrawing::ActiveEffectDrawer().DrawDecal(Render::EffectDrawing::Decal(
                BITMAP_ENERGY_FIELD, o->Position[0], o->Position[1], 2.0f, 2.0f, Light, Angle));
        }
        break;
    }
    return false;
}

void M37Kanturu1st::MoveKanturu1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_SATYROS:
    {
        if ((o->AnimationFrame >= 3.6f && o->AnimationFrame <= 6.0f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 3.6f && o->AnimationFrame <= 6.0f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(0.3f, 1.0f, 3.5f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, -200.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[44], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[44], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 1);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_BERSERK:
    {
        if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(0.3f, 2.0f, 0.5f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(-20.f, 0.f, 0.f, StartRelative);
                Vector(100.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[33], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 1);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_BLADE_HUNTER:
    {
        float Start_Frame = 5.9f;
        float End_Frame = 7.55f;
        if ((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 1.2f, 2.f, Light);

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

                b->TransformPosition(BoneTransform[55], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[54], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_GIGANTIS:
    {
        if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 1.2f, 2.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[34], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[33], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_GENOCIDER:
    {
        if ((o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 0.2f, 0.0f, Light);

            //				vec3_t vPos, vRelative;
            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 18; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(-40.f, 0.f, 0.f, StartRelative);
                Vector(10.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[49], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[49], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 0);

                b->TransformPosition(
                    BoneTransform[51], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[51], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 1);

                b->TransformPosition(
                    BoneTransform[50], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[50], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 2);

                b->TransformPosition(
                    BoneTransform[52], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[52], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 3);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_BERSERKER_WARRIOR:
    {
        if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(0.3f, 2.0f, 0.5f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(-20.f, 0.f, 0.f, StartRelative);
                Vector(100.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[33], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 1);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_KENTAUROS_WARRIOR:
    {
        if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 1.2f, 2.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[34], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[33], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_GIGANTIS_WARRIOR:
    {
        if ((o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
            && o->CurrentAction == MONSTER01_ATTACK1)
            || (o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
                && o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 0.2f, 0.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 18; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame,
                    o->PriorAnimationFrame, o->PriorAction, o->Angle,
                    o->HeadAngle);

                Vector(-40.f, 0.f, 0.f, StartRelative);
                Vector(10.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(
                    BoneTransform[49], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[49], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 0);

                b->TransformPosition(
                    BoneTransform[51], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[51], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 1);

                b->TransformPosition(
                    BoneTransform[50], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[50], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 2);

                b->TransformPosition(
                    BoneTransform[52], StartRelative, StartPos, false);
                b->TransformPosition(
                    BoneTransform[52], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 0, true, 3);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}
