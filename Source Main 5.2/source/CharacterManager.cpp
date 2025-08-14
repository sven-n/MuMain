// CharacterManager.cpp: implementation of the CCharacterManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharacterManager.h"
#include "CSItemOption.h"
#include "GlobalText.h"
#include "SkillManager.h"
#include "ZzzInfomation.h"


CCharacterManager gCharacterManager;

CCharacterManager::CCharacterManager() // OK
{
}

CCharacterManager::~CCharacterManager() // OK
{
}

CLASS_TYPE CCharacterManager::ChangeServerClassTypeToClientClassType(const SERVER_CLASS_TYPE byServerClassType)
{
    switch (byServerClassType)
    {
    case DarkWizard:
        return CLASS_WIZARD;
    case SoulMaster:
        return CLASS_SOULMASTER;
    case GrandMaster:
        return CLASS_GRANDMASTER;
    case DarkKnight:
        return CLASS_KNIGHT;
    case BladeKnight:
        return CLASS_BLADEKNIGHT;
    case BladeMaster:
        return CLASS_BLADEMASTER;
    case FairyElf:
        return CLASS_ELF;
    case MuseElf:
        return CLASS_MUSEELF;
    case HighElf:
        return CLASS_HIGHELF;
    case MagicGladiator:
        return CLASS_DARK;
    case DuelMaster:
        return CLASS_DUELMASTER;
    case DarkLord:
        return CLASS_DARK_LORD;
    case LordEmperor:
        return CLASS_LORDEMPEROR;
    case Summoner:
        return CLASS_SUMMONER;
    case BloodySummoner:
        return CLASS_BLOODYSUMMONER;
    case DimensionMaster:
        return CLASS_DIMENSIONMASTER;
    case RageFighter:
        return CLASS_RAGEFIGHTER;
    case FistMaster:
        return CLASS_TEMPLENIGHT;
    }

    return CLASS_WIZARD;
}

CLASS_TYPE CCharacterManager::GetBaseClass(CLASS_TYPE iClass)
{
    switch (iClass)
    {
    case CLASS_GRANDMASTER:
    case CLASS_SOULMASTER:
        return CLASS_WIZARD;
    case CLASS_BLADEKNIGHT:
    case CLASS_BLADEMASTER:
        return CLASS_KNIGHT;
    case CLASS_MUSEELF:
    case CLASS_HIGHELF:
        return CLASS_ELF;
    case CLASS_BLOODYSUMMONER:
    case CLASS_DIMENSIONMASTER:
        return CLASS_SUMMONER;
    case CLASS_DUELMASTER:
        return CLASS_DARK;
    case CLASS_LORDEMPEROR:
        return CLASS_DARK_LORD;
    case CLASS_TEMPLENIGHT:
        return CLASS_RAGEFIGHTER;
    }

    return iClass;
}

bool CCharacterManager::IsSecondClass(const CLASS_TYPE byClass)
{
    switch (byClass)
    {
    case CLASS_SOULMASTER:
    case CLASS_BLADEKNIGHT:
    case CLASS_MUSEELF:
    case CLASS_BLOODYSUMMONER:
        return true;
    }

    return false;
}

bool CCharacterManager::IsThirdClass(const CLASS_TYPE byClass)
{
    switch (byClass)
    {
    case CLASS_GRANDMASTER:
    case CLASS_BLADEMASTER:
    case CLASS_HIGHELF:
    case CLASS_DIMENSIONMASTER:
    case CLASS_DUELMASTER:
    case CLASS_LORDEMPEROR:
    case CLASS_TEMPLENIGHT:
        return true;
    }

    return false;
}

bool CCharacterManager::IsMasterLevel(const CLASS_TYPE byClass)
{
    return this->IsThirdClass(byClass);
}

const wchar_t* CCharacterManager::GetCharacterClassText(const CLASS_TYPE byCharacterClass)
{
    if (byCharacterClass == CLASS_WIZARD)
    {
        return GlobalText[20];
    }
    if (byCharacterClass == CLASS_SOULMASTER)
    {
        return GlobalText[25];
    }
    if (byCharacterClass == CLASS_GRANDMASTER)
    {
        return GlobalText[1669];
    }
    if (byCharacterClass == CLASS_KNIGHT)
    {
        return GlobalText[21];
    }
    if (byCharacterClass == CLASS_BLADEKNIGHT)
    {
        return GlobalText[26];
    }
    if (byCharacterClass == CLASS_BLADEMASTER)
    {
        return GlobalText[1668];
    }
    if (byCharacterClass == CLASS_ELF)
    {
        return GlobalText[22];
    }
    if (byCharacterClass == CLASS_MUSEELF)
    {
        return GlobalText[27];
    }
    if (byCharacterClass == CLASS_HIGHELF)
    {
        return GlobalText[1670];
    }
    if (byCharacterClass == CLASS_DARK)
    {
        return GlobalText[23];
    }
    if (byCharacterClass == CLASS_DUELMASTER)
    {
        return GlobalText[1671];
    }
    if (byCharacterClass == CLASS_DARK_LORD)
    {
        return GlobalText[24];
    }
    if (byCharacterClass == CLASS_LORDEMPEROR)
    {
        return GlobalText[1672];
    }
    if (byCharacterClass == CLASS_SUMMONER)
        return GlobalText[1687];
    if (byCharacterClass == CLASS_BLOODYSUMMONER)
        return GlobalText[1688];
    if (byCharacterClass == CLASS_DIMENSIONMASTER)
        return GlobalText[1689];
    if (byCharacterClass == CLASS_RAGEFIGHTER)
        return GlobalText[3150];	// 3150 "레이지파이터"
    if (byCharacterClass == CLASS_TEMPLENIGHT)
        return GlobalText[3151];
    // 3151 "템플나이트"
    return GlobalText[2305];
}

CLASS_SKIN_INDEX CCharacterManager::GetSkinModelIndex(const CLASS_TYPE byClass)
{
    switch (byClass)
    {
    case CLASS_BLOODYSUMMONER: return SKIN_CLASS_BLOODYSUMMONER;
    case CLASS_GRANDMASTER: return SKIN_CLASS_GRANDMASTER;
    case CLASS_BLADEMASTER: return SKIN_CLASS_BLADEMASTER;
    case CLASS_HIGHELF: return SKIN_CLASS_HIGHELF;
    case CLASS_DUELMASTER:return SKIN_CLASS_DUELMASTER;
    case CLASS_LORDEMPEROR: return SKIN_CLASS_LORDEMPEROR;
    case CLASS_DIMENSIONMASTER: return SKIN_CLASS_DIMENSIONMASTER;
    case CLASS_TEMPLENIGHT: return SKIN_CLASS_TEMPLENIGHT;
    }

    return static_cast<CLASS_SKIN_INDEX>(byClass);
}

BYTE CCharacterManager::GetStepClass(const CLASS_TYPE byClass)
{
    if (IsThirdClass(byClass))
    {
        return 3;
    }
    else if (this->IsSecondClass(byClass) == true && this->IsThirdClass(byClass) == false)
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

int CCharacterManager::GetEquipedBowType(CHARACTER* pChar)
{
    if ((pChar->Weapon[1].Type != MODEL_BOLT) && ((pChar->Weapon[1].Type >= MODEL_BOW) && (pChar->Weapon[1].Type < MODEL_BOW + MAX_ITEM_INDEX)))
    {
        return BOWTYPE_BOW;
    }
    else if ((pChar->Weapon[0].Type != MODEL_ARROWS) && ((pChar->Weapon[0].Type >= MODEL_CROSSBOW) && (pChar->Weapon[0].Type < MODEL_BOW + MAX_ITEM_INDEX)))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType()
{
    if ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOLT) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type >= ITEM_BOW) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type < ITEM_BOW + MAX_ITEM_INDEX)))
    {
        return BOWTYPE_BOW;
    }
    else if ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_ARROWS) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type >= ITEM_CROSSBOW) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type < ITEM_BOW + MAX_ITEM_INDEX)))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType(ITEM* pItem)
{
    if (((pItem->Type >= ITEM_BOW) && (pItem->Type <= ITEM_CHAOS_NATURE_BOW)) || (pItem->Type == ITEM_CELESTIAL_BOW) || ((pItem->Type >= ITEM_ARROW_VIPER_BOW) && (pItem->Type <= ITEM_STINGER_BOW)) || (pItem->Type == ITEM_AIR_LYN_BOW))
    {
        return BOWTYPE_BOW;
    }

    else if (((pItem->Type >= ITEM_CROSSBOW) && (pItem->Type <= ITEM_AQUAGOLD_CROSSBOW)) || (pItem->Type == ITEM_SAINT_CROSSBOW) || ((pItem->Type >= ITEM_DIVINE_CB_OF_ARCHANGEL) && (pItem->Type <= ITEM_GREAT_REIGN_CROSSBOW)))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType_Skill()
{
    if ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOLT) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type >= ITEM_BOW) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type < ITEM_BOW + MAX_ITEM_INDEX)))
    {
        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type == ITEM_ARROWS)
            return BOWTYPE_BOW;
    }
    else if ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_ARROWS) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type >= ITEM_CROSSBOW) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type < ITEM_BOW + MAX_ITEM_INDEX)))
    {
        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type == ITEM_BOLT)
            return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

bool CCharacterManager::IsEquipedWing()
{
    ITEM* pEquippedItem = &CharacterMachine->Equipment[EQUIPMENT_WING];

    if ((pEquippedItem->Type >= ITEM_WING && pEquippedItem->Type <= ITEM_WINGS_OF_DARKNESS)
        || (pEquippedItem->Type >= ITEM_WING_OF_STORM && pEquippedItem->Type <= ITEM_WING_OF_DIMENSION)
        || (pEquippedItem->Type == ITEM_CAPE_OF_LORD)
        || (ITEM_WING + 130 <= pEquippedItem->Type && pEquippedItem->Type <= ITEM_WING + 134)
        || (pEquippedItem->Type >= ITEM_CAPE_OF_FIGHTER && pEquippedItem->Type <= ITEM_CAPE_OF_OVERRULE) || (pEquippedItem->Type == ITEM_WING + 135))
    {
        return true;
    }

    return false;
}

void CCharacterManager::GetMagicSkillDamage(int iType, int* piMinDamage, int* piMaxDamage)
{
    if (AT_SKILL_SUMMON_EXPLOSION <= iType && iType <= AT_SKILL_SUMMON_REQUIEM)
    {
        *piMinDamage = CharacterMachine->Character.MagicDamageMin;
        *piMaxDamage = CharacterMachine->Character.MagicDamageMax;
        return;
    }

    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    int Damage = p->Damage;

    *piMinDamage = CharacterMachine->Character.MagicDamageMin + Damage;
    *piMaxDamage = CharacterMachine->Character.MagicDamageMax + Damage + Damage / 2;

    Damage = 0;
    g_csItemOption.PlusSpecial((WORD*)&Damage, AT_SET_OPTION_IMPROVE_MAGIC_POWER);
    if (Damage != 0)
    {
        float fratio = 1.f + (float)Damage / 100.f;
        *piMinDamage *= fratio;
        *piMaxDamage *= fratio;
    }

    Damage = 0;
    g_csItemOption.PlusMastery(&Damage, p->MasteryType);
    g_csItemOption.PlusSpecial((WORD*)&Damage, AT_SET_OPTION_IMPROVE_SKILL_ATTACK);
    *piMinDamage += Damage;
    *piMaxDamage += Damage;
}

void CCharacterManager::GetCurseSkillDamage(int iType, int* piMinDamage, int* piMaxDamage)
{
    if (CLASS_SUMMONER != gCharacterManager.GetBaseClass(CharacterMachine->Character.Class))
        return;

    if (AT_SKILL_SUMMON_EXPLOSION <= iType && iType <= AT_SKILL_SUMMON_REQUIEM)
    {
        SKILL_ATTRIBUTE* p = &SkillAttribute[iType];
        *piMinDamage = CharacterMachine->Character.CurseDamageMin + p->Damage;
        *piMaxDamage = CharacterMachine->Character.CurseDamageMax + p->Damage + p->Damage / 2;
    }
    else
    {
        *piMinDamage = CharacterMachine->Character.CurseDamageMin;
        *piMaxDamage = CharacterMachine->Character.CurseDamageMax;
    }
}

void CCharacterManager::GetSkillDamage(int iType, int* piMinDamage, int* piMaxDamage)
{
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    int Damage = p->Damage;

    *piMinDamage = Damage;
    *piMaxDamage = Damage + Damage / 2;

    Damage = 0;
    g_csItemOption.PlusMastery(&Damage, p->MasteryType);
    g_csItemOption.PlusSpecial((WORD*)&Damage, AT_SET_OPTION_IMPROVE_SKILL_ATTACK);
    *piMinDamage += Damage;
    *piMaxDamage += Damage;
}