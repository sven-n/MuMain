#pragma once

class CCharacterManager
{
public:
    CCharacterManager();
    virtual ~CCharacterManager();
    CLASS_TYPE ChangeServerClassTypeToClientClassType(const SERVER_CLASS_TYPE byServerClassType);
    bool IsSecondClass(const CLASS_TYPE byClass);
    bool IsThirdClass(const CLASS_TYPE byClass);
    bool IsMasterLevel(const CLASS_TYPE byClass);
    CLASS_TYPE GetBaseClass(CLASS_TYPE iClass);
    const wchar_t* GetCharacterClassText(const CLASS_TYPE byClass);
    
    int IsFemale(CLASS_TYPE iClass) { return (this->GetBaseClass(iClass) == CLASS_ELF || this->GetBaseClass(iClass) == CLASS_SUMMONER); }
    CLASS_SKIN_INDEX GetSkinModelIndex(const CLASS_TYPE byClass);
    BYTE GetStepClass(const CLASS_TYPE byClass);
    int GetEquipedBowType(CHARACTER* pChar);
    int GetEquipedBowType();
    int GetEquipedBowType(ITEM* pItem);
    int GetEquipedBowType_Skill();
    bool IsEquipedWing();
    void GetMagicSkillDamage(int iType, int* piMinDamage, int* piMaxDamage);
    void GetCurseSkillDamage(int iType, int* piMinDamage, int* piMaxDamage);
    void GetSkillDamage(int iType, int* piMinDamage, int* piMaxDamage);
public:
};

extern CCharacterManager gCharacterManager;
