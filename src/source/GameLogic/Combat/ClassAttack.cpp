#include "stdafx.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/CombatTarget.h"
#include "GameLogic/Combat/SkillCast.h"
#include "Character/CharacterManager.h" // gCharacterManager
#include "GameLogic/Buffs/w_Buff.h"     // UnRegisterBuff

// Includes mirror ZzzInterface.cpp, the unit these functions were extracted from.
#include "Core/Platform/Imm.h"
#include "UI/Legacy/UIManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/PlayerActionState.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzInterface.h"
#include "Core/Input/ImeInput.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Effects/ZzzEffect.h"
#include "Scenes/SceneCore.h"
#include "Engine/Pathing/ZzzPath.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include "GameLogic/Events/MatchEvent.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Character/CSParts.h"
#include "UI/Legacy/UIMapName.h"	// rozy
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Events/Event.h"
#include "UI/NewUI/NewUISystem.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "UI/Legacy/UIControls.h"
#include "GameLogic/Social/PartyManager.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "World/MapInfra/w_MapHeaders.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "UI/NewUI/HUD/NewUIGensRanking.h"
#include "GameLogic/Social/MonkSystem.h"

// File-scope state still owned by ZzzInterface.cpp (no shared header yet).
extern void SetPlayerBow(CHARACTER* c);
void UnRegisterBuff(eBuffState buff, OBJECT* o); // free function defined in ZzzCharacter.cpp
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter, SelectedNpc, SelectedItem, SelectedOperate;
extern int Attacking;
extern int TargetX, TargetY, TargetType, TargetNpc;
extern float TargetAngle;
extern bool EnableFastInput;
extern int MouseUpdateTime, MouseUpdateTimeMax;
extern int g_iFollowCharacter;
extern int StandTime;
extern BOOL g_bWhileMovingZone;
extern DWORD g_dwLatestZoneMoving;

namespace GameLogic::Combat
{
void AttackElf(CHARACTER* c, int Skill, float Distance)
{
    OBJECT* o = &c->Object;
    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
    if (g_isCharacterBuff(o, eBuff_InfinityArrow))
        iMana += CharacterMachine->InfinityArrowAdditionalMana;
    if (iMana > CharacterAttribute->Mana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();

        if (Index != -1)
        {
            SendRequestUse(Index, 0);
        }
        return;
    }

    if (iSkillMana > CharacterAttribute->SkillMana)
    {
        return;
    }
    if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
    {
        return;
    }

    int iEnergy;
    gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
    if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
    {
        return;
    }

    bool Success = CheckTarget(c);

    switch (Skill)
    {
    case AT_SKILL_SUMMON:
    case AT_SKILL_SUMMON + 1:
    case AT_SKILL_SUMMON + 2:
    case AT_SKILL_SUMMON + 3:
    case AT_SKILL_SUMMON + 4:
    case AT_SKILL_SUMMON + 5:
    case AT_SKILL_SUMMON + 6:
#ifdef ADD_ELF_SUMMON
    case AT_SKILL_SUMMON + 7:
#endif // ADD_ELF_SUMMON
        if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.InChaosCastle() == false)
            if (!g_Direction.m_CKanturu.IsMayaScene())
            {
                SendRequestMagic(Skill, HeroKey);
                SetPlayerMagic(c);
            }
        return;

    case AT_SKILL_PENETRATION:
    case AT_SKILL_PENETRATION_STR:
        if ((o->Type == MODEL_PLAYER) && (gCharacterManager.GetEquipedBowType(Hero) != BOWTYPE_NONE))
        {
            if (!CheckArrow())
                break;
            if (CheckTile(c, o, Distance))
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                SetPlayerAttack(c);
                if (o->Type != MODEL_PLAYER)
                {
                    CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 0, (c->Skill));
                }
            }
        }
        break;

    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
        {
            if (CharactersClient[SelectedCharacter].Object.Kind != KIND_PLAYER)
            {
                Attacking = -1;
                return;
            }

            if (c != Hero && !g_pPartyManager->IsPartyMember(SelectedCharacter))
                return;

            c->TargetCharacter = SelectedCharacter;

            ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
            g_MovementSkill.m_bMagic = TRUE;
            g_MovementSkill.m_iSkill = Hero->CurrentSkill;
            g_MovementSkill.m_iTarget = SelectedCharacter;

            if (!CheckTile(c, o, Distance))
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                {
                    c->Movement = true;
                    c->MovementType = MOVEMENT_SKILL;
                    SendMove(c, o);
                }
                return;
            }

            SendRequestMagic(Skill, CharactersClient[g_MovementSkill.m_iTarget].Key);
        }
        else
        {
            SendRequestMagic(Skill, HeroKey);
        }
        SetPlayerMagic(c);
        return;
    }
    if (SelectedCharacter != -1)
    {
        ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
        g_MovementSkill.m_bMagic = TRUE;
        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
        g_MovementSkill.m_iTarget = CheckAttack() ? SelectedCharacter : -1;
    }
    if (!CheckTile(c, o, Distance))
    {
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && (
            Skill == AT_SKILL_HEALING
            || Skill == AT_SKILL_HEALING_STR
            || Skill == AT_SKILL_ATTACK
            || Skill == AT_SKILL_ATTACK_STR
            || Skill == AT_SKILL_ATTACK_MASTERY
            || Skill == AT_SKILL_DEFENSE
            || Skill == AT_SKILL_DEFENSE_STR
            || Skill == AT_SKILL_DEFENSE_MASTERY
            || CheckAttack()
            || Skill == AT_SKILL_FIRE_SCREAM
            || Skill == AT_SKILL_FIRE_SCREAM_STR
            )
            )
        {
            if (CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                {
                    c->Movement = true;
                    c->MovementType = MOVEMENT_SKILL;
                    SendMove(c, o);
                }
            }
        }

        if (Skill != AT_SKILL_STUN && Skill != AT_SKILL_REMOVAL_STUN && Skill != AT_SKILL_MANA && Skill != AT_SKILL_INVISIBLE && Skill != AT_SKILL_REMOVAL_INVISIBLE
            && Skill != AT_SKILL_PLASMA_STORM_FENRIR
            )
            return;
    }
    bool Wall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
    if (Wall)
    {
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
        {
            if (CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                switch (Skill)
                {
                case AT_SKILL_HEALING:
                case AT_SKILL_HEALING_STR:
                case AT_SKILL_ATTACK:
                case AT_SKILL_ATTACK_STR:
                case AT_SKILL_ATTACK_MASTERY:
                case AT_SKILL_DEFENSE:
                case AT_SKILL_DEFENSE_STR:
                case AT_SKILL_DEFENSE_MASTERY:
                    UseSkillElf(c, o);
                    return;
                }
            }
            if (CheckAttack())
            {
                if (((Skill == AT_SKILL_ICE_ARROW) || (Skill == AT_SKILL_ICE_ARROW_STR) || (Skill == AT_SKILL_DEEPIMPACT)) && ((o->Type == MODEL_PLAYER) && (gCharacterManager.GetEquipedBowType(Hero) != BOWTYPE_NONE)))
                {
                    UseSkillElf(c, o);
                }
            }
        }
    }

    switch (Skill)
    {
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR:
    {
        if (g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow) == false)
        {
            SendRequestMagic(Skill, HeroKey);
            if ((c->Helper.Type == MODEL_HORN_OF_FENRIR)
                || (c->Helper.Type == MODEL_HORN_OF_UNIRIA)
                || (c->Helper.Type == MODEL_HORN_OF_DINORANT)
                || (c->Helper.Type == MODEL_DARK_HORSE_ITEM))
                SetPlayerMagic(c);
            else
                SetAction(o, PLAYER_RUSH1);

            c->Movement = 0;
        }
    }
    break;

    case AT_SKILL_HELLOWIN_EVENT_1:
    case AT_SKILL_HELLOWIN_EVENT_2:
    case AT_SKILL_HELLOWIN_EVENT_3:
    case AT_SKILL_HELLOWIN_EVENT_4:
    case AT_SKILL_HELLOWIN_EVENT_5:
        SendRequestMagic(Skill, HeroKey);
        //					SetPlayerMagic(c);
        c->Movement = 0;
        break;
    case AT_SKILL_RECOVER:
    {
        vec3_t Light, Position, P, dp;

        float Matrix[3][4];

        Vector(0.f, -220.f, 130.f, P);
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(P, Matrix, dp);
        VectorAdd(dp, o->Position, Position);
        Vector(0.7f, 0.6f, 0.f, Light);
        CreateEffect(BITMAP_IMPACT, Position, o->Angle, Light, 0, o);
        SetAction(o, PLAYER_RECOVER_SKILL);

        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
        {
            SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
        }
        else
        {
            SendRequestMagic(Skill, HeroKey);
        }
        //			UseSkillElf( c, o);
    }
    break;
    case AT_SKILL_MULTI_SHOT:
    {
        if (!CheckArrow())
            break;

        if (gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
            return;
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

        if (CheckTile(c, o, Distance))
        {
            BYTE PathX[1];
            BYTE PathY[1];
            PathX[0] = (c->PositionX);
            PathY[0] = (c->PositionY);

            SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

            BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

            BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
            WORD TKey = 0xffff;
            if (g_MovementSkill.m_iTarget != -1)
            {
                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
            }

            SendRequestMagicContinue(Skill, (c->PositionX),
                (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
            SetAttackSpeed();
            //									SetAction(o, PLAYER_SKILL_FLAMESTRIKE);
            c->Movement = 0;
            //c->AttackTime = 15;

            SetPlayerBow(c);
            vec3_t Light, Position, P, dp;

            float Matrix[3][4];
            Vector(0.f, 20.f, 0.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);
            Vector(0.8f, 0.9f, 1.6f, Light);
            CreateEffect(MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);

            Vector(0.f, -20.f, 0.f, P);
            Vector(0.f, 0.f, 0.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);

            CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);

            Vector(0.f, 20.f, 0.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);
            CreateEffect(MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);

            Vector(0.f, -120.f, 145.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);

            short Key = -1;
            for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* tc = &CharactersClient[i];
                if (tc == c)
                {
                    Key = i;
                    break;
                }
            }

            CreateEffect(MODEL_BLADE_SKILL, Position, o->Angle, Light, 1, o, Key);
            PlayBuffer(SOUND_SKILL_MULTI_SHOT);
        }
    }
    break;
    case AT_SKILL_IMPROVE_AG:
        SendRequestMagic(Skill, HeroKey);
        SetPlayerMagic(c);
        c->Movement = 0;
        break;
    case AT_SKILL_PLASMA_STORM_FENRIR:
    {
        if (CheckAttack())
        {
            g_MovementSkill.m_iTarget = SelectedCharacter;
        }
        else
        {
            g_MovementSkill.m_iTarget = -1;
        }

        int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
        int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
        if (CheckTile(c, o, Distance))
        {
            BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
            WORD TKey = 0xffff;
            if (g_MovementSkill.m_iTarget != -1)
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                c->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, &o->m_bySkillSerialNum);
                c->Movement = 0;

                if (o->Type == MODEL_PLAYER)
                {
                    SetAction_Fenrir_Skill(c, o);
                }
            }
            else
            {
                c->m_iFenrirSkillTarget = -1;
            }
        }
        else
        {
            if (g_MovementSkill.m_iTarget != -1)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                {
                    c->Movement = true;
                }
            }
            else
            {
                Attacking = -1;
            }
        }
    }
    break;
    }

    c->SkillSuccess = true;
}

void AttackKnight(CHARACTER* c, ActionSkillType Skill, float Distance)
{
    OBJECT* o = &c->Object;
    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana,
        NULL, &iSkillMana);

    int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
    int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

    if (((iTypeR != -1 &&
        (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
        (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX))
        || Skill == AT_SKILL_SWELL_LIFE
        || Skill == AT_SKILL_SWELL_LIFE_STR
        || Skill == AT_SKILL_SWELL_LIFE_PROFICIENCY
        || Skill == AT_SKILL_ADD_CRITICAL
        || Skill == AT_SKILL_ADD_CRITICAL_STR1
        || Skill == AT_SKILL_ADD_CRITICAL_STR2
        || Skill == AT_SKILL_ADD_CRITICAL_STR3
        || Skill == AT_SKILL_PARTY_TELEPORT
        || Skill == AT_SKILL_THUNDER_STRIKE
        || Skill == AT_SKILL_EARTHSHAKE
        || Skill == AT_SKILL_EARTHSHAKE_STR
        || Skill == AT_SKILL_EARTHSHAKE_MASTERY
        || Skill == AT_SKILL_BRAND_OF_SKILL
        || Skill == AT_SKILL_PLASMA_STORM_FENRIR
        || Skill == AT_SKILL_FIRE_SCREAM
        || Skill == AT_SKILL_FIRE_SCREAM_STR
        || Skill == AT_SKILL_GIGANTIC_STORM
        || Skill == AT_SKILL_CHAOTIC_DISEIER
        || Skill == AT_SKILL_TWISTING_SLASH
        || Skill == AT_SKILL_TWISTING_SLASH_STR
        || Skill == AT_SKILL_TWISTING_SLASH_STR_MG
        || Skill == AT_SKILL_TWISTING_SLASH_MASTERY
        ))
    {
        bool Success = true;

        if (Skill == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
        {
            Success = false;
        }

        if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(Skill))
        {
            Success = false;
        }

        if (Skill == AT_SKILL_PARTY_TELEPORT && g_DuelMgr.IsDuelEnabled())
        {
            Success = false;
        }

        if (Skill == AT_SKILL_PARTY_TELEPORT && (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
        {
            Success = false;
        }

        if (Skill == AT_SKILL_EARTHSHAKE || Skill == AT_SKILL_EARTHSHAKE_STR || Skill == AT_SKILL_EARTHSHAKE_MASTERY)
        {
            BYTE t_DarkLife = 0;
            t_DarkLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
            if (t_DarkLife == 0) Success = false;
        }

        if (gMapManager.InChaosCastle())
        {
            if (Skill == AT_SKILL_EARTHSHAKE
                || Skill == AT_SKILL_EARTHSHAKE_STR
                || Skill == AT_SKILL_EARTHSHAKE_MASTERY
                || Skill == AT_SKILL_RIDER
                || (static_cast<int>(Skill) >= static_cast<int>(AT_PET_COMMAND_DEFAULT) && static_cast<int>(Skill) <= static_cast<int>(AT_PET_COMMAND_TARGET))
                || Skill == AT_SKILL_PLASMA_STORM_FENRIR
                )
            {
                Success = false;
            }
        }
        else
        {
            if (Skill == AT_SKILL_EARTHSHAKE || Skill == AT_SKILL_EARTHSHAKE_STR || Skill == AT_SKILL_EARTHSHAKE_MASTERY)
            {
                BYTE t_DarkLife = 0;
                t_DarkLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
                if (t_DarkLife == 0) Success = false;
            }
        }

        if (iMana > CharacterAttribute->Mana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();
            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            Success = false;
        }
        if (Success && iSkillMana > CharacterAttribute->SkillMana)
        {
            Success = false;
        }
        if (Success && !gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
        {
            Success = false;
        }

        int iEnergy;
        gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
        if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
        {
            Success = false;
        }
        if (ClassIndex == CLASS_DARK_LORD)
        {
            int iCharisma;
            gSkillManager.GetSkillInformation_Charisma(Skill, &iCharisma);
            if (iCharisma > (CharacterAttribute->Charisma + CharacterAttribute->AddCharisma))
            {
                Success = false;
            }
        }

        if (Success)
        {
            switch (Skill)
            {
            case AT_SKILL_TWISTING_SLASH:
            case AT_SKILL_TWISTING_SLASH_STR:
            case AT_SKILL_TWISTING_SLASH_STR_MG:
            case AT_SKILL_TWISTING_SLASH_MASTERY:
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                    SetAttackSpeed();
                    SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);

                    c->Movement = 0;
                }
                break;
            case AT_SKILL_FIRE_SLASH:
            case AT_SKILL_FIRE_SLASH_STR:
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                {
                    WORD Strength;
                    const WORD notStrength = 596;
                    Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
                    if (Strength < notStrength)
                    {
                        break;
                    }

                    if (CheckTile(c, o, Distance))
                    {
                        BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                        WORD TKey = 0xffff;
                        if (g_MovementSkill.m_iTarget != -1)
                        {
                            TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        }
                        SendRequestMagicContinue(Skill, (c->PositionX),
                            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, 0, TKey, 0);
                        SetAttackSpeed();

                        SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);
                        c->Movement = 0;
                    }
                }
                break;
            case AT_SKILL_POWER_SLASH:
            case AT_SKILL_POWER_SLASH_STR:
                if (c->Helper.Type<MODEL_HORN_OF_UNIRIA || c->Helper.Type>MODEL_DARK_HORSE_ITEM && c->Helper.Type != MODEL_HORN_OF_FENRIR)
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                    if (CheckTile(c, o, Distance))
                    {
                        BYTE PathX[1];
                        BYTE PathY[1];
                        PathX[0] = (c->PositionX);
                        PathY[0] = (c->PositionY);

                        SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                        BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                        BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                        WORD TKey = 0xffff;
                        if (g_MovementSkill.m_iTarget != -1)
                        {
                            TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        }
                        SendRequestMagicContinue(Skill, (c->PositionX),
                            (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                        SetAttackSpeed();
                        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
                        }
                        else
                        {
                            SetAction(o, PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
                        }
                        c->Movement = 0;
                    }
                }
                break;
            case AT_SKILL_CHAOTIC_DISEIER:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                if (CheckTile(c, o, Distance))
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        if (TKey == 0xffff)
                        {
                            CHARACTER* st = &CharactersClient[g_MovementSkill.m_iTarget];
                            TKey = st->Key;
                        }
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetAttackSpeed();
                    //									SetAction(o, PLAYER_SKILL_FLAMESTRIKE);
                    c->Movement = 0;

                    //									SetPlayerMagic(c);

                    if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
                    {
                        SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
                    }
                    else if ((c->Helper.Type >= MODEL_HORN_OF_UNIRIA) && (c->Helper.Type <= MODEL_DARK_HORSE_ITEM))
                    {
                        SetAction(o, PLAYER_ATTACK_RIDE_STRIKE);
                    }
                    else
                    {
                        SetAction(o, PLAYER_ATTACK_STRIKE);
                    }

                    vec3_t Light, Position, P, dp;

                    float Matrix[3][4];
                    Vector(0.f, -20.f, 0.f, P);
                    Vector(0.f, 0.f, 0.f, P);
                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(P, Matrix, dp);
                    VectorAdd(dp, o->Position, Position);

                    Vector(0.5f, 0.5f, 0.5f, Light);
                    for (int i = 0; i < 5; ++i)
                    {
                        CreateEffect(BITMAP_SHINY + 6, Position, o->Angle, Light, 3, o, -1, 0, 0, 0, 0.5f);
                    }

                    VectorCopy(o->Position, Position);

                    for (int i = 0; i < 8; i++)
                    {
                        Position[0] = (o->Position[0] - 119.f) + (float)(rand() % 240);
                        Position[2] = (o->Position[2] + 49.f) + (float)(rand() % 60);
                        CreateJoint(BITMAP_2LINE_GHOST, Position, o->Position, o->Angle, 0, o, 20.f, o->PKKey, 0, o->m_bySkillSerialNum);
                    }
                    if (c == Hero && SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                    {
                        vec3_t Pos;
                        CHARACTER* sc = &CharactersClient[SelectedCharacter];
                        VectorCopy(sc->Object.Position, Pos);
                        CreateBomb(Pos, true, 6);
                    }
                    PlayBuffer(SOUND_SKILL_CAOTIC);
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
            break;
            case AT_SKILL_FLAME_STRIKE:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                if (CheckTile(c, o, Distance))
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetAttackSpeed();
                    SetAction(o, PLAYER_SKILL_FLAMESTRIKE);
                    c->Movement = 0;
                    //c->AttackTime = 15;
                }
            }
            break;
            case AT_SKILL_GIGANTIC_STORM:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                if (CheckTile(c, o, Distance))
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetAttackSpeed();
                    SetAction(o, PLAYER_SKILL_GIGANTICSTORM);
                    c->Movement = 0;
                }
            }
            break;
            case AT_SKILL_PARTY_TELEPORT:
                if (gMapManager.IsCursedTemple()
                    && !g_pMyInventory->IsItem(ITEM_POTION + 64, true))
                {
                    return;
                }
                SendRequestMagic(Skill, HeroKey);
                c->Movement = 0;
                break;

            case AT_SKILL_ADD_CRITICAL:
            case AT_SKILL_ADD_CRITICAL_STR1:
            case AT_SKILL_ADD_CRITICAL_STR2:
            case AT_SKILL_ADD_CRITICAL_STR3:
                SendRequestMagic(Skill, HeroKey);
                c->Movement = 0;
                break;
            case AT_SKILL_BRAND_OF_SKILL:
                SendRequestMagic(Skill, HeroKey);
                c->Movement = 0;
                break;
            case AT_SKILL_FIRE_SCREAM:
            case AT_SKILL_FIRE_SCREAM_STR:
                if (CheckTile(c, o, Distance))
                {
                    int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                    int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    CheckClientArrow(o);
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, pos, TKey, 0);

                    SetAttackSpeed();
                    {
                        if ((c->Helper.Type >= MODEL_HORN_OF_UNIRIA && c->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_ATTACK_RIDE_STRIKE);
                        }
                        else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
                        }
                        else
                        {
                            SetAction(o, PLAYER_ATTACK_STRIKE);
                        }
                    }
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
                break;
            case AT_SKILL_EARTHSHAKE:
            case AT_SKILL_EARTHSHAKE_STR:
            case AT_SKILL_EARTHSHAKE_MASTERY:
                if (c->Helper.Type != MODEL_DARK_HORSE_ITEM || c->SafeZone)
                {
                    break;
                }

            case AT_SKILL_THUNDER_STRIKE:
                if (CheckTile(c, o, Distance))
                {
                    int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                    int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, pos, TKey, 0);
                    SetAttackSpeed();

                    if (Skill == AT_SKILL_THUNDER_STRIKE)
                    {
                        if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_FLASH);
                        }
                        else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_FLASH);
                        }
                        else
                        {
                            SetAction(o, PLAYER_SKILL_FLASH);
                        }
                    }
                    else if (Skill == AT_SKILL_EARTHSHAKE || Skill == AT_SKILL_EARTHSHAKE_STR || Skill == AT_SKILL_EARTHSHAKE_MASTERY)
                    {
                        SetAction(o, PLAYER_ATTACK_DARKHORSE);
                        PlayBuffer(SOUND_EARTH_QUAKE);
                    }
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
                break;
            case AT_SKILL_SWELL_LIFE:
            case AT_SKILL_SWELL_LIFE_STR:
            case AT_SKILL_SWELL_LIFE_PROFICIENCY:
                SendRequestMagic(Skill, HeroKey);
                SetAction(o, PLAYER_SKILL_VITALITY);
                c->Movement = 0;
                break;
            case AT_SKILL_RAGEFUL_BLOW:
            case AT_SKILL_RAGEFUL_BLOW_STR:
            case AT_SKILL_RAGEFUL_BLOW_MASTERY:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);

                if (CheckTile(c, o, Distance))
                {
                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, 0);
                    SetAction(o, PLAYER_ATTACK_SKILL_FURY_STRIKE);
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
            }
            break;
            case AT_SKILL_STRIKE_OF_DESTRUCTION:
            case AT_SKILL_STRIKE_OF_DESTRUCTION_STR:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance))
                {
                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, TargetX, TargetY, (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, 0);
                    SetAction(o, PLAYER_SKILL_BLOW_OF_DESTRUCTION);
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
            }
            break;
            case AT_SKILL_PLASMA_STORM_FENRIR:
            {
                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance))
                {
                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                        gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        c->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
                        SendRequestMagicContinue(Skill, (c->PositionX),
                            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, &o->m_bySkillSerialNum);
                        c->Movement = 0;

                        if (o->Type == MODEL_PLAYER)
                        {
                            SetAction_Fenrir_Skill(c, o);
                        }
                    }
                    else
                    {
                        c->m_iFenrirSkillTarget = -1;
                    }
                }
                else
                {
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                        {
                            c->Movement = true;
                        }
                    }
                    else
                    {
                        Attacking = -1;
                    }
                }
            }
            break;
            }
        }
    }

    c->SkillSuccess = true;
}

void AttackWizard(CHARACTER* c, int Skill, float Distance)
{
    OBJECT* o = &c->Object;
    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    if (Skill == AT_SKILL_NOVA_BEGIN || Skill == AT_SKILL_NOVA)
    {
        gSkillManager.GetSkillInformation(AT_SKILL_NOVA, 1, NULL, &iMana, NULL, &iSkillMana);

        if (Skill == AT_SKILL_NOVA)
        {
            iSkillMana = 0;
        }
    }
    else
    {
        gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
    }

    int iEnergy;
    gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
    if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
    {
        return;
    }

    if (iMana > CharacterAttribute->Mana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();
        if (Index != -1)
        {
            SendRequestUse(Index, 0);
        }
        return;
    }

    if (iSkillMana > CharacterAttribute->SkillMana)
    {
        if (Skill == AT_SKILL_NOVA_BEGIN || Skill == AT_SKILL_NOVA)
        {
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            MouseRButton = false;

            MouseRButtonPress = 0;
        }
        return;
    }

    if (gSkillManager.CheckSkillDelay(Hero->CurrentSkill) == false)
    {
        return;
    }
    bool Success = CheckTarget(c);

    switch (Skill)
    {
    case AT_SKILL_NOVA_BEGIN:
    {
        SendRequestMagic(Skill, HeroKey);

        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_HELL_BEGIN);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_NOVA:
    {
        int iTargetKey = getTargetCharacterKey(c, SelectedCharacter);
        if (iTargetKey == -1) {
            iTargetKey = HeroKey;
        }
        SendRequestMagic(Skill, iTargetKey);

        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_HELL_START);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
        {
            if (CharactersClient[SelectedCharacter].Object.Kind != KIND_PLAYER)
            {
                Attacking = -1;
                return;
            }
            else
            {
                if (c != Hero && !g_pPartyManager->IsPartyMember(SelectedCharacter))
                    return;

                c->TargetCharacter = SelectedCharacter;

                if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                {
                    ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
                    g_MovementSkill.m_bMagic = TRUE;
                    g_MovementSkill.m_iSkill = Hero->CurrentSkill;
                    g_MovementSkill.m_iTarget = SelectedCharacter;
                }

                if (!CheckTile(c, o, Distance))
                {
                    if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                        {
                            c->Movement = true;
                            c->MovementType = MOVEMENT_SKILL;
                            SendMove(c, o);
                        }
                    }

                    return;
                }

                SendRequestMagic(Skill, CharactersClient[g_MovementSkill.m_iTarget].Key);
            }
        }
        else
        {
            SendRequestMagic(Skill, HeroKey);
        }
        SetPlayerMagic(c);
        break;
    case AT_SKILL_HELL_FIRE:
    case AT_SKILL_HELL_FIRE_STR:
    {
        SendRequestMagicContinue(Skill, (c->PositionX),
            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_HELL);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_INFERNO:
    case AT_SKILL_INFERNO_STR:
    case AT_SKILL_INFERNO_STR_MG:
    {
        SendRequestMagicContinue(Skill, (c->PositionX),
            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_INFERNO);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_PLASMA_STORM_FENRIR:
    {
        if (CheckAttack())
        {
            g_MovementSkill.m_iTarget = SelectedCharacter;
        }
        else
        {
            g_MovementSkill.m_iTarget = -1;
        }

        int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
        int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
        if (CheckTile(c, o, Distance))//&& CheckAttack())
        {
            BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
            WORD TKey = 0xffff;
            if (g_MovementSkill.m_iTarget != -1)
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                c->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, &o->m_bySkillSerialNum);
                c->Movement = 0;

                if (o->Type == MODEL_PLAYER)
                {
                    SetAction_Fenrir_Skill(c, o);
                }
            }
            else
            {
                c->m_iFenrirSkillTarget = -1;
            }
        }
        else
        {
            if (g_MovementSkill.m_iTarget != -1)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                {
                    c->Movement = true;
                }
            }
            else
            {
                Attacking = -1;
            }
        }
    }
    return;
    case AT_SKILL_BLOCKING:
    case AT_SKILL_FALLING_SLASH:
    case AT_SKILL_FALLING_SLASH_STR:
    case AT_SKILL_LUNGE:
    case AT_SKILL_LUNGE_STR:
    case AT_SKILL_UPPERCUT:
    case AT_SKILL_CYCLONE:
    case AT_SKILL_CYCLONE_STR:
    case AT_SKILL_CYCLONE_STR_MG:
    case AT_SKILL_SLASH:
    case AT_SKILL_SLASH_STR:
    case AT_SKILL_IMPALE:
        return;
    case AT_SKILL_EXPANSION_OF_WIZARDRY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_STR:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY:
    {
        if (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower) == false)
        {
            SendRequestMagic(Skill, HeroKey);

            c->Movement = 0;
        }
    }break;
    }

    if (SelectedCharacter != -1)
    {
        ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
        g_MovementSkill.m_bMagic = TRUE;
        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
        if (CheckAttack())
        {
            g_MovementSkill.m_iTarget = SelectedCharacter;
        }
        else
        {
            g_MovementSkill.m_iTarget = -1;
        }
    }

    if (!CheckTile(c, o, Distance))
    {
        if (SelectedCharacter != -1 && CheckAttack())
        {
            if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
            {
                c->Movement = true;
                c->MovementType = MOVEMENT_SKILL;
                SendMove(c, o);
            }
        }

        if (Skill != AT_SKILL_STUN && Skill != AT_SKILL_REMOVAL_STUN
            && Skill != AT_SKILL_MANA && Skill != AT_SKILL_INVISIBLE
            && Skill != AT_SKILL_REMOVAL_INVISIBLE && Skill != AT_SKILL_PLASMA_STORM_FENRIR
            && Skill != AT_SKILL_ALICE_BERSERKER
            && Skill != AT_SKILL_ALICE_BERSERKER_STR
            && Skill != AT_SKILL_ALICE_WEAKNESS && Skill != AT_SKILL_ALICE_ENERVATION
            )
            return;
    }

    bool Wall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
    if (Wall)
    {
        if (SelectedCharacter != -1)
        {
            if (CheckAttack())
            {
                UseSkillWizard(c, o);
            }
        }
        if (Success)
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
            switch (Skill)
            {
            case AT_SKILL_STORM:
            case AT_SKILL_EVIL_SPIRIT:
            case AT_SKILL_EVIL_SPIRIT_STR:
            case AT_SKILL_EVIL_SPIRIT_STR_MG:
            {
                SendRequestMagicContinue(Skill, (c->PositionX), (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, &o->m_bySkillSerialNum);
                SetPlayerMagic(c);
            }
            return;
            case AT_SKILL_FLASH:
            {
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
                SetAttackSpeed();

                if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                    SetAction(o, PLAYER_SKILL_FLASH);
                else
                    SetAction(o, PLAYER_SKILL_FLASH);

                c->Movement = 0;
                StandTime = 0;
            }
            return;
            case AT_SKILL_FLAME:
            case AT_SKILL_FLAME_STR:
            case AT_SKILL_FLAME_STR_MG:
                SendRequestMagicContinue(Skill, (BYTE)(c->TargetPosition[0] / TERRAIN_SCALE), (BYTE)(c->TargetPosition[1] / TERRAIN_SCALE), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
                SetPlayerMagic(c);
                return;
            case AT_SKILL_DECAY:
            case AT_SKILL_DECAY_STR:
            case AT_SKILL_ICE_STORM:
            {
                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (BYTE)(c->TargetPosition[0] / TERRAIN_SCALE), (BYTE)(c->TargetPosition[1] / TERRAIN_SCALE), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                SetPlayerMagic(c);
                c->Movement = 0;
            }
            return;
            }
        }
    }
    if (ClassIndex == CLASS_WIZARD && Success)
    {
        switch (Skill)
        {
        case AT_SKILL_TELEPORT_ALLY:
            if (gMapManager.IsCursedTemple()
                && !g_pMyInventory->IsItem(ITEM_POTION + 64, true))
            {
                return;
            }
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
            {
                if (!g_pPartyManager->IsPartyMember(SelectedCharacter))
                    return;

                if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
                {
                    return;
                }

                CHARACTER* tc = &CharactersClient[SelectedCharacter];
                OBJECT* to = &tc->Object;
                bool Success = false;
                if (to->Type == MODEL_PLAYER)
                {
                    if (to->CurrentAction != PLAYER_SKILL_TELEPORT)
                        Success = true;
                }
                else
                {
                    if (to->CurrentAction != MONSTER01_SHOCK)
                        Success = true;
                }
                if (Success && to->Teleport != TELEPORT_BEGIN && to->Teleport != TELEPORT && to->Alpha >= 0.7f)
                {
                    int Wall, indexX, indexY, TargetIndex, count = 0;
                    int PositionX = (int)(c->Object.Position[0] / TERRAIN_SCALE);
                    int PositionY = (int)(c->Object.Position[1] / TERRAIN_SCALE);

                    while (1)
                    {
                        indexX = rand() % 3;
                        indexY = rand() % 3;

                        if (indexX != 1 || indexY != 1)
                        {
                            TargetX = (PositionX - 1) + indexX;
                            TargetY = (PositionY - 1) + indexY;

                            TargetIndex = TERRAIN_INDEX(TargetX, TargetY);

                            Wall = TerrainWall[TargetIndex];

                            if ((Wall & TW_ACTION) == TW_ACTION)
                            {
                                Wall -= TW_ACTION;
                            }
                            if (gMapManager.WorldActive == WD_30BATTLECASTLE)
                            {
                                int ax = (Hero->PositionX);
                                int ay = (Hero->PositionY);
                                if ((ax >= 150 && ax <= 200) && (ay >= 180 && ay <= 230))
                                    Wall = 0;
                            }
                            if (Wall == 0) break;

                            count++;
                        }

                        if (count > 10) return;
                    }
                    to->Angle[2] = CreateAngle2D(to->Position, tc->TargetPosition);

                    SocketClient->ToGameServer()->SendTeleportTarget(tc->Key, TargetX, TargetY);
                    SetPlayerTeleport(tc);
                }
            }
            return;

        case AT_SKILL_TELEPORT:
        {
            if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem()
                || g_isCharacterBuff(o, eDeBuff_Stun)
                || g_isCharacterBuff(o, eDeBuff_Sleep)
                )
            {
                return;
            }

            WORD byHeroPriorSkill = g_pSkillList->GetHeroPriorSkill();
            if (c == Hero && byHeroPriorSkill == AT_SKILL_NOVA)
            {
                g_pSkillList->SetHeroPriorSkill(AT_SKILL_TELEPORT);
                SendRequestMagic(byHeroPriorSkill, HeroKey);
                SetAttackSpeed();
                SetAction(&Hero->Object, PLAYER_SKILL_HELL_BEGIN);
                Hero->Movement = 0;
                return;
            }

            bool Success = false;
            if (o->Type == MODEL_PLAYER)
            {
                if (o->CurrentAction != PLAYER_SKILL_TELEPORT)
                    Success = true;
            }
            else
            {
                if (o->CurrentAction != MONSTER01_SHOCK)
                    Success = true;
            }
            if (Success && o->Teleport != TELEPORT_BEGIN && o->Teleport != TELEPORT && o->Alpha >= 0.7f)
            {
                int TargetIndex = TERRAIN_INDEX_REPEAT(TargetX, TargetY);
                int Wall = TerrainWall[TargetIndex];
                if ((Wall & TW_ACTION) == TW_ACTION) Wall -= TW_ACTION;
                if ((Wall & TW_HEIGHT) == TW_HEIGHT) Wall -= TW_HEIGHT;
                if (Wall == 0)
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    bool bResult;
                    //SendRequestMagicTeleport(&bResult, 0, TargetX, TargetY);
                    if (g_bWhileMovingZone || (GetTickCount() - g_dwLatestZoneMoving < 3000))\
                    {
                        bResult = false;
                    }
                    else
                    {
                        SocketClient->ToGameServer()->SendEnterGateRequest(0, TargetX, TargetY);
                        bResult = true;
                    }

                    if (bResult)
                    {
                        if (g_isCharacterBuff(o, eDeBuff_Stun))
                        {
                            UnRegisterBuff(eDeBuff_Stun, o);
                        }

                        if (g_isCharacterBuff(o, eDeBuff_Sleep))
                        {
                            UnRegisterBuff(eDeBuff_Sleep, o);
                        }

                        SetPlayerTeleport(c);
                    }
                }
            }
        }
        return;
        }
    }
    else if (ClassIndex == CLASS_SUMMONER && Success)
    {
        if (g_SummonSystem.SendRequestSummonSkill(Skill, c, o) == TRUE)
        {
            return;
        }

        int iEnergy;
        gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
        if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
        {
            return;
        }

        g_MovementSkill.m_bMagic = TRUE;
        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
        g_MovementSkill.m_iTarget = CheckAttack() ? SelectedCharacter : -1;

        switch (Skill)
        {
        case AT_SKILL_ALICE_THORNS:
        {
            if (SelectedCharacter == -1 || CharactersClient[SelectedCharacter].Object.Kind != KIND_PLAYER)
            {
                LetHeroStop();

                switch (c->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(o, PLAYER_SKILL_SLEEP_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(o, PLAYER_SKILL_SLEEP_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(o, PLAYER_SKILL_SLEEP_FENRIR);
                    break;
                default:
                    SetAction(o, PLAYER_SKILL_SLEEP);
                    break;
                }
                SendRequestMagic(Skill, HeroKey);
            }

            else if (0 == CharactersClient[SelectedCharacter].Dead && CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);

                if (CheckTile(c, o, Distance) && c->SafeZone == false)
                {
                    bool bNoneWall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
                    if (bNoneWall)
                    {
                        UseSkillSummon(c, o);
                    }
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
        }
        break;
        case AT_SKILL_ALICE_BERSERKER:
        case AT_SKILL_ALICE_BERSERKER_STR:
        case AT_SKILL_ALICE_WEAKNESS:
        case AT_SKILL_ALICE_ENERVATION:
            UseSkillSummon(c, o);
            break;
        case AT_SKILL_LIGHTNING_SHOCK:
        case AT_SKILL_LIGHTNING_SHOCK_STR:
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

            if (CheckTile(c, o, Distance))
            {
                BYTE PathX[1];
                BYTE PathY[1];
                PathX[0] = (c->PositionX);
                PathY[0] = (c->PositionY);
                SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                SetAttackSpeed();
                SetAction(o, PLAYER_SKILL_LIGHTNING_SHOCK);
                c->Movement = 0;
            }
        }
        break;
        }

        switch (Skill)
        {
        case AT_SKILL_ALICE_DRAINLIFE:
        case AT_SKILL_ALICE_DRAINLIFE_STR:
        case AT_SKILL_ALICE_LIGHTNINGORB:
        case AT_SKILL_ALICE_CHAINLIGHTNING:
        case AT_SKILL_ALICE_CHAINLIGHTNING_STR:
        {
            c->TargetCharacter = SelectedCharacter;
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Dead == 0)
            {
                TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

                if (CheckAttack() == true)
                {
                    if (CheckTile(c, o, Distance) && c->SafeZone == false)
                    {
                        bool bNoneWall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
                        if (bNoneWall)
                        {
                            UseSkillSummon(c, o);
                        }
                    }
                    else
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                        {
                            c->Movement = true;
                        }
                    }
                }
            }
        }
        break;
        case AT_SKILL_ALICE_SLEEP:
        case AT_SKILL_ALICE_SLEEP_STR:
        case AT_SKILL_ALICE_BLIND:
        {
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                if (gMapManager.InChaosCastle() == true
                    || gMapManager.IsCursedTemple() == true
                    || (gMapManager.InBattleCastle() == true && battleCastle::IsBattleCastleStart() == true)
                    || g_DuelMgr.IsDuelEnabled()
                    )
                {
                }
                else
                {
                    break;
                }
            }
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Dead == 0)
            {
                TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

                if (CheckAttack() == true)
                {
                    if (CheckTile(c, o, Distance) && c->SafeZone == false)
                    {
                        bool bNoneWall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
                        if (bNoneWall)
                        {
                            UseSkillSummon(c, o);
                        }
                    }
                    else
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                        {
                            c->Movement = true;
                        }
                    }
                }
            }
        }
        break;
        }
    }

    c->SkillSuccess = true;
}

void AttackCommon(CHARACTER* c, int Skill, float Distance)
{
    OBJECT* o = &c->Object;

    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);

    if (o->Type == MODEL_PLAYER)
    {
        bool Success = true;

        if (iMana > CharacterAttribute->Mana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();
            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            Success = false;
        }
        if (Success && iSkillMana > CharacterAttribute->SkillMana)
        {
            Success = false;
        }
        if (Success && !gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
        {
            Success = false;
        }

        int iEnergy;
        gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
        if (Success && iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
        {
            Success = false;
        }

        switch (Skill)
        {
        case    AT_SKILL_STUN:
        {
            //	                        if ( CheckTile( c, o, Distance ) )
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (c->PositionX), (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, pos, TKey, 0);
                SetAttackSpeed();

                if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
                {
                    SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
                }
                else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
                {
                    SetAction(o, PLAYER_SKILL_RIDER);
                }
                else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
                {
                    SetAction(o, PLAYER_SKILL_RIDER_FLY);
                }
                else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                {
                    SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                {
                    SetAction(o, PLAYER_SKILL_VITALITY);
                }
                c->Movement = 0;
            }
        }
        break;

        case    AT_SKILL_REMOVAL_STUN:
        {
            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else
                if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
                {
                    SetAction(o, PLAYER_SKILL_RIDER);
                }
                else
                    if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
                    {
                        SetAction(o, PLAYER_SKILL_RIDER_FLY);
                    }
                    else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                    {
                        SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
                    }
                    else
                    {
                        SetAction(o, PLAYER_ATTACK_REMOVAL);
                    }
            c->Movement = 0;
        }
        break;

        case    AT_SKILL_MANA:
            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_SKILL_VITALITY);
            c->Movement = 0;
            break;

        case    AT_SKILL_INVISIBLE:

            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                if (CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
                    SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_SKILL_VITALITY);
            c->Movement = 0;
            break;

        case AT_SKILL_REMOVAL_INVISIBLE:
            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_ATTACK_REMOVAL);
            c->Movement = 0;
            break;

        case AT_SKILL_REMOVAL_BUFF:

            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_SKILL_VITALITY);
            c->Movement = 0;
            break;
        }
    }
    c->SkillSuccess = true;
}
}
