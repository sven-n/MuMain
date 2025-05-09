// SkillManager.h: interface for the CSkillManager class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "_enum.h"

typedef struct DemendConditionInfo
{
    WORD SkillType;
    wchar_t SkillName[100];
    WORD SkillLevel;
    WORD SkillStrength;
    WORD SkillDexterity;
    WORD SkillVitality;
    WORD SkillEnergy;
    WORD SkillCharisma;

    DemendConditionInfo() : SkillType(0), SkillLevel(0), SkillStrength(0),
        SkillDexterity(0), SkillVitality(0), SkillEnergy(0), SkillCharisma(0)
    {
        ZeroMemory(SkillName, 100);
    }
    BOOL operator<=(const DemendConditionInfo& rhs) const
    {
        return
            SkillLevel <= rhs.SkillLevel
            && SkillStrength <= rhs.SkillStrength
            && SkillDexterity <= rhs.SkillDexterity
            && SkillVitality <= rhs.SkillVitality
            && SkillEnergy <= rhs.SkillEnergy
            && SkillCharisma <= rhs.SkillCharisma;
    }
} DemendConditionInfo;

class CSkillManager
{
public:
    CSkillManager();
    virtual ~CSkillManager();
    bool FindHeroSkill(ActionSkillType eSkillType);
    void GetSkillInformation(int iType, int iLevel, wchar_t* lpszName, int* piMana, int* piDistance, int* piSkillMana = NULL);
    void GetSkillInformation_Energy(int iType, int* piEnergy);
    void GetSkillInformation_Charisma(int iType, int* piCharisma);
    float GetSkillDistance(int Index, CHARACTER* c = NULL);
    void GetSkillInformation_Damage(int iType, int* piDamage);
    bool CheckSkillDelay(int SkillIndex);
    void CalcSkillDelay(int time);
    BYTE GetSkillMasteryType(ActionSkillType iType);
    ActionSkillType MasterSkillToBaseSkillIndex(ActionSkillType masterSkill);
    bool skillVScharactorCheck(const DemendConditionInfo& basicInfo, const DemendConditionInfo& heroInfo);
    bool AreSkillRequirementsFulfilled(ActionSkillType skilltype);
public:
};

extern CSkillManager gSkillManager;