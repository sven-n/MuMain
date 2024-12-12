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

    if (Index == AT_SKILL_BLOW_UP + 4)
    {
        if (Distance != 3)
            int aaa = 0;
    }

    if (c != NULL)
    {
        if (c->Helper.Type == MODEL_DARK_HORSE_ITEM)
        {
            Distance += 2;
        }
    }

    return Distance;
}

bool CSkillManager::CheckSkillDelay(int SkillIndex)
{
    int Skill = CharacterAttribute->Skill[SkillIndex];

    int Delay = SkillAttribute[Skill].Delay;

    if (!CheckAttack() && (Skill == AT_SKILL_GIANTSWING || Skill == AT_SKILL_DRAGON_LOWER ||
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

BYTE CSkillManager::GetSkillMasteryType(int iType)
{
    BYTE MasteryType = 255;
    SKILL_ATTRIBUTE* p = &SkillAttribute[iType];

    MasteryType = p->MasteryType;

    return MasteryType;
}

int CSkillManager::MasterSkillToBaseSkillIndex(int iMasterSkillIndex)
{
    int iBaseSkillIndex = 0;
    switch (iMasterSkillIndex)
    {
    case AT_SKILL_EVIL_SPIRIT_UP:
    case AT_SKILL_EVIL_SPIRIT_UP + 1:
    case AT_SKILL_EVIL_SPIRIT_UP + 2:
    case AT_SKILL_EVIL_SPIRIT_UP + 3:
    case AT_SKILL_EVIL_SPIRIT_UP + 4:
    case AT_SKILL_EVIL_SPIRIT_UP_M:
    case AT_SKILL_EVIL_SPIRIT_UP_M + 1:
    case AT_SKILL_EVIL_SPIRIT_UP_M + 2:
    case AT_SKILL_EVIL_SPIRIT_UP_M + 3:
    case AT_SKILL_EVIL_SPIRIT_UP_M + 4:
    {
        iBaseSkillIndex = AT_SKILL_EVIL;
    }
    break;
    case AT_SKILL_SOUL_UP:
    case AT_SKILL_SOUL_UP + 1:
    case AT_SKILL_SOUL_UP + 2:
    case AT_SKILL_SOUL_UP + 3:
    case AT_SKILL_SOUL_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_WIZARDDEFENSE;
    }
    break;
    case AT_SKILL_HELL_FIRE_UP:
    case AT_SKILL_HELL_FIRE_UP + 1:
    case AT_SKILL_HELL_FIRE_UP + 2:
    case AT_SKILL_HELL_FIRE_UP + 3:
    case AT_SKILL_HELL_FIRE_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_BLAST_HELL;
    }
    break;
    case AT_SKILL_ICE_UP:
    case AT_SKILL_ICE_UP + 1:
    case AT_SKILL_ICE_UP + 2:
    case AT_SKILL_ICE_UP + 3:
    case AT_SKILL_ICE_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_BLAST_FREEZE;
    }
    break;
    case AT_SKILL_TORNADO_SWORDA_UP:
    case AT_SKILL_TORNADO_SWORDA_UP + 1:
    case AT_SKILL_TORNADO_SWORDA_UP + 2:
    case AT_SKILL_TORNADO_SWORDA_UP + 3:
    case AT_SKILL_TORNADO_SWORDA_UP + 4:
    case AT_SKILL_TORNADO_SWORDB_UP:
    case AT_SKILL_TORNADO_SWORDB_UP + 1:
    case AT_SKILL_TORNADO_SWORDB_UP + 2:
    case AT_SKILL_TORNADO_SWORDB_UP + 3:
    case AT_SKILL_TORNADO_SWORDB_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_WHEEL;
    }
    break;
    case AT_SKILL_BLOW_UP:
    case AT_SKILL_BLOW_UP + 1:
    case AT_SKILL_BLOW_UP + 2:
    case AT_SKILL_BLOW_UP + 3:
    case AT_SKILL_BLOW_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_DEATHSTAB;
    }
    break;
    case AT_SKILL_ANGER_SWORD_UP:
    case AT_SKILL_ANGER_SWORD_UP + 1:
    case AT_SKILL_ANGER_SWORD_UP + 2:
    case AT_SKILL_ANGER_SWORD_UP + 3:
    case AT_SKILL_ANGER_SWORD_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_FURY_STRIKE;
    }
    break;
    case AT_SKILL_LIFE_UP:
    case AT_SKILL_LIFE_UP + 1:
    case AT_SKILL_LIFE_UP + 2:
    case AT_SKILL_LIFE_UP + 3:
    case AT_SKILL_LIFE_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_VITALITY;
    }
    break;
    case AT_SKILL_HEAL_UP:
    case AT_SKILL_HEAL_UP + 1:
    case AT_SKILL_HEAL_UP + 2:
    case AT_SKILL_HEAL_UP + 3:
    case AT_SKILL_HEAL_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_HEALING;
    }
    break;
    case AT_SKILL_DEF_POWER_UP:
    case AT_SKILL_DEF_POWER_UP + 1:
    case AT_SKILL_DEF_POWER_UP + 2:
    case AT_SKILL_DEF_POWER_UP + 3:
    case AT_SKILL_DEF_POWER_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_DEFENSE;
    }
    break;
    case AT_SKILL_ATT_POWER_UP:
    case AT_SKILL_ATT_POWER_UP + 1:
    case AT_SKILL_ATT_POWER_UP + 2:
    case AT_SKILL_ATT_POWER_UP + 3:
    case AT_SKILL_ATT_POWER_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_ATTACK;
    }
    break;
    case AT_SKILL_MANY_ARROW_UP:
    case AT_SKILL_MANY_ARROW_UP + 1:
    case AT_SKILL_MANY_ARROW_UP + 2:
    case AT_SKILL_MANY_ARROW_UP + 3:
    case AT_SKILL_MANY_ARROW_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_CROSSBOW;
    }
    break;
    case AT_SKILL_BLOOD_ATT_UP:
    case AT_SKILL_BLOOD_ATT_UP + 1:
    case AT_SKILL_BLOOD_ATT_UP + 2:
    case AT_SKILL_BLOOD_ATT_UP + 3:
    case AT_SKILL_BLOOD_ATT_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_REDUCEDEFENSE;
    }
    break;
    case AT_SKILL_POWER_SLASH_UP:
    case AT_SKILL_POWER_SLASH_UP + 1:
    case AT_SKILL_POWER_SLASH_UP + 2:
    case AT_SKILL_POWER_SLASH_UP + 3:
    case AT_SKILL_POWER_SLASH_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_ICE_BLADE;
    }
    break;
    case AT_SKILL_BLAST_UP:
    case AT_SKILL_BLAST_UP + 1:
    case AT_SKILL_BLAST_UP + 2:
    case AT_SKILL_BLAST_UP + 3:
    case AT_SKILL_BLAST_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_BLAST;
    }
    break;
    case AT_SKILL_ASHAKE_UP:
    case AT_SKILL_ASHAKE_UP + 1:
    case AT_SKILL_ASHAKE_UP + 2:
    case AT_SKILL_ASHAKE_UP + 3:
    case AT_SKILL_ASHAKE_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_DARK_HORSE;
    }
    break;
    case AT_SKILL_FIRE_BUST_UP:
    case AT_SKILL_FIRE_BUST_UP + 1:
    case AT_SKILL_FIRE_BUST_UP + 2:
    case AT_SKILL_FIRE_BUST_UP + 3:
    case AT_SKILL_FIRE_BUST_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_LONGPIER_ATTACK;
    }
    break;
    case AT_SKILL_FIRE_SCREAM_UP:
    case AT_SKILL_FIRE_SCREAM_UP + 1:
    case AT_SKILL_FIRE_SCREAM_UP + 2:
    case AT_SKILL_FIRE_SCREAM_UP + 3:
    case AT_SKILL_FIRE_SCREAM_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_DARK_SCREAM;
    }
    break;
    case AT_SKILL_ALICE_SLEEP_UP:
    case AT_SKILL_ALICE_SLEEP_UP + 1:
    case AT_SKILL_ALICE_SLEEP_UP + 2:
    case AT_SKILL_ALICE_SLEEP_UP + 3:
    case AT_SKILL_ALICE_SLEEP_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_ALICE_SLEEP;
    }
    break;
    case AT_SKILL_ALICE_CHAINLIGHTNING_UP:
    case AT_SKILL_ALICE_CHAINLIGHTNING_UP + 1:
    case AT_SKILL_ALICE_CHAINLIGHTNING_UP + 2:
    case AT_SKILL_ALICE_CHAINLIGHTNING_UP + 3:
    case AT_SKILL_ALICE_CHAINLIGHTNING_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_ALICE_CHAINLIGHTNING;
    }
    break;
    case AT_SKILL_LIGHTNING_SHOCK_UP:
    case AT_SKILL_LIGHTNING_SHOCK_UP + 1:
    case AT_SKILL_LIGHTNING_SHOCK_UP + 2:
    case AT_SKILL_LIGHTNING_SHOCK_UP + 3:
    case AT_SKILL_LIGHTNING_SHOCK_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_ALICE_LIGHTNINGORB;
    }
    break;
    case AT_SKILL_ALICE_DRAINLIFE_UP:
    case AT_SKILL_ALICE_DRAINLIFE_UP + 1:
    case AT_SKILL_ALICE_DRAINLIFE_UP + 2:
    case AT_SKILL_ALICE_DRAINLIFE_UP + 3:
    case AT_SKILL_ALICE_DRAINLIFE_UP + 4:
    {
        iBaseSkillIndex = AT_SKILL_ALICE_DRAINLIFE;
    }
    break;
    default:
    {
        iBaseSkillIndex = iMasterSkillIndex;
    }
    }

    return iBaseSkillIndex;
}

bool CSkillManager::skillVScharactorCheck(const DemendConditionInfo& basicInfo, const DemendConditionInfo& heroInfo)
{
    if (basicInfo <= heroInfo)
    {
        return true;
    }
    return false;
}

bool CSkillManager::DemendConditionCheckSkill(WORD SkillType)
{
    if (SkillType >= MAX_SKILLS)
    {
        return false;
    }

    if ((true == gMapManager.IsEmpireGuardian()) && (SkillType == AT_SKILL_TELEPORT_B || SkillType == AT_SKILL_TELEPORT))
    {
        return false;
    }

    if (SkillAttribute[SkillType].Energy == 0)
    {
        return true;
    }

    SkillType = MasterSkillToBaseSkillIndex(SkillType);

    bool result = true;

    DemendConditionInfo BasicCharacterInfo;

    BasicCharacterInfo.SkillLevel = SkillAttribute[SkillType].Level;
    BasicCharacterInfo.SkillStrength = SkillAttribute[SkillType].Strength;
    BasicCharacterInfo.SkillDexterity = SkillAttribute[SkillType].Dexterity;
    BasicCharacterInfo.SkillVitality = 0;
    BasicCharacterInfo.SkillEnergy = (20 + (SkillAttribute[SkillType].Energy * SkillAttribute[SkillType].Level) * 0.04);
    BasicCharacterInfo.SkillCharisma = SkillAttribute[SkillType].Charisma;

    DemendConditionInfo HeroCharacterInfo;

    HeroCharacterInfo.SkillLevel = CharacterMachine->Character.Level;
    HeroCharacterInfo.SkillStrength = CharacterMachine->Character.Strength + CharacterMachine->Character.AddStrength;
    HeroCharacterInfo.SkillDexterity = CharacterMachine->Character.Dexterity + CharacterMachine->Character.AddDexterity;
    HeroCharacterInfo.SkillVitality = CharacterMachine->Character.Vitality + CharacterMachine->Character.AddVitality;
    HeroCharacterInfo.SkillEnergy = CharacterMachine->Character.Energy + CharacterMachine->Character.AddEnergy;
    HeroCharacterInfo.SkillCharisma = CharacterMachine->Character.Charisma + CharacterMachine->Character.AddCharisma;

    result = skillVScharactorCheck(BasicCharacterInfo, HeroCharacterInfo);

    return result;
}