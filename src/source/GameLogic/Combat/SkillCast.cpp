#include "stdafx.h"
#include "GameLogic/Combat/SkillCast.h"
#include "Character/CharacterManager.h" // gCharacterManager
#include "GameLogic/Social/MonkSystem.h" // g_CMonkSystem

// Includes mirror ZzzInterface.cpp, the unit these were extracted from.
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
#include "GameLogic/Combat/SkillExecution.h"
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

// File-scope state still owned by ZzzInterface.cpp (no shared header yet).
extern void SetPlayerBow(CHARACTER* c);
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter, SelectedNpc, SelectedItem, SelectedOperate;
extern int Attacking, ActionTarget;
extern int TargetX, TargetY, TargetType, TargetNpc;
extern float TargetAngle;
extern bool EnableFastInput;
extern int MouseUpdateTime, MouseUpdateTimeMax;
extern int g_iFollowCharacter;
extern int StandTime;

namespace GameLogic::Combat
{
bool CastWarriorSkill(CHARACTER* c, OBJECT* o, ITEM* p, ActionSkillType iSkill)
{
    if (c == NULL)	return false;
    if (o == NULL)	return false;
    //if (p == NULL)	return false;
    bool Success = false;

    if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_CLIENT)
    {
        return false;
    }

    TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

    g_MovementSkill.m_bMagic = FALSE;
    g_MovementSkill.m_iSkill = iSkill;
    g_MovementSkill.m_iTarget = SelectedCharacter;
    float Distance = gSkillManager.GetSkillDistance(iSkill, c) * 1.2f;

    if ((gMapManager.InBloodCastle() == true)
        && ((iSkill >= AT_SKILL_FALLING_SLASH && iSkill <= AT_SKILL_SLASH)
            || iSkill == AT_SKILL_FALLING_SLASH_STR
            || iSkill == AT_SKILL_LUNGE_STR
            || iSkill == AT_SKILL_CYCLONE_STR
            || iSkill == AT_SKILL_CYCLONE_STR_MG
            || iSkill == AT_SKILL_SLASH_STR
            ))
    {
        Distance = 1.8f;
    }

    if (CheckTile(c, o, Distance))
    {
        UseSkillWarrior(c, o);
        Success = true;
    }
    else if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
    {
        c->Movement = true;
        c->MovementType = MOVEMENT_SKILL;
        SendMove(c, o);
    }

    return (Success);
}

bool SkillWarrior(CHARACTER* c, ITEM* p)
{
    OBJECT* o = &c->Object;
    if (o->Type == MODEL_PLAYER)
    {
        if (o->CurrentAction == PLAYER_DEFENSE1) return false;
        if (o->CurrentAction >= PLAYER_ATTACK_SKILL_SWORD1 && o->CurrentAction <= PLAYER_ATTACK_SKILL_SWORD4 || o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO) return false;
    }
    else
    {
        if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2) return false;
    }
    auto Skill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];
    if (Skill == AT_SKILL_RIDER
        || Skill == AT_SKILL_FIRE_SLASH
        || Skill == AT_SKILL_FIRE_SLASH_STR
        || Skill == AT_SKILL_TWISTING_SLASH
        || Skill == AT_SKILL_TWISTING_SLASH_STR
        || Skill == AT_SKILL_TWISTING_SLASH_STR_MG
        || Skill == AT_SKILL_TWISTING_SLASH_MASTERY
        || Skill == AT_SKILL_DEATHSTAB
        || Skill == AT_SKILL_DEATHSTAB_STR
        || (Skill == AT_SKILL_IMPALE && (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA || Hero->Helper.Type == MODEL_HORN_OF_DINORANT || Hero->Helper.Type == MODEL_DARK_HORSE_ITEM || Hero->Helper.Type == MODEL_HORN_OF_FENRIR))
        || Skill == AT_SKILL_FORCE
        || Skill == AT_SKILL_FORCE_WAVE
        || Skill == AT_SKILL_FORCE_WAVE_STR
        || Skill == AT_SKILL_FIREBURST
        || Skill == AT_SKILL_FIREBURST_STR
        || Skill == AT_SKILL_FIREBURST_MASTERY
        || Skill == AT_SKILL_RUSH
        || Skill == AT_SKILL_SPIRAL_SLASH
        || Skill == AT_SKILL_SPACE_SPLIT
        )
    {
        switch (Skill)
        {
        case AT_SKILL_IMPALE:
            if (!(o->Type == MODEL_PLAYER && Hero->Weapon[0].Type != -1 &&
                (Hero->Weapon[0].Type >= MODEL_SPEAR && Hero->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)))
            {
                return false;
            }
            break;
        case AT_SKILL_DEATHSTAB:
        case AT_SKILL_DEATHSTAB_STR:
            if (!(Hero->Weapon[0].Type != -1 && (Hero->Weapon[0].Type < MODEL_STAFF || Hero->Weapon[0].Type >= MODEL_STAFF + MAX_ITEM_INDEX)))
            {
                return false;
            }
            break;
        case AT_SKILL_SPIRAL_SLASH:
            if (Hero->Weapon[0].Type < MODEL_SWORD || Hero->Weapon[0].Type >= MODEL_SWORD + MAX_ITEM_INDEX)
            {
                return false;
            }
            break;
        }
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

        if (!gSkillManager.CheckSkillDelay(g_MovementSkill.m_iSkill))
        {
            return false;
        }
        if (CheckAttack())
        {
            return (CastWarriorSkill(c, o, p, Skill));
        }
    }


    auto baseSkill = gSkillManager.MasterSkillToBaseSkillIndex(Skill);
    bool Success = false;
    for (int i = 0; i < p->SpecialNum; i++)
    {
        if (baseSkill == p->Special[i]) // current skill is available as weapon skill?
        {
            int iMana;
            gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL);
            if (CharacterAttribute->Mana < iMana)
            {
                int Index = g_pMyInventory->FindManaItemIndex();

                if (Index != -1)
                {
                    SendRequestUse(Index, 0);
                }
                continue;
            }

            if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
            {
                continue;
            }

            switch (Skill)
            {
            case AT_SKILL_BLOCKING:
                c->Movement = false;
                if (o->Type == MODEL_PLAYER)
                    SetAction(o, PLAYER_DEFENSE1);
                else
                    SetPlayerAttack(c);
                SendRequestMagic(Skill, Hero->Key);
                Success = true;
                break;
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
            case AT_SKILL_RIDER:
                if (CheckAttack())
                    Success = CastWarriorSkill(c, o, p, Skill);
                break;
            }
        }
    }

    if (Skill == AT_SKILL_FIRE_SCREAM
        || Skill == AT_SKILL_FIRE_SCREAM_STR
        || Skill == AT_SKILL_CHAOTIC_DISEIER
        )
    {
        int iMana;
        gSkillManager.GetSkillInformation(Skill, 1, nullptr, &iMana, nullptr);
        if (CharacterAttribute->Mana < iMana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();

            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            return Success;
        }

        float distance = gSkillManager.GetSkillDistance(Skill, c);
        if (CheckTile(c, o, distance))
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
            WORD TKey = 0xffff;

            if (g_MovementSkill.m_iTarget != -1)
            {
                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
            }
        }
    }

    return Success;
}

void UseSkillWarrior(CHARACTER* c, OBJECT* o)
{
    auto Skill = g_MovementSkill.m_bMagic ? CharacterAttribute->Skill[g_MovementSkill.m_iSkill] : static_cast<ActionSkillType>(g_MovementSkill.m_iSkill);
    LetHeroStop();
    c->Movement = false;
    if (o->Type == MODEL_PLAYER)
    {
        SetAttackSpeed();

        switch (Skill)
        {
        case AT_SKILL_IMPALE:
            if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
                SetAction(o, PLAYER_FENRIR_ATTACK_SPEAR);
            else
                SetAction(o, PLAYER_ATTACK_SKILL_SPEAR);
            break;
        case AT_SKILL_DEATHSTAB:
        case AT_SKILL_DEATHSTAB_STR:
            SetAction(o, PLAYER_ATTACK_DEATHSTAB);
            break;
        case AT_SKILL_TWISTING_SLASH:
        case AT_SKILL_TWISTING_SLASH_STR:
        case AT_SKILL_TWISTING_SLASH_STR_MG:
        case AT_SKILL_TWISTING_SLASH_MASTERY:
        case AT_SKILL_FIRE_SLASH:
        case AT_SKILL_FIRE_SLASH_STR:
            SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);
            break;
        case AT_SKILL_RIDER:
            if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            else
                SetAction(o, PLAYER_SKILL_RIDER);
            break;
        case AT_SKILL_FIRE_SCREAM:
        case AT_SKILL_FIRE_SCREAM_STR:
            break;
        case AT_SKILL_CHAOTIC_DISEIER:
            break;
        case AT_SKILL_FORCE:
        case AT_SKILL_FORCE_WAVE:
        case AT_SKILL_FORCE_WAVE_STR:
        case AT_SKILL_FIREBURST:
        case AT_SKILL_FIREBURST_STR:
        case AT_SKILL_FIREBURST_MASTERY:
        case AT_SKILL_SPACE_SPLIT:
            break;
        case AT_SKILL_RUSH:
            SetAction(o, PLAYER_ATTACK_RUSH);
            break;
        default:
            if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
            {
                auto baseSkill = gSkillManager.MasterSkillToBaseSkillIndex(Skill);
                SetAction(o, static_cast<int>(PLAYER_ATTACK_SKILL_SWORD1) + baseSkill - AT_SKILL_FALLING_SLASH);
            }
            break;
        }
    }
    else
    {
        SetPlayerAttack(c);
    }

    vec3_t Light;
    Vector(1.f, 1.f, 1.f, Light);

    if (Skill != AT_SKILL_FORCE
        && Skill != AT_SKILL_FORCE_WAVE
        && Skill != AT_SKILL_FORCE_WAVE_STR
        && Skill != AT_SKILL_FIREBURST
        && Skill != AT_SKILL_FIREBURST_STR
        && Skill != AT_SKILL_FIREBURST_MASTERY
        && Skill != AT_SKILL_SPACE_SPLIT
        && Skill != AT_SKILL_FIRE_SCREAM
        && Skill != AT_SKILL_FIRE_SCREAM_STR
        && Skill != AT_SKILL_KILLING_BLOW
        && Skill != AT_SKILL_KILLING_BLOW_STR
        && Skill != AT_SKILL_KILLING_BLOW_MASTERY
        && Skill != AT_SKILL_BEAST_UPPERCUT
        && Skill != AT_SKILL_BEAST_UPPERCUT_STR
        && Skill != AT_SKILL_BEAST_UPPERCUT_MASTERY
        )
    {
        CreateParticle(BITMAP_SHINY + 2, o->Position, o->Angle, Light, 0, 0.f, o);
        PlayBuffer(static_cast<ESound>(SOUND_BRANDISH_SWORD01 + rand() % 2));
    }

    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
    }

    if (Skill != AT_SKILL_CHAOTIC_DISEIER)
    {
        WORD TKey = 0xffff;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
		{
			TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
		}

        if (Skill == AT_SKILL_TWISTING_SLASH
            || Skill == AT_SKILL_TWISTING_SLASH_STR
            || Skill == AT_SKILL_TWISTING_SLASH_STR_MG
            || Skill == AT_SKILL_TWISTING_SLASH_MASTERY
            || Skill == AT_SKILL_FIRE_SLASH
            || Skill == AT_SKILL_FIRE_SLASH_STR
            )
        {
            SendRequestMagicContinue(Skill, (c->PositionX), (c->PositionY), 
                (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
        }
        else
        {
            SendRequestMagic(Skill, TKey);
        }
    }

    if (((!g_isCharacterBuff(o, eDeBuff_Harden)) && c->Helper.Type != MODEL_DARK_HORSE_ITEM)
        && Skill != AT_SKILL_FIRE_SCREAM
        && Skill != AT_SKILL_FIRE_SCREAM_STR
        )
    {
        BYTE positionX = (BYTE)(c->TargetPosition[0] / TERRAIN_SCALE);
        BYTE positionY = (BYTE)(c->TargetPosition[1] / TERRAIN_SCALE);

        if ((gMapManager.InBloodCastle() == true)
            || Skill == AT_SKILL_FORCE
            || Skill == AT_SKILL_FORCE_WAVE
            || Skill == AT_SKILL_FORCE_WAVE_STR
            || Skill == AT_SKILL_SPIRAL_SLASH
            || Skill == AT_SKILL_RUSH
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CASTLE_GATE1
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_GUARDIAN_STATUE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_LIFE_STONE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CANON_TOWER
            )
        {
            int angle = abs((int)(o->Angle[2] / 45.f));
            switch (angle)
            {
            case 0: positionY++; break;
            case 1: positionX--; positionY++; break;
            case 2: positionX--; break;
            case 3: positionX--; positionY--; break;
            case 4: positionY--; break;
            case 5: positionX++; positionY--; break;
            case 6: positionX++; break;
            case 7: positionX++; positionY++; break;
            }
        }

#ifdef SEND_POSITION_TO_SERVER
        int TargetIndex = TERRAIN_INDEX(positionX, positionY);

        if ((TerrainWall[TargetIndex] & TW_NOMOVE) != TW_NOMOVE && (TerrainWall[TargetIndex] & TW_NOGROUND) != TW_NOGROUND)
        {
            if (Skill != AT_SKILL_IMPALE
                && Skill != AT_SKILL_DEATHSTAB
                && Skill != AT_SKILL_DEATHSTAB_STR
                && Skill != AT_SKILL_RIDER
                && Skill != AT_SKILL_FORCE
                && Skill != AT_SKILL_FORCE_WAVE
                && Skill != AT_SKILL_FORCE_WAVE_STR
                && Skill != AT_SKILL_FIREBURST
                && Skill != AT_SKILL_FIREBURST_STR
                && Skill != AT_SKILL_FIREBURST_MASTERY
                && Skill != AT_SKILL_SPACE_SPLIT
                )
            {
                SocketClient->ToGameServer()->SendInstantMoveRequest(positionX, positionY);
            }
        }
#endif
    }

    c->SkillSuccess = true;
}

void UseSkillWizard(CHARACTER* c, OBJECT* o)
{
    auto Skill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];

    switch (Skill)
    {
    case AT_SKILL_IMPALE:
    case AT_SKILL_DEATHSTAB:
    case AT_SKILL_DEATHSTAB_STR:
        return;
    }

    if (Skill == AT_SKILL_DEATH_CANNON)
    {
        if (Hero->Weapon[0].Type < MODEL_STAFF || Hero->Weapon[0].Type >= MODEL_STAFF + MAX_ITEM_INDEX)
            return;
    }

    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
    }

    WORD TKey = 0xffff;
    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
    }

    switch (Skill)
    {
    case AT_SKILL_POISON:
    case AT_SKILL_POISON_STR:
    case AT_SKILL_METEO:
    case AT_SKILL_LIGHTNING:
    case AT_SKILL_LIGHTNING_STR:
    case AT_SKILL_LIGHTNING_STR_MG:
    case AT_SKILL_ENERGYBALL:
    case AT_SKILL_POWERWAVE:
    case AT_SKILL_ICE:
    case AT_SKILL_ICE_STR:
    case AT_SKILL_ICE_STR_MG:
    case AT_SKILL_FIREBALL:
    case AT_SKILL_JAVELIN:
        SendRequestMagic(Skill, TKey);
        SetPlayerMagic(c);
        LetHeroStop();
        break;
    case AT_SKILL_DEATH_CANNON:
        SendRequestMagic(Skill, TKey);
        SetAction(o, PLAYER_ATTACK_DEATH_CANNON);
        SetAttackSpeed();
        LetHeroStop();
        break;
    case AT_SKILL_BLAST:
    case AT_SKILL_BLAST_STR:
    case AT_SKILL_BLAST_STR_MG:
    {
        SendRequestMagicContinue(Skill, (int)(c->TargetPosition[0] / 100.f), (int)(c->TargetPosition[1] / 100.f), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
    }
    SetPlayerMagic(c);
    LetHeroStop();
    break;
    }

    c->SkillSuccess = true;
}

void UseSkillElf(CHARACTER* c, OBJECT* o)
{
    LetHeroStop();
    int Skill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];

    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
    }

    WORD TKey = 0xffff;
    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
    }

    switch (Skill)
    {
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_RECOVER:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
        SendRequestMagic(Skill, TKey);
        SetPlayerMagic(c);
        break;

    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
    {
        WORD Dexterity;
        const WORD notDexterity = 646;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < notDexterity)
        {
            break;
        }
        if (!CheckArrow())
            break;
        SendRequestMagic(Skill, TKey);
        SetPlayerAttack(c);
    }
    break;

    case AT_SKILL_DEEPIMPACT:
        if (!CheckArrow())
            break;
        SendRequestMagic(Skill, TKey);
        SetPlayerHighBowAttack(c);
        break;
    }

    c->SkillSuccess = true;
}

void UseSkillSummon(CHARACTER* pCha, OBJECT* pObj)
{
    int iSkill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];

    switch (iSkill)
    {
    case AT_SKILL_ALICE_DRAINLIFE:
    case AT_SKILL_ALICE_DRAINLIFE_STR:
    case AT_SKILL_ALICE_LIGHTNINGORB:
    {
        LetHeroStop();
        if (iSkill == AT_SKILL_ALICE_DRAINLIFE || iSkill == AT_SKILL_ALICE_DRAINLIFE_STR)
        {
            switch (pCha->Helper.Type)
            {
            case MODEL_HORN_OF_UNIRIA:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE_UNI);
                break;
            case MODEL_HORN_OF_DINORANT:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE_DINO);
                break;
            case MODEL_HORN_OF_FENRIR:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE_FENRIR);
                break;
            default:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE);
                break;
            }
        }
        else if (iSkill == AT_SKILL_ALICE_LIGHTNINGORB)
        {
            switch (pCha->Helper.Type)
            {
            case MODEL_HORN_OF_UNIRIA:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB_UNI);
                break;
            case MODEL_HORN_OF_DINORANT:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB_DINO);
                break;
            case MODEL_HORN_OF_FENRIR:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB_FENRIR);
                break;
            default:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB);
                break;
            }
        }

        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            WORD wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
            SendRequestMagicContinue(iSkill, (int)(pCha->TargetPosition[0] / 100.f), (int)(pCha->TargetPosition[1] / 100.f), (BYTE)(pObj->Angle[2] / 360.f * 256.f), 0, 0, wTargetKey, 0);
        }
    }
    break;
    case AT_SKILL_ALICE_CHAINLIGHTNING:
    case AT_SKILL_ALICE_CHAINLIGHTNING_STR:
    {
        LetHeroStop();

        switch (pCha->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR);
            break;
        default:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING);
            break;
        }

        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            WORD wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;

            SendRequestMagicContinue(iSkill, (int)(pCha->TargetPosition[0] / 100.f),
                (int)(pCha->TargetPosition[1] / 100.f),
                (BYTE)(pObj->Angle[2] / 360.f * 256.f),
                0, 0, wTargetKey, 0);
        }
    }
    break;
    case AT_SKILL_ALICE_SLEEP:
    case AT_SKILL_ALICE_SLEEP_STR:
    case AT_SKILL_ALICE_BLIND:
    case AT_SKILL_ALICE_THORNS:
    {
        LetHeroStop();

        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, pCha->TargetPosition);
            pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);

            WORD wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
            SendRequestMagic(iSkill, wTargetKey);
        }
    }
    break;
    case AT_SKILL_ALICE_BERSERKER:
    case AT_SKILL_ALICE_BERSERKER_STR:
        LetHeroStop();
        switch (pCha->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(pObj, PLAYER_SKILL_SLEEP_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(pObj, PLAYER_SKILL_SLEEP_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(pObj, PLAYER_SKILL_SLEEP_FENRIR);
            break;
        default:
            SetAction(pObj, PLAYER_SKILL_SLEEP);
            break;
        }
        SendRequestMagic(iSkill, HeroKey);
        break;
    case AT_SKILL_ALICE_WEAKNESS:
    case AT_SKILL_ALICE_ENERVATION:
        LetHeroStop();
        SendRequestMagicContinue(iSkill, pCha->PositionX, pCha->PositionY, (BYTE)(pObj->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
        switch (pCha->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(pObj, PLAYER_SKILL_SLEEP_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(pObj, PLAYER_SKILL_SLEEP_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(pObj, PLAYER_SKILL_SLEEP_FENRIR);
            break;
        default:
            SetAction(pObj, PLAYER_SKILL_SLEEP);
            break;
        }
        break;
    }

    pCha->SkillSuccess = true;
}

void UseSkillRagefighter(CHARACTER* pCha, OBJECT* pObj)
{
    int iSkill = g_MovementSkill.m_bMagic ? CharacterAttribute->Skill[g_MovementSkill.m_iSkill] : g_MovementSkill.m_iSkill;

    ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
    ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

    if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level))
        return;

    if (g_CMonkSystem.IsRideNotUseSkill(iSkill, pCha->Helper.Type))
        return;

    if (!g_CMonkSystem.IsSwordformGlovesUseSkill(iSkill))
        return;

    LetHeroStop();
    pCha->Movement = false;

    if (pObj->Type == MODEL_PLAYER)
    {
        g_CMonkSystem.SetRageSkillAni(iSkill, pObj);
        SetAttackSpeed();
    }

    switch (iSkill)
    {
    case AT_SKILL_KILLING_BLOW:
    case AT_SKILL_KILLING_BLOW_STR:
    case AT_SKILL_KILLING_BLOW_MASTERY:
    case AT_SKILL_OCCUPY:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
			VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, pCha->TargetPosition);
            pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);
        }
        SendRequestMagic(iSkill, wTargetKey);

        BYTE TargetPosX = (BYTE)(pCha->TargetPosition[0] / TERRAIN_SCALE);
        BYTE TargetPosY = (BYTE)(pCha->TargetPosition[1] / TERRAIN_SCALE);

        if ((gMapManager.InBloodCastle()) || iSkill == AT_SKILL_OCCUPY
            || (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && (
               CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CASTLE_GATE1
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_GUARDIAN_STATUE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_LIFE_STONE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CANON_TOWER))
            )
        {
            int angle = abs((int)(pObj->Angle[2] / 45.f));
            switch (angle)
            {
            case 0: TargetPosY++; break;
            case 1: TargetPosX--; TargetPosY++; break;
            case 2: TargetPosX--; break;
            case 3: TargetPosX--; TargetPosY--; break;
            case 4: TargetPosY--; break;
            case 5: TargetPosX++; TargetPosY--; break;
            case 6: TargetPosX++; break;
            case 7: TargetPosX++; TargetPosY++; break;
            }
        }

        int TargetIndex = TERRAIN_INDEX(TargetPosX, TargetPosY);

        vec3_t vDis;
        Vector(0.0f, 0.0f, 0.0f, vDis);
        VectorSubtract(pCha->TargetPosition, pCha->Object.Position, vDis);
        VectorNormalize(vDis);
        VectorScale(vDis, TERRAIN_SCALE, vDis);
        VectorSubtract(pCha->TargetPosition, vDis, vDis);
        BYTE CharPosX = (BYTE)(vDis[0] / TERRAIN_SCALE);
        BYTE CharPosY = (BYTE)(vDis[1] / TERRAIN_SCALE);

#ifdef SEND_POSITION_TO_SERVER
        if ((TerrainWall[TargetIndex] & TW_NOMOVE) != TW_NOMOVE && (TerrainWall[TargetIndex] & TW_NOGROUND) != TW_NOGROUND)
        {
            SocketClient->ToGameServer()->SendInstantMoveRequest(CharPosX, CharPosY);
        }
#endif
        
        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_BEAST_UPPERCUT:
    case AT_SKILL_BEAST_UPPERCUT_STR:
    case AT_SKILL_BEAST_UPPERCUT_MASTERY:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }

        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        g_CMonkSystem.InitConsecutiveState(3.0f, 7.0f);
    }
    break;
    case AT_SKILL_CHAIN_DRIVE:
    case AT_SKILL_CHAIN_DRIVE_STR:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }
        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        g_CMonkSystem.InitConsecutiveState(3.0f, 12.0f);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_DRAGON_KICK:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }
        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        g_CMonkSystem.InitConsecutiveState(3.0f);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_DARKSIDE:
    case AT_SKILL_DARKSIDE_STR:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }

        SocketClient->ToGameServer()->SendRageAttackRangeRequest(iSkill, wTargetKey);
        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_DRAGON_ROAR:
    case AT_SKILL_DRAGON_ROAR_STR:
    {
        BYTE angle = (BYTE)((((pObj->Angle[2] + 180.f) / 360.f) * 255.f));
        WORD TKey = 0xffff;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            TKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }
        BYTE byValue = GetDestValue((pCha->PositionX), (pCha->PositionY), TargetX, TargetY);
        SendRequestMagicContinue(iSkill, pCha->PositionX, pCha->PositionY, ((pObj->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_ATT_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
    {
        SendRequestMagic(iSkill, HeroKey);
        if (rand_fps_check(2))
        {
            SetAction(pObj, PLAYER_SKILL_ATT_UP_OURFORCES);
            PlayBuffer(SOUND_RAGESKILL_BUFF_1);
        }
        else
        {
            SetAction(pObj, PLAYER_SKILL_HP_UP_OURFORCES);
            PlayBuffer(SOUND_RAGESKILL_BUFF_2);
        }
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_PLASMA_STORM_FENRIR:
    {
        pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);

        gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

        BYTE pos = CalcTargetPos(pObj->Position[0], pObj->Position[1], pCha->TargetPosition[0], pCha->TargetPosition[1]);
        WORD TKey = 0xffff;
        TKey = getTargetCharacterKey(pCha, g_MovementSkill.m_iTarget);
        pCha->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
        SendRequestMagicContinue(iSkill, (pCha->PositionX), (pCha->PositionY), (BYTE)(pObj->Angle[2] / 360.f * 256.f), 0, pos, TKey, &pObj->m_bySkillSerialNum);
        pCha->Movement = 0;

        if (pObj->Type == MODEL_PLAYER)
        {
            SetAction_Fenrir_Skill(pCha, pObj);
        }
    }
    break;
    default:
        break;
    }

    pCha->SkillSuccess = true;
}

void AttackRagefighter(CHARACTER* pCha, int nSkill, float fDistance)
{
    OBJECT* pObj = &pCha->Object;

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(nSkill, 1, NULL, &iMana, NULL, &iSkillMana);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"AttackRagefighter ID : %d, Dis : %.2f | %d %d / %d | %d", nSkill, fDistance, iMana, iSkillMana, CharacterAttribute->Mana, gSkillManager.CheckSkillDelay(Hero->CurrentSkill));

    if (CharacterAttribute->Mana < iMana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();

        if (Index != -1)
            SendRequestUse(Index, 0);

        return;
    }

    if (iSkillMana > CharacterAttribute->SkillMana)
        return;

    /*if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill)) {
        g_ConsoleDebug->Write(MCD_RECEIVE, L"CheckSkillDelay %d", Hero->CurrentSkill);
        return;
    }*/

    bool bSuccess = CheckTarget(pCha);
    bool bCheckAttack = CheckAttack();
    g_MovementSkill.m_bMagic = TRUE;
    g_MovementSkill.m_iSkill = Hero->CurrentSkill;

    if (bCheckAttack)
        g_MovementSkill.m_iTarget = SelectedCharacter;
    else
        g_MovementSkill.m_iTarget = -1;

    g_ConsoleDebug->Write(MCD_SEND, L"AttackRagefighter ID : %d, Success : %d, SelectedCharacter: %d %d | 5d", nSkill, bSuccess, SelectedCharacter, CharactersClient[SelectedCharacter].Dead, bCheckAttack);

    if (bSuccess)
    {
        switch (nSkill)
        {
        case AT_SKILL_KILLING_BLOW:
        case AT_SKILL_KILLING_BLOW_STR:
        case AT_SKILL_KILLING_BLOW_MASTERY:
        case AT_SKILL_BEAST_UPPERCUT:
        case AT_SKILL_BEAST_UPPERCUT_STR:
        case AT_SKILL_BEAST_UPPERCUT_MASTERY:
        case AT_SKILL_CHAIN_DRIVE:
        case AT_SKILL_CHAIN_DRIVE_STR:
        case AT_SKILL_DRAGON_KICK:
        case AT_SKILL_DRAGON_ROAR:
        case AT_SKILL_DRAGON_ROAR_STR:
        case AT_SKILL_OCCUPY:
        case AT_SKILL_PHOENIX_SHOT:
        {
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Dead == 0)
            {
                pCha->TargetCharacter = SelectedCharacter;
                
                TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

                if (bCheckAttack)
                {
                    fDistance = gSkillManager.GetSkillDistance(nSkill, pCha) * 1.2f;
                    if (CheckTile(pCha, pObj, fDistance) && pCha->SafeZone == false)
                    {
                        bool bNoneWall = CheckWall((pCha->PositionX), (pCha->PositionY), TargetX, TargetY);
                        g_ConsoleDebug->Write(MCD_SEND, L"check wall %d", bNoneWall);
                        if (bNoneWall)
                            UseSkillRagefighter(pCha, pObj);
                    }
                    else
                    {
                        if (PathFinding2(pCha->PositionX, pCha->PositionY, TargetX, TargetY, &pCha->Path, fDistance))
                        {
                            pCha->Movement = true;
                            pCha->MovementType = MOVEMENT_SKILL;
                            SendMove(pCha, pObj);
                        }
                    }
                }
            }
        }
        break;
        case AT_SKILL_DARKSIDE:
        case AT_SKILL_DARKSIDE_STR:
            UseSkillRagefighter(pCha, pObj);
            break;
        case AT_SKILL_ATT_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES:
        case AT_SKILL_DEF_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
            UseSkillRagefighter(pCha, pObj);
            break;
        case AT_SKILL_PLASMA_STORM_FENRIR:
        {
            if (gMapManager.InChaosCastle())
                break;

            int nTargetX = (int)(pCha->TargetPosition[0] / TERRAIN_SCALE);
            int nTargetY = (int)(pCha->TargetPosition[1] / TERRAIN_SCALE);
            if (CheckTile(pCha, pObj, fDistance))
            {
                if (g_MovementSkill.m_iTarget != -1)
                {
                    UseSkillRagefighter(pCha, pObj);
                }
                else
                {
                    pCha->m_iFenrirSkillTarget = -1;
                }
            }
            else
            {
                if (g_MovementSkill.m_iTarget != -1)
                {
                    if (PathFinding2(pCha->PositionX, pCha->PositionY, nTargetX, nTargetY, &pCha->Path, fDistance * 1.2f))
                    {
                        pCha->Movement = true;
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

    pCha->SkillSuccess = true;
}

bool UseSkillRagePosition(CHARACTER* pCha)
{
    OBJECT* pObj = &pCha->Object;

    if (pObj->CurrentAction == PLAYER_SKILL_GIANTSWING || pObj->CurrentAction == PLAYER_SKILL_STAMP || pObj->CurrentAction == PLAYER_SKILL_DRAGONKICK)
    {
        if (g_CMonkSystem.IsConsecutiveAtt(pObj->AnimationFrame))
        {
            int iSkill = g_MovementSkill.m_bMagic ? CharacterAttribute->Skill[g_MovementSkill.m_iSkill] : g_MovementSkill.m_iSkill;
            return g_CMonkSystem.SendAttackPacket(pCha, g_MovementSkill.m_iTarget, iSkill);
        }

        if (pObj->CurrentAction == PLAYER_SKILL_STAMP && pObj->AnimationFrame >= 2.0f)
        {
            pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
            g_CMonkSystem.RageCreateEffect(pObj, AT_SKILL_BEAST_UPPERCUT);
        }
    }
    else if (pObj->CurrentAction == PLAYER_SKILL_DARKSIDE_READY)
    {
        int AttTime = (int)(2.5f / Models[pObj->Type].Actions[PLAYER_SKILL_DARKSIDE_READY].PlaySpeed);
        if (pCha->AttackTime >= AttTime)
        {
            if (g_CMonkSystem.SendDarksideAtt(pObj))
            {
                pCha->AttackTime = 1;
                return true;
            }
        }
        return false;
    }
    else if (pObj->CurrentAction == PLAYER_SKILL_THRUST)
    {
        pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);
        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
    }
    else
    {
        g_CMonkSystem.InitConsecutiveState();
        g_CMonkSystem.InitEffectOnce();
    }
    return false;
}

bool SkillElf(CHARACTER* c, ITEM* p)
{
    OBJECT* o = &c->Object;
    bool Success = false;
    auto currentSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    for (int i = 0; i < p->SpecialNum; i++)
    {
        int Spe_Num = p->Special[i];
        if (Spe_Num == AT_SKILL_TRIPLE_SHOT && (AT_SKILL_TRIPLE_SHOT_STR == currentSkill || AT_SKILL_TRIPLE_SHOT_MASTERY == currentSkill))
        {
            Spe_Num = currentSkill;
        }

        if (currentSkill == Spe_Num)
        {
            int iMana, iSkillMana;
            gSkillManager.GetSkillInformation(Spe_Num, 1, NULL, &iMana, NULL, &iSkillMana);

            if (g_isCharacterBuff(o, eBuff_InfinityArrow))
                iMana += CharacterMachine->InfinityArrowAdditionalMana;

            if (CharacterAttribute->Mana <= iMana)
            {
                int Index = g_pMyInventory->FindManaItemIndex();

                if (Index != -1)
                {
                    SendRequestUse(Index, 0);
                }
                continue;
            }

            if (iSkillMana > CharacterAttribute->SkillMana)
            {
                return (FALSE);
            }
            if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
            {
                return false;
            }
            float Distance = gSkillManager.GetSkillDistance(Spe_Num, c);
            switch (Spe_Num)
            {
            case AT_SKILL_TRIPLE_SHOT:
            case AT_SKILL_TRIPLE_SHOT_STR:
            case AT_SKILL_TRIPLE_SHOT_MASTERY:
                if (!CheckArrow())
                {
                    i = p->SpecialNum;
                    continue;
                }
                if (CheckTile(c, o, Distance))
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Spe_Num, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                    SetPlayerAttack(c);
                    if (o->Type != MODEL_PLAYER && o->Kind != KIND_PLAYER)
                        CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 1);
                    Success = true;
                }
                break;

            case AT_SKILL_BLAST_CROSSBOW4:
                if (!CheckArrow())
                {
                    i = p->SpecialNum;
                    continue;
                }
                if (CheckTile(c, o, Distance))
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Spe_Num, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetPlayerAttack(c);
                    if (o->Type != MODEL_PLAYER)
                        CreateArrows(c, o, NULL, FindHotKey((c->Skill)), Spe_Num - AT_SKILL_BLAST_CROSSBOW4 + 2);

                    Success = true;
                }
                break;
            }
        }
    }
    return Success;
}

}
