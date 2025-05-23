#include "stdafx.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"
#include "ZzzCharacter.h"
#include "DSPlaySound.h"
#include "MapManager.h"
#include "BoneManager.h"

DWORD g_MusicStartStamp2 = 0;

bool M33Aida::IsInAida()
{
    return (gMapManager.WorldActive == WD_33AIDA || gMapManager.WorldActive == WD_54CHARACTERSCENE) ? true : false;
}

bool M33Aida::IsInAidaSection2(const vec3_t Position)
{
    if (Position[0] > 5449.f && Position[0] < 17822.f && Position[1] > 6784.f && Position[1] < 22419.f)
        return true;
    return false;
}

bool M33Aida::CreateAidaObject(OBJECT* pObject)
{
    if (!IsInAida())
        return false;

    return true;
}

bool M33Aida::MoveAidaObject(OBJECT* pObject)
{
    if (!IsInAida())
        return false;

    float Luminosity;
    vec3_t Light;

    switch (pObject->Type)
    {
    case 25:
    {
        pObject->BlendMeshTexCoordV -= 0.015f;
    }
    break;
    case 28:
    {
        pObject->BlendMeshTexCoordV -= 0.015f;
    }
    break;
    case 30:
    {
        Luminosity = (float)(rand() % 5) * 0.01f;
        Vector(Luminosity + 0.4f, Luminosity + 0.6f, Luminosity + 0.4f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 2, PrimaryTerrainLight);
    }
    break;
    case 71:
    {
        Luminosity = (float)(rand() % 5) * 0.01f;
        Vector(Luminosity + 0.9f, Luminosity + 0.2f, Luminosity + 0.2f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 2, PrimaryTerrainLight);
    }
    break;
    case 41:
    {
        pObject->Alpha = 0.5f;
    }
    break;
    case    56:
    case    57:
    case    58:
    case	59:
    case	62:
    case	63:
    case	67:
    case	70:
        pObject->HiddenMesh = -2;
        break;
    case	64:
        pObject->Velocity = 0.05f;
        break;
    }

    PlayBuffer(SOUND_AIDA_AMBIENT);

    return true;
}

bool M33Aida::RenderAidaObjectVisual(OBJECT* pObject, BMD* pModel)
{
    if (!IsInAida())
        return false;

    vec3_t p, Position, Light;

    switch (pObject->Type)
    {
    case 30:  // 풀
    {
        Vector(0.0f, -3.0f, 1.0f, p);
        pModel->TransformPosition(BoneTransform[6], p, Position, false);
        Vector(0.1f, 0.1f, 0.3f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 5.5f, Light, pObject);
        Vector(0.15f, 0.15f, 0.15f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(0.0f, -3.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[7], p, Position, false);
        Vector(0.1f, 0.1f, 0.3f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 4.5f, Light, pObject);
        Vector(0.15f, 0.15f, 0.15f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(0.0f, -3.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[8], p, Position, false);
        Vector(0.1f, 0.1f, 0.3f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 4.0f, Light, pObject);
        Vector(0.15f, 0.15f, 0.15f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(-3.0f, -1.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[12], p, Position, false);
        Vector(0.1f, 0.1f, 0.3f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 4.5f, Light, pObject);
        Vector(0.15f, 0.15f, 0.15f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(-3.0f, -3.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[13], p, Position, false);
        Vector(0.1f, 0.1f, 0.3f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 4.0f, Light, pObject);
        Vector(0.15f, 0.15f, 0.15f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(0.0f, 2.0f, -8.0f, p);
        pModel->TransformPosition(BoneTransform[17], p, Position, false);
        Vector(0.1f, 0.1f, 0.3f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 4.0f, Light, pObject);
        Vector(0.15f, 0.15f, 0.15f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);
    }
    break;
    case 41:
    {
        pModel->BeginRender(1.0f);

        pModel->BodyLight[0] = 0.52f;
        pModel->BodyLight[1] = 0.52f;
        pModel->BodyLight[2] = 0.52f;

        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        pModel->EndRender();
    }
    break;
    case 56:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(2))
        {
            CreateParticle(BITMAP_WATERFALL_1, pObject->Position, pObject->Angle, Light, 2, pObject->Scale);
        }
        break;
    case 57:
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_WATERFALL_3, pObject->Position, pObject->Angle, Light, 4, pObject->Scale);
        break;
    case 58:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(3))
        {
            CreateParticle(BITMAP_WATERFALL_2, pObject->Position, pObject->Angle, Light, 2, pObject->Scale);
        }
        break;
    case 59:
        if (pObject->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.01f, 0.03f, 0.05f, Light);
            for (int i = 0; i < 20; ++i)
            {
                CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject);
            }
        }
        break;
    case 60:
    {
        if (rand_fps_check(25)) {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_BUTTERFLY01, pObject->Position, pObject->Angle, Light, 3, pObject);
        }
        pObject->HiddenMesh = -2;	//. Hide Object
    }
    break;
    case 62:
        if (pObject->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.05f, Light);
            for (int i = 0; i < 20; ++i)
            {
                CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject);
            }
        }
        break;
    case 63:
        if (pObject->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.02f, 0.02f, Light);
            for (int i = 0; i < 20; ++i)
            {
                CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject);
            }
        }
        break;
    case 65:
    case 66:
    {
        pModel->BeginRender(1.0f);

        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime % 100000 * 0.00005f, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        pModel->EndRender();
    }
    break;
    case	67:
        Vector(0.3f, 0.3f, 0.3f, pObject->Light);
        CreateParticleFpsChecked(BITMAP_FLAME, pObject->Position, pObject->Angle, pObject->Light, 7, pObject->Scale);
        break;
    case	70:
    {
        int time = timeGetTime() % 1024;
        if (rand_fps_check(5) && (time >= 0 && time < 30))
        {
            Vector(0.1f, 0.1f, 0.1f, Light);
            CreateEffect(MODEL_GHOST, pObject->Position, pObject->Angle, Light, 0, pObject, -1, 0, pObject->Scale);
        }
    }
    break;
    case	71:
    {
        Vector(0.0f, -3.0f, 1.0f, p);
        pModel->TransformPosition(BoneTransform[6], p, Position, false);
        Vector(0.5f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 6.0f, Light, pObject);
        Vector(0.15f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(0.0f, -3.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[7], p, Position, false);
        Vector(0.25f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 5.5f, Light, pObject);
        Vector(0.15f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(0.0f, -3.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[8], p, Position, false);
        Vector(0.25f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 5.0f, Light, pObject);
        Vector(0.15f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(-3.0f, -1.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[12], p, Position, false);
        Vector(0.25f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 5.5f, Light, pObject);
        Vector(0.15f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(-3.0f, -3.0f, 0.0f, p);
        pModel->TransformPosition(BoneTransform[13], p, Position, false);
        Vector(0.25f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 5.0f, Light, pObject);
        Vector(0.15f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);

        Vector(0.0f, 2.0f, -8.0f, p);
        pModel->TransformPosition(BoneTransform[17], p, Position, false);
        Vector(0.25f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 5.0f, Light, pObject);
        Vector(0.15f, 0.0f, 0.0f, Light);
        CreateSprite(BITMAP_SPARK + 1, Position, 3.0f, Light, pObject);
    }
    break;
    case 75:
    {
        Vector(0.f, 0.f, 0.f, p);
        pModel->TransformPosition(BoneTransform[4], p, Position, false);
        float fLumi = (sinf(WorldTime * 0.002f) + 1.0f) * 0.5f;
        Vector(fLumi, fLumi, fLumi, Light);
        Vector(1.0f, 1.0f, 1.f, Light);
        CreateSprite(BITMAP_FLARE, Position, 3.0f, Light, pObject, (WorldTime / 10.0f));
    }
    break;
    case 77:
    case 78:
    {
        pModel->BeginRender(1.0f);
        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, (int)WorldTime % 100000 * 0.0002f, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;
        pModel->EndRender();
    }
    break;
    }

    return true;
}
bool M33Aida::RenderAidaObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon)
{
    if (IsInAida())
    {
    }

    return RenderAidaMonsterObjectMesh(pObject, pModel, ExtraMon);
}

CHARACTER* M33Aida::CreateAidaMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;
    switch (iType)
    {
    case MONSTER_WITCH_QUEEN:
    {
        OpenMonsterModel(MONSTER_MODEL_WITCH_QUEEN);
        pCharacter = CreateCharacter(Key, MODEL_WITCH_QUEEN, PosX, PosY);
        pCharacter->Object.Scale = 1.4f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster100_Footstepst", 0);
        BoneManager::RegisterBone(pCharacter, L"Monster100_L_Hand", 76);
        BoneManager::RegisterBone(pCharacter, L"Monster100_R_Hand", 94);
        BoneManager::RegisterBone(pCharacter, L"Monster100_Pelvis", 2);
        BoneManager::RegisterBone(pCharacter, L"Monster100_Head", 57);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z02", 107);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z03", 108);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z04", 109);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z05", 110);
    }
    break;
    case MONSTER_BLUE_GOLEM:
    {
        OpenMonsterModel(MONSTER_MODEL_GOLDEN_STONE_GOLEM);
        pCharacter = CreateCharacter(Key, MODEL_GOLDEN_STONE_GOLEM, PosX, PosY);
        pCharacter->Object.Scale = 1.35f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster101_L_Arm", 12);
        BoneManager::RegisterBone(pCharacter, L"Monster101_R_Arm", 20);
        BoneManager::RegisterBone(pCharacter, L"Monster101_Head", 6);
    }
    break;
    case MONSTER_DEATH_RIDER:
    {
        OpenMonsterModel(MONSTER_MODEL_DEATH_RIDER);
        pCharacter = CreateCharacter(Key, MODEL_DEATH_RIDER, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster102_Footstepst", 0);
        BoneManager::RegisterBone(pCharacter, L"Monster102_Head", 6);
    }
    break;
    case MONSTER_FOREST_ORC:
    {
        OpenMonsterModel(MONSTER_MODEL_FOREST_ORC);
        pCharacter = CreateCharacter(Key, MODEL_FOREST_ORC, PosX, PosY);
        pCharacter->Object.Scale = 1.15f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_DEATH_TREE:
    {
        OpenMonsterModel(MONSTER_MODEL_DEATH_TREE);
        pCharacter = CreateCharacter(Key, MODEL_DEATH_TREE, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster104_Horn0", 37);
        BoneManager::RegisterBone(pCharacter, L"Monster104_Horn1", 38);
        BoneManager::RegisterBone(pCharacter, L"Monster104_Horn2", 39);
        BoneManager::RegisterBone(pCharacter, L"Monster104_Horn3", 40);
        BoneManager::RegisterBone(pCharacter, L"Monster104_Horn4", 44);
        BoneManager::RegisterBone(pCharacter, L"Monster104_Horn5", 45);
        BoneManager::RegisterBone(pCharacter, L"Monster104_Footsteps", 1);
    }
    break;
    case MONSTER_HELL_MAINE:
    {
        OpenMonsterModel(MONSTER_MODEL_HELL_MAINE);
        pCharacter = CreateCharacter(Key, MODEL_HELL_MAINE, PosX, PosY);
        pCharacter->Object.Scale = 1.8f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster105_R_Eye", 9);
        BoneManager::RegisterBone(pCharacter, L"Monster105_L_Eye", 10);
        BoneManager::RegisterBone(pCharacter, L"Monster105_L_Arm00", 33);
        BoneManager::RegisterBone(pCharacter, L"Monster105_L_Arm01", 34);
        BoneManager::RegisterBone(pCharacter, L"Monster105_L_Arm02", 35);
        BoneManager::RegisterBone(pCharacter, L"Monster105_L_Hand", 20);
        BoneManager::RegisterBone(pCharacter, L"Monster105_R_Hand", 39);
        //			BoneManager::RegisterBone(pCharacter, L"Monster105_Footsteps", 1);
    }
    break;
    case MONSTER_BLOODY_ORC:
    {
        OpenMonsterModel(MONSTER_MODEL_BLOODY_ORC);
        pCharacter = CreateCharacter(Key, MODEL_BLOODY_ORC, PosX, PosY);
        //pCharacter->Object.Scale = 1.15f;
        pCharacter->Object.Scale = 1.35f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_BLOODY_DEATH_RIDER:
    {
        OpenMonsterModel(MONSTER_MODEL_BLOODY_DEATH_RIDER);
        pCharacter = CreateCharacter(Key, MODEL_BLOODY_DEATH_RIDER, PosX, PosY);
        //pCharacter->Object.Scale = 1.1f;
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster102_Footstepst", 0);
        BoneManager::RegisterBone(pCharacter, L"Monster102_Head", 6);
    }
    break;
    case MONSTER_BLOODY_GOLEM:
    {
        OpenMonsterModel(MONSTER_MODEL_BLOODY_GOLEM);
        pCharacter = CreateCharacter(Key, MODEL_BLOODY_GOLEM, PosX, PosY);
        //pCharacter->Object.Scale = 1.35f;
        pCharacter->Object.Scale = 1.40f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster101_L_Arm", 12);
        BoneManager::RegisterBone(pCharacter, L"Monster101_R_Arm", 20);
        BoneManager::RegisterBone(pCharacter, L"Monster101_Head", 6);
    }
    break;
    case MONSTER_BLOODY_WITCH_QUEEN:
    {
        OpenMonsterModel(MONSTER_MODEL_BLOODY_WITCH_QUEEN);
        pCharacter = CreateCharacter(Key, MODEL_BLOODY_WITCH_QUEEN, PosX, PosY);
        //pCharacter->Object.Scale = 1.4f;
        pCharacter->Object.Scale = 1.75f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster100_Footstepst", 0);
        BoneManager::RegisterBone(pCharacter, L"Monster100_L_Hand", 76);
        BoneManager::RegisterBone(pCharacter, L"Monster100_R_Hand", 94);
        BoneManager::RegisterBone(pCharacter, L"Monster100_Pelvis", 2);
        BoneManager::RegisterBone(pCharacter, L"Monster100_Head", 57);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z02", 107);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z03", 108);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z04", 109);
        BoneManager::RegisterBone(pCharacter, L"Monster100_z05", 110);
    }
    break;
    }
    return pCharacter;
}

bool M33Aida::MoveAidaMonsterVisual(OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_WITCH_QUEEN:
    {
        vec3_t Light;
        Vector(0.7f, 0.1f, 0.1f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    case MODEL_GOLDEN_STONE_GOLEM:
    {
        vec3_t Light;
        Vector(0.f, 0.0f, 0.7f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    case MODEL_HELL_MAINE:
    {
        vec3_t Light;
        Vector(1.f, 0.0f, 0.0f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    case MODEL_BLOODY_GOLEM:
    {
        vec3_t Light;
        Vector(0.f, 0.0f, 0.7f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    case MODEL_BLOODY_WITCH_QUEEN:
    {
        vec3_t Light;
        Vector(0.7f, 0.1f, 0.1f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    }
    return false;
}

void M33Aida::MoveAidaBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_DEATH_RIDER:
    {
        if (pObject->AnimationFrame <= 5.06f && (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(20.f, 0.f, 0.f, StartRelative);
                Vector(100.f, 0.f, 0.f, EndRelative);

                pModel->TransformPosition(BoneTransform[18], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[20], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 23);

                pModel->TransformPosition(BoneTransform[25], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[26], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 24);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_FOREST_ORC:
    {
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  Light;
            Vector(0.5f, 0.5f, 0.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(20.f, 0.f, 0.f, StartRelative);
                Vector(60.f, 0.f, 0.f, EndRelative);

                pModel->TransformPosition(BoneTransform[35], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 23);

                pModel->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[41], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 24);

                pModel->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[45], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 25);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_DEATH_TREE:
    {
        if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            vec3_t  Light;
            Vector(0.3f, 0.3f, 0.3f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(0.f, 0.f, 20.f, StartRelative);
                Vector(0.f, 0.f, 70.f, EndRelative);

                pModel->TransformPosition(BoneTransform[26], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 24);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_BLOODY_ORC:
    {
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  Light;
            Vector(0.5f, 0.5f, 0.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(20.f, 0.f, 0.f, StartRelative);
                Vector(60.f, 0.f, 0.f, EndRelative);

                pModel->TransformPosition(BoneTransform[35], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 23);

                pModel->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[41], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 24);

                pModel->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[45], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 25);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_BLOODY_DEATH_RIDER:
    {
        if (pObject->AnimationFrame <= 5.06f && (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(20.f, 0.f, 0.f, StartRelative);
                Vector(100.f, 0.f, 0.f, EndRelative);

                pModel->TransformPosition(BoneTransform[18], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[20], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 23);

                pModel->TransformPosition(BoneTransform[25], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[26], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 24);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}

bool M33Aida::RenderAidaMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_WITCH_QUEEN:
    {
        vec3_t Position, Light, Angle;
        float Random_Light = (float)(rand() % 30) / 100.0f + 0.6f;
        Vector(Random_Light + 0.5f, Random_Light - 0.05f, Random_Light + 0.5f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster100_L_Hand", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.8f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z02", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.1f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z03", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.0f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z04", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.7f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z05", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.8f, Light, pObject);

        Vector(0.7f, 0.5f, 0.7f, Light);
        vec3_t Relative = { 0.0f, 0.0f, -65.0f };
        BoneManager::GetBonePosition(pObject, L"Monster100_Footstepst", Relative, Position);
        CreateParticleFpsChecked(BITMAP_LIGHT + 1, Position, Angle, Light, 4, 4.0f);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_WITCHQUEEN_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_WITCHQUEEN_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_CHAOS_THUNDER01 + rand() % 2));
                PlayBuffer(SOUND_AIDA_WITCHQUEEN_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_WITCHQUEEN_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->AnimationFrame >= 4.0f)
            {
                vec3_t Relative = { 70.0f, 0.0f, 0.0f };
                BoneManager::GetBonePosition(pObject, L"Monster100_Footstepst", Relative, Position);

                Vector(0.5f, 0.2f, 0.5f, Light);
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, Light, 27, 2.0f);
                Vector(1.0f, 1.0f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_LIGHT + 1, Position, Angle, Light, 2, 1.0f);
            }
            if (pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
            {
                vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
                vec3_t Relative = { 70.0f, 0.0f, 0.0f };
                BoneManager::GetBonePosition(pObject, L"Monster100_Footstepst", Relative, Position);
                Vector(1.0f, 1.0f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_LIGHT + 1, Position, Angle, Light, 3, 1.3f);
            }
            if (pCharacter->TargetCharacter >= 0 && pCharacter->TargetCharacter < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* tc = &CharactersClient[pCharacter->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t vTemp;
                VectorCopy(to->Position, vTemp);
                vTemp[2] += 100.0f;

                if (pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
                {
                    CreateParticleFpsChecked(BITMAP_LIGHT + 1, vTemp, Angle, Light, 3, 1.3f);
                }
                if (pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK2)
                {
                    CreateJointFpsChecked(BITMAP_JOINT_THUNDER, to->Position, vTemp, pObject->Angle, 16);
                }
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_GOLDEN_STONE_GOLEM:
    {
        vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
        Vector(1.0f, 1.0f, 1.0f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster101_L_Arm", Position);
        CreateParticleFpsChecked(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
        BoneManager::GetBonePosition(pObject, L"Monster101_R_Arm", Position);
        CreateParticleFpsChecked(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
        BoneManager::GetBonePosition(pObject, L"Monster101_Head", Position);
        CreateParticleFpsChecked(BITMAP_WATERFALL_2, Position, Angle, Light, 1);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_BLUEGOLEM_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_BLUEGOLEM_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_BLUEGOLEM_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_BLUEGOLEM_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
            Vector(1.0f, 1.0f, 1.0f, Light);
            if (pObject->AnimationFrame >= 4.0f)
            {
                Vector(0.0f, 45.0f, 45.0f, Angle);
                vec3_t Relative = { 30.0f, 0.0f, 0.0f };
                BoneManager::GetBonePosition(pObject, L"Monster101_L_Arm", Relative, Position);
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, Light, 25);

                if (pObject->AnimationFrame >= 5.0f && pObject->AnimationFrame <= 5.5f)
                {
                    Vector(5.0f, 5.0f, 5.0f, Light);
                    BoneManager::GetBonePosition(pObject, L"Monster101_L_Arm", Relative, Position);
                    CreateParticleFpsChecked(BITMAP_SHOCK_WAVE, Position, Angle, Light, 3, 0.5f);
                }
            }

            if (pCharacter->TargetCharacter >= 0 && pCharacter->TargetCharacter < MAX_CHARACTERS_CLIENT && pObject->AnimationFrame >= 6.9f)
            {
                CHARACTER* tc = &CharactersClient[pCharacter->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t vTemp;
                VectorCopy(to->Position, vTemp);
                vTemp[2] += 100.0f;

                CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vTemp, pObject->Position, to->Angle, 16, pObject, 20.0f);
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_DEATH_RIDER:
    {
        vec3_t Relative, Position, Light, Angle = { 0.0f, 0.0f, 0.0f };

        float Random_Light = (float)(rand() % 10) / 100.0f + 0.2f;
        Vector(60.0f, -30.0f, 0.0f, Relative);
        Vector(0.0f, Random_Light, 0.0f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster102_Head", Relative, Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 3.0f, Light, pObject);
        CreateParticleFpsChecked(BITMAP_SPARK + 1, Position, Angle, Light, 7);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_DEATHRAIDER_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHRAIDER_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHRAIDER_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHRAIDER_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;

    case MODEL_FOREST_ORC:
    {
        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_FORESTORC_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_FORESTORC_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_FORESTORC_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_FORESTORC_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_DEATH_TREE:
    {
        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_DEATHTREE_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHTREE_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHTREE_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHTREE_DIE);
            }
        }

        vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
        float Random_Light;

        for (int i = 0; i < 6; i++)
        {
            wchar_t Bone_Name[256];
            swprintf(Bone_Name, L"Monster104_Horn%d", i);
            Random_Light = (float)(rand() % 10) / 100.0f + 0.8f;
            Vector(0.4f, Random_Light, 0.5f, Light);
            BoneManager::GetBonePosition(pObject, Bone_Name, Position);
            CreateSprite(BITMAP_LIGHT + 1, Position, 0.4f, Light, pObject);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, Position, Angle, Light, 6);
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t Position, Relative = { 0.0f, -100.0f, 20.0f }, Light = { 0.5f, 0.7f, 0.5f };
            BoneManager::GetBonePosition(pObject, L"Monster104_Footsteps", Relative, Position);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, pObject->Angle, Light, 26);
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_HELL_MAINE:
    {
        vec3_t Relative, Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
        float Random_Light;

        Random_Light = (float)(rand() % 8) / 10.0f + 0.6f;
        Vector(Random_Light, 0.0f, 0.0f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster105_R_Eye", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.3f, Light, pObject);

        BoneManager::GetBonePosition(pObject, L"Monster105_L_Eye", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.3f, Light, pObject);

        BoneManager::GetBonePosition(pObject, L"Monster105_L_Arm00", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.0f, Light, pObject);

        BoneManager::GetBonePosition(pObject, L"Monster105_L_Arm01", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.0f, Light, pObject);

        BoneManager::GetBonePosition(pObject, L"Monster105_L_Arm02", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.0f, Light, pObject);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_HELL_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_HELL_ATTACK3);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_HELL_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK3)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_HELL_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_HELL_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            BoneManager::GetBonePosition(pObject, L"Monster105_L_Hand", Position);
            CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, Angle, Light, 6, 5.0f);

            BoneManager::GetBonePosition(pObject, L"Monster105_R_Hand", Position);
            CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, Angle, Light, 6, 5.0f);
        }
        else
        {
            BoneManager::GetBonePosition(pObject, L"Monster105_L_Hand", Position);
            CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, Angle, Light, 6, 2.5f);

            Vector(0.0f, 0.0f, -10.0f, Relative);
            BoneManager::GetBonePosition(pObject, L"Monster105_R_Hand", Relative, Position);
            CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, Angle, Light, 6, 2.1f);
        }
        if ((pObject->AnimationFrame >= 9.0f && pObject->AnimationFrame <= 10.0f) && pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            CreateEffectFpsChecked(BITMAP_BOSS_LASER, Position, pObject->Angle, Light, 1);
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_BLOODY_ORC:
    {
        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_FORESTORC_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_FORESTORC_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_FORESTORC_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_FORESTORC_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_BLOODY_DEATH_RIDER:
    {
        vec3_t Relative, Position, Light, Angle = { 0.0f, 0.0f, 0.0f };

        float Random_Light = (float)(rand() % 10) / 100.0f + 0.2f;
        Vector(60.0f, -30.0f, 0.0f, Relative);
        Vector(0.0f, Random_Light, 0.0f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster102_Head", Relative, Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 3.0f, Light, pObject);
        CreateParticleFpsChecked(BITMAP_SPARK + 1, Position, Angle, Light, 7);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_DEATHRAIDER_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHRAIDER_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHRAIDER_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_DEATHRAIDER_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;

    case MODEL_BLOODY_GOLEM:
    {
        vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
        Vector(1.0f, 1.0f, 1.0f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster101_L_Arm", Position);
        CreateParticleFpsChecked(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
        BoneManager::GetBonePosition(pObject, L"Monster101_R_Arm", Position);
        CreateParticleFpsChecked(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
        BoneManager::GetBonePosition(pObject, L"Monster101_Head", Position);
        CreateParticleFpsChecked(BITMAP_WATERFALL_2, Position, Angle, Light, 1);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_BLUEGOLEM_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_BLUEGOLEM_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_BLUEGOLEM_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_BLUEGOLEM_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
            Vector(1.0f, 1.0f, 1.0f, Light);
            if (pObject->AnimationFrame >= 4.0f)
            {
                Vector(0.0f, 45.0f, 45.0f, Angle);
                vec3_t Relative = { 30.0f, 0.0f, 0.0f };
                BoneManager::GetBonePosition(pObject, L"Monster101_L_Arm", Relative, Position);
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, Light, 25);

                if (pObject->AnimationFrame >= 5.0f && pObject->AnimationFrame <= 5.5f)
                {
                    Vector(5.0f, 5.0f, 5.0f, Light);
                    BoneManager::GetBonePosition(pObject, L"Monster101_L_Arm", Relative, Position);
                    CreateParticleFpsChecked(BITMAP_SHOCK_WAVE, Position, Angle, Light, 3, 0.5f);
                }
            }

            if (pCharacter->TargetCharacter >= 0 && pCharacter->TargetCharacter < MAX_CHARACTERS_CLIENT && pObject->AnimationFrame >= 6.9f)
            {
                CHARACTER* tc = &CharactersClient[pCharacter->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t vTemp;
                VectorCopy(to->Position, vTemp);
                vTemp[2] += 100.0f;

                CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vTemp, pObject->Position, to->Angle, 16, pObject, 20.0f);
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_BLOODY_WITCH_QUEEN:
    {
        vec3_t Position, Light, Angle;
        float Random_Light = (float)(rand() % 30) / 100.0f + 0.6f;
        Vector(Random_Light + 0.5f, Random_Light - 0.05f, Random_Light + 0.5f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster100_L_Hand", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.8f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z02", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.1f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z03", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.0f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z04", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 1.7f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster100_z05", Position);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.8f, Light, pObject);

        Vector(0.7f, 0.5f, 0.7f, Light);
        vec3_t Relative = { 0.0f, 0.0f, -65.0f };
        BoneManager::GetBonePosition(pObject, L"Monster100_Footstepst", Relative, Position);
        CreateParticleFpsChecked(BITMAP_LIGHT + 1, Position, Angle, Light, 4, 4.0f);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_AIDA_WITCHQUEEN_MOVE1 + rand() % 2));
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_WITCHQUEEN_ATTACK2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_CHAOS_THUNDER01 + rand() % 2));
                PlayBuffer(SOUND_AIDA_WITCHQUEEN_ATTACK1);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_AIDA_WITCHQUEEN_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (pObject->AnimationFrame >= 4.0f)
            {
                vec3_t Relative = { 70.0f, 0.0f, 0.0f };
                BoneManager::GetBonePosition(pObject, L"Monster100_Footstepst", Relative, Position);

                Vector(0.5f, 0.2f, 0.5f, Light);
                CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, Light, 27, 2.0f);
                Vector(1.0f, 1.0f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_LIGHT + 1, Position, Angle, Light, 2, 1.0f);
            }
            if (pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
            {
                vec3_t Position, Light, Angle = { 0.0f, 0.0f, 0.0f };
                vec3_t Relative = { 70.0f, 0.0f, 0.0f };
                BoneManager::GetBonePosition(pObject, L"Monster100_Footstepst", Relative, Position);
                Vector(1.0f, 1.0f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_LIGHT + 1, Position, Angle, Light, 3, 1.3f);
            }
            if (pCharacter->TargetCharacter >= 0 && pCharacter->TargetCharacter < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* tc = &CharactersClient[pCharacter->TargetCharacter];
                OBJECT* to = &tc->Object;

                vec3_t vTemp;
                VectorCopy(to->Position, vTemp);
                vTemp[2] += 100.0f;

                if (pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
                {
                    CreateParticleFpsChecked(BITMAP_LIGHT + 1, vTemp, Angle, Light, 3, 1.3f);
                }
                if (pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK2)
                {
                    CreateJointFpsChecked(BITMAP_JOINT_THUNDER, to->Position, vTemp, pObject->Angle, 16);
                }
            }
        }

        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    }
    return false;
}

bool M33Aida::RenderAidaMonsterObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon)
{
    switch (pObject->Type)
    {
    case MODEL_GOLDEN_STONE_GOLEM:
    {
        pModel->BeginRender(1.f);

        pModel->BodyLight[0] = 0.9f;
        pModel->BodyLight[1] = 0.9f;
        pModel->BodyLight[2] = 0.9f;

        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime % 10000 * 0.0003f, -(int)WorldTime % 10000 * 0.0003f);
        pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        pModel->EndRender();

        return true;
    }
    break;
    case MODEL_DEATH_RIDER:
    {
        pModel->BeginRender(1.f);

        pModel->BodyLight[0] = 0.9f;
        pModel->BodyLight[1] = 0.9f;
        pModel->BodyLight[2] = 0.9f;

        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = 0;
        pModel->RenderMesh(3, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime % 10000 * 0.0006f, -(int)WorldTime % 10000 * 0.0006f);
        pModel->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        pModel->EndRender();

        return true;
    }
    break;
    case MODEL_HELL_MAINE:
    {
        pModel->BeginRender(1.f);

        pModel->BodyLight[0] = 1.0f;
        pModel->BodyLight[1] = 1.0f;
        pModel->BodyLight[2] = 1.0f;

        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->BodyLight[0] = 1.0f;
        pModel->BodyLight[1] = 0.0f;
        pModel->BodyLight[2] = 0.0f;
        pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        pModel->BodyLight[0] = 1.0f;
        pModel->BodyLight[1] = 1.0f;
        pModel->BodyLight[2] = 1.0f;
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        pModel->StreamMesh = 1;
        pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->BodyLight[0] = 0.5f;
        pModel->BodyLight[1] = 0.0f;
        pModel->BodyLight[2] = 0.0f;
        pModel->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        pModel->EndRender();

        return true;
    }
    break;
    case MODEL_BLOODY_DEATH_RIDER:
    {
        pModel->BeginRender(1.f);

        pModel->BodyLight[0] = 0.9f;
        pModel->BodyLight[1] = 0.9f;
        pModel->BodyLight[2] = 0.9f;

        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = 0;
        pModel->RenderMesh(3, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime % 10000 * 0.0006f, -(int)WorldTime % 10000 * 0.0006f);
        pModel->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        pModel->EndRender();

        return true;
    }
    break;
    case MODEL_BLOODY_GOLEM:
    {
        pModel->BeginRender(1.f);

        pModel->BodyLight[0] = 0.9f;
        pModel->BodyLight[1] = 0.9f;
        pModel->BodyLight[2] = 0.9f;

        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime % 10000 * 0.0003f, -(int)WorldTime % 10000 * 0.0003f);
        pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        pModel->EndRender();

        return true;
    }
    break;
    }
    return false;
}

bool M33Aida::AttackEffectAidaMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    if (!IsInAida())
        return false;

    switch (pCharacter->MonsterIndex)
    {
    case MONSTER_WITCH_QUEEN:
    {
        if (pCharacter->CheckAttackTime(10) && pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(BITMAP_JOINT_FORCE, pObject->Position, pObject->Angle, Light, 1);
            pCharacter->SetLastAttackEffectTime();
        }
    }
    return true;
    case MONSTER_DEATH_TREE:
    {
        if (pCharacter->CheckAttackTime(10) && pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_TREE_ATTACK, pObject->Position, pObject->Angle, Light);
            pCharacter->SetLastAttackEffectTime();
        }
    }
    return true;
    case MONSTER_HELL_MAINE:
    {
        for (int i = 0; i < 5; i++)
        {
            if (pCharacter->CheckAttackTime(10) && pObject->CurrentAction == MONSTER01_ATTACK1)
            {
                vec3_t Light;
                Vector(1.f, 1.f, 1.f, Light);
                CreateEffect(MODEL_STORM, pObject->Position, pObject->Angle, Light, 3 + i);
                pCharacter->SetLastAttackEffectTime();
            }
        }
    }
    return true;
    case MONSTER_BLOODY_WITCH_QUEEN:
    {
        if (pCharacter->CheckAttackTime(10) && pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(BITMAP_JOINT_FORCE, pObject->Position, pObject->Angle, Light, 1);
            pCharacter->SetLastAttackEffectTime();
        }
    }
    return true;
    }
    return false;
}

bool M33Aida::SetCurrentActionAidaMonster(CHARACTER* pCharacter, OBJECT* pObject)
{
    if (!IsInAida())
        return false;

    switch (pCharacter->MonsterIndex)
    {
    case MONSTER_WITCH_QUEEN:
    case MONSTER_BLUE_GOLEM:
    case MONSTER_HELL_MAINE:
        return CheckMonsterSkill(pCharacter, pObject);
    case MONSTER_BLOODY_ORC:
    case MONSTER_BLOODY_DEATH_RIDER:
        return CheckMonsterSkill(pCharacter, pObject);
    case MONSTER_BLOODY_GOLEM:
    {
        if (pCharacter->MonsterSkill == ATMON_SKILL_EX_BLOODYGOLUEM_ATTACKSKILL)
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        }
        else
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        }
        return true;
    }
    return true;
    case MONSTER_BLOODY_WITCH_QUEEN:
    {
        if (pCharacter->MonsterSkill == ATMON_SKILL_EX_BLOODYWITCHQUEEN_ATTACKSKILL)
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        }
        else
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        }
        return true;
    }
    return true;
    }
    return false;
}

bool M33Aida::CreateMist(PARTICLE* pParticleObj)
{
    if (!IsInAida())
        return false;
    if (!IsInAidaSection2(Hero->Object.Position))
        return false;

    return false;
}