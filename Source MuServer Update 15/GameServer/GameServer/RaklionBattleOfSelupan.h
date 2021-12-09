// RaklionBattleOfSelupan.h: interface for the CRaklionBattleOfSelupan class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "RaklionSelupan.h"

#define SELUPAN_STATE_NONE 0
#define SELUPAN_STATE_STANDBY 1
#define SELUPAN_STATE_PATTERN1 2
#define SELUPAN_STATE_PATTERN2 3
#define SELUPAN_STATE_PATTERN3 4
#define SELUPAN_STATE_PATTERN4 5
#define SELUPAN_STATE_PATTERN5 6
#define SELUPAN_STATE_PATTERN6 7
#define SELUPAN_STATE_PATTERN7 8
#define SELUPAN_STATE_DIE 9
#define MAX_STATE_PATTERN_COUNT 7
#define STATE_PATTERN1 0
#define STATE_PATTERN2 1
#define STATE_PATTERN3 2
#define STATE_PATTERN4 3
#define STATE_PATTERN5 4
#define STATE_PATTERN6 5
#define STATE_PATTERN7 6
#define MAX_SELUPAN_SKILL 9
#define SELUPAN_SKILL_FIRSTSKILL 0
#define SELUPAN_SKILL_POISON_ATTACK 1
#define SELUPAN_SKILL_ICESTORM 2
#define SELUPAN_SKILL_ICESTRIKE 3
#define SELUPAN_SKILL_SUMMON 4
#define SELUPAN_SKILL_HEAL 5
#define SELUPAN_SKILL_FREEZE 6
#define SELUPAN_SKILL_TELEPORT 7
#define SELUPAN_SKILL_INVINCIBILITY 8

struct RAKLION_STATE_SKILL_INFO
{
	BOOL IsSkillEnabled[MAX_SELUPAN_SKILL];
};

class CRaklionBattleOfSelupan
{
public:
	CRaklionBattleOfSelupan();
	virtual ~CRaklionBattleOfSelupan();
	void ResetAllData();
	void Run();
	void ProcState_NONE();
	void ProcState_STANDBY();
	void ProcState_PATTERN1();
	void ProcState_PATTERN2();
	void ProcState_PATTERN3();
	void ProcState_PATTERN4();
	void ProcState_PATTERN5();
	void ProcState_PATTERN6();
	void ProcState_PATTERN7();
	void ProcState_DIE();
	void SetState(int state);
	void SetState_NONE();
	void SetState_STANDBY();
	void SetState_PATTERN1();
	void SetState_PATTERN2();
	void SetState_PATTERN3();
	void SetState_PATTERN4();
	void SetState_PATTERN5();
	void SetState_PATTERN6();
	void SetState_PATTERN7();
	void SetState_DIE();
	void PatternStateSkillUseProc(int skill);
	void SetPatternCondition(int pattern,int condition);
	void SelupanLifeCheck();
	void CreateSelupan();
	void DeleteSelupan();
	void DeleteSummonMonster();
	void SetSelupanObjIndex(int index);
	void SetSelupanSkillDelay(int delay);
	void SetBattleOfSelupanState(int state);
	void SetSuccessValue(int success);
	int GetSuccessValue();
	int GetBattleUserCount();
	int GetSelupanObjIndex();
	int GetBattleOfSelupanState();
private:
	int m_IsUsed;
	int m_BattleOfSelupanState;
	int m_PatternCondition[6];
	DWORD m_SkillDelay;
	DWORD m_SkillDelayTick;
	int m_IsUseFirstSkill;
	int m_IsSucccess;
	CRaklionSelupan m_RaklionSelupan;
	RAKLION_STATE_SKILL_INFO m_PatternSkill[MAX_STATE_PATTERN_COUNT];
};
