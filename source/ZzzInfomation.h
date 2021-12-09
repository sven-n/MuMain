#ifndef __ZZZITEM_H__
#define __ZZZITEM_H__

#pragma once

#include "GlobalText.h"

void SaveTextFile(char *FileName);

extern char AbuseFilter[MAX_FILTERS][20];
extern char AbuseNameFilter[MAX_NAMEFILTERS][20];
extern int  AbuseFilterNumber;
extern int  AbuseNameFilterNumber;

void OpenFilterFile(char *FileName);
void OpenNameFilterFile(char *FileName);

extern GATE_ATTRIBUTE *GateAttribute;

void OpenGateScript(char *FileName);
void OpenMonsterSkillScript(char *FileName);

extern DIALOG_SCRIPT g_DialogScript[MAX_DIALOG];

void OpenDialogFile(char *FileName);

extern ITEM_ATTRIBUTE *ItemAttribute;

extern int  ConvertItemType(BYTE *Item);
extern void OpenItemScript(char *FileName);
extern BYTE getSkillIndexByBook ( int Type );
extern void ItemConvert(ITEM *ip,BYTE Attribute1,BYTE Attribute2, BYTE Attribute3 );
extern float CalcDurabilityPercent ( BYTE dur, BYTE maxDur, int Level, int option, int ExtOption=0 );
extern int ItemValue(ITEM *ip,int goldType=1);
extern void PrintItem(char *FileName);

extern MONSTER_SCRIPT MonsterScript[MAX_MONSTER];

extern void  OpenMonsterScript(char *FileName);
extern void  MonsterConvert(MONSTER *m,int Level);
extern char* getMonsterName ( int type );

extern SKILL_ATTRIBUTE *SkillAttribute;

void OpenSkillScript(char *FileName);

BOOL IsValidateSkillIdx( INT iSkillIdx );
BOOL IsCorrectSkillType( INT iSkillSeq, eTypeSkill iSkillTypeIdx );
BOOL IsCorrectSkillType_FrendlySkill( INT iSkillSeq );
BOOL IsCorrectSkillType_Buff( INT iSkillSeq );
BOOL IsCorrectSkillType_DeBuff( INT iSkillSeq );
BOOL IsCorrectSkillType_CommonAttack( INT iSkillSeq );

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
	void InitAddValue ();
    void SetCharacter(BYTE Class);
	void InputEnemyAttribute(MONSTER *Enemy);
    bool IsZeroDurability();
	void CalculateDamage();
    void CalculateCriticalDamage();
    void CalculateMagicDamage();
	void CalculateCurseDamage();
	void CalculateAttackRating();
	void CalculateAttackSpeed();
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
	void getAllAddStateOnlyExValues( int &iAddStrengthExValues, int &iAddDexterityExValues, int &iAddVitalityExValues,	int &iAddEnergyExValues, int &iAddCharismaExValues );
};


inline DWORD GenerateCheckSum2( BYTE *pbyBuffer, DWORD dwSize, WORD wKey)
{
	DWORD dwKey = ( DWORD)wKey;
	DWORD dwResult = dwKey << 9;
	for ( DWORD dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
	{
		DWORD dwTemp;
		memcpy( &dwTemp, pbyBuffer + dwChecked, sizeof ( DWORD));

		switch ( ( dwChecked / 4 + wKey) % 2)
		{
		case 0:
			dwResult ^= dwTemp;
			break;
		case 1:
			dwResult += dwTemp;
			break;
		}
		if ( 0 == ( dwChecked % 16))
		{
			dwResult ^= ( ( dwKey + dwResult) >> ( ( dwChecked / 4) % 8 + 1));
		}
	}

	return ( dwResult);
}

bool IsCepterItem(int iType);
extern CHARACTER_MACHINE   *CharacterMachine;
extern CHARACTER_ATTRIBUTE *CharacterAttribute;

#endif
