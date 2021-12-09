// CastleDeep.h: interface for the CCastleDeep class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

enum eCastleDeepState
{
	CD_STATE_BLANK = 0,
	CD_STATE_EMPTY = 1,
	CD_STATE_START = 2,
};

struct CASTLE_DEEP_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct CASTLE_DEEP_GROUP
{
	int Index;
	int Rate;
};

struct CASTLE_DEEP_SPAWN
{
	int Index;
	int Group;
	int Minute;
};

struct CASTLE_DEEP_MONSTER
{
	int Index;
	int Group;
	int Class;
	int Count;
	int X;
	int Y;
	int TX;
	int TY;
};

class CCastleDeep
{
public:
	CCastleDeep();
	virtual ~CCastleDeep();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_START();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_START();
	void CheckSync();
	void ClearMonster();
	void AddMonster(int stage,int group);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void StartLD();
public:
	int m_State;
	int m_RemainTime;
	int m_TargetTime;
	int m_TickCount;
	int m_TimeNotify;
	int m_Stage;
	int m_Group;
	std::vector<CASTLE_DEEP_START_TIME> m_CastleDeepStartTime;
	std::vector<CASTLE_DEEP_GROUP> m_CastleDeepGroup;
	std::vector<CASTLE_DEEP_SPAWN> m_CastleDeepSpawn;
	std::vector<CASTLE_DEEP_MONSTER> m_CastleDeepMonster;
};

extern CCastleDeep gCastleDeep;
