// SkillManager.cpp: implementation of the CSkillManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillManager.h"
#include "Character/CharacterManager.h"
#include "World/MapInfra/MapManager.h"
#include "Engine/Object/ZzzCharacter.h"

CSkillManager gSkillManager;
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
    m_bSkillRequirementsCacheDirty = true;
    memset(m_aSkillRequirementsFulfilled, 0, sizeof(m_aSkillRequirementsFulfilled));
}

CSkillManager::~CSkillManager() // OK
{
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

    // BMD Energy is dual-purpose. For Level=0 skills it's the direct cost
    // (Falling Slash=0, Summon Goblin=90, Summon Soldier=280, etc., matching
    // OpenMU server's energyRequirement values). For Level>0 skills it's a
    // per-level scaling factor that the formula multiplies up to produce the
    // actual requirement (also matching server).
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

bool CSkillManager::IsSkillDisabled(ActionSkillType Type, int Energy, int Charisma)
{
    int SkillEnergy = 0;
    GetSkillInformation_Energy(Type, &SkillEnergy);

    switch (Type)
    {
    case 17:SkillEnergy = 0; break;
    case 30:SkillEnergy = 30; break;
    case 31:SkillEnergy = 60; break;
    case 32:SkillEnergy = 90; break;
    case 33:SkillEnergy = 130; break;
    case 34:SkillEnergy = 170; break;
    case 35:SkillEnergy = 210; break;
    case 36:SkillEnergy = 300; break;
    case 37:SkillEnergy = 500; break;
    case 60:SkillEnergy = 15; break;
    case AT_SKILL_EARTHSHAKE_STR:
    case AT_SKILL_EARTHSHAKE_MASTERY:
    case AT_SKILL_EARTHSHAKE:    SkillEnergy = 0; break;
    case AT_PET_COMMAND_DEFAULT: SkillEnergy = 0; break;
    case AT_PET_COMMAND_RANDOM:  SkillEnergy = 0; break;
    case AT_PET_COMMAND_OWNER:   SkillEnergy = 0; break;
    case AT_PET_COMMAND_TARGET:  SkillEnergy = 0; break;
    case AT_SKILL_PLASMA_STORM_FENRIR: SkillEnergy = 0; break;
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR: SkillEnergy = 0; break;
    case AT_SKILL_STRIKE_OF_DESTRUCTION:
    case AT_SKILL_STRIKE_OF_DESTRUCTION_STR: SkillEnergy = 0; break;
    case AT_SKILL_RECOVER:
    case AT_SKILL_GAOTIC:
    case AT_SKILL_MULTI_SHOT:
    case AT_SKILL_FIRE_SCREAM_STR:
    case AT_SKILL_FIRE_SCREAM:
        SkillEnergy = 0;
        break;

    case AT_SKILL_EXPLODE:
        SkillEnergy = 0;
        break;
    }

    if (Type >= AT_SKILL_STUN && Type <= AT_SKILL_REMOVAL_BUFF)
    {
        SkillEnergy = 0;
    }
    else
        if ((Type >= 18 && Type <= 23) || (Type >= 41 && Type <= 43) || (Type >= 47 && Type <= 49) || Type == 24 || Type == 51 || Type == 52 || Type == 55 || Type == 56)
        {
            SkillEnergy = 0;
        }
        else if (Type == 44 || Type == 45 || Type == 46 || Type == 57 || Type == 73 || Type == 74)
        {
            SkillEnergy = 0;
        }

    if (Charisma > 0)
    {
        const int SkillCharisma = SkillAttribute[Type].Charisma;
        if (Charisma < SkillCharisma)
        {
            return true;
        }
    }

    if (Energy < SkillEnergy)
    {
        return true;
    }

    return false;
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

bool CSkillManager::AreSkillRequirementsFulfilled(ActionSkillType skillType)
{
    if (skillType >= MAX_SKILLS)
    {
        return false;
    }

    // Rebuild cache if dirty (on first call after stat/skill changes)
    if (m_bSkillRequirementsCacheDirty)
    {
        RebuildSkillRequirementsCache();
    }

    return m_aSkillRequirementsFulfilled[skillType];
}

void CSkillManager::InvalidateSkillRequirementsCache()
{
    m_bSkillRequirementsCacheDirty = true;
}

void CSkillManager::InitializeSkillRequirementsCache()
{
    m_bSkillRequirementsCacheDirty = true;
    RebuildSkillRequirementsCache();
}

void CSkillManager::RebuildSkillRequirementsCache()
{
    if (!m_bSkillRequirementsCacheDirty)
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
            m_aSkillRequirementsFulfilled[skillType] = false;
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

        m_aSkillRequirementsFulfilled[skillType] = (skillRequirements <= heroCharacterInfo);
    }

    m_bSkillRequirementsCacheDirty = false;
}