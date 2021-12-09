// InvasionManager.h: interface for the CInvasionManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

#define MAX_INVASION 30
#define MAX_INVASION_MONSTER 500
#define MAX_INVASION_RESPAWN_GROUP 20

enum eInvasionState
{
	INVASION_STATE_BLANK = 0,
	INVASION_STATE_EMPTY = 1,
	INVASION_STATE_START = 2,
};

struct INVASION_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct INVASION_RESPWAN_INFO
{
	int Group;
	int Map;
	int Value;
};

struct INVASION_MONSTER_INFO
{
	int Group;
	int MonsterClass;
	int RegenType;
	int RegenTime;
};

struct INVASION_INFO
{
	int Index;
	int State;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int MonsterIndex[MAX_INVASION_MONSTER];
	int RespawnMessage;
	int DespawnMessage;
	int BossIndex;
	int BossMessage;
	int InvasionTime;
	std::vector<INVASION_START_TIME> StartTime;
	std::vector<INVASION_RESPWAN_INFO> RespawnInfo[MAX_INVASION_RESPAWN_GROUP];
	std::vector<INVASION_MONSTER_INFO> MonsterInfo;
};

class CInvasionManager
{
public:
	CInvasionManager();
	virtual ~CInvasionManager();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(INVASION_INFO* lpInfo);
	void ProcState_EMPTY(INVASION_INFO* lpInfo);
	void ProcState_START(INVASION_INFO* lpInfo);
	void SetState(INVASION_INFO* lpInfo,int state);
	void SetState_BLANK(INVASION_INFO* lpInfo);
	void SetState_EMPTY(INVASION_INFO* lpInfo);
	void SetState_START(INVASION_INFO* lpInfo);
	void CheckSync(INVASION_INFO* lpInfo);
	int GetState(int index);
	int GetRemainTime(int index);
	bool AddMonster(INVASION_INFO* lpInfo,int aIndex);
	bool DelMonster(INVASION_INFO* lpInfo,int aIndex);
	int* GetMonster(INVASION_INFO* lpInfo,int aIndex);
	void CleanMonster(INVASION_INFO* lpInfo);
	void ClearMonster(INVASION_INFO* lpInfo);
	int GetMonsterCount(INVASION_INFO* lpInfo);
	void SetMonster(INVASION_INFO* lpInfo,INVASION_RESPWAN_INFO* lpRespawnInfo,INVASION_MONSTER_INFO* lpMonsterInfo);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void StartInvasion(int index);
private:
	INVASION_INFO m_InvasionInfo[MAX_INVASION];
};

extern CInvasionManager gInvasionManager;
