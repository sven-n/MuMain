#include "stdafx.h"
#include "GameLogic/Combat/SkillExecution.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/CombatTarget.h"
#include "GameLogic/Combat/SkillCast.h" // per-class executors this dispatches to
#include "Character/CharacterManager.h"   // gCharacterManager

// Includes mirror ZzzInterface.cpp, the unit this was extracted from.
#include <imm.h>
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
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/CombatTarget.h"
#include "GameLogic/Combat/SkillCast.h"
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

// File-scope state still owned by ZzzInterface.cpp (no shared header yet).
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter, SelectedNpc, SelectedItem, SelectedOperate;
extern int Attacking, ActionTarget;
extern int TargetX, TargetY, TargetType, TargetNpc;
extern float TargetAngle;
extern bool EnableFastInput;
extern int MouseUpdateTime, MouseUpdateTimeMax;
extern int g_iFollowCharacter;

namespace GameLogic::Combat
{
bool CanExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance)
{
    OBJECT* o = &c->Object;

    if (c->Dead > 0)
    {
        return false;
    }

    if (c->SafeZone)
    {
        if ((gMapManager.InBloodCastle() == true) || gMapManager.InChaosCastle() == true)
        {
            if (Skill != AT_SKILL_HEALING
                && Skill != AT_SKILL_HEALING_STR
                && Skill != AT_SKILL_DEFENSE
                && Skill != AT_SKILL_DEFENSE_STR
                && Skill != AT_SKILL_DEFENSE_MASTERY
                && Skill != AT_SKILL_ATTACK
                && Skill != AT_SKILL_ATTACK_STR
                && Skill != AT_SKILL_ATTACK_MASTERY
                && Skill != AT_SKILL_SOUL_BARRIER
                && Skill != AT_SKILL_SOUL_BARRIER_STR
                && Skill != AT_SKILL_SOUL_BARRIER_PROFICIENCY
                && Skill != AT_SKILL_SWELL_LIFE
                && Skill != AT_SKILL_SWELL_LIFE_STR
                && Skill != AT_SKILL_SWELL_LIFE_PROFICIENCY
                && Skill != AT_SKILL_INFINITY_ARROW
                && Skill != AT_SKILL_INFINITY_ARROW_STR
                && Skill != AT_SKILL_EXPANSION_OF_WIZARDRY
                && Skill != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
                && Skill != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY
                && Skill != AT_SKILL_RECOVER
                && Skill != AT_SKILL_ALICE_BERSERKER
                && Skill != AT_SKILL_ALICE_BERSERKER_STR
                && Skill != AT_SKILL_IMPROVE_AG
                && Skill != AT_SKILL_ADD_CRITICAL
                && Skill != AT_SKILL_ADD_CRITICAL_STR1
                && Skill != AT_SKILL_ADD_CRITICAL_STR2
                && Skill != AT_SKILL_ADD_CRITICAL_STR3
                && Skill != AT_SKILL_PARTY_TELEPORT
                && (Skill<AT_SKILL_STUN || Skill>AT_SKILL_REMOVAL_BUFF)
                && Skill != AT_SKILL_BRAND_OF_SKILL
                )
            {
                return false;
            }
        }
        return false;
    }

    if (gMapManager.IsCursedTemple())
    {
        if (!g_pCursedTempleWindow->IsCursedTempleSkillKey(SelectedCharacter))
        {
            if (g_CursedTemple->IsPartyMember(SelectedCharacter) == true)
            {
                if (!IsCorrectSkillType_FrendlySkill(Skill) && !IsCorrectSkillType_Buff(Skill))
                {
                    return false;
                }
            }
            else
            {
                if (IsCorrectSkillType_FrendlySkill(Skill) || IsCorrectSkillType_Buff(Skill))
                {
                    if (-1 != SelectedCharacter)
                    {
                        return false;
                    }
                }
            }
        }
    }

    if (Skill == AT_SKILL_SUMMON_EXPLOSION || Skill == AT_SKILL_SUMMON_REQUIEM || Skill == AT_SKILL_SUMMON_POLLUTION)
    {
        CheckTarget(c);
        if (!CheckTile(c, o, Distance))
        {
            return false;
        }
    }

    if (!gSkillManager.AreSkillAttributeRequirementsMet(Skill))
    {
        return false;
    }

    if (CheckSkillUseCondition(o, Skill) == false)
    {
        return false;
    }

    if (CheckMana(c, Skill) == false)
    {
        return false;
    }

    return true;
}

bool CheckMana(CHARACTER* c, int Skill)
{
    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
    if (CharacterAttribute->Mana < iMana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();

        if (Index != -1)
        {
            SendRequestUse(Index, 0);
        }
        return false;
    }
    if (iSkillMana > CharacterAttribute->SkillMana)
    {
        return false;
    }
    return true;
}

int ExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance)
{
    OBJECT* o = &c->Object;

    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    if (!CanExecuteSkill(c, Skill, Distance))
    {
        return 0;
    }

    int iSkillIndex = g_pSkillList->GetSkillIndex(Skill);
    if (iSkillIndex == -1)
    {
        return 0;
    }

    Hero->CurrentSkill = iSkillIndex;

    if (gMapManager.IsCursedTemple() && g_pCursedTempleWindow->IsCursedTempleSkillKey(SelectedCharacter))
    {
        g_pCursedTempleWindow->SetCursedTempleSkill(c, o, SelectedCharacter);
        return 0;
    }

    if (c->Movement)
    {
        c->SkillSuccess = false;
    }

    if (
        g_pOption->IsAutoAttack()
        && gMapManager.WorldActive != WD_6STADIUM
        && gMapManager.InChaosCastle() == false)
    {
        if (ClassIndex == CLASS_ELF
            && (Skill != AT_SKILL_TRIPLE_SHOT
                && Skill != AT_SKILL_TRIPLE_SHOT_STR
                && Skill != AT_SKILL_TRIPLE_SHOT_MASTERY
                && Skill != AT_SKILL_MULTI_SHOT
                && Skill != AT_SKILL_BOW
                && Skill != AT_SKILL_PENETRATION
                && Skill != AT_SKILL_PENETRATION_STR
                && Skill != AT_SKILL_BLAST_CROSSBOW4
                && Skill != AT_SKILL_PLASMA_STORM_FENRIR
                ))
        {
            Attacking = -1;
        }
        else if (ClassIndex == CLASS_KNIGHT && (Skill == AT_SKILL_SWELL_LIFE || Skill == AT_SKILL_SWELL_LIFE_STR || Skill == AT_SKILL_SWELL_LIFE_PROFICIENCY))
        {
            Attacking = -1;
        }
        else if (ClassIndex == CLASS_DARK_LORD
            && (Skill == AT_SKILL_ADD_CRITICAL || Skill == AT_SKILL_ADD_CRITICAL_STR1 || Skill == AT_SKILL_ADD_CRITICAL_STR2 || Skill == AT_SKILL_ADD_CRITICAL_STR3
                || Skill == AT_SKILL_PARTY_TELEPORT))
        {
            Attacking = -1;
        }
        else if (ClassIndex == CLASS_WIZARD
            && (Skill == AT_SKILL_NOVA_BEGIN || Skill == AT_SKILL_NOVA))
        {
            Attacking = -1;
        }
        else if (Skill >= AT_SKILL_STUN && Skill <= AT_SKILL_REMOVAL_BUFF)
        {
            Attacking = -1;
        }
        else if (Skill == AT_SKILL_BRAND_OF_SKILL)
        {
            Attacking = -1;
        }
        else if (Skill == AT_SKILL_ALICE_THORNS
            || Skill == AT_SKILL_ALICE_BERSERKER
            || Skill == AT_SKILL_ALICE_BERSERKER_STR
            || Skill == AT_SKILL_ALICE_SLEEP
            || Skill == AT_SKILL_ALICE_SLEEP_STR
            || Skill == AT_SKILL_ALICE_BLIND || Skill == AT_SKILL_ALICE_WEAKNESS || Skill == AT_SKILL_ALICE_ENERVATION)
        {
            Attacking = -1;
        }
        else if (AT_SKILL_ATT_UP_OURFORCES == Skill
            || AT_SKILL_HP_UP_OURFORCES == Skill
            || AT_SKILL_HP_UP_OURFORCES_STR == Skill
            || AT_SKILL_DEF_UP_OURFORCES == Skill
            || AT_SKILL_DEF_UP_OURFORCES_STR == Skill
            || AT_SKILL_DEF_UP_OURFORCES_MASTERY == Skill
            )
        {
            Attacking = -1;
        }
        else
        {
            Attacking = 2;
        }
    }

    if (o->Type == MODEL_PLAYER)
    {
        if (o->CurrentAction < PLAYER_STOP_MALE
            || o->CurrentAction > PLAYER_STOP_RIDE_WEAPON
            && o->CurrentAction != PLAYER_STOP_TWO_HAND_SWORD_TWO
            && o->CurrentAction != PLAYER_SKILL_HELL_BEGIN
            && o->CurrentAction != PLAYER_DARKLORD_STAND
            && o->CurrentAction != PLAYER_STOP_RIDE_HORSE
            && o->CurrentAction != PLAYER_FENRIR_STAND
            && o->CurrentAction != PLAYER_FENRIR_STAND_TWO_SWORD
            && o->CurrentAction != PLAYER_FENRIR_STAND_ONE_RIGHT
            && o->CurrentAction != PLAYER_FENRIR_STAND_ONE_LEFT
            && !(o->CurrentAction >= PLAYER_RAGE_FENRIR_STAND && o->CurrentAction <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
            && o->CurrentAction != PLAYER_RAGE_UNI_STOP_ONE_RIGHT
            && o->CurrentAction != PLAYER_STOP_RAGEFIGHTER)
        {
            MouseRButtonPress = 0;
            return 0;
        }
    }
    else
    {
        if (o->CurrentAction < MONSTER01_STOP1 || o->CurrentAction > MONSTER01_STOP2)
        {
            return 0;
        }
    }

    if (ClassIndex != CLASS_WIZARD)
    {
        CheckTarget(c);

        if (CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY))
        {
            for (int i = EQUIPMENT_WEAPON_RIGHT; i <= EQUIPMENT_WEAPON_LEFT; i++)
            {
                if (ClassIndex == CLASS_KNIGHT
                    || ClassIndex == CLASS_DARK
                    || ClassIndex == CLASS_DARK_LORD
                    || ClassIndex == CLASS_RAGEFIGHTER)
                {
                    bool bOk = false;
                    if (c->Helper.Type != MODEL_HORN_OF_UNIRIA
                        && c->Helper.Type != MODEL_HORN_OF_DINORANT
                        && c->Helper.Type != MODEL_DARK_HORSE_ITEM
                        && c->Helper.Type != MODEL_HORN_OF_FENRIR
                        )
                    {
                        bOk = true;
                    }
                    else
                    {
                        switch (Skill)
                        {
                        case AT_SKILL_CHAOTIC_DISEIER:
                        case AT_SKILL_IMPALE:
                        case AT_SKILL_RIDER:
                        case AT_SKILL_DEATHSTAB:
                        case AT_SKILL_DEATHSTAB_STR:
                        case AT_SKILL_FORCE:
                        case AT_SKILL_FORCE_WAVE:
                        case AT_SKILL_FORCE_WAVE_STR:
                        case AT_SKILL_FIREBURST:
                        case AT_SKILL_FIREBURST_STR:
                        case AT_SKILL_FIREBURST_MASTERY:
                        case AT_SKILL_EARTHSHAKE:
                        case AT_SKILL_EARTHSHAKE_STR:
                        case AT_SKILL_EARTHSHAKE_MASTERY:
                        case AT_SKILL_THUNDER_STRIKE:
                        case AT_SKILL_SPACE_SPLIT:
                        case AT_SKILL_PLASMA_STORM_FENRIR:
                        case AT_SKILL_FIRE_SCREAM:
                        case AT_SKILL_FIRE_SCREAM_STR:
                            bOk = true;
                            break;
                        }
                    }

                    if (bOk)
                    {
                        g_MovementSkill.m_bMagic = TRUE;
                        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
                        g_MovementSkill.m_iTarget = CheckAttack() ? SelectedCharacter : -1;
                        if (SkillWarrior(c, &CharacterMachine->Equipment[i]))
                        {
                            return (int) ExecuteSkillComplete(c);
                        }
                    }
                }
                if (ClassIndex == CLASS_ELF)
                {
                    g_MovementSkill.m_bMagic = TRUE;
                    g_MovementSkill.m_iSkill = Hero->CurrentSkill;
                    g_MovementSkill.m_iTarget = CheckAttack() ? SelectedCharacter : -1;
                    if (SkillElf(c, &CharacterMachine->Equipment[i]))
                    {
                        return (int) ExecuteSkillComplete(c);
                    }
                }
            }
        }
        else
        {
            if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
            {
                SendMove(c, o);
                return 0;
            }
            else
            {
                ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
                g_MovementSkill.m_iTarget = -1;
                return -1;
            }
        }
    }
    if (ClassIndex == CLASS_ELF)
    {
        GameLogic::Combat::AttackElf(c, Skill, Distance);
    }
    if (ClassIndex == CLASS_KNIGHT || ClassIndex == CLASS_DARK || ClassIndex == CLASS_DARK_LORD)
    {
        GameLogic::Combat::AttackKnight(c, Skill, Distance);
    }
    if (ClassIndex == CLASS_RAGEFIGHTER)
    {
        AttackRagefighter(c, Skill, Distance);
    }
    if (ClassIndex == CLASS_WIZARD || ClassIndex == CLASS_DARK || ClassIndex == CLASS_SUMMONER)
    {
        GameLogic::Combat::AttackWizard(c, Skill, Distance);
    }
    if ((Skill >= AT_SKILL_STUN && Skill <= AT_SKILL_REMOVAL_BUFF))
    {
        GameLogic::Combat::AttackCommon(c, Skill, Distance);
    }

    return (int) ExecuteSkillComplete(c);
}

bool ExecuteSkillComplete(CHARACTER* c)
{
    return c->SkillSuccess && !c->Movement;
}
}
