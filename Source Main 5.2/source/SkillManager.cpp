// SkillManager.cpp: implementation of the CSkillManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillManager.h"
#include "CharacterManager.h"
#include "MapManager.h"
#include "ZzzCharacter.h"

CSkillManager gSkillManager;
extern bool CheckAttack();

CSkillManager::CSkillManager() // OK
{
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
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    if (piEnergy)
    {
        if (p->Energy == 0)
        {
            *piEnergy = 0;
        }
        else
        {
            *piEnergy = 20 + (p->Energy * p->Level * 4 / 100);

            if (iType == AT_SKILL_SUMMON_EXPLOSION || iType == AT_SKILL_SUMMON_REQUIEM) {
                *piEnergy = 20 + (p->Energy * p->Level * 3 / 100);
            }

            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT) {
                *piEnergy = 10 + (p->Energy * p->Level * 4 / 100);
            }
        }
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
    iSkillNumber = min(iSkillNumber, MAX_SKILLS);

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

    if ((true == gMapManager.IsEmpireGuardian()) && (skillType == AT_SKILL_TELEPORT_ALLY || skillType == AT_SKILL_TELEPORT))
    {
        return false;
    }

    //if (SkillAttribute[SkillType].Energy == 0)
    //{
    //    return true;
    //}

    skillType = MasterSkillToBaseSkillIndex(skillType);

    DemendConditionInfo skillRequirements;

    skillRequirements.SkillLevel = SkillAttribute[skillType].Level;
    skillRequirements.SkillStrength = SkillAttribute[skillType].Strength;
    skillRequirements.SkillDexterity = SkillAttribute[skillType].Dexterity;
    skillRequirements.SkillVitality = 0;
    skillRequirements.SkillEnergy = (20 + (SkillAttribute[skillType].Energy * SkillAttribute[skillType].Level) * 0.04);
    skillRequirements.SkillCharisma = SkillAttribute[skillType].Charisma;

    DemendConditionInfo heroCharacterInfo;

    heroCharacterInfo.SkillLevel = CharacterMachine->Character.Level;
    heroCharacterInfo.SkillStrength = CharacterMachine->Character.Strength + CharacterMachine->Character.AddStrength;
    heroCharacterInfo.SkillDexterity = CharacterMachine->Character.Dexterity + CharacterMachine->Character.AddDexterity;
    heroCharacterInfo.SkillVitality = CharacterMachine->Character.Vitality + CharacterMachine->Character.AddVitality;
    heroCharacterInfo.SkillEnergy = CharacterMachine->Character.Energy + CharacterMachine->Character.AddEnergy;
    heroCharacterInfo.SkillCharisma = CharacterMachine->Character.Charisma + CharacterMachine->Character.AddCharisma;

    auto result = skillRequirements <= heroCharacterInfo;

    return result;
}