#ifndef __ZZZITEM_H__
#define __ZZZITEM_H__

#pragma once

#include "GlobalText.h"

void SaveTextFile(wchar_t* FileName);

extern wchar_t AbuseFilter[MAX_FILTERS][20];
extern wchar_t AbuseNameFilter[MAX_NAMEFILTERS][20];
extern int  AbuseFilterNumber;
extern int  AbuseNameFilterNumber;

void OpenFilterFile(wchar_t* FileName);
void OpenNameFilterFile(wchar_t* FileName);

extern GATE_ATTRIBUTE* GateAttribute;

void OpenGateScript(wchar_t* FileName);
void OpenMonsterSkillScript(wchar_t* FileName);

extern DIALOG_SCRIPT g_DialogScript[MAX_DIALOG];

void OpenDialogFile(wchar_t* FileName);

extern ITEM_ATTRIBUTE* ItemAttribute;

extern void OpenItemScript(wchar_t* FileName);
extern ActionSkillType GetSkillByBook(int Type);
void SetItemAttributes(ITEM* ip);
extern float CalcDurabilityPercent(BYTE dur, BYTE maxDur, int Level, int excellentFlags, int ancientDiscriminator = 0);
extern int64_t ItemValue(ITEM* ip, int goldType = 1);
extern void PrintItem(wchar_t* FileName);

extern MONSTER_SCRIPT MonsterScript[MAX_MONSTER];

extern void  OpenMonsterScript(wchar_t* FileName);
extern void  MonsterConvert(MONSTER* m, int Level);
extern wchar_t* getMonsterName(int type);

extern SKILL_ATTRIBUTE* SkillAttribute;

void OpenSkillScript(wchar_t* FileName);

BOOL IsValidateSkillIdx(INT iSkillIdx);
BOOL IsCorrectSkillType(INT iSkillSeq, eTypeSkill iSkillTypeIdx);
BOOL IsCorrectSkillType_FrendlySkill(INT iSkillSeq);
BOOL IsCorrectSkillType_Buff(INT iSkillSeq);
BOOL IsCorrectSkillType_DeBuff(INT iSkillSeq);
BOOL IsCorrectSkillType_CommonAttack(INT iSkillSeq);

extern CLASS_ATTRIBUTE ClassAttribute[MAX_CLASS];

bool IsRequireEquipItem(ITEM* pItem);

class CHARACTER_MACHINE
{
public:
    //input
    CHARACTER_ATTRIBUTE Character;
    ITEM				Equipment[MAX_EQUIPMENT];
    DWORD				Gold;
    int                 StorageGold;
    MONSTER             Enemy;
    //output
    WORD    AttackDamageRight;
    WORD    AttackDamageLeft;
    WORD    CriticalDamage;
    //final output
    WORD    FinalAttackDamageRight;
    WORD    FinalAttackDamageLeft;
    WORD    FinalHitPoint;
    WORD    FinalAttackRating;
    WORD    FinalDefenseRating;
    bool    FinalSuccessAttack;
    bool    FinalSuccessDefense;
    // packet
    BYTE	PacketSerial;
    BYTE	InfinityArrowAdditionalMana;

    void Init();
    void InitAddValue();
    void SetCharacter(CLASS_TYPE Class);
    void InputEnemyAttribute(MONSTER* Enemy);
    bool IsZeroDurability();
    void CalculateDamage();
    void CalculateCriticalDamage();
    void CalculateMagicDamage();
    void CalculateCurseDamage();
    void CalculateAttackRating();
    void CalculateSuccessfulBlocking();
    void CalculateAttackRatingPK();
    void CalculateSuccessfulBlockingPK();
    void CalculateDefense();
    void CalculateMagicDefense();
    void CalculateWalkSpeed();
    void CalculateNextExperince();
    void CalulateMasterLevelNextExperience();
    void CalculateAll();
    void CalculateBasicState();
    void getAllAddStateOnlyExValues(int& iAddStrengthExValues, int& iAddDexterityExValues, int& iAddVitalityExValues, int& iAddEnergyExValues, int& iAddCharismaExValues);
};

inline DWORD GenerateCheckSum2(BYTE* pbyBuffer, DWORD dwSize, WORD wKey)
{
    DWORD dwKey = (DWORD)wKey;
    DWORD dwResult = dwKey << 9;
    for (DWORD dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
    {
        DWORD dwTemp;
        memcpy(&dwTemp, pbyBuffer + dwChecked, sizeof(DWORD));

        switch ((dwChecked / 4 + wKey) % 2)
        {
        case 0:
            dwResult ^= dwTemp;
            break;
        case 1:
            dwResult += dwTemp;
            break;
        }
        if (0 == (dwChecked % 16))
        {
            dwResult ^= ((dwKey + dwResult) >> ((dwChecked / 4) % 8 + 1));
        }
    }

    return (dwResult);
}

bool IsCepterItem(int iType);
extern CHARACTER_MACHINE* CharacterMachine;
extern CHARACTER_ATTRIBUTE* CharacterAttribute;

#endif
