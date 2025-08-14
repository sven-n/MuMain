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
#include "MapManager.h"
#include "BoneManager.h"

bool M34CryingWolf2nd::IsCyringWolf2nd()
{
    return (gMapManager.WorldActive == WD_35CRYWOLF_2ND) ? true : false;
}

bool M34CryingWolf2nd::CreateCryingWolf2ndObject(OBJECT* pObject)
{
    if (!IsCyringWolf2nd())
        return false;

    return true;
}
bool M34CryingWolf2nd::MoveCryingWolf2ndObject(OBJECT* pObject)
{
    if (!IsCyringWolf2nd())
        return false;

    float Luminosity;
    vec3_t Light;

    switch (pObject->Type)
    {
    case 2:
    case 5:
        pObject->HiddenMesh = -2;
        break;
    case 3:
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
        pObject->HiddenMesh = -2;
        break;
    }

    return true;
}

bool M34CryingWolf2nd::RenderCryingWolf2ndObjectVisual(OBJECT* pObject, BMD* pModel)
{
    if (!IsCyringWolf2nd())
        return false;

    vec3_t Light;

    switch (pObject->Type)
    {
    case 2:
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle, Light, 21, pObject->Scale);
        }
        break;
    case 3:
        if (rand_fps_check(2)) {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle, Light, 5, pObject->Scale);
        }
        break;
    case 5:
    {
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(2)) {
            if ((int)((pObject->Timer += FPS_ANIMATION_FACTOR) + 2) % 4 == 0)
            {
                CreateParticle(BITMAP_ADV_SMOKE + 1, pObject->Position, pObject->Angle, Light);
                CreateParticle(BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 0);
            }
        }
        if (rand_fps_check(2)) {
            if ((int)(pObject->Timer += FPS_ANIMATION_FACTOR) % 4 == 0)
            {
                CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 6);
                CreateParticle(BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 1);
            }
        }
    }
    break;
    case 6:
    {
        Vector(1.f, 1.f, 1.f, Light);
        Vector(0.2f, 0.2f, 0.2f, Light);

        //CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 8, o->Scale);
        if (pObject->HiddenMesh != -2)
        {
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject);
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 2, pObject->Scale, pObject);
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 3, pObject->Scale, pObject);
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 4, pObject->Scale, pObject);
        }
        pObject->HiddenMesh = -2;
        //}
    }
    break;
    }

    return true;
}

bool M34CryingWolf2nd::RenderCryingWolf2ndObjectMesh(OBJECT* pObject, BMD* pModel)
{
    if (!IsCyringWolf2nd())
        return false;

    return RenderCryingWolf2ndMonsterObjectMesh(pObject, pModel);
}

CHARACTER* M34CryingWolf2nd::CreateCryingWolf2ndMonster(int iType, int PosX, int PosY, int Key)
{
    if (!IsCyringWolf2nd())
        return false;
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case MONSTER_WEREWOLFHERO:
    {
        OpenMonsterModel(MONSTER_MODEL_WEREWOLF_HERO);
        pCharacter = CreateCharacter(Key, MODEL_WEREWOLF_HERO, PosX, PosY);
        pCharacter->Object.Scale = 1.25f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster95_Head", 6);
    }
    break;
    case MONSTER_VALAM:
    {
        OpenMonsterModel(MONSTER_MODEL_VALAM);
        pCharacter = CreateCharacter(Key, MODEL_VALAM, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;

        BoneManager::RegisterBone(pCharacter, L"Monster96_Top", 27);
        BoneManager::RegisterBone(pCharacter, L"Monster96_Center", 28);
        BoneManager::RegisterBone(pCharacter, L"Monster96_Bottom", 29);
    }
    break;
    case MONSTER_SOLAM:
    {
        OpenMonsterModel(MONSTER_MODEL_SOLAM);
        pCharacter = CreateCharacter(Key, MODEL_SOLAM, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_SCOUT:
    {
        OpenMonsterModel(MONSTER_MODEL_SCOUT);
        pCharacter = CreateCharacter(Key, MODEL_SCOUT, PosX, PosY);
        pCharacter->Object.Scale = 1.2f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_HAMMER_SCOUT:
    {
        OpenMonsterModel(MONSTER_MODEL_BALRAM);
        pCharacter = CreateCharacter(Key, MODEL_BALRAM, PosX, PosY);
        pCharacter->Object.Scale = 1.25f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    }

    return pCharacter;
}

bool M34CryingWolf2nd::MoveCryingWolf2ndMonsterVisual(OBJECT* pObject, BMD* pModel)
{
    if (!IsCyringWolf2nd())
        return false;
    switch (pObject->Type)
    {
    case MODEL_WEREWOLF_HERO:
    {
        vec3_t Position, Light;

        if (pObject->CurrentAction != MONSTER01_DIE) {
            Vector(0.9f, 0.2f, 0.1f, Light);
            BoneManager::GetBonePosition(pObject, L"Monster95_Head", Position);
            CreateSprite(BITMAP_LIGHT, Position, 3.5f, Light, pObject);
        }

        Vector(0.9f, 0.2f, 0.1f, Light);
        //. Walking & Running Scene Processing
        if (pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10)) {
                CreateParticle(BITMAP_SMOKE + 1, pObject->Position, pObject->Angle, Light);
            }
        }
    }
    break;
    case MODEL_SOLAM:
    {
        vec3_t Position, Position2, Light;
        Vector(0.f, 0.f, 0.f, Position);

        if (pObject->CurrentAction == MONSTER01_ATTACK2 && pObject->AnimationFrame >= 4.0f && pObject->AnimationFrame <= 6.0f)
        {
            float Matrix[3][4];
            AngleMatrix(pObject->Angle, Matrix);
            VectorRotate(pObject->Direction, Matrix, Position2);
            VectorAdd(Position, Position2, Position);

            Vector(1.f, 0.0f, 0.5f, Light);
            CreateEffect(MODEL_PIERCING2, pObject->Position, pObject->Angle, Light, 1);
        }
    }
    break;
    case MODEL_VALAM:
    {
        vec3_t Position, Light;

        auto Rotation = (float)(rand() % 360);
        float Luminosity = sinf(WorldTime * 0.0012f) * 0.8f + 1.3f;

        float fScalePercent = 1.f;
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
            fScalePercent = .5f;

        BoneManager::GetBonePosition(pObject, L"Monster96_Center", Position);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 0.1f, Light);
        CreateSprite(BITMAP_LIGHT, Position, fScalePercent, Light, pObject);

        Vector(0.5f, 0.5f, 0.5f, Light);

        BoneManager::GetBonePosition(pObject, L"Monster96_Top", Position);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, pObject, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, pObject, 360.f - Rotation);

        BoneManager::GetBonePosition(pObject, L"Monster96_Bottom", Position);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, pObject, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, pObject, 360.f - Rotation);
    }
    break;
    }
    return false;
}
bool M34CryingWolf2nd::RenderCryingWolf2ndMonsterObjectMesh(OBJECT* pObject, BMD* pModel)
{
    return false;
}

bool M34CryingWolf2nd::RenderCryingWolf2ndMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    return false;
}

void M34CryingWolf2nd::MoveCryingWolf2ndBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_WEREWOLF_HERO:
    {
        if (pObject->CurrentAction == MONSTER01_ATTACK2)
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

                Vector(0.f, 0.f, -90.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                pModel->TransformPosition(BoneTransform[80], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[80], EndRelative, EndPos, false);

                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 80);

                Vector(0.f, 0.f, 90.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                pModel->TransformPosition(BoneTransform[82], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[82], EndRelative, EndPos, false);

                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 84);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_SOLAM:
    {
        if (pObject->CurrentAction == MONSTER01_ATTACK1)
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

                Vector(0.f, 0.f, 120.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                pModel->TransformPosition(BoneTransform[25], StartRelative, StartPos, false);
                pModel->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);

                CreateBlur(pCharacter, StartPos, EndPos, Light, 3, true, 25);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}

bool M34CryingWolf2nd::AttackEffectCryingWolf2ndMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    if (!IsCyringWolf2nd())
        return false;

    switch (pObject->Type)
    {
    case MODEL_VALAM:
    {
        if (pCharacter->CheckAttackTime(14))
        {
            CreateEffect(MODEL_ARROW_NATURE, pObject->Position, pObject->Angle, pObject->Light, 1, pObject, pObject->PKKey);
            pCharacter->SetLastAttackEffectTime();
            return true;
        }
    }
    break;
    case MODEL_BALRAM:
    {
        if (pCharacter->CheckAttackTime(14))
        {
            CreateEffect(MODEL_ARROW_HOLY, pObject->Position, pObject->Angle, pObject->Light, 1, pObject, pObject->PKKey);
            pCharacter->SetLastAttackEffectTime();
            return true;
        }
    }
    break;
    }
    return false;
}