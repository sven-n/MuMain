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
#include "DSPlaySound.h"
#include "MapManager.h"
#include "BoneManager.h"
#include "SkillManager.h"

DWORD g_MusicStartStamp = 0;

bool M31HuntingGround::IsInHuntingGround()
{
    return (gMapManager.WorldActive == WD_31HUNTING_GROUND) ? true : false;
}

bool M31HuntingGround::IsInHuntingGroundSection2(const vec3_t Position)
{
    if (Position[0] > 5449.f && Position[0] < 17822.f && Position[1] > 6784.f && Position[1] < 22419.f)
        return true;
    return false;
}

bool M31HuntingGround::CreateHuntingGroundObject(OBJECT* pObject)
{
    if (!IsInHuntingGround())
        return false;

    switch (pObject->Type)
    {
    case 27:
    case 54:
        pObject->Timer = float(rand() % 1000) * 0.01f;
        break;
    }

    return true;
}
bool M31HuntingGround::MoveHuntingGroundObject(OBJECT* pObject)
{
    if (!IsInHuntingGround())
        return false;

    float Luminosity;
    vec3_t Light;

    switch (pObject->Type)
    {
    case 1:
    {
        int time = timeGetTime() % 1024;
        if (time >= 0 && time < 10) {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_BUTTERFLY01, pObject->Position, pObject->Angle, Light, 0, pObject);
        }
        pObject->HiddenMesh = -2;	//. Hide Object
    }
    break;
    case 44:
    {
        int time = timeGetTime() % 1024;
        if (time >= 0 && time < 10) {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_BUTTERFLY01, pObject->Position, pObject->Angle, Light, 1, pObject);
        }
        pObject->HiddenMesh = -2;	//. Hide Object
    }
    break;
    case 45:
    {
        int time = timeGetTime() % 1024;
        if (time >= 0 && time < 10) {
            vec3_t Light;
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_BUTTERFLY01, pObject->Position, pObject->Angle, Light, 2, pObject);
        }
        pObject->HiddenMesh = -2;	//. Hide Object
    }
    break;
    case 3:
    case 53:
        pObject->HiddenMesh = -2;
        break;
    case 27:
    {
        float vibration = sinf(pObject->Timer + WorldTime * 0.0024f) * 0.3f;
        pObject->Position[2] += vibration;
    }
    break;
    case 42:
    {
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity * 0.9f, Luminosity * 0.2f, Luminosity * 0.1f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
        pObject->HiddenMesh = -2;
    }
    break;
    }

    //. 배경음악 컨트롤
    if (::timeGetTime() - g_MusicStartStamp > 300000) {
        g_MusicStartStamp = ::timeGetTime();
        PlayBuffer(SOUND_BC_HUNTINGGROUND_AMBIENT);
    }
    return true;
}

bool M31HuntingGround::RenderHuntingGroundObjectVisual(OBJECT* pObject, BMD* pModel)
{
    if (!IsInHuntingGround())
        return false;

    vec3_t Light;

    switch (pObject->Type)
    {
    case 3:
        if (rand_fps_check(3)) {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_WATERFALL_3, pObject->Position, pObject->Angle, Light, 3, pObject->Scale);
        }
        break;
    case 53:
        if (rand_fps_check(3)) {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle, Light, 22, pObject->Scale);
        }
        break;
    case 49:
    {
        vec3_t Relative, Position;
        Vector(0.f, -10.f, 0.f, Relative);
        pModel->TransformPosition(BoneTransform[3], Relative, Position, false);

        Vector(1.f, 0.f, 0.f, Light);
        float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
        CreateSprite(BITMAP_LIGHT, Position, Luminosity * 1.2f + 0.3f, Light, pObject);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);
        Vector(0.3f, 0.3f, 0.3f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.2f, Light, NULL, rand() % 360);
    }
    break;
    }

    return true;
}
bool M31HuntingGround::RenderHuntingGroundObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon)
{
    if (IsInHuntingGround())
    {
        if (pObject->Type == 27 || pObject->Type == 54)
        {
            vec3_t LightBackup;
            VectorCopy(pModel->BodyLight, LightBackup);		//. backup
            float Luminosity = sinf(pObject->Timer + WorldTime * 0.0012f) * 0.5f + 0.9f;
            pModel->BodyLight[0] *= Luminosity;
            pModel->BodyLight[1] *= Luminosity;
            pModel->BodyLight[2] *= Luminosity;
            pModel->RenderBody(RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, pObject->HiddenMesh);

            VectorCopy(LightBackup, pModel->BodyLight);		//. restore

            return true;
        }
        if (pObject->Type == 10)
        {
            pModel->BodyLight[0] = 0.56f;
            pModel->BodyLight[1] = 0.80f;
            pModel->BodyLight[2] = 0.81f;
            pModel->RenderBody(RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, pObject->HiddenMesh);
            return true;
        }
        if (pObject->Type == 52)
        {
            float Luminosity = sinf(pObject->Timer + WorldTime * 0.0009f) * 0.5f + 0.9f;
            pModel->BodyLight[0] *= Luminosity;
            pModel->BodyLight[1] *= Luminosity;
            pModel->BodyLight[2] *= Luminosity;
            pModel->RenderBody(RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, pObject->HiddenMesh);
        }
    }
    return RenderHuntingGroundMonsterObjectMesh(pObject, pModel, ExtraMon);
}

CHARACTER* M31HuntingGround::CreateHuntingGroundMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;
    switch (iType)
    {
    case MONSTER_LIZARD_WARRIOR:
    {
        OpenMonsterModel(MONSTER_MODEL_LIZARD_WARRIOR);   //  81
        pCharacter = CreateCharacter(Key, MODEL_LIZARD_WARRIOR, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster81_EyeRight", 29);
        BoneManager::RegisterBone(pCharacter, L"Monster81_EyeLeft", 30);
    }
    break;
    case MONSTER_FIRE_GOLEM:
    {
        OpenMonsterModel(MONSTER_MODEL_FIRE_GOLEM);   //  82
        pCharacter = CreateCharacter(Key, MODEL_FIRE_GOLEM, PosX, PosY);
        pCharacter->Object.Scale = 1.8f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster82_RHand", 45);
        BoneManager::RegisterBone(pCharacter, L"Monster82_LHand", 34);
        BoneManager::RegisterBone(pCharacter, L"Monster82_Eye", 48);
        BoneManager::RegisterBone(pCharacter, L"Monster82_Back", 46);
    }
    break;
    case MONSTER_QUEEN_BEE:
    {
        OpenMonsterModel(MONSTER_MODEL_QUEEN_BEE);	//	83
        pCharacter = CreateCharacter(Key, MODEL_QUEEN_BEE, PosX, PosY);
        pCharacter->Object.Scale = 1.4f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster83_Tail", 62);
    }
    break;
    case MONSTER_GIGAS_GOLEM:
    case MONSTER_POISON_GOLEM:
    {
        OpenMonsterModel(MONSTER_MODEL_POISON_GOLEM);	// 84
        pCharacter = CreateCharacter(Key, MODEL_POISON_GOLEM, PosX, PosY);
        pCharacter->Object.Scale = 1.4f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster84_PoisonTop", 53);
        BoneManager::RegisterBone(pCharacter, L"Monster84_PoisonRight", 54);
        BoneManager::RegisterBone(pCharacter, L"Monster84_PoisonLeft", 55);
        BoneManager::RegisterBone(pCharacter, L"Monster84_LeftHand", 37);
        BoneManager::RegisterBone(pCharacter, L"Monster84_RightHand", 50);
    }
    break;
    case MONSTER_AXE_HERO:
    case MONSTER_AXE_WARRIOR:
    {
        OpenMonsterModel(MONSTER_MODEL_AXE_HERO);	//85
        pCharacter = CreateCharacter(Key, MODEL_AXE_HERO, PosX, PosY);
        pCharacter->Object.Scale = 0.7f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster85_LeftEye", 18);
        BoneManager::RegisterBone(pCharacter, L"Monster85_RightEye", 19);
    }
    break;
    case MONSTER_EROHIM:
    {
        OpenMonsterModel(MONSTER_MODEL_EROHIM);	//87
        pCharacter = CreateCharacter(Key, MODEL_EROHIM, PosX, PosY);
        pCharacter->Object.Scale = 2.f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster87_LeftEye", 8);
        BoneManager::RegisterBone(pCharacter, L"Monster87_RightEye", 9);
        BoneManager::RegisterBone(pCharacter, L"Monster87_LeftHand", 16);

        PlayBuffer(SOUND_BC_EROHIM_ENTER);
    }
    break;
    case MONSTER_PK_DARK_KNIGHT:
    {
        pCharacter = CreateCharacter(Key, MODEL_PLAYER, PosX, PosY);
        pCharacter->Skin = 1;
        pCharacter->BodyPart[BODYPART_HELM].Type = MODEL_DRAGON_HELM;
        pCharacter->BodyPart[BODYPART_ARMOR].Type = MODEL_DRAGON_ARMOR;
        pCharacter->BodyPart[BODYPART_PANTS].Type = MODEL_DRAGON_PANTS;
        pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_DRAGON_GLOVES;
        pCharacter->BodyPart[BODYPART_BOOTS].Type = MODEL_DRAGON_BOOTS;
        pCharacter->Weapon[0].Type = MODEL_SWORD_OF_DESTRUCTION;
        pCharacter->Weapon[1].Type = MODEL_DRAGON_SHIELD;
        int Level = 9;
        pCharacter->BodyPart[BODYPART_HELM].Level = Level;
        pCharacter->BodyPart[BODYPART_ARMOR].Level = Level;
        pCharacter->BodyPart[BODYPART_PANTS].Level = Level;
        pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
        pCharacter->BodyPart[BODYPART_BOOTS].Level = Level;
        pCharacter->PK = PVP_MURDERER2;
        SetCharacterScale(pCharacter);
    }
    break;
    }
    return pCharacter;
}

bool M31HuntingGround::MoveHuntingGroundMonsterVisual(OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_FIRE_GOLEM:
    {
        vec3_t Light;
        Vector(1.f, 0.2f, 0.1f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    }
    return false;
}
void M31HuntingGround::MoveHuntingGroundBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_AXE_HERO:
    {
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  Light;
            Vector(1.f, 1.f, 1.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(0.f, -10.f, -80.f, StartRelative);
                Vector(30.f, -30.f, -230.f, EndRelative);
                pModel->TransformPosition(BoneTransform[23], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[23], EndRelative, EndPos, false);

                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 23);

                Vector(30.f, 10.f, 80.f, StartRelative);
                Vector(30.f, -65.f, 230, EndRelative);
                pModel->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);

                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 34);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}
bool M31HuntingGround::RenderHuntingGroundMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_LIZARD_WARRIOR:
    {
        vec3_t Position, Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        BoneManager::GetBonePosition(pObject, L"Monster81_EyeRight", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster81_EyeLeft", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15)) {
                PlayBuffer(static_cast<ESound>(SOUND_BC_LIZARDWARRIOR_MOVE1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_LIZARDWARRIOR_ATTACK1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_DIE) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_BC_LIZARDWARRIOR_DIE);
            }
        }
        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_FIRE_GOLEM:
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);

        vec3_t Position, Relative;

        //. Dying Scene Processing
        if (pObject->CurrentAction == MONSTER01_DIE)
        {
            pModel->RenderMeshEffect(0, MODEL_GOLEM_STONE);
            pObject->Live = false;
            PlayBuffer(SOUND_BC_FIREGOLEM_DIE);
        }
        else if (rand_fps_check(1))
        {
            Vector(5.f, 10.f, 5.f, Relative);
            BoneManager::GetBonePosition(pObject, L"Monster82_LHand", Relative, Position);
            CreateParticle(BITMAP_TRUE_FIRE, Position, pObject->Angle, Light, 3, 3.4f, pObject);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 21, 2.5f);

            BoneManager::GetBonePosition(pObject, L"Monster82_RHand", Position);
            CreateParticle(BITMAP_TRUE_FIRE, Position, pObject->Angle, Light, 4, 3.4f, pObject);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 21, 2.5f);

            Vector(-20.f, -2.f, -10.f, Relative);
            BoneManager::GetBonePosition(pObject, L"Monster82_Eye", Relative, Position);
            CreateParticle(BITMAP_TRUE_FIRE, Position, pObject->Angle, Light, 0, 0.8f);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 21, 0.8f);

            BoneManager::GetBonePosition(pObject, L"Monster82_Back", Position);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 21, 0.8f);

            if (rand_fps_check(20)) {
                PlayBuffer(static_cast<ESound>(SOUND_BC_FIREGOLEM_MOVE1 + rand() % 2));
            }
        }

        //. Attack Scene Processing
        if (pObject->CurrentAction == MONSTER01_ATTACK1 && pObject->AnimationFrame >= 8.4f && pObject->AnimationFrame <= 10.2f)
        {
            if (pObject->SubType == FALSE)
            {
                pObject->SubType = TRUE;

                BoneManager::GetBonePosition(pObject, L"Monster82_RHand", Position);
                Position[2] = pObject->Position[2];

                vec3_t Angle;
                Vector(-90.f, 0.f, pObject->Angle[2], Angle);

                CreateEffect(BITMAP_CRATER, Position, Angle, Light, 2);
                CreateJoint(BITMAP_JOINT_FORCE, Position, Position, Angle, 6, NULL, 200.f);
                CreateEffect(BITMAP_FLAME, Position, pObject->Angle, pObject->Light, 3);

                for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
                {
                    CHARACTER* tc = &CharactersClient[i];
                    OBJECT* to = &tc->Object;
                    if (to->Kind == KIND_PLAYER && to->Type == MODEL_PLAYER && to->Visible == true && to->Live == true)
                    {
                        float Distance = VectorDistance2D(to->Position, Position);
                        if (Distance > 200.f && Distance < 1100.f)
                        {
                            CreateEffect(BITMAP_CRATER, to->Position, Angle, Light, 2);
                            CreateJoint(BITMAP_JOINT_FORCE, to->Position, to->Position, Angle, 6, NULL, 200.f);
                            CreateEffect(BITMAP_FLAME, to->Position, to->Angle, Light, 3);
                        }
                    }
                }

                PlayBuffer(SOUND_BC_FIREGOLEM_ATTACK1);
            }
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK2 && pObject->AnimationFrame >= 5.0f && pObject->AnimationFrame <= 5.7f)
        {
            if (pObject->SubType == FALSE)
            {
                pObject->SubType = TRUE;

                Vector(80.f, 40.f, -25.f, Relative);
                BoneManager::GetBonePosition(pObject, L"Monster82_LHand", Relative, Position);
                CreateBomb(Position, true);

                PlayBuffer(SOUND_BC_FIREGOLEM_ATTACK2);
            }
        }
        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_QUEEN_BEE:
    {
        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_QUEENBEE_MOVE1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_QUEENBEE_ATTACK1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_DIE) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_BC_AXEWARRIOR_DIE);
            }
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK2) {
            vec3_t Position, Relative;
            Vector(rand() % 12 - 6, rand() % 12 - 6, rand() % 12 - 6, Relative);
            BoneManager::GetBonePosition(pObject, L"Monster83_Tail", Relative, Position);

            vec3_t Light;
            Vector(1.f, 0.3f, 0.f, Light);

            CreateSprite(BITMAP_SHINY + 1, Position, 2.5f, pObject->Light, pObject, 0.f, 1);
            CreateSprite(BITMAP_MAGIC + 1, Position, 0.8f, pObject->Light, pObject, 0.f);
            CreateParticleFpsChecked(BITMAP_ENERGY, Position, pObject->Angle, Light);
            CreateParticleFpsChecked(BITMAP_ENERGY, Position, pObject->Angle, Light);
        }
        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;

    case MODEL_POISON_GOLEM:
    {
        vec3_t Light, Position;

        Vector(0.4f, 1.0f, 0.7f, Light);

        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_POISONGOLEM_MOVE1 + rand() % 2));
            }
        }

        //. Dying Scene Processing
        if (pObject->CurrentAction == MONSTER01_DIE)
        {
            pModel->RenderMeshEffect(0, MODEL_GOLEM_STONE, 1);
            pObject->Live = false;
            PlayBuffer(SOUND_BC_POISONGOLEM_DIE);
        }

        //. Attack Scene Processing
        if (pObject->CurrentAction == MONSTER01_ATTACK1) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_BC_POISONGOLEM_ATTACK3);
            }

            vec3_t Angle;
            Vector(-55.f, sinf(WorldTime * 0.03f) * 45.f, pObject->Angle[2], Angle);
            if (rand_fps_check(2)) {
                if (rand_fps_check(2))
                    BoneManager::GetBonePosition(pObject, L"Monster84_PoisonRight", Position);
                else
                    BoneManager::GetBonePosition(pObject, L"Monster84_PoisonLeft", Position);
                CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 11, (float)(rand() % 32 + 50) * 0.025f);
            }
            CreateEffectFpsChecked(MODEL_BIG_STONE_PART2, Position, Angle, Light, 3);
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK2 && pObject->AnimationFrame >= 3.5f && pObject->AnimationFrame <= 4.2f) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_POISONGOLEM_ATTACK1 + rand() % 2));
            }

            BoneManager::GetBonePosition(pObject, L"Monster84_RightHand", Position);
            Position[2] = pObject->Position[2];
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, pObject->Angle, Light, 11, (float)(rand() % 32 + 50) * 0.05f);

            BoneManager::GetBonePosition(pObject, L"Monster84_LeftHand", Position);
            Position[2] = pObject->Position[2];
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, pObject->Angle, Light, 11, (float)(rand() % 32 + 50) * 0.05f);
        }

        if (rand_fps_check(10)) {
            BoneManager::GetBonePosition(pObject, L"Monster84_PoisonTop", Position);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 23);
        }
        if (rand_fps_check(10)) {
            BoneManager::GetBonePosition(pObject, L"Monster84_PoisonRight", Position);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 23);
        }
        if (rand_fps_check(10)) {
            BoneManager::GetBonePosition(pObject, L"Monster84_PoisonLeft", Position);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 23);
        }
        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;

    case MODEL_AXE_HERO:
    {
        vec3_t Position, Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        BoneManager::GetBonePosition(pObject, L"Monster85_LeftEye", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.7f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster85_RightEye", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.7f, Light, pObject);

        //. Walking & Running Scene Processing
        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10)) {
                CreateParticle(BITMAP_SMOKE + 1, pObject->Position, pObject->Angle, Light);
                PlayBuffer(static_cast<ESound>(SOUND_BC_AXEWARRIOR_MOVE1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_AXEWARRIOR_ATTACK1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_DIE) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_BC_AXEWARRIOR_DIE);
            }
        }
        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    case MODEL_EROHIM:
    {
        vec3_t Position, Relative, Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        Vector(0.f, -2.f, 0.f, Relative);
        BoneManager::GetBonePosition(pObject, L"Monster87_LeftEye", Relative, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);
        BoneManager::GetBonePosition(pObject, L"Monster87_RightEye", Relative, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, pObject);

        if (pObject->CurrentAction == MONSTER01_ATTACK1) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(static_cast<ESound>(SOUND_BC_EROHIM_ATTACK1 + rand() % 2));
            }
        }
        if (pObject->CurrentAction == MONSTER01_DIE) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                PlayBuffer(SOUND_BC_EROHIM_DIE);
            }
        }

        if (pObject->CurrentAction == MONSTER01_ATTACK2) {
            if (pObject->SubType == FALSE) {
                pObject->SubType = TRUE;
                CreateEffect(MODEL_SKILL_FISSURE, pObject->Position, pObject->Angle, pObject->Light, 0, pObject);
                PlayBuffer(SOUND_BC_EROHIM_ATTACK3);
            }
            Vector(0.f, 0.f, -10.f, Relative);
            BoneManager::GetBonePosition(pObject, L"Monster87_LeftHand", Relative, Position);
            CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, pObject->Angle, Light, 3, 3.f, pObject);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, pObject->Angle, Light, 21, 2.f);
        }
        if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
            pObject->SubType = FALSE;
    }
    break;
    }
    return false;
}
bool M31HuntingGround::RenderHuntingGroundMonsterObjectMesh(OBJECT* pObject, BMD* pModel, bool ExtraMon)
{
    switch (pObject->Type)
    {
    case MODEL_LIZARD_WARRIOR:
    {
        pModel->RenderBody(RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, 1);
        pModel->BeginRender(1.f);
        vec3_t LightBackup;
        VectorCopy(pModel->BodyLight, LightBackup);		//. backup
        Vector(0.6f, 0.4f, 0.4f, pModel->BodyLight);
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        VectorCopy(LightBackup, pModel->BodyLight);		//. restore
        pModel->EndRender();
        return true;
    }
    break;
    case MODEL_FIRE_GOLEM:
    {
        pModel->BeginRender(1.f);
        vec3_t LightBackup;
        VectorCopy(pModel->BodyLight, LightBackup);		//. backup
        float Luminosity = sinf(WorldTime * 0.0012f) * 0.8f + 1.3f;
        if (Luminosity > 1.3f)
            Luminosity = 1.3f;

        pModel->BodyLight[0] *= Luminosity;
        pModel->BodyLight[1] *= Luminosity;
        pModel->BodyLight[2] *= Luminosity;

        pModel->StreamMesh = 0;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, (int)WorldTime % 10000 * 0.0002f, (int)WorldTime % 10000 * 0.0002f);
        pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->StreamMesh = -1;

        VectorCopy(LightBackup, pModel->BodyLight);		//. restore

        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->EndRender();
        return true;
    }
    break;
    case MODEL_QUEEN_BEE:
    {
        pModel->BeginRender(1.f);

        pModel->RenderBody(RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, 0);

        vec3_t LightBackup;
        VectorCopy(pModel->BodyLight, LightBackup);		//. backup

        //Vector ( 0.7f, 0.7f, 0.7f, pModel->BodyLight );
        Vector(1.f, 1.f, 1.f, pModel->BodyLight);
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, -1, 1.f, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        //Vector ( 0.75f, 0.65f, 0.5f, pModel->BodyLight );
        Vector(0.8f, 0.6f, 1.f, pModel->BodyLight);
        pModel->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, -1, 1.f, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        VectorCopy(LightBackup, pModel->BodyLight);		//. restore

        pModel->EndRender();
        return true;
    }
    break;
    case MODEL_POISON_GOLEM:
    {
        pModel->BeginRender(1.f);
        vec3_t LightBackup;
        if (ExtraMon)
        {
            Vector(1.f, 1.f, 1.f, pModel->BodyLight);
        }
        VectorCopy(pModel->BodyLight, LightBackup);		//. backup
        float Luminosity = sinf(WorldTime * 0.0012f) * 0.8f + 1.3f;
        if (Luminosity > 1.3f)
            Luminosity = 1.3f;

        pModel->BodyLight[0] *= (Luminosity * 0.1f);
        pModel->BodyLight[1] *= Luminosity;
        pModel->BodyLight[2] *= (Luminosity * 0.1f);

        pModel->StreamMesh = 1;
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, (int)WorldTime % 10000 * 0.0002f, (int)WorldTime % 10000 * 0.0002f);

        if (ExtraMon)
        {
            pModel->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        else
            pModel->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        pModel->StreamMesh = -1;

        VectorCopy(LightBackup, pModel->BodyLight);		//. restore

        pModel->RenderMesh(0, RENDER_TEXTURE, sinf(WorldTime * 0.0012f) * 0.2f + 0.8f, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        if (ExtraMon)
        {
            pModel->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        pModel->StreamMesh = 2;
        pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, 2, 0.5f, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        if (ExtraMon)
        {
            pModel->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_CHROME);
            pObject->Scale = pModel->BodyScale;
        }
        pModel->StreamMesh = -1;
        pModel->EndRender();
        return true;
    }
    break;
    case MODEL_AXE_HERO:
    {
        pModel->BeginRender(1.f);
        vec3_t LightBackup;
        if (ExtraMon)
        {
            Vector(1.f, 1.f, 1.f, pModel->BodyLight);
        }
        VectorCopy(pModel->BodyLight, LightBackup);		//. backup
        pModel->BodyLight[0] *= 0.5f;
        pModel->BodyLight[1] *= 0.6f;
        pModel->BodyLight[2] *= 0.8f;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        if (ExtraMon)
            pModel->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_CHROME);

        Vector(0.5f, 0.8f, 0.6f, pModel->BodyLight);
        pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        if (ExtraMon)
            pModel->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_CHROME);

        VectorCopy(LightBackup, pModel->BodyLight);		//. restore
        pModel->EndRender();
        return true;
    }
    break;
    case MODEL_EROHIM:
    {
        pModel->RenderBody(RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, 5);

        pModel->BeginRender(1.f);

        pObject->Alpha = 1.0f;
        pModel->RenderMesh(5, RENDER_TEXTURE, pObject->Alpha, -1, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_HGBOSS_WING);
        pModel->RenderMesh(5, RENDER_TEXTURE, pObject->Alpha, -1, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        vec3_t LightBackup;
        VectorCopy(pModel->BodyLight, LightBackup);		//. backup

        Vector(1.f, 0.5f, 0.f, pModel->BodyLight);
        pModel->RenderMesh(4, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        Vector(0.75f, 0.65f, 0.5f, pModel->BodyLight);
        pModel->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(3, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);

        float Luminosity = sinf(WorldTime * 0.0012f) * 0.3f + 0.6f;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, 0, Luminosity, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_HGBOSS_PATTERN);

        VectorCopy(LightBackup, pModel->BodyLight);		//. restore

        pModel->EndRender();

        return true;
    }
    break;
    case MODEL_FISSURE:
    {
        pModel->BeginRender(1.f);

        Vector(1.f, 1.f, 1.f, pModel->BodyLight);
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, -1, 1.f, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, 0, 1.f, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV, BITMAP_FISSURE_FIRE);

        pModel->EndRender();

        return true;
    }
    break;
    case MODEL_FISSURE_LIGHT:
    {
        pModel->BeginRender(1.f);

        Vector(1.f, 1.f, 1.f, pModel->BodyLight);

        pObject->BlendMeshTexCoordU = sinf(WorldTime * 0.00008f) * 2.5f;
        pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, 0, 1.f, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
        pObject->BlendMeshTexCoordU = 1.f;

        pModel->EndRender();
        return true;
    }
    break;
    }
    return false;
}

bool M31HuntingGround::AttackEffectHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    if (!IsInHuntingGround())
        return false;

    switch (pCharacter->MonsterIndex)
    {
    case MONSTER_LIZARD_WARRIOR:
        break;
    case MONSTER_QUEEN_BEE:
    case MONSTER_GIGAS_GOLEM:
    case MONSTER_POISON_GOLEM:
    case MONSTER_FIRE_GOLEM:
    case MONSTER_EROHIM:
        return true;
    case MONSTER_AXE_HERO:
    case MONSTER_AXE_WARRIOR:
        break;
    }
    return false;
}
bool M31HuntingGround::SetCurrentActionHuntingGroundMonster(CHARACTER* pCharacter, OBJECT* pObject)
{
    if (!IsInHuntingGround())
        return false;

    switch (pCharacter->MonsterIndex)
    {
    case MONSTER_FIRE_GOLEM:
        if (pCharacter->Skill == AT_SKILL_BOSS) {
            SetAction(pObject, MONSTER01_ATTACK1);
        }
        else {
            SetAction(pObject, MONSTER01_ATTACK2);
        }

        return true;
    }

    return false;
}

bool M31HuntingGround::CreateMist(PARTICLE* pParticleObj)
{
    if (!IsInHuntingGround())
        return false;
    if (!IsInHuntingGroundSection2(Hero->Object.Position))
        return false;

    if (rand_fps_check(30)) {
        vec3_t Light;
        Vector(0.05f, 0.05f, 0.1f, Light);

        vec3_t TargetPosition = { 0.f, 0.f, 0.f }, TargetAngle = { 0.f, 0.f, 0.f };
        switch (rand() % 8)
        {
        case 0:
            TargetPosition[0] = Hero->Object.Position[0] + (300 + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] + (300 + rand() % 250);
            break;
        case 1:
            TargetPosition[0] = Hero->Object.Position[0] + (250 + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] - (250 + rand() % 250);
            break;
        case 2:
            TargetPosition[0] = Hero->Object.Position[0] - (200 + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] + (200 + rand() % 250);
            break;
        case 3:
            TargetPosition[0] = Hero->Object.Position[0] - (300 + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] - (300 + rand() % 250);
            break;
        case 4:
            TargetPosition[0] = Hero->Object.Position[0] + (400 + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1];
            break;
        case 5:
            TargetPosition[0] = Hero->Object.Position[0] - (400 + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1];
            break;
        case 6:
            TargetPosition[0] = Hero->Object.Position[0];
            TargetPosition[1] = Hero->Object.Position[1] + (400 + rand() % 250);
            break;
        case 7:
            TargetPosition[0] = Hero->Object.Position[0];
            TargetPosition[1] = Hero->Object.Position[1] - (400 + rand() % 250);
            break;
        }

        if (Hero->Movement)
        {
            float Matrix[3][4];
            AngleMatrix(Hero->Object.Angle, Matrix);
            vec3_t Velocity, Direction;
            Vector(0.f, -45.f * CharacterMoveSpeed(Hero), 0.f, Velocity);
            VectorRotate(Velocity, Matrix, Direction);
            VectorAddScaled(TargetPosition, Direction, TargetPosition, FPS_ANIMATION_FACTOR);
        }
        if (Hero->Movement || (rand_fps_check(2))) {
            TargetPosition[2] = (rand() % 20) + RequestTerrainHeight(TargetPosition[0], TargetPosition[1]);
            CreateParticle(BITMAP_CLOUD, TargetPosition, TargetAngle, Light, 8, 0.4f);
        }
        pParticleObj->Live = false;

        return true;
    }

    return false;
}