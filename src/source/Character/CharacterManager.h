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
    bool IsMasterExperienceActive(const CLASS_TYPE byClass, const int level);
    // The hero's master level; 0 for a hero whose class has none, and 0 after a character is picked until the server
    // sends the new hero's (ForgetHeroMasterLevel).
    int GetHeroMasterLevel();
    // Forgets the master level values of the hero before, when a character is picked: the new hero's come from the
    // server after it enters the game.
    void ForgetHeroMasterLevel();
    CLASS_TYPE GetBaseClass(CLASS_TYPE iClass);
    const wchar_t* GetCharacterClassText(const CLASS_TYPE byClass);

    int IsFemale(CLASS_TYPE iClass)
    {
        return (this->GetBaseClass(iClass) == CLASS_ELF || this->GetBaseClass(iClass) == CLASS_SUMMONER);
    }
    CLASS_SKIN_INDEX GetSkinModelIndex(const CLASS_TYPE byClass);
    BYTE GetStepClass(const CLASS_TYPE byClass);
    int GetEquipedBowType(const CHARACTER* pChar);
    int GetEquipedBowType();
    int GetEquipedBowType(ITEM* pItem);
    int GetEquipedBowType_Skill();
    void GetMagicSkillDamage(int iType, int* piMinDamage, int* piMaxDamage);
    void GetCurseSkillDamage(int iType, int* piMinDamage, int* piMaxDamage);
    void GetSkillDamage(int iType, int* piMinDamage, int* piMaxDamage);

public:
};

extern CCharacterManager gCharacterManager;
