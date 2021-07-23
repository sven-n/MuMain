#ifndef __ZZZITEM_H__
#define __ZZZITEM_H__

#pragma once

#include "GlobalText.h"
#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include "_struct.h"
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE

///////////////////////////////////////////////////////////////////////////////
// filter
///////////////////////////////////////////////////////////////////////////////

extern CGlobalText GlobalText;

void OpenTextScriptFile(char *FileName);
void OpenTextFile(char *FileName);
void SaveTextFile(char *FileName);

///////////////////////////////////////////////////////////////////////////////
// server list
///////////////////////////////////////////////////////////////////////////////

#ifndef KJH_ADD_SERVER_LIST_SYSTEM				// #ifndef
void OpenServerListScript(char *FileName);
void SaveServerListFile(char *FileName);
void OpenServerListFile(char *FileName);
#endif // KJH_ADD_SERVER_LIST_SYSTEM

///////////////////////////////////////////////////////////////////////////////
// filter
///////////////////////////////////////////////////////////////////////////////

extern char AbuseFilter[MAX_FILTERS][20];
extern char AbuseNameFilter[MAX_NAMEFILTERS][20];
extern int  AbuseFilterNumber;
extern int  AbuseNameFilterNumber;

void OpenFilterFileText(char *FileName);
void SaveFilterFile(char *FileName);
void OpenFilterFile(char *FileName);
void OpenNameFilterFileText(char *FileName);
void SaveNameFilterFile(char *FileName);
void OpenNameFilterFile(char *FileName);

///////////////////////////////////////////////////////////////////////////////
// gate
///////////////////////////////////////////////////////////////////////////////

extern GATE_ATTRIBUTE *GateAttribute;

void OpenGateScriptText(char *FileName);
void SaveGateScript(char *FileName);
void OpenGateScript(char *FileName);
void OpenMonsterSkillScript(char *FileName);


///////////////////////////////////////////////////////////////////////////////
// Dialog
///////////////////////////////////////////////////////////////////////////////

extern DIALOG_SCRIPT g_DialogScript[MAX_DIALOG];

void OpenDialogFileText(char *FileName);
void SaveDialogFile(char *FileName);
void OpenDialogFile(char *FileName);

///////////////////////////////////////////////////////////////////////////////
// item
///////////////////////////////////////////////////////////////////////////////

extern ITEM_ATTRIBUTE *ItemAttribute;

extern int  ConvertItemType(BYTE *Item);
extern void OpenItemScriptText(char *FileName);
extern void SaveItemScript(char *FileName);
extern void OpenItemScript(char *FileName);
extern BYTE getSkillIndexByBook ( int Type );
extern void ItemConvert(ITEM *ip,BYTE Attribute1,BYTE Attribute2, BYTE Attribute3 );
extern float CalcDurabilityPercent ( BYTE dur, BYTE maxDur, int Level, int option, int ExtOption=0 );
extern int ItemValue(ITEM *ip,int goldType=1);
extern int ItemValueOld(ITEM *ip,int goldType=1);
#ifndef KJH_DEL_PC_ROOM_SYSTEM				// #ifndef
#ifndef ASG_PCROOM_POINT_SYSTEM_MODIFY		// 정리시 삭제.
#ifdef ADD_PCROOM_POINT_SYSTEM
int GetItemBuyingTermsPoint(ITEM* pItem);
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY	// 정리시 삭제.
#endif // KJH_DEL_PC_ROOM_SYSTEM
extern void PrintItem(char *FileName);

///////////////////////////////////////////////////////////////////////////////
// monster
///////////////////////////////////////////////////////////////////////////////

extern MONSTER_SCRIPT MonsterScript[MAX_MONSTER];

extern void  OpenMonsterScript(char *FileName);
extern void  MonsterConvert(MONSTER *m,int Level);
extern char* getMonsterName ( int type );

///////////////////////////////////////////////////////////////////////////////
// skill
///////////////////////////////////////////////////////////////////////////////

extern SKILL_ATTRIBUTE *SkillAttribute;

void OpenSkillScriptText(char *FileName);
void SaveSkillScript(char *FileName);
void OpenSkillScript(char *FileName);

#ifdef LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER
BOOL IsValidateSkillIdx( INT iSkillIdx );

BOOL IsCorrectSkillType( INT iSkillSeq, eTypeSkill iSkillTypeIdx );

BOOL IsCorrectSkillType_FrendlySkill( INT iSkillSeq );

BOOL IsCorrectSkillType_Buff( INT iSkillSeq );

BOOL IsCorrectSkillType_DeBuff( INT iSkillSeq );

BOOL IsCorrectSkillType_CommonAttack( INT iSkillSeq );

#endif // LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER

///////////////////////////////////////////////////////////////////////////////
// class
///////////////////////////////////////////////////////////////////////////////

extern CLASS_ATTRIBUTE ClassAttribute[MAX_CLASS];

///////////////////////////////////////////////////////////////////////////////
// character machine
///////////////////////////////////////////////////////////////////////////////

#ifdef PSW_BUGFIX_REQUIREEQUIPITEM
bool IsRequireEquipItem(ITEM* pItem);
#endif //PSW_BUGFIX_REQUIREEQUIPITEM

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
    BYTE GetSkillMasteryType ( int iType );
    void SetCharacter(BYTE Class);
	void InputEnemyAttribute(MONSTER *Enemy);
    bool IsZeroDurability();
	void CalculateDamage();
    void CalculateCriticalDamage();
    void CalculateMagicDamage();
	void CalculateCurseDamage();	// 저주력 데미지 계산.
	void CalculateAttackRating();
	void CalculateAttackSpeed();
	void CalculateSuccessfulBlocking();
	//대인전용 공격 성공률과 방어율만 따로 계산한다.
	void CalculateAttackRatingPK();
	void CalculateSuccessfulBlockingPK();
	void CalculateDefense();
	void CalculateMagicDefense();
	void CalculateWalkSpeed();
	// 레벨업 했을 때 초기 경험치와 다음 경험치를 갱신
	void CalculateNextExperince();
	void CalulateMasterLevelNextExperience();
    void CalculateAll();

	// 정보 얻기
	void GetMagicSkillDamage( int iType, int *piMinDamage, int *piMaxDamage);
	void GetCurseSkillDamage(int iType, int *piMinDamage, int *piMaxDamage);	// 저주력 스킬 데미지.
    void GetSkillDamage( int iType, int *piMinDamage, int *piMaxDamage);

#ifdef PSW_SECRET_ITEM
	void CalculateBasicState();
#endif //PSW_SECRET_ITEM

#ifdef LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM
	void getAllAddStateOnlyExValues( int &iAddStrengthExValues, 
									int &iAddDexterityExValues, 
									int &iAddVitalityExValues,
									int &iAddEnergyExValues, 
									int &iAddCharismaExValues );
#endif // LDS_FIX_APPLY_BUFF_STATEVALUES_WHEN_CALCULATE_CHARACTERSTATE_WITH_SETITEM
};


inline DWORD GenerateCheckSum2( BYTE *pbyBuffer, DWORD dwSize, WORD wKey)
{
	DWORD dwKey = ( DWORD)wKey;
	DWORD dwResult = dwKey << 9;
	for ( DWORD dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
	{
		DWORD dwTemp;
		memcpy( &dwTemp, pbyBuffer + dwChecked, sizeof ( DWORD));

		// 1. 단계
		switch ( ( dwChecked / 4 + wKey) % 2)
		{
		case 0:
			dwResult ^= dwTemp;
			break;
		case 1:
			dwResult += dwTemp;
			break;
		}

		// 2. 단계
		if ( 0 == ( dwChecked % 16))
		{
			dwResult ^= ( ( dwKey + dwResult) >> ( ( dwChecked / 4) % 8 + 1));
		}
	}

	return ( dwResult);
}


// skill.
//  스킬 정보를 얻는다.
void GetSkillInformation( int iType, int iLevel, char *lpszName, int *piMana, int *piDistance, int *piSkillMana = NULL);
void GetSkillInformation_Energy(int iType, int *piEnergy);
void GetSkillInformation_Charisma(int iType, int *piCharisma);
// 스킬의 거리를 얻는다.
float GetSkillDistance(int Index, CHARACTER* c=NULL);
//  스킬 딜레이 값을 계산한다.
void GetSkillInformation_Damage(int iType, int *piDamage);
bool CheckSkillDelay ( int SkillIndex );
void CalcSkillDelay ( int time );

#ifdef STATE_LIMIT_TIME
void SetWorldStateTime ( DWORD State, int Time );
#endif// STATE_LIMIT_TIME
void RenderSkillDelay ( int SkillIndex, int x, int y, int Width, int Height );

bool IsGuildMaster();			// 길마인가
bool IsSubGuildMaster();		// 부길마인가
bool IsBattleMaster();			// 배틀마스터인가

#ifdef CSK_FIX_EPSOLUTESEPTER
bool IsCepterItem(int iType);
#endif // CSK_FIX_EPSOLUTESEPTER

extern CHARACTER_MACHINE   *CharacterMachine;
extern CHARACTER_ATTRIBUTE *CharacterAttribute;

#endif
