// CharacterManager.cpp: implementation of the CCharacterManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharacterManager.h"
#include "CSItemOption.h"
#include "GlobalText.h"
#include "SkillManager.h"
#include "ZzzInfomation.h"

#include <algorithm>
#include <array>
#include <type_traits>

namespace
{
    template <typename T, typename U, typename V>
    constexpr bool InRange(const T value, const U minValue, const V maxValue)
    {
        using Common = std::common_type_t<T, U, V>;
        const Common commonValue = static_cast<Common>(value);
        return (commonValue >= static_cast<Common>(minValue)) && (commonValue <= static_cast<Common>(maxValue));
    }

    struct ClassTextEntry
    {
        CLASS_TYPE type;
        int textIndex;
    };

    constexpr int kThirdGenWingMin = ITEM_WING + 130;
    constexpr int kThirdGenWingMax = ITEM_WING + 134;
    constexpr int kSpecialWingType = ITEM_WING + 135;

    constexpr int kDefaultClassTextIndex = 2305;

    constexpr std::array<ClassTextEntry, 18> kClassTextEntries{{
        { CLASS_WIZARD, 20 },
        { CLASS_SOULMASTER, 25 },
        { CLASS_GRANDMASTER, 1669 },
        { CLASS_KNIGHT, 21 },
        { CLASS_BLADEKNIGHT, 26 },
        { CLASS_BLADEMASTER, 1668 },
        { CLASS_ELF, 22 },
        { CLASS_MUSEELF, 27 },
        { CLASS_HIGHELF, 1670 },
        { CLASS_DARK, 23 },
        { CLASS_DUELMASTER, 1671 },
        { CLASS_DARK_LORD, 24 },
        { CLASS_LORDEMPEROR, 1672 },
        { CLASS_SUMMONER, 1687 },
        { CLASS_BLOODYSUMMONER, 1688 },
        { CLASS_DIMENSIONMASTER, 1689 },
        { CLASS_RAGEFIGHTER, 3150 },
        { CLASS_TEMPLENIGHT, 3151 },
    }};

    constexpr bool IsWingType(int type)
    {
        return InRange(type, ITEM_WING, ITEM_WINGS_OF_DARKNESS) ||
               InRange(type, ITEM_WING_OF_STORM, ITEM_WING_OF_DIMENSION) ||
               type == ITEM_CAPE_OF_LORD ||
               InRange(type, kThirdGenWingMin, kThirdGenWingMax) ||
               InRange(type, ITEM_CAPE_OF_FIGHTER, ITEM_CAPE_OF_OVERRULE) ||
               type == kSpecialWingType;
    }

    constexpr bool IsBowModel(int type)
    {
        return InRange(type, MODEL_BOW, MODEL_CHAOS_NATURE_BOW) ||
               type == MODEL_CELESTIAL_BOW ||
               InRange(type, MODEL_ARROW_VIPER_BOW, MODEL_STINGER_BOW) ||
               type == MODEL_AIR_LYN_BOW;
    }

    constexpr bool IsCrossbowModel(int type)
    {
        return InRange(type, MODEL_CROSSBOW, MODEL_AQUAGOLD_CROSSBOW) ||
               type == MODEL_SAINT_CROSSBOW ||
               InRange(type, MODEL_DIVINE_CB_OF_ARCHANGEL, MODEL_GREAT_REIGN_CROSSBOW);
    }

    constexpr bool IsGeneralBowItem(int type)
    {
        return InRange(type, ITEM_BOW, ITEM_CHAOS_NATURE_BOW) ||
               type == ITEM_CELESTIAL_BOW ||
               InRange(type, ITEM_ARROW_VIPER_BOW, ITEM_STINGER_BOW) ||
               type == ITEM_AIR_LYN_BOW;
    }

    constexpr bool IsGeneralCrossbowItem(int type)
    {
        return InRange(type, ITEM_CROSSBOW, ITEM_AQUAGOLD_CROSSBOW) ||
               type == ITEM_SAINT_CROSSBOW ||
               InRange(type, ITEM_DIVINE_CB_OF_ARCHANGEL, ITEM_GREAT_REIGN_CROSSBOW);
    }

    constexpr bool IsEquippedBowItem(int type)
    {
        return IsGeneralBowItem(type);
    }

    constexpr bool IsEquippedCrossbowItem(int type)
    {
        return IsGeneralCrossbowItem(type);
    }
}

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
    const auto it = std::find_if(kClassTextEntries.begin(), kClassTextEntries.end(),
        [byCharacterClass](const ClassTextEntry& entry) { return entry.type == byCharacterClass; });
    return (it != kClassTextEntries.end()) ? GlobalText[it->textIndex] : GlobalText[kDefaultClassTextIndex];
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
    if (IsBowModel(pChar->Weapon[1].Type))
    {
        return BOWTYPE_BOW;
    }
    if (IsCrossbowModel(pChar->Weapon[0].Type))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType()
{
    const ITEM& left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
    if (IsEquippedBowItem(left.Type))
    {
        return BOWTYPE_BOW;
    }

    const ITEM& right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
    if (IsEquippedCrossbowItem(right.Type))
    {
        return BOWTYPE_CROSSBOW;
    }

    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType(ITEM* pItem)
{
    if (IsGeneralBowItem(pItem->Type))
    {
        return BOWTYPE_BOW;
    }

    if (IsGeneralCrossbowItem(pItem->Type))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType_Skill()
{
    const ITEM& left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
    const ITEM& right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];

    if (IsEquippedBowItem(left.Type) && right.Type == ITEM_ARROWS)
    {
        return BOWTYPE_BOW;
    }

    if (IsEquippedCrossbowItem(right.Type) && left.Type == ITEM_BOLT)
    {
        return BOWTYPE_CROSSBOW;
    }

    return BOWTYPE_NONE;
}

bool CCharacterManager::IsEquipedWing()
{
    const ITEM& equippedWing = CharacterMachine->Equipment[EQUIPMENT_WING];
    return IsWingType(equippedWing.Type);
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