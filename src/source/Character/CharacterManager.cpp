// CharacterManager.cpp: implementation of the CCharacterManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharacterManager.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/ItemCategories.h"

#include <algorithm>
#include <array>

namespace
{
    struct ClassTextEntry
    {
        CLASS_TYPE type;
        const wchar_t* const* text;
    };

    const wchar_t* const* const kDefaultClassText = &I18N::Game::ErrorCode;
    constexpr int kMasterExperienceUnlockLevel = 400;

    const ClassTextEntry kClassTextEntries[] = {
        {CLASS_WIZARD, &I18N::Game::DarkWizard},
        {CLASS_SOULMASTER, &I18N::Game::SoulMaster},
        {CLASS_GRANDMASTER, &I18N::Game::GrandMaster},
        {CLASS_KNIGHT, &I18N::Game::DarkKnight},
        {CLASS_BLADEKNIGHT, &I18N::Game::BladeKnight},
        {CLASS_BLADEMASTER, &I18N::Game::BladeMaster},
        {CLASS_ELF, &I18N::Game::Elf},
        {CLASS_MUSEELF, &I18N::Game::MuseElf},
        {CLASS_HIGHELF, &I18N::Game::HighElf},
        {CLASS_DARK, &I18N::Game::MagicGladiator},
        {CLASS_DUELMASTER, &I18N::Game::DualMaster},
        {CLASS_DARK_LORD, &I18N::Game::DarkLord},
        {CLASS_LORDEMPEROR, &I18N::Game::LordEmperor},
        {CLASS_SUMMONER, &I18N::Game::Summoner},
        {CLASS_BLOODYSUMMONER, &I18N::Game::BloodySummoner},
        {CLASS_DIMENSIONMASTER, &I18N::Game::DimensionMaster},
        {CLASS_RAGEFIGHTER, &I18N::Game::RageFighter},
        {CLASS_TEMPLENIGHT, &I18N::Game::FistMaster},
    };
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

bool CCharacterManager::IsMasterExperienceActive(const CLASS_TYPE byClass, const int level)
{
    return this->IsMasterLevel(byClass) && level >= kMasterExperienceUnlockLevel;
}

const wchar_t* CCharacterManager::GetCharacterClassText(const CLASS_TYPE byCharacterClass)
{
    const auto it =
        std::find_if(std::begin(kClassTextEntries), std::end(kClassTextEntries),
                     [byCharacterClass](const ClassTextEntry& entry) { return entry.type == byCharacterClass; });
    return (it != std::end(kClassTextEntries)) ? *it->text : *kDefaultClassText;
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
    if (GameLogic::Items::IsBowModel(pChar->Weapon[1].Type))
    {
        return BOWTYPE_BOW;
    }
    if (GameLogic::Items::IsCrossbowModel(pChar->Weapon[0].Type))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType()
{
    const ITEM& left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
    if (GameLogic::Items::IsBowType(left.Type))
    {
        return BOWTYPE_BOW;
    }

    const ITEM& right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
    if (GameLogic::Items::IsCrossbowType(right.Type))
    {
        return BOWTYPE_CROSSBOW;
    }

    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType(ITEM* pItem)
{
    if (GameLogic::Items::IsBowType(pItem->Type))
    {
        return BOWTYPE_BOW;
    }

    if (GameLogic::Items::IsCrossbowType(pItem->Type))
    {
        return BOWTYPE_CROSSBOW;
    }
    return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType_Skill()
{
    const ITEM& left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
    const ITEM& right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];

    if (GameLogic::Items::IsBowType(left.Type) && right.Type == ITEM_ARROWS)
    {
        return BOWTYPE_BOW;
    }

    if (GameLogic::Items::IsCrossbowType(right.Type) && left.Type == ITEM_BOLT)
    {
        return BOWTYPE_CROSSBOW;
    }

    return BOWTYPE_NONE;
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
