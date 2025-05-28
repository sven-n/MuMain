// GM3rdChangeUp.cpp: implementation of the CGM3rdChangeUp class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "UIWindows.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "CDirection.h"
#include "MapManager.h"
#include "BoneManager.h"
#include "dsplaysound.h"
#include "CSChaosCastle.h"



SEASON3A::CGM3rdChangeUp::CGM3rdChangeUp() : m_nDarkElfAppearance(false)
{
}

SEASON3A::CGM3rdChangeUp::~CGM3rdChangeUp()
{
}

SEASON3A::CGM3rdChangeUp& SEASON3A::CGM3rdChangeUp::Instance()
{
    static CGM3rdChangeUp s_GM3rdChangeUp;
    return s_GM3rdChangeUp;
}

bool SEASON3A::CGM3rdChangeUp::IsBalgasBarrackMap()
{
    return WD_41CHANGEUP3RD_1ST == gMapManager.WorldActive ? true : false;
}

bool SEASON3A::CGM3rdChangeUp::IsBalgasRefugeMap()
{
    return WD_42CHANGEUP3RD_2ND == gMapManager.WorldActive ? true : false;
}

bool SEASON3A::CGM3rdChangeUp::CreateBalgasBarrackObject(OBJECT* pObject)
{
    if (!IsBalgasBarrackMap())
        return false;

    return true;
}

bool SEASON3A::CGM3rdChangeUp::CreateBalgasRefugeObject(OBJECT* pObject)
{
    if (!IsBalgasRefugeMap())
        return false;

    return true;
}

bool SEASON3A::CGM3rdChangeUp::MoveObject(OBJECT* pObject)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap()))
        return false;

    PlayEffectSound(pObject);

    vec3_t Light;
    float Luminosity;

    switch (pObject->Type)
    {
    case 2:
    case 5:
    case 58:
    case 59:
    case 60:
        pObject->HiddenMesh = -2;
        break;
    case 3:
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3,
            PrimaryTerrainLight);
        pObject->HiddenMesh = -2;
        break;
    case 57:
        pObject->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
        break;
    case 84:
    {
        pObject->Position[2]
            = RequestTerrainHeight(pObject->Position[0], pObject->Position[1])
            + sinf(WorldTime * 0.0005f) * 150.f - 100.f;
    }
    break;
    case 78:
        pObject->Alpha = 0.5f;
        break;
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
        pObject->HiddenMesh = -2;
        break;
    }

    return true;
}

bool SEASON3A::CGM3rdChangeUp::RenderObjectVisual(OBJECT* pObject, BMD* pModel)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap()))
        return false;

    vec3_t Light;

    switch (pObject->Type)
    {
    case 2:
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle, Light, 13, pObject->Scale);
        }
        break;
    case 3:
        if (rand_fps_check(3)) {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle, Light, 0, pObject->Scale);
        }
        break;
    case 5:
    {
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(2)) {
            if ((int)((pObject->Timer+=FPS_ANIMATION_FACTOR) + 2) % 4 == 0)
            {
                CreateParticle(BITMAP_ADV_SMOKE + 1, pObject->Position, pObject->Angle, Light);
                CreateParticle(BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 0);
            }
        }
        if (rand_fps_check(2)) {
            if ((int)(pObject->Timer+=FPS_ANIMATION_FACTOR) % 4 == 0)
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

        if (pObject->HiddenMesh != -2) {
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject);
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 2, pObject->Scale, pObject);
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 3, pObject->Scale, pObject);
            CreateParticleFpsChecked(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 4, pObject->Scale, pObject);
        }
        pObject->HiddenMesh = -2;
    }
    break;
    case 58:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(2))
            CreateParticle(BITMAP_WATERFALL_1, pObject->Position, pObject->Angle, Light, 2, pObject->Scale);
        break;
    case 59:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(3))
            CreateParticle(BITMAP_WATERFALL_2, pObject->Position, pObject->Angle, Light, 1, pObject->Scale);
        break;
    case 60:
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_WATERFALL_3, pObject->Position, pObject->Angle, Light, 3, pObject->Scale);
        break;
    case 85:
        if (rand_fps_check(2))
        {
            if ((int)((pObject->Timer+=FPS_ANIMATION_FACTOR) + 2) % 4 == 0)
            {
                if (rand_fps_check(2))
                {
                    Vector(0.f, 0.0f, 0.0f, Light);
                    CreateParticle(BITMAP_ADV_SMOKE + 1, pObject->Position, pObject->Angle, Light, 1, pObject->Scale);
                }
                Vector(1.f, 0.4f, 0.4f, Light);
                CreateParticle(BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 2, pObject->Scale);
            }
        }
        Vector(1.f, 0.4f, 0.4f, Light);
        if (rand_fps_check(2))
        {
            if ((int)(pObject->Timer+=FPS_ANIMATION_FACTOR) % 4 == 0)
            {
                CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 14, pObject->Scale, 0);
                CreateParticle(BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 2, pObject->Scale * 2);
            }
        }
        break;
    case 88:
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(4))
        {
            CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 10, pObject->Scale, pObject);
        }
        break;
    case 89:
        if (rand_fps_check(4))
        {
            float fRed = (rand() % 3) * 0.01f + 0.015f;
            Vector(fRed, 0.0f, 0.0f, Light);
            CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 11, pObject->Scale, pObject);
        }
        break;
    case 90:
    {
        Vector(1.0f, 0.4f, 0.4f, Light);
        vec3_t vAngle;
        if (rand_fps_check(3))
        {
            Vector((float)(rand() % 40 + 120), 0.f, (float)(rand() % 30), vAngle);
            VectorAdd(vAngle, pObject->Angle, vAngle);
            CreateJoint(BITMAP_JOINT_SPARK, pObject->Position, pObject->Position, vAngle, 4, pObject, pObject->Scale);
            CreateParticle(BITMAP_SPARK, pObject->Position, vAngle, Light, 9, pObject->Scale);
        }
    }
    break;
    case 92:
    {
        Vector(1.0f, 0.4f, 0.4f, Light);
        float fSin = (sinf(WorldTime * 0.0005f) + 1.f) * 0.5f;
        if (fSin > 0.9f)
        {
            for (int i = 0; i < 2; ++i)
            {
                CreateEffectFpsChecked(BITMAP_FIRE_RED, pObject->Position, pObject->Angle, Light, 0, NULL, -1, 0, pObject->Scale);
            }
        }
    }
    break;
    }

    return true;
}

bool SEASON3A::CGM3rdChangeUp::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (IsBalgasBarrackMap() || IsBalgasRefugeMap())
    {
        switch (o->Type)
        {
        case 79:
            b->StreamMesh = 0;
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 10000 * 0.0001f);
            b->StreamMesh = -1;
            return true;
        case 81:
        {
            float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f + 0.3f;
            b->RenderBody(RENDER_TEXTURE, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        return true;
        case 82:
        case 83:
        {
            float fLumi = (sinf(WorldTime * 0.0005f)) * 10.f;
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        return true;
        case 57:
        case 78:
        case 84:
            o->m_bRenderAfterCharacter = true;
            return true;
        }
    }
    return RenderMonsterObjectMesh(o, b, ExtraMon);
}

void SEASON3A::CGM3rdChangeUp::RenderAfterObjectMesh(OBJECT* o, BMD* b)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap()))
        return;

    switch (o->Type)
    {
    case 57:
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
            o->BlendMeshLight, o->BlendMeshTexCoordU,
            o->BlendMeshTexCoordV, o->HiddenMesh);
        break;
    case 78:
        b->BodyLight[0] = 0.52f;
        b->BodyLight[1] = 0.52f;
        b->BodyLight[2] = 0.52f;

        b->StreamMesh = 0;
        b->RenderMesh(
            0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh,
            o->BlendMeshLight, -(int)WorldTime % 100000 * 0.00001f,
            o->BlendMeshTexCoordV);
        b->StreamMesh = -1;
        break;
    case 84:
        b->StreamMesh = 0;
        float fLumi = (sinf(WorldTime * 0.001f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, 0, fLumi, (int)WorldTime % 10000 * 0.0001f, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->StreamMesh = -1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        break;
    }
}

bool SEASON3A::CGM3rdChangeUp::CreateFireSnuff(PARTICLE* o)
{
    if (IsBalgasRefugeMap() == true)
    {
        o->Type = BITMAP_FIRE_SNUFF;
        o->Scale = rand() % 50 / 100.f + 0.4f;
        vec3_t Position;
        Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800), Hero->Object.Position[1] + (float)(rand() % 1400 - 500), Hero->Object.Position[2] + (float)(rand() % 300 + 50), Position);

        VectorCopy(Position, o->Position);
        VectorCopy(Position, o->StartPosition);
        o->Velocity[0] = -(float)(rand() % 64 + 64) * 0.1f;
        if (Position[1] < CameraPosition[1] + 400.f)
        {
            o->Velocity[0] = -o->Velocity[0] + 2.2f;
        }
        o->Velocity[1] = (float)(rand() % 32 - 16) * 0.1f;
        o->Velocity[2] = (float)(rand() % 32 - 16) * 0.1f;
        o->TurningForce[0] = (float)(rand() % 16 - 8) * 0.1f;
        o->TurningForce[1] = (float)(rand() % 64 - 32) * 0.1f;
        o->TurningForce[2] = (float)(rand() % 16 - 8) * 0.1f;
        return true;
    }
    return false;
}

void SEASON3A::CGM3rdChangeUp::PlayEffectSound(OBJECT* o)
{
    switch (o->Type)
    {
    case 74:
    case 75:
        ::PlayBuffer(static_cast<ESound>(SOUND_3RD_CHANGE_UP_BG_CAGE1 + rand() % 2));
        break;
    case 79:
        ::PlayBuffer(SOUND_3RD_CHANGE_UP_BG_VOLCANO);
        break;
    case 92:
    {
        float fSin = (sinf(WorldTime * 0.0005f) + 1.f) * 0.5f;
        if (fSin > 0.9f)
            ::PlayBuffer(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR);
    }
    break;
    }
}

void SEASON3A::CGM3rdChangeUp::PlayBGM()
{
    if (IsBalgasBarrackMap())
    {
        if (::IsEndMp3())
            ::StopMp3(MUSIC_BALGAS_BARRACK);
        ::PlayMp3(MUSIC_BALGAS_BARRACK);
    }
    else if (IsBalgasRefugeMap())
    {
        if (::IsEndMp3())
            ::StopMp3(MUSIC_BALGAS_REFUGE);
        ::PlayMp3(MUSIC_BALGAS_REFUGE);
    }
    else
    {
        ::StopMp3(MUSIC_BALGAS_BARRACK);
        ::StopMp3(MUSIC_BALGAS_REFUGE);
    }
}

CHARACTER* SEASON3A::CGM3rdChangeUp::CreateBalgasBarrackMonster(int iType, int PosX, int PosY, int Key)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap() || gMapManager.InDevilSquare()))
        return NULL;

    CHARACTER* c = NULL;

    switch (iType)
    {
    case MONSTER_BALRAM_TRAINEE:
    case MONSTER_BALRAM_TRAINEE_SOLDIER:
    {
        OpenMonsterModel(MONSTER_MODEL_BALRAM);
        c = CreateCharacter(Key, MODEL_BALRAM, PosX, PosY);
        c->Object.Scale = 1.25f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_DEATH_SPIRIT_TRAINEE_SOLDIER:
    {
        OpenMonsterModel(MONSTER_MODEL_DEATH_SPIRIT);
        c = CreateCharacter(Key, MODEL_DEATH_SPIRIT, PosX, PosY);
        c->Object.Scale = 1.25f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;

        BoneManager::RegisterBone(c, L"Monster94_zx", 27);
        BoneManager::RegisterBone(c, L"Monster94_zx01", 28);
    }
    break;
    case MONSTER_SORAM_TRAINEE:
    case MONSTER_SORAM_TRAINEE_SOLDIER:
    {
        OpenMonsterModel(MONSTER_MODEL_SORAM);
        c = CreateCharacter(Key, MODEL_SORAM, PosX, PosY);
        c->Object.Scale = 1.3f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_DARK_ELF_TRAINEE_SOLDIER:
    {
        m_nDarkElfAppearance = true;

        OpenMonsterModel(MONSTER_MODEL_DARK_ELF);
        c = CreateCharacter(Key, MODEL_DARK_ELF, PosX, PosY);
        //			c->Object.Scale = 1.5f;
        c->Object.Scale = 1.7f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        BoneManager::RegisterBone(c, L"Left_Hand", 17);
    }
    break;
    }
    return c;
}

bool SEASON3A::CGM3rdChangeUp::SetCurrentActionBalgasBarrackMonster(CHARACTER* c, OBJECT* o)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap()))
        return false;

    switch (c->MonsterIndex)
    {
    case MONSTER_BALRAM_TRAINEE_SOLDIER:
    case MONSTER_DEATH_SPIRIT_TRAINEE_SOLDIER:
    case MONSTER_SORAM_TRAINEE_SOLDIER:
    case MONSTER_DARK_ELF_TRAINEE_SOLDIER:
        return CheckMonsterSkill(c, o);
    }
    return false;
}

bool SEASON3A::CGM3rdChangeUp::AttackEffectBalgasBarrackMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap()))
        return false;

    switch (o->Type)
    {
    case MODEL_BALRAM:
        if (c->CheckAttackTime(14))
        {
            CreateEffect(MODEL_ARROW_HOLY, o->Position, o->Angle, o->Light, 1, o, o->PKKey);
            c->SetLastAttackEffectTime();
            return true;
        }
        break;
    }

    return false;
}

bool SEASON3A::CGM3rdChangeUp::MoveBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_BALRAM:
    {
        vec3_t Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10)) {
                CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
            }
        }
    }
    break;
    case MODEL_DEATH_SPIRIT:
    {
        vec3_t Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10))
            {
                CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
            }
        }
    }
    break;
    case MODEL_SORAM:
    {
        float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
        vec3_t Light;
        vec3_t EndPos, EndRelative;
        Vector(1.f, 1.f, 1.f, Light);

        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->AnimationFrame >= 6.5f && o->AnimationFrame < (6.5f + fActionSpeed) && rand_fps_check(1))
            {
                Vector(0.0f, 0.0f, 0.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[27], EndRelative, EndPos, true);
                CreateEffect(BITMAP_CRATER, EndPos, o->Angle, o->Light, 2);
                CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
                CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
                Vector(1.0f, 0.6f, 0.4f, Light);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);

                for (int iu = 0; iu < 4; iu++)
                {
                    CreateEffect(MODEL_BIG_STONE1, EndPos, o->Angle, o->Light, 10);
                    CreateEffect(MODEL_STONE2, EndPos, o->Angle, o->Light);
                }
            }
        }
        else
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
                }
            }
    }
    break;
    case MODEL_DARK_ELF:
        if (m_nDarkElfAppearance)
        {
            m_nDarkElfAppearance = false;

            vec3_t Light;
            vec3_t EndPos, EndRelative;

            Vector(1.f, 0.2f, 0.2f, Light);
            Vector(0.0f, 0.0f, 0.0f, EndRelative);
            b->TransformPosition(o->BoneTransform[27], EndRelative, EndPos, true);
            CreateEffect(BITMAP_CRATER, EndPos, o->Angle, o->Light, 2);
            CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
            CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
            Vector(1.0f, 0.2f, 0.2f, Light);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
        }
        break;
    }
    return false;
}

void SEASON3A::CGM3rdChangeUp::MoveBalgasBarrackBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    vec3_t Angle, Position;
    float Matrix[3][4];
    vec3_t  p, p2, EndPos;
    vec3_t  TempAngle;

    switch (o->Type)
    {
    case MODEL_DEATH_SPIRIT:
    {
        if ((o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(0.2f, 1.f, 0.4f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;

            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 250.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[27], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);

                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);

            if (o->AnimationFrame > 4.5f && o->AnimationFrame < 5.0f)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;
                vec3_t Angle = { 0.f, 0.f, o->Angle[2] };
                vec3_t Pos = { 0.f, 0.f, (to->BoundingBoxMax[2] / 1.f) };

                Vector(80.f, 0.f, 20.f, p);
                b->TransformPosition(o->BoneTransform[0], p, Position, true);
                Position[2] += 50.0f;
                Angle[2] = o->Angle[2] + 90;
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 0, to);
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 1, to);
                Angle[2] = o->Angle[2] - 90;
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 0, to);
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 1, to);
            }
        }
    }
    break;
    case MODEL_SORAM:
    {
        vec3_t  Light;
        Vector(1.0f, 1.0f, 1.0f, Light);

        if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; ++i)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 100.f, -150.f, EndRelative);
                b->TransformPosition(BoneTransform[16], EndRelative, EndPos, false);

                if (o->AnimationFrame > 5.0f && o->AnimationFrame < 7.0f && rand_fps_check(1))
                {
                    CreateParticle(BITMAP_FIRE, EndPos, o->Angle, Light);
                }

                Vector(0.f, -150.f, 0.f, p);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, p2);
                o->Angle[2] -= 18;
                Vector((float)(rand() % 60 + 60 - 90), 0.f, (float)(rand() % 30 + 90), Angle);
                VectorAdd(Angle, o->Angle, Angle);
                VectorCopy(p2, Position);

                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);
        }
    }
    break;
    case MODEL_DARK_ELF:
    {
        if ((o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.f, 1.f, 1.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;

            if (o->CurrentAction == MONSTER01_ATTACK2 && (o->AnimationFrame > 4.5f && o->AnimationFrame < 5.0f))
                CreateEffectFpsChecked(MODEL_DARK_ELF_SKILL, o->Position, o->Angle, o->Light, 2, o);

            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, -60.f, StartRelative);
                Vector(0.f, 0.f, -150.f, EndRelative);
                b->TransformPosition(BoneTransform[c->Weapon[0].LinkBone], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[c->Weapon[0].LinkBone], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);

                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3 && rand_fps_check(1))
        {
            vec3_t Position, Light;
            BoneManager::GetBonePosition(o, L"Left_Hand", Position);

            Vector(0.2f, 0.2f, 0.7f, Light);
            CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light, 27, 1.0f);
            Vector(0.3f, 0.3f, 0.4f, Light);
            CreateParticle(BITMAP_LIGHT + 1, Position, o->Angle, Light, 2, 0.8f);
            CreateParticle(BITMAP_LIGHT + 1, Position, o->Angle, Light, 2, 0.6f);
        }
    }
    break;
    }
}

bool SEASON3A::CGM3rdChangeUp::RenderMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_BALRAM:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
        return true;
    }
    break;
    case MODEL_DEATH_SPIRIT:
    {
        b->BeginRender(o->Alpha);
        static float aaa = 0.f;
        aaa += 0.1f;
        if (aaa > 1.0f)
            aaa = 0.f;
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            if (i == 2)
            {
                if (o->Alpha > 0.3f)
                    o->Alpha = 0.3f;

                o->BlendMesh = -2;
                b->EndRender();
                b->BeginRender(o->Alpha);
                Models[o->Type].StreamMesh = i;
            }
            else
                o->BlendMesh = -1;

            o->BlendMeshTexCoordU = (int)WorldTime % 10000 * 0.0005f;
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        b->EndRender();
        o->Alpha = 1.0f;
        return true;
    }
    break;
    case MODEL_SORAM:
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->BeginRender(o->Alpha);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
        b->EndRender();
        return true;
    }
    break;
    case MODEL_DARK_ELF:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME6, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME6, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return true;
    }
    return false;
}

bool SEASON3A::CGM3rdChangeUp::RenderBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!(IsBalgasBarrackMap() || IsBalgasRefugeMap()))
        return false;

    switch (o->Type)
    {
    case MODEL_BALRAM:
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_BALRAM_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALRAM_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALRAM_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALRAM_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;

        return true;
    case MODEL_DEATH_SPIRIT:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_DEATHSPIRIT_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DEATHSPIRIT_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DEATHSPIRIT_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DEATHSPIRIT_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;

        vec3_t Position, Light;
        int dummy = rand() % 14;
        auto Data = (float)((float)dummy / (float)100);
        auto Rot = (float)(rand() % 360);
        Vector(1.0f, 1.0f, 1.0f, Light);
        BoneManager::GetBonePosition(o, L"Monster94_zx", Position);
        CreateSprite(BITMAP_DS_EFFECT, Position, 1.5f, Light, o);
        Vector(0.3f, 0.3f, 0.7f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 3.5f + (Data * 5), Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 3.5f + (Data * 5), Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 3.5f + (Data * 5), Light, o);
        CreateSprite(BITMAP_DS_SHOCK, Position, 0.8f + Data, Light, o, Rot);

        BoneManager::GetBonePosition(o, L"Monster94_zx01", Position);
        Vector(0.1f, 0.0f, 0.6f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.8f, Light, o, Rot);
        if (rand_fps_check(2))
        {
            Vector(0.7f, 0.7f, 1.0f, Light);
            CreateSprite(BITMAP_SHINY + 1, Position, 0.8f, Light, o, 360.f - Rot);
        }
        Vector(0.3f, 0.3f, 0.7f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
    }
    return true;
    case MODEL_SORAM:
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_SORAM_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SORAM_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SORAM_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SORAM_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
        return true;
    case MODEL_DARK_ELF:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_DARKELF_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_SKILL1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK4)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_SKILL2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;

        vec3_t vRelativePos, vPos, vLight;
        Vector(0.f, 0.f, 0.f, vRelativePos);
        Vector(0.f, 0.f, 0.f, vPos);
        Vector(0.6f, 0.6f, 0.9f, vLight);
        int iBoneThunder[] = { 6, 15, 27, 17, 29, 3, 34, 44, 45, 40 };
        if (rand_fps_check(2))
        {
            for (int i = 0; i < 10; ++i)
            {
                b->TransformPosition(o->BoneTransform[iBoneThunder[i]], vRelativePos, vPos, true);
                if (rand_fps_check(2))
                {
                    CreateEffect(MODEL_FENRIR_THUNDER, vPos, o->Angle, vLight, 1, o);
                }
            }
        }
    }
    return true;
    }
    return false;
}