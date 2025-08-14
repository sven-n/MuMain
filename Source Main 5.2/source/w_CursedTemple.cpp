// w_CursedTemple.cpp: implementation of the CursedTemple class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CursedTemple.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzEffect.h"
#include "UIManager.h"
#include "CSParts.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "NewUIInventoryCtrl.h"
#include "MapManager.h"




extern int TextNum;
extern wchar_t TextList[50][100];
extern int  TextListColor[50];


using namespace SEASON3A;

CursedTemple* CursedTemple::GetInstance()
{
    static CursedTemple s_Instance;
    return &s_Instance;
}

CursedTemple::CursedTemple() : m_IsTalkEnterNpc(false), m_HolyItemPlayerIndex(0xffff)
{
    Initialize();
}

CursedTemple::~CursedTemple()
{
    Destroy();
}

void CursedTemple::Initialize()
{
    ResetCursedTemple();
}

void CursedTemple::Destroy()
{
    if (m_TerrainWaterIndex.size() != 0) m_TerrainWaterIndex.clear();
}

void CursedTemple::ResetCursedTemple()
{
    m_CursedTempleState = eCursedTempleState_None;
    m_InterfaceState = false;
    m_AlliedPoint = 0;
    m_IllusionPoint = 0;
    m_ShowAlliedPointEffect = false;
    m_ShowIllusionPointEffect = false;
    m_bGaugebarEnabled = false;
    m_fGaugebarCloseTimer = 0;
}

void CursedTemple::SetInterfaceState(bool state, int subtype)
{
    if (subtype == -1)
    {
        m_InterfaceState = state;
    }
    else if (subtype == 0)
    {
        if (m_CursedTempleState == eCursedTempleState_Wait
            || m_CursedTempleState == eCursedTempleState_Ready)
        {
            m_InterfaceState = state;
        }
    }

    if (m_InterfaceState == false)
    {
        if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT))
        {
            SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
            g_pNewUISystem->HideAll();
        }
    }
}

bool CursedTemple::GetInterfaceState(int type, int subtype)
{
    if (!gMapManager.IsCursedTemple()) return true;

    bool result = m_InterfaceState;

    if (type == INTERFACE_COMMAND)
    {
        auto tempSubtype = (COMMAND_TYPE)(subtype);

        if (subtype == COMMAND_PARTY)
        {
            return false;
        }
        else if (tempSubtype == COMMAND_TRADE || tempSubtype == COMMAND_PURCHASE || tempSubtype == COMMAND_BATTLE || tempSubtype == COMMAND_END)
        {
            return result;
        }
        else
        {
            return true;
        }
    }

    return result;
}

bool CursedTemple::IsPartyMember(DWORD selectcharacterindex)
{
    if (PartyNumber == 0) return false;

    CHARACTER* c = &CharactersClient[selectcharacterindex];
    if (c == NULL) return false;

    for (int i = 0; i < PartyNumber; ++i)
    {
        PARTY_t* p = &Party[i];
        int length = max(1, wcslen(c->ID));
        if (!wcsncmp(p->Name, c->ID, length)) return true;
    }
    return false;
}

bool CursedTemple::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (!gMapManager.IsCursedTemple())
        return false;

    switch (c->MonsterIndex)
    {
    case MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT2_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT3_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT4_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT5_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT6_POISON:
        return CheckMonsterSkill(c, o);
    }

    return false;
}

CHARACTER* CursedTemple::CreateCharacters(EMonsterType iType, int iPosX, int iPosY, int iKey)
{
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case MONSTER_STONE_STATUE:
    {
        OpenNpc(MODEL_CURSEDTEMPLE_STATUE);
        pCharacter = CreateCharacter(iKey, MODEL_CURSEDTEMPLE_STATUE, iPosX, iPosY);
        pCharacter->Object.EnableShadow = false;
        pCharacter->Object.m_bRenderShadow = false;
        pCharacter->Object.m_fEdgeScale = 1.03f;
        pCharacter->Object.PKKey = 0;
    }
    break;
    case MONSTER_MU_ALLIES_GENERAL:
    {
        OpenNpc(MODEL_CURSEDTEMPLE_ALLIED_NPC);
        pCharacter = CreateCharacter(iKey, MODEL_CURSEDTEMPLE_ALLIED_NPC, iPosX, iPosY);
        pCharacter->Object.Scale = 1.2f;
    }
    break;
    case MONSTER_ILLUSION_ELDER:
    {
        OpenNpc(MODEL_CURSEDTEMPLE_ILLUSION_NPC);
        pCharacter = CreateCharacter(iKey, MODEL_CURSEDTEMPLE_ILLUSION_NPC, iPosX, iPosY);
        pCharacter->Object.Scale = 1.2f;
    }
    break;
    case MONSTER_ALLIANCE_ITEM_STORAGE:
    {
        OpenNpc(MODEL_CURSEDTEMPLE_ALLIED_BASKET);
        pCharacter = CreateCharacter(iKey, MODEL_CURSEDTEMPLE_ALLIED_BASKET, iPosX, iPosY);
        pCharacter->Object.Scale = 1.8f;
        pCharacter->Object.m_fEdgeScale = 1.03f;
        m_ShowAlliedPointEffect = false;
    }
    break;
    case MONSTER_ILLUSION_ITEM_STORAGE:
    {
        OpenNpc(MODEL_CURSEDTEMPLE_ILLUSION__BASKET);
        pCharacter = CreateCharacter(iKey, MODEL_CURSEDTEMPLE_ILLUSION__BASKET, iPosX, iPosY);

        pCharacter->Object.Scale = 1.5f;
        pCharacter->Object.m_fEdgeScale = 1.03f;
        m_ShowIllusionPointEffect = false;
    }
    break;
    case MONSTER_MIRAGE:
    {
        OpenNpc(MODEL_CURSEDTEMPLE_ENTER_NPC);
        pCharacter = CreateCharacter(iKey, MODEL_CURSEDTEMPLE_ENTER_NPC, iPosX, iPosY);
        pCharacter->Object.Scale = 0.95f;
    }
    break;
    case MONSTER_ILLUSION_SORCERER_SPIRIT1_LIGHTNING:
    case MONSTER_ILLUSION_SORCERER_SPIRIT2_LIGHTNING:
    case MONSTER_ILLUSION_SORCERER_SPIRIT3_LIGHTNING:
    case MONSTER_ILLUSION_SORCERER_SPIRIT4_LIGHTNING:
    case MONSTER_ILLUSION_SORCERER_SPIRIT5_LIGHTNING:
    case MONSTER_ILLUSION_SORCERER_SPIRIT6_LIGHTNING:
    {
        OpenMonsterModel(MONSTER_MODEL_ILLUSION_SORCERER_SPIRIT_LIGHTNING);
        pCharacter = CreateCharacter(iKey, MODEL_ILLUSION_SORCERER_SPIRIT_LIGHTNING, iPosX, iPosY);
        pCharacter->Object.Scale = 1.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_ILLUSION_SORCERER_SPIRIT1_ICE:
    case MONSTER_ILLUSION_SORCERER_SPIRIT2_ICE:
    case MONSTER_ILLUSION_SORCERER_SPIRIT3_ICE:
    case MONSTER_ILLUSION_SORCERER_SPIRIT4_ICE:
    case MONSTER_ILLUSION_SORCERER_SPIRIT5_ICE:
    case MONSTER_ILLUSION_SORCERER_SPIRIT6_ICE:
    {
        OpenMonsterModel(MONSTER_MODEL_ILLUSION_SORCERER_SPIRIT_ICE);
        pCharacter = CreateCharacter(iKey, MODEL_ILLUSION_SORCERER_SPIRIT_ICE, iPosX, iPosY);
        pCharacter->Object.Scale = 1.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT2_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT3_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT4_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT5_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT6_POISON:
    {
        OpenMonsterModel(MONSTER_MODEL_ILLUSION_SORCERER_SPIRIT_POISON);
        pCharacter = CreateCharacter(iKey, MODEL_ILLUSION_SORCERER_SPIRIT_POISON, iPosX, iPosY);
        pCharacter->Object.Scale = 1.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_MU_ALLIES:
    {
        pCharacter = CreateCharacter(iKey, MODEL_PLAYER, iPosX, iPosY);
        pCharacter->Object.Scale = 1.f;
        pCharacter->Object.SubType = MODEL_CURSEDTEMPLE_ALLIED_PLAYER;
    }
    break;
    case MONSTER_ILLUSION_SORCERER:
    {
        pCharacter = CreateCharacter(iKey, MODEL_PLAYER, iPosX, iPosY);
        pCharacter->Object.Scale = 1.f;
        pCharacter->Object.SubType = MODEL_CURSEDTEMPLE_ILLUSION_PLAYER;
    }
    break;
    }

    return pCharacter;
}

bool CursedTemple::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!gMapManager.IsCursedTemple())
        return false;

    vec3_t p, Light;
    Vector(0.f, 0.f, 0.f, p);
    Vector(1.f, 1.f, 1.f, Light);
    switch (c->MonsterIndex)
    {
    case MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT2_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT3_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT4_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT5_POISON:
    case MONSTER_ILLUSION_SORCERER_SPIRIT6_POISON:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                Vector(0.4f, 0.6f, 1.f, Light);

                for (int i = 0; i < 5; i++)
                    CreateParticleFpsChecked(BITMAP_SMOKE, to->Position, o->Angle, Light, 1);

                PlayBuffer(SOUND_HEART);
            }
        }
    }
    return true;
    }

    return false;
}

bool CursedTemple::CreateObject(OBJECT* o)
{
    if (!gMapManager.IsCursedTemple()) return false;

    return false;
}

bool CursedTemple::MoveObject(OBJECT* o)
{
    if (!gMapManager.IsCursedTemple()) return false;

    if (m_bGaugebarEnabled == true && WorldTime - m_fGaugebarCloseTimer > 3000.0f)
    {
        SetGaugebarEnabled(false);
    }

    switch (gMapManager.WorldActive)
    {
    case WD_45CURSEDTEMPLE_LV1:
    case WD_45CURSEDTEMPLE_LV2:
    case WD_45CURSEDTEMPLE_LV3:
    case WD_45CURSEDTEMPLE_LV4:
    case WD_45CURSEDTEMPLE_LV5:
    case WD_45CURSEDTEMPLE_LV6:
    {
        switch (o->Type)
        {
        case 64:
        case 65:
        case 80:
            o->BlendMeshLight = (float)sinf(WorldTime * 0.0010f) * 0.5f + 0.5f;
            break;
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
            o->HiddenMesh = -2;
            break;
        }
    }
    return true;
    }

    return false;
}

void CursedTemple::MoveMonsterSoundVisual(OBJECT* o, BMD* b)
{
    if (!gMapManager.IsCursedTemple()) return;

    float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);

    switch (o->Type)
    {
    case MODEL_ILLUSION_SORCERER_SPIRIT_LIGHTNING:
    case MODEL_ILLUSION_SORCERER_SPIRIT_ICE:
    {
        if (o->CurrentAction == MONSTER01_STOP1)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER1_IDLE);
            }
        }
        else if (o->CurrentAction == MONSTER01_WALK)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER_MOVE);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER1_DEATH);
            }
        }

        if (o->DamageTime == 14)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER1_DAMAGE);
            }
        }
    }
    break;
    case MODEL_ILLUSION_SORCERER_SPIRIT_POISON:
    {
        /*
                    if(o->CurrentAction == MONSTER01_STOP1)
                    {
                        if(o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
                        {
                            PlayBuffer( SOUND_CURSEDTEMPLE_MONSTER2_IDLE );
                        }
                    }
        */
        if (o->CurrentAction == MONSTER01_WALK)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER_MOVE);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER2_DEATH);
            }
        }

        if (o->DamageTime == 14)
        {
            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (0.7f + fActionSpeed))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_MONSTER2_DAMAGE);
            }
        }
    }
    break;
    }
}

bool CursedTemple::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (!gMapManager.IsCursedTemple()) return false;

    switch (o->Type)
    {
    case MODEL_CURSEDTEMPLE_ALLIED_NPC:
    {
        o->Position[2] = 225.f;
    }
    return true;
    case MODEL_CURSEDTEMPLE_ILLUSION_NPC:
    {
        o->Position[2] = 250.f;
    }
    return true;
    }
    return false;
}

void CursedTemple::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!gMapManager.IsCursedTemple()) return;

    switch (o->Type)
    {
    case MODEL_ILLUSION_SORCERER_SPIRIT_POISON:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  Light;
            Vector(1.0f, 0.2f, 0.5f, Light);

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

                b->TransformPosition(BoneTransform[19], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[21], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 1);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(BoneTransform[25], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 1);

                fAnimationFrame += fSpeedPerFrame;
            }

            for (int j = 0; j < 10; j++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(BoneTransform[25], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 1);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}

bool CursedTemple::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (!gMapManager.IsCursedTemple()) return false;

    vec3_t Light;

    switch (gMapManager.WorldActive)
    {
    case WD_45CURSEDTEMPLE_LV1:
    case WD_45CURSEDTEMPLE_LV2:
    case WD_45CURSEDTEMPLE_LV3:
    case WD_45CURSEDTEMPLE_LV4:
    case WD_45CURSEDTEMPLE_LV5:
    case WD_45CURSEDTEMPLE_LV6:
    {
        switch (o->Type)
        {
        case 54:
        {
            vec3_t position;
            VectorCopy(o->Position, position);
            position[2] -= 100.f;

            float Rotation = (int)WorldTime % 3600 / (float)10.f;

            Vector(0.15f, 0.15f, 0.15f, o->Light);
            CreateParticleFpsChecked(BITMAP_EFFECT, position, o->Angle, o->Light);
            CreateParticleFpsChecked(BITMAP_EFFECT, position, o->Angle, o->Light, 3);
        }
        break;
        case 70:
        {
            if (rand_fps_check(3))
            {
                float fLumi = (rand() % 10) * 0.007f + 0.03f;
                Vector(54.f / 256.f * fLumi, 177.f / 256.f * fLumi, 150.f / 256.f * fLumi, Light);
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 15, o->Scale, o);
            }
        }
        return true;
        case 71:
        {
            if (rand_fps_check(3))
            {
                float fLumi = (rand() % 10) * 0.007f + 0.03f;
                Vector(221.f / 256.f * fLumi, 121.f / 256.f * fLumi, 201.f / 256.f * fLumi, Light);
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 15, o->Scale, o);
            }
        }
        return true;
        case 72:
        {
            if (rand_fps_check(3))
            {
                float fLumi = (rand() % 10) * 0.007f + 0.03f;
                Vector(54.f / 256.f * fLumi, 177.f / 256.f * fLumi, 150.f / 256.f * fLumi, Light);
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 15, o->Scale, o);
                Vector(221.f / 256.f * fLumi, 121.f / 256.f * fLumi, 201.f / 256.f * fLumi, Light);
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 15, o->Scale, o);
            }
        }
        return true;
        case 73:
        {
            if (rand_fps_check(3))
            {
                float fLumi = (rand() % 10) * 0.002f + 0.03f;
                Vector(1.2f * fLumi, 1.2f * fLumi, 1.2f * fLumi, Light);
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 15, o->Scale, o);
            }
        }
        return true;
        case 74:
        {
            if (rand_fps_check(2))
            {
                Vector(0.f, 0.f, 0.f, Light);
                CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 16, o->Scale, o);
            }
        }
        return true;
        case 75:
        {
            if (rand() % 35 == 5)
            {
                float fLumi = (rand() % 10) * 0.05f + 0.03f;
                Vector(256.f / 256.f * fLumi, 256.f / 256.f * fLumi, 256.f / 256.f * fLumi, Light);
                CreateParticleFpsChecked(BITMAP_GHOST_CLOUD1, o->Position, o->Angle, Light, 0, o->Scale, o);
            }
        }
        return true;
        case 76:
        {
            float fLumi = (rand() % 100) * 0.01;
            //Vector(180.f/255.f+fLumi, 71.f/255.f, 55.f/255.f, Light);
            Vector(180.f / 255.f + fLumi, 71.f / 255.f, 55.f / 255.f, Light);
            vec3_t vPos;
            VectorCopy(o->Position, vPos);
            for (int i = 0; i < 1; ++i)
            {
                CreateParticleFpsChecked(BITMAP_TORCH_FIRE, vPos, o->Angle, Light, 0, o->Scale, o);
            }
            VectorCopy(o->Position, vPos);
            vPos[2] += 20.f;
            CreateSprite(BITMAP_LIGHT, vPos, o->Scale * 6.f, Light, o);
        }
        return true;
        case 77:
        {
            float fLumi = (rand() % 100) * 0.005;
            Vector(55.f / 256.f, 71.f / 256.f, 180.f / 256.f + fLumi, Light);
            //Vector(54.f/256.f, 177.f/256.f+fLumi, 150.f/256.f, Light);
            vec3_t vPos;
            VectorCopy(o->Position, vPos);
            for (int i = 0; i < 1; ++i)
            {
                CreateParticleFpsChecked(BITMAP_TORCH_FIRE, vPos, o->Angle, Light, 0, o->Scale, o);
            }
            VectorCopy(o->Position, vPos);
            vPos[2] += 20.f;
            CreateSprite(BITMAP_LIGHT, vPos, o->Scale * 6.f, Light, o);
        }
        return true;
        case 78:
        {
            int iTime = timeGetTime() % 500;
            int iRand = rand() % 485;
            if (iTime >= iRand + (rand() % 5) && iTime < iRand + (rand() % 10 + 5))
            {
                Vector(1.f, 0.8f, 0.8f, Light);
                for (int i = 0; i < 4; ++i)
                {
                    CreateEffectFpsChecked(MODEL_FALL_STONE_EFFECT, o->Position, o->Angle, Light);
                }
                CreateEffectFpsChecked(MODEL_FALL_STONE_EFFECT, o->Position, o->Angle, Light, 1);
                Vector(0.7f, 0.7f, 0.8f, Light);
                vec3_t vPos;
                VectorCopy(o->Position, vPos);
                vPos[0] += (float)(rand() % 80 - 40);
                vPos[1] += (float)(rand() % 80 - 40);
                CreateParticleFpsChecked(BITMAP_WATERFALL_3 + (rand() % 2), vPos, o->Angle, Light, 2);
                Vector(0.9f, 0.0f, 0.0f, Light);
                CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, Light, 0, 1.5f);
            }
        }
        return true;
        case 79:
        {
            if (rand_fps_check(1))
            {
                for (int i = 0; i < 5; ++i)
                {
                    float fLumi = (rand() % 10) * 0.03f + 0.008f;
                    Vector(100.f / 256.f * fLumi, 110.f / 256.f * fLumi, 160.f / 256.f * fLumi, Light);
                    CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 17, o->Scale, o);
                }
            }
        }
        return true;
        }
    }
    return true;
    }
    return false;
}

bool CursedTemple::RenderObject_AfterCharacter(OBJECT* o, BMD* b)
{
    if (!gMapManager.IsCursedTemple()) return false;

    switch (o->Type)
    {
    case 64:
    case 65:
    case 66:
    case 80:
    {
        if (o->Type == 64 || o->Type == 65 || o->Type == 66 || o->Type == 80)
        {
            b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    return true;
    }

    return false;
}

bool CursedTemple::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    vec3_t Light;

    MoveMonsterSoundVisual(o, b);

    switch (o->Type)
    {
    case MODEL_CURSEDTEMPLE_ILLUSION_NPC:
    {
        vec3_t vRelativePos, vWorldPos;
        int boneindex[11] = { 34, 48, 49, 20, 45, 19, 44, 22, 21, 47, 46 };
        for (int i = 0; i < 11; ++i)
        {
            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[boneindex[i]], vRelativePos, vWorldPos, true);
            Vector(0.4f, 0.4f, 1.0f, Light);
            CreateSprite(BITMAP_LIGHT, vWorldPos, 0.7f, Light, o);	// flare01.jpg
        }

        Vector(0.f, 0.f, 0.f, Light);

        for (int j = 0; j < 3; ++j)
        {
            int randtemp = rand() % 4;
            if (randtemp == 0)
            {
                Vector(static_cast<float>(rand() % 120), static_cast<float>(rand() % 70), static_cast<float>(rand() % 50), vRelativePos);
            }
            else if (randtemp == 1)
            {
                Vector(-static_cast<float>(rand() % 120), -static_cast<float>(rand() % 70), static_cast<float>(rand() % 50), vRelativePos);
            }
            else if (randtemp == 2)
            {
                Vector(static_cast<float>(rand() % 120), -static_cast<float>(rand() % 70), static_cast<float>(rand() % 50), vRelativePos);
            }
            else if (randtemp == 3)
            {
                Vector(-static_cast<float>(rand() % 120), static_cast<float>(rand() % 70), static_cast<float>(rand() % 50), vRelativePos);
            }

            b->TransformPosition(o->BoneTransform[1], vRelativePos, vWorldPos, true);
            CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 0, 0.7f);
        }
    }
    break;
    case MODEL_CURSEDTEMPLE_ENTER_NPC:
    {
        vec3_t vRelativePos, vWorldPos;

        if (o->CurrentAction == MONSTER01_STOP2)
        {
            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);

            if (o->AnimationFrame > 0.5f && o->AnimationFrame < (8.5f + fActionSpeed))
            {
                Vector(0.f, 0.f, 0.f, vRelativePos);
                b->TransformPosition(o->BoneTransform[22], vRelativePos, vWorldPos, true);

                Vector(0.8f, 0.8f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_LIGHT + 2, vWorldPos, o->Angle, Light, 2, 0.7f);

                Light[0] = static_cast<float>((rand() % 100) * 0.01f);
                Light[1] = static_cast<float>((rand() % 100) * 0.01f);
                Light[2] = static_cast<float>((rand() % 100) * 0.01f);

                CreateParticleFpsChecked(BITMAP_SHINY, vWorldPos, o->Angle, Light, 3, 0.8f);
            }

            if (o->AnimationFrame > 7.3f && o->AnimationFrame < (7.5f + fActionSpeed))
            {
                Vector(0.7f, 0.7f, 1.0f, Light);
                CreateParticleFpsChecked(BITMAP_GM_AURORA, vWorldPos, o->Angle, Light, 3, 0.7f);
            }
        }
    }
    return true;

    case MODEL_ILLUSION_SORCERER_SPIRIT_POISON:
    {
        vec3_t vRelativePos, vWorldPos;
        Vector(0.f, 0.f, 0.f, vRelativePos);
        if (o->CurrentAction == MONSTER01_DIE)
        {
            int boneindex[6] = { 6, 2, 19, 25, 35, 40 };

            for (int i = 0; i < 6; ++i)
            {
                b->TransformPosition(o->BoneTransform[boneindex[i]], vRelativePos, vWorldPos, true);
                CreateParticleFpsChecked(BITMAP_SMOKE + 3, vWorldPos, o->Angle, Light, 3, 0.5f);
            }
        }
    }
    return true;
    case MODEL_ILLUSION_SORCERER_SPIRIT_ICE:
    {
        vec3_t vRelativePos, vWorldPos;
        float fLuminosity = (float)sinf((WorldTime) * 0.002f) * 0.2f;

        Vector(0.4f, 0.4f, 1.0f, Light);
        Vector(-2.f, 14.f, 0.f, vRelativePos);
        b->TransformPosition(o->BoneTransform[8], vRelativePos, vWorldPos, true);
        CreateSprite(BITMAP_SHINY + 1, vWorldPos, 0.9f, Light, o, -WorldTime * 0.08f);
        CreateSprite(BITMAP_LIGHT + 3, vWorldPos, 0.8f, Light, o, WorldTime * 0.3f);

        Vector(0.f, 0.f, 0.f, vRelativePos);

        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            Vector(0.4f, 0.4f, 1.0f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[17], vRelativePos, vWorldPos, true);
            CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
            CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 4, 0.8f);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vWorldPos, o->Angle, Light, 15, 0.7f + (fLuminosity * 0.05f));

            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[41], vRelativePos, vWorldPos, true);
            CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
            CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 4, 0.8f);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vWorldPos, o->Angle, Light, 15, 0.7f + (fLuminosity * 0.05f));
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            int boneindex[6] = { 7, 2, 14, 38, 73, 78 };

            for (int i = 0; i < 4; ++i)
            {
                b->TransformPosition(o->BoneTransform[boneindex[i]], vRelativePos, vWorldPos, true);
                CreateParticleFpsChecked(BITMAP_SMOKE + 3, vWorldPos, o->Angle, Light, 3, 0.5f);
            }
        }
    }
    return true;
    case MODEL_ILLUSION_SORCERER_SPIRIT_LIGHTNING:
    {
        vec3_t vRelativePos, vWorldPos;

        Vector(0.6f, 0.0f, 0.0f, Light);
        Vector(-2.f, 14.f, 0.f, vRelativePos);
        b->TransformPosition(o->BoneTransform[8], vRelativePos, vWorldPos, true);
        CreateSprite(BITMAP_SHINY + 1, vWorldPos, 0.5f, Light, o, -WorldTime * 0.08f);
        CreateSprite(BITMAP_LIGHT + 3, vWorldPos, 0.6f, Light, o, WorldTime * 0.3f);

        Vector(0.f, 0.f, 0.f, vRelativePos);

        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            Vector(0.6f, 0.0f, 0.0f, Light);
            b->TransformPosition(o->BoneTransform[17], vRelativePos, vWorldPos, true);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 4, 1.f);
            CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);

            b->TransformPosition(o->BoneTransform[41], vRelativePos, vWorldPos, true);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateEffectFpsChecked(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, Light, 2, o);
            CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 4, 1.f);
            CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            int boneindex[6] = { 7, 2, 14, 38, 73, 78 };

            for (int i = 0; i < 6; ++i)
            {
                b->TransformPosition(o->BoneTransform[boneindex[i]], vRelativePos, vWorldPos, true);
                CreateParticleFpsChecked(BITMAP_SMOKE + 3, vWorldPos, o->Angle, Light, 3, 0.5f);
            }
        }
    }
    return true;
    case MODEL_CURSEDTEMPLE_STATUE:
        if (o->CurrentAction == MONSTER01_DIE)
        {
            vec3_t vRelativePos, vWorldPos, Light;
            Vector(1.0f, 1.0f, 1.0f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[13], vRelativePos, vWorldPos, true);
            vRelativePos[0] = vWorldPos[0];
            vRelativePos[1] = vWorldPos[1];
            vRelativePos[2] = vWorldPos[2];
            if (o->PKKey == 0)
            {
                o->PKKey = 1;
                EarthQuake = (float)(rand() % 16 - 8) * 0.1f;
                vWorldPos[2] = vRelativePos[2] + 250;
                CreateEffectFpsChecked(MODEL_CURSEDTEMPLE_STATUE_PART2, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                for (int i = 0; i < 60; ++i)
                {
                    vWorldPos[0] = vRelativePos[0] + rand() % 80 - 40;
                    vWorldPos[1] = vRelativePos[1] + rand() % 80 - 40;
                    vWorldPos[2] = vRelativePos[2] + (rand() % 250);
                    CreateEffectFpsChecked(MODEL_CURSEDTEMPLE_STATUE_PART1, vWorldPos, o->Angle, Light, 0, o, 0, 0);
                }
                Vector(0.5f, 0.5f, 0.5f, Light);

                for (int i = 0; i < 160; ++i)
                {
                    vWorldPos[0] = vRelativePos[0] + rand() % 140 - 70;
                    vWorldPos[1] = vRelativePos[1] + rand() % 140 - 70;
                    vWorldPos[2] = vRelativePos[2] + (rand() % 400) - 100;
                    CreateParticleFpsChecked(BITMAP_SMOKE, vWorldPos, o->Angle, Light, 48, 1.0f);
                }
            }
        }
        else
        {
            vec3_t vRelativePos, vWorldPos, Light;
            Vector(0.2f, 0.3f, 0.4f + (rand() % 3) * 0.1f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[13], vRelativePos, vWorldPos, true);
            CreateParticleFpsChecked(BITMAP_FLARE + 1, vWorldPos, o->Angle, Light, 0, 0.15f);
            vWorldPos[2] += 30;
            CreateParticleFpsChecked(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 1, 8.0f);

            vWorldPos[2] += 160;
            Vector(0.2f, 0.1f, 0.0f, Light);
            b->TransformPosition(o->BoneTransform[14], vRelativePos, vWorldPos, true);
            CreateParticleFpsChecked(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 1, 4.0f);
        }
        return true;
    case MODEL_CURSEDTEMPLE_ALLIED_BASKET:
    {
        vec3_t vRelativePos, vWorldPos, Light;
        if (m_ShowAlliedPointEffect)
        {
            Vector(1.0f, 1.0f, 0.5f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[5], vRelativePos, vWorldPos, true);
            CreateEffectFpsChecked(BITMAP_FLARE, vWorldPos, o->Angle, Light, 1);

            Vector(0.9f, 0.7f, 0.4f, Light);
            CreateEffectFpsChecked(BITMAP_MAGIC, o->Position, o->Angle, Light, 8);

            m_ShowAlliedPointEffect = false;
        }
    }
    return true;
    case MODEL_CURSEDTEMPLE_ILLUSION__BASKET:
    {
        vec3_t vRelativePos, vWorldPos, Light;
        if (m_ShowIllusionPointEffect)
        {
            Vector(0.5f, 1.0f, 1.0f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            b->TransformPosition(o->BoneTransform[3], vRelativePos, vWorldPos, true);
            CreateEffectFpsChecked(BITMAP_FLARE, vWorldPos, o->Angle, Light, 2);

            Vector(0.7f, 0.8f, 0.9f, Light);
            CreateEffectFpsChecked(BITMAP_MAGIC, o->Position, o->Angle, Light, 8);

            m_ShowIllusionPointEffect = false;
        }
    }
    return true;
    }
    return false;
}

void CursedTemple::UpdateTempleSystemMsg(int _Value)
{
    wchar_t szText[256] = { 0, };
    swprintf(szText, GlobalText[2367]);
    switch (_Value)
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        g_pSystemLogBox->AddText(GlobalText[2367], SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    case 4:
        g_pSystemLogBox->AddText(GlobalText[2368], SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    case 5:
        swprintf(szText, GlobalText[829], 6);
        g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    case 6:
        break;
    case 7:
        g_pSystemLogBox->AddText(GlobalText[2865], SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    case 8:
        g_pSystemLogBox->AddText(GlobalText[2175], SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    }
}

void CursedTemple::SetGaugebarEnabled(bool bFlag)
{
    m_bGaugebarEnabled = bFlag;
    m_fGaugebarCloseTimer = WorldTime + 10000000.0f;
}

void CursedTemple::SetGaugebarCloseTimer()
{
    m_fGaugebarCloseTimer = WorldTime;
}

bool CursedTemple::IsGaugebarEnabled()
{
    return m_bGaugebarEnabled;
}

bool CursedTemple::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (!gMapManager.IsCursedTemple()) return false;

    switch (o->Type)
    {
    case MODEL_CURSEDTEMPLE_STATUE:
        if (o->CurrentAction != MONSTER01_DIE)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->RenderMesh(5, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        return true;
    case MODEL_CURSEDTEMPLE_ALLIED_BASKET:
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return true;
    case MODEL_CURSEDTEMPLE_ILLUSION__BASKET:
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return true;
    case MODEL_CURSEDTEMPLE_ILLUSION_NPC:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        float fLumi2 = (sinf(WorldTime * 0.002f) + 1.f);

        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 5, fLumi2, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CURSEDTEMPLE_NPC_MESH_EFFECT);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, fLumi);
    }
    break;
    case MODEL_CURSEDTEMPLE_ENTER_NPC:
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        Vector(10.f, 10.f, 10.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 3, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime % 2000 * 0.001f);
    }
    return true;
    case MODEL_ILLUSION_SORCERER_SPIRIT_LIGHTNING:
    case MODEL_ILLUSION_SORCERER_SPIRIT_ICE:
    {
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.8f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, WorldTime * 0.02f, 0, fLumi * 4.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        fLumi = (sinf(WorldTime * 0.006f) + 1.f) * 0.9f;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, WorldTime * 0.02f, 0, fLumi * 3.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, WorldTime * 0.002f, 0, o->BlendMeshLight * 2.f, -WorldTime * 0.002f, WorldTime * 0.02f);
    }
    return true;
    case 54:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, fLumi + 0.3f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 2);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    return true;
    }
    return false;
}

void CursedTemple::Process()
{
    if (!gMapManager.IsCursedTemple()) return;
}

void CursedTemple::Draw()
{
    if (!gMapManager.IsCursedTemple()) return;
}

void CursedTemple::PlayBGM()
{
    if (m_CursedTempleState == eCursedTempleState_Wait)
    {
        ::PlayMp3(MUSIC_CURSEDTEMPLE_WAIT);

        if (::IsEndMp3()) ::StopMp3(MUSIC_CURSEDTEMPLE_WAIT);
    }
    else if (m_CursedTempleState == eCursedTempleState_Ready
        || m_CursedTempleState == eCursedTempleState_Play)
    {
        ::StopMp3(MUSIC_CURSEDTEMPLE_WAIT);
        ::PlayMp3(MUSIC_CURSEDTEMPLE_GAME);

        if (::IsEndMp3()) ::StopMp3(MUSIC_CURSEDTEMPLE_GAME);
    }
    else if (m_CursedTempleState == eCursedTempleState_None)
    {
        ::StopMp3(MUSIC_CURSEDTEMPLE_WAIT);
        ::StopMp3(MUSIC_CURSEDTEMPLE_GAME);
    }
}

bool CursedTemple::IsHolyItemPickState()
{
    if (!gMapManager.IsCursedTemple()) return false;

    if (m_CursedTempleState != eCursedTempleState_Play)
    {
        return false;
    }

    return true;
}

void CursedTemple::ReceiveCursedTempleState(const eCursedTempleState state)
{
    m_CursedTempleState = state;

    if (m_CursedTempleState == eCursedTempleState_Wait)
    {
        SetInterfaceState(false);
    }
    else if (m_CursedTempleState == eCursedTempleState_Ready)
    {
        SetInterfaceState(false);
    }
    else if (m_CursedTempleState == eCursedTempleState_Play)
    {
        m_TerrainWaterIndex.clear();
        SetTerrainWaterState(m_TerrainWaterIndex, 0);
        SetInterfaceState(false);
    }
    else if (m_CursedTempleState == eCursedTempleState_End)
    {
        SetTerrainWaterState(m_TerrainWaterIndex, 1);
        m_TerrainWaterIndex.clear();
        SetInterfaceState(false);
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    }
}

void CursedTemple::ReceiveCursedTempleInfo(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TAMPLE_STATE)ReceiveBuffer;

    if (m_AlliedPoint != data->btAlliedPoint)
    {
        m_ShowAlliedPointEffect = true;
    }
    else if (m_IllusionPoint != data->btIllusionPoint)
    {
        m_ShowIllusionPointEffect = true;
    }
    m_AlliedPoint = data->btAlliedPoint;
    m_IllusionPoint = data->btIllusionPoint;

    if (m_HolyItemPlayerIndex != 0xffff && data->btUserIndex == 0xffff)
    {
        DeleteEffect(MODEL_CURSEDTEMPLE_HOLYITEM);
    }

    if (data->btUserIndex != 0xffff)
    {
        WORD holyitemkey = data->btUserIndex;
        WORD holyitemcharacterindex = FindCharacterIndex(holyitemkey);

        if (holyitemcharacterindex != MAX_CHARACTERS_CLIENT)
        {
            CHARACTER* c = &CharactersClient[holyitemcharacterindex];
            OBJECT* o = &c->Object;
            OBJECT* ho = &Hero->Object;

            if (o->Live && !SearchEffect(MODEL_CURSEDTEMPLE_HOLYITEM, o))
            {
                PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM3);

                vec3_t tempPosition, p;
                Vector(70.f, 0.f, 0.f, p);
                BMD* b = &Models[o->Type];
                VectorCopy(o->Position, b->BodyOrigin);

                float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);

                float fSpeedPerFrame = fActionSpeed / 10.f;

                float fAnimationFrame = o->AnimationFrame - fActionSpeed;

                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                b->TransformPosition(BoneTransform[20], p, tempPosition, true);
                CreateEffect(MODEL_CURSEDTEMPLE_HOLYITEM, tempPosition, o->Angle, o->Light, 0, o);
            }
        }
    }

    m_HolyItemPlayerIndex = data->btUserIndex;
}