// SkillManager.cpp: implementation of the CSkillManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillManager.h"
#include "Character/CharacterManager.h"
#include "World/MapInfra/MapManager.h"
#include "Engine/Object/ZzzCharacter.h"

CSkillManager gSkillManager;

const std::map<ActionSkillType, ActionSkillType> SKILL_REPLACEMENTS = {
    {AT_SKILL_POISON_STR, AT_SKILL_POISON},
    {AT_SKILL_LIGHTNING_STR, AT_SKILL_LIGHTNING},
    {AT_SKILL_LIGHTNING_STR_MG, AT_SKILL_LIGHTNING},
    {AT_SKILL_FLAME_STR, AT_SKILL_FLAME},
    {AT_SKILL_FLAME_STR_MG, AT_SKILL_FLAME},
    {AT_SKILL_ICE_STR, AT_SKILL_ICE},
    {AT_SKILL_ICE_STR_MG, AT_SKILL_ICE},
    {AT_SKILL_EVIL_SPIRIT_STR, AT_SKILL_EVIL_SPIRIT},
    {AT_SKILL_EVIL_SPIRIT_STR_MG, AT_SKILL_EVIL_SPIRIT},
    {AT_SKILL_HELL_FIRE_STR, AT_SKILL_HELL_FIRE},
    {AT_SKILL_BLAST_STR, AT_SKILL_BLAST},
    {AT_SKILL_BLAST_STR_MG, AT_SKILL_BLAST},
    {AT_SKILL_INFERNO_STR, AT_SKILL_INFERNO},
    {AT_SKILL_INFERNO_STR_MG, AT_SKILL_INFERNO},
    {AT_SKILL_SOUL_BARRIER_STR, AT_SKILL_SOUL_BARRIER},
    {AT_SKILL_SOUL_BARRIER_PROFICIENCY, AT_SKILL_SOUL_BARRIER_STR},
    {AT_SKILL_FALLING_SLASH_STR, AT_SKILL_FALLING_SLASH},
    {AT_SKILL_LUNGE_STR, AT_SKILL_LUNGE},
    {AT_SKILL_CYCLONE_STR, AT_SKILL_CYCLONE},
    {AT_SKILL_CYCLONE_STR_MG, AT_SKILL_CYCLONE},
    {AT_SKILL_SLASH_STR, AT_SKILL_SLASH},
    {AT_SKILL_TRIPLE_SHOT_STR, AT_SKILL_TRIPLE_SHOT},
    {AT_SKILL_TRIPLE_SHOT_MASTERY, AT_SKILL_TRIPLE_SHOT_STR},
    {AT_SKILL_HEALING_STR, AT_SKILL_HEALING},
    {AT_SKILL_DEFENSE_STR, AT_SKILL_DEFENSE},
    {AT_SKILL_DEFENSE_MASTERY, AT_SKILL_DEFENSE_STR},
    {AT_SKILL_ATTACK_STR, AT_SKILL_ATTACK},
    {AT_SKILL_ATTACK_MASTERY, AT_SKILL_ATTACK_STR},
    {AT_SKILL_DECAY_STR, AT_SKILL_DECAY},
    {AT_SKILL_TWISTING_SLASH_STR, AT_SKILL_TWISTING_SLASH},
    {AT_SKILL_TWISTING_SLASH_STR_MG, AT_SKILL_TWISTING_SLASH},
    {AT_SKILL_TWISTING_SLASH_MASTERY, AT_SKILL_TWISTING_SLASH_STR},
    {AT_SKILL_RAGEFUL_BLOW_STR, AT_SKILL_RAGEFUL_BLOW},
    {AT_SKILL_RAGEFUL_BLOW_MASTERY, AT_SKILL_RAGEFUL_BLOW_STR},
    {AT_SKILL_DEATHSTAB_STR, AT_SKILL_DEATHSTAB},
    {AT_SKILL_SWELL_LIFE_STR, AT_SKILL_SWELL_LIFE},
    {AT_SKILL_SWELL_LIFE_PROFICIENCY, AT_SKILL_SWELL_LIFE_STR},
    {AT_SKILL_ICE_ARROW_STR, AT_SKILL_ICE_ARROW},
    {AT_SKILL_PENETRATION_STR, AT_SKILL_PENETRATION},
    {AT_SKILL_FIRE_SLASH_STR, AT_SKILL_FIRE_SLASH},
    {AT_SKILL_POWER_SLASH_STR, AT_SKILL_POWER_SLASH},
    {AT_SKILL_FORCE_WAVE_STR, AT_SKILL_FORCE_WAVE},
    {AT_SKILL_FIREBURST_STR, AT_SKILL_FIREBURST},
    {AT_SKILL_FIREBURST_MASTERY, AT_SKILL_FIREBURST_STR},
    {AT_SKILL_EARTHSHAKE_STR, AT_SKILL_EARTHSHAKE},
    {AT_SKILL_EARTHSHAKE_MASTERY, AT_SKILL_EARTHSHAKE_STR},
    {AT_SKILL_ADD_CRITICAL_STR1, AT_SKILL_ADD_CRITICAL},
    {AT_SKILL_ADD_CRITICAL_STR2, AT_SKILL_ADD_CRITICAL_STR1},
    {AT_SKILL_ADD_CRITICAL_STR3, AT_SKILL_ADD_CRITICAL_STR2},
    {AT_SKILL_INFINITY_ARROW_STR, AT_SKILL_INFINITY_ARROW},
    {AT_SKILL_FIRE_SCREAM_STR, AT_SKILL_FIRE_SCREAM},
    {AT_SKILL_ALICE_DRAINLIFE_STR, AT_SKILL_ALICE_DRAINLIFE},
    {AT_SKILL_ALICE_CHAINLIGHTNING_STR, AT_SKILL_ALICE_CHAINLIGHTNING},
    {AT_SKILL_ALICE_BERSERKER_STR, AT_SKILL_ALICE_BERSERKER},
    {AT_SKILL_ALICE_SLEEP_STR, AT_SKILL_ALICE_SLEEP},
    {AT_SKILL_LIGHTNING_SHOCK_STR, AT_SKILL_LIGHTNING_SHOCK},
    {AT_SKILL_STRIKE_OF_DESTRUCTION_STR, AT_SKILL_STRIKE_OF_DESTRUCTION},
    {AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY, AT_SKILL_EXPANSION_OF_WIZARDRY_STR},
    {AT_SKILL_EXPANSION_OF_WIZARDRY_STR, AT_SKILL_EXPANSION_OF_WIZARDRY},
    {AT_SKILL_KILLING_BLOW_STR, AT_SKILL_KILLING_BLOW},
    {AT_SKILL_KILLING_BLOW_MASTERY, AT_SKILL_KILLING_BLOW_STR},
    {AT_SKILL_BEAST_UPPERCUT_STR, AT_SKILL_BEAST_UPPERCUT},
    {AT_SKILL_BEAST_UPPERCUT_MASTERY, AT_SKILL_BEAST_UPPERCUT_STR},
    {AT_SKILL_CHAIN_DRIVE_STR, AT_SKILL_CHAIN_DRIVE},
    {AT_SKILL_DARKSIDE_STR, AT_SKILL_DARKSIDE},
    {AT_SKILL_DRAGON_ROAR_STR, AT_SKILL_DRAGON_ROAR},
    {AT_SKILL_HP_UP_OURFORCES_STR, AT_SKILL_HP_UP_OURFORCES},
    {AT_SKILL_DEF_UP_OURFORCES_STR, AT_SKILL_DEF_UP_OURFORCES},
    {AT_SKILL_DEF_UP_OURFORCES_MASTERY, AT_SKILL_DEF_UP_OURFORCES_STR},
};

extern bool CheckAttack();

namespace
{
// Energy requirement formula constants. The scaling formula for a skill is
// `<base> + (BMD.Energy * BMD.Level * <scale>) / 100`, where <base> and
// <scale> vary by class and skill family.
constexpr int ENERGY_REQ_BASE_DEFAULT = 20;
constexpr int ENERGY_REQ_BASE_KNIGHT = 10;
constexpr int ENERGY_REQ_SCALE_DEFAULT_PERCENT = 4;
constexpr int ENERGY_REQ_SCALE_SUMMON_PERCENT = 3;
}

CSkillManager::CSkillManager() // OK
{
    m_bSkillAttributeRequirementsCacheDirty = true;
    memset(m_aSkillAttributeRequirementsMet, 0, sizeof(m_aSkillAttributeRequirementsMet));
}

CSkillManager::~CSkillManager() // OK
{
}

int CSkillManager::FindSkillIndex(int skillType) const
{
    if (skillType == AT_SKILL_NOVA_BEGIN)
        skillType = AT_SKILL_NOVA;
    for (int slot = 0; slot < MAX_MAGIC; ++slot)
    {
        if (CharacterAttribute->Skill[slot] == skillType)
            return slot;
    }
    return -1;
}

bool CSkillManager::FindHeroSkill(ActionSkillType eSkillType)
{
    for (int i = 0; i < CharacterAttribute->SkillNumber; ++i)
    {
        if (CharacterAttribute->Skill[i] == eSkillType)
        {
            return true;
        }
    }
    return false;
}

void CSkillManager::GetSkillInformation(int iType, int iLevel, wchar_t* lpszName, int* piMana, int* piDistance, int* piSkillMana)
{
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];
    if (lpszName)
    {
        wcscpy(lpszName, p->Name);
        // int wchars_num = MultiByteToWideChar(CP_UTF8, 0, p->Name, -1, NULL, 0);
        // MultiByteToWideChar(CP_UTF8, 0, p->Name, -1, lpszName, wchars_num);
    }
    if (piMana)
    {
        *piMana = p->Mana;
    }
    if (piDistance)
    {
        *piDistance = p->Distance;
    }
    if (piSkillMana)
    {
        *piSkillMana = p->AbilityGuage;
    }
}

void CSkillManager::GetSkillInformation_Energy(int iType, int* piEnergy)
{
    if (!piEnergy) return;

    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    // Skills with no energy cost in the BMD are free regardless of their
    // character-level requirement. Without this short-circuit the formula
    // below would charge a fixed ENERGY_REQ_BASE_DEFAULT (20) for any
    // Level>0 skill that the BMD author left at Energy=0, which gates many
    // low-tier or special skills incorrectly.
    if (p->Energy == 0)
    {
        *piEnergy = 0;
        return;
    }

    // BMD Energy is dual-purpose. For Level=0 skills it's the direct cost
    // (Summon Goblin=30, Summon Soldier=300, etc.). For Level>0 skills it's
    // a per-level scaling factor that the formula multiplies up.
    if (p->Level == 0)
    {
        *piEnergy = p->Energy;
        return;
    }

    *piEnergy = ENERGY_REQ_BASE_DEFAULT + (p->Energy * p->Level * ENERGY_REQ_SCALE_DEFAULT_PERCENT / 100);

    if (iType == AT_SKILL_SUMMON_EXPLOSION || iType == AT_SKILL_SUMMON_REQUIEM)
    {
        *piEnergy = ENERGY_REQ_BASE_DEFAULT + (p->Energy * p->Level * ENERGY_REQ_SCALE_SUMMON_PERCENT / 100);
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT)
    {
        *piEnergy = ENERGY_REQ_BASE_KNIGHT + (p->Energy * p->Level * ENERGY_REQ_SCALE_DEFAULT_PERCENT / 100);
    }
}

void CSkillManager::GetSkillInformation_Charisma(int iType, int* piCharisma)
{
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    if (piCharisma)
    {
        *piCharisma = p->Charisma;
    }
}

void CSkillManager::GetSkillInformation_Damage(int iType, int* piDamage)
{
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    if (piDamage)
    {
        *piDamage = p->Damage;
    }
}

float CSkillManager::GetSkillDistance(int Index, CHARACTER* c)
{
    auto Distance = (float)(SkillAttribute[Index].Distance);

    if (c != nullptr && c->Helper.Type == MODEL_DARK_HORSE_ITEM)
    {
        Distance += 2;
    }

    return Distance;
}

bool CSkillManager::CheckSkillDelay(int SkillIndex)
{
    int Skill = CharacterAttribute->Skill[SkillIndex];

    int Delay = SkillAttribute[Skill].Delay;

    if (!CheckAttack() && (
        Skill == AT_SKILL_CHAIN_DRIVE ||
        Skill == AT_SKILL_CHAIN_DRIVE_STR ||
        Skill == AT_SKILL_DRAGON_ROAR ||
        Skill == AT_SKILL_DRAGON_ROAR_STR ||
        Skill == AT_SKILL_DRAGON_KICK))
    {
        return false;
    }

    if (Delay > 0)
    {
        if (CharacterAttribute->SkillDelay[SkillIndex] > 0)
        {
            return false;
        }

        int iCharisma;
        GetSkillInformation_Charisma(Skill, &iCharisma);
        if (iCharisma > (CharacterAttribute->Charisma + CharacterAttribute->AddCharisma))
        {
            return false;
        }

        CharacterAttribute->SkillDelay[SkillIndex] = Delay;
    }
    return true;
}
void CSkillManager::CalcSkillDelay(int time)
{
    int iSkillNumber;
    iSkillNumber = CharacterAttribute->SkillNumber + 2;
    iSkillNumber = std::min<int>(iSkillNumber, MAX_SKILLS);

    for (int i = 0; i < iSkillNumber; ++i)
    {
        if (CharacterAttribute->SkillDelay[i] <= 0)
            continue;

        CharacterAttribute->SkillDelay[i] -= time;
        if (CharacterAttribute->SkillDelay[i] < 0)
        {
            CharacterAttribute->SkillDelay[i] = 0;
        }
    }
}

BYTE CSkillManager::GetSkillMasteryType(ActionSkillType iType)
{
    BYTE MasteryType = 255;
    if (const SKILL_ATTRIBUTE* p = &SkillAttribute[iType])
    {
        MasteryType = p->MasteryType;
    }

    return MasteryType;
}

ActionSkillType CSkillManager::MasterSkillToBaseSkillIndex(ActionSkillType masterSkill)
{
    auto baseSkill = masterSkill;

    while (true)
    {
        if (auto search = SKILL_REPLACEMENTS.find(baseSkill); search != SKILL_REPLACEMENTS.end())
        {
            baseSkill = search->second;
        }
        else
        {
            break;
        }
    }

    return baseSkill;
}

bool CSkillManager::skillVScharactorCheck(const DemendConditionInfo& basicInfo, const DemendConditionInfo& heroInfo)
{
    if (basicInfo <= heroInfo)
    {
        return true;
    }
    return false;
}

bool CSkillManager::AreSkillAttributeRequirementsMet(ActionSkillType skillType)
{
    if (skillType >= MAX_SKILLS)
    {
        return false;
    }

    // Rebuild cache if dirty (on first call after stat/skill changes)
    if (m_bSkillAttributeRequirementsCacheDirty)
    {
        RebuildSkillAttributeRequirementsCache();
    }

    return m_aSkillAttributeRequirementsMet[skillType];
}

void CSkillManager::InvalidateSkillAttributeRequirementsCache()
{
    m_bSkillAttributeRequirementsCacheDirty = true;
}

void CSkillManager::InitializeSkillAttributeRequirementsCache()
{
    m_bSkillAttributeRequirementsCacheDirty = true;
    RebuildSkillAttributeRequirementsCache();
}

void CSkillManager::RebuildSkillAttributeRequirementsCache()
{
    if (!m_bSkillAttributeRequirementsCacheDirty)
    {
        return;
    }

    const bool isGuardian = gMapManager.IsEmpireGuardian();

    DemendConditionInfo heroCharacterInfo;
    heroCharacterInfo.SkillLevel = CharacterMachine->Character.Level;
    heroCharacterInfo.SkillStrength = CharacterMachine->Character.Strength + CharacterMachine->Character.AddStrength;
    heroCharacterInfo.SkillDexterity = CharacterMachine->Character.Dexterity + CharacterMachine->Character.AddDexterity;
    heroCharacterInfo.SkillVitality = CharacterMachine->Character.Vitality + CharacterMachine->Character.AddVitality;
    heroCharacterInfo.SkillEnergy = CharacterMachine->Character.Energy + CharacterMachine->Character.AddEnergy;
    heroCharacterInfo.SkillCharisma = CharacterMachine->Character.Charisma + CharacterMachine->Character.AddCharisma;

    for (int skillType = 0; skillType < MAX_SKILLS; ++skillType)
    {
        ActionSkillType baseSkill = MasterSkillToBaseSkillIndex(static_cast<ActionSkillType>(skillType));
        if (isGuardian && (baseSkill == AT_SKILL_TELEPORT_ALLY || baseSkill == AT_SKILL_TELEPORT))
        {
            m_aSkillAttributeRequirementsMet[skillType] = false;
            continue;
        }

        DemendConditionInfo skillRequirements;
        skillRequirements.SkillLevel = SkillAttribute[baseSkill].Level;
        skillRequirements.SkillStrength = SkillAttribute[baseSkill].Strength;
        skillRequirements.SkillDexterity = SkillAttribute[baseSkill].Dexterity;
        skillRequirements.SkillVitality = 0;
        int reqEnergy = 0;
        GetSkillInformation_Energy(baseSkill, &reqEnergy);
        skillRequirements.SkillEnergy = static_cast<WORD>(reqEnergy);
        skillRequirements.SkillCharisma = SkillAttribute[baseSkill].Charisma;

        m_aSkillAttributeRequirementsMet[skillType] = (skillRequirements <= heroCharacterInfo);
    }

    m_bSkillAttributeRequirementsCacheDirty = false;
}