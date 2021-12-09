// Crywolf.h: interface for the CCrywolf class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CrywolfObjInfo.h"
#include "CrywolfStateTimeInfo.h"
#include "Map.h"

#define MAX_CRYWOLF_STATE_TIME 20
#define MAX_CRYWOLF_MONSTER_GROUP 20
#define MAX_CRYWOLF_STATE 7
#define MAX_CRYWOLF_OCCUPATION_STATE 3
#define MAX_CRYWOLF_RANK 5

#define CRYWOLF_STATE_NONE 0
#define CRYWOLF_STATE_NOTIFY1 1
#define CRYWOLF_STATE_NOTIFY2 2
#define CRYWOLF_STATE_READY 3
#define CRYWOLF_STATE_START 4
#define CRYWOLF_STATE_END 5
#define CRYWOLF_STATE_ENDCYCLE 6

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CRYWOLF_ALTAR_CONTRACT_RECV
{
	PSBMSG_HEAD header; // C1:BD:03
	BYTE index[2];
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CRYWOLF_INFO_SEND
{
	PSBMSG_HEAD header; // C1:BD:00
	BYTE OccupationState;
	BYTE CrywolfState;
};

struct PMSG_CRYWOLF_STATUE_ALTAR_INFO_SEND
{
	PSBMSG_HEAD header; // C1:BD:02
	DWORD CrywolfStatueHP;
	BYTE AltarState1;
	BYTE AltarState2;
	BYTE AltarState3;
	BYTE AltarState4;
	BYTE AltarState5;
};

struct PMSG_CRYWOLF_ALTAR_CONTRACT_SEND
{
	PSBMSG_HEAD header; // C1:BD:03
	BYTE result;
	BYTE state;
	BYTE index[2];
};

struct PMSG_CRYWOLF_LEFT_TIME_SEND
{
	PSBMSG_HEAD header; // C1:BD:04
	BYTE hour;
	BYTE minute;
};

struct PMSG_CRYWOLF_BOSS_MONSTER_INFO_SEND
{
	PSBMSG_HEAD header; // C1:BD:05
	DWORD BalgassHP;
	BYTE DarkElfCount;
};

struct PMSG_CRYWOLF_STAGE_EFFECT_SEND
{
	PSBMSG_HEAD header; // C1:BD:06
	BYTE state;
};

struct PMSG_CRYWOLF_RANK_SEND
{
	PSBMSG_HEAD header; // C1:BD:07
	BYTE rank;
	DWORD experience;
};

struct PMSG_CRYWOLF_RANK_LIST_SEND
{
	PSBMSG_HEAD header; // C1:BD:08
	BYTE count;
};

struct PMSG_CRYWOLF_RANK_LIST
{
	BYTE rank;
	char name[10];
	DWORD score;
	BYTE Class;
};

struct PMSG_CRYWOLF_CHAOS_RATE_SEND
{
	PSBMSG_HEAD header; // C1:BD:09
	BYTE rate;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CCrywolf
{
public:
	CCrywolf();
	virtual ~CCrywolf();
	void DelAllData();
	void Load(char* path);
	void MainProc();
	void ProcState_NONE();
	void ProcState_NOTIFY1();
	void ProcState_NOTIFY2();
	void ProcState_READY();
	void ProcState_START();
	void ProcState_END();
	void ProcState_ENDCYCLE();
	void SetState(int state);
	void SetState_NONE();
	void SetState_NOTIFY1();
	void SetState_NOTIFY2();
	void SetState_READY();
	void SetState_START();
	void SetState_END();
	void SetState_ENDCYCLE();
	void CheckStateTimeSync();
	void CrywolfSecondAct();
	void CrywolfServerGroupSync();
	void CrywolfInfoDBSave();
	void CrywolfInfoDBLoad();
	void ApplyCrywolfDBInfo(int state);
	void ResetAllUserMVPScore();
	void TurnUpBoss();
	void ChangeAI(int AIOrder);
	void CrywolfNpcAct(int aIndex);
	void CrywolfMonsterAct(int aIndex);
	void CreateCrywolfCommonMonster();
	void RemoveCrywolfCommonMonster();
	void NotifyCrywolfCurrentState();
	void NotifyCrywolfStatueAndAltarInfo();
	void NotifyCrywolfBossMonsterInfo();
	void NotifyCrywolfStateLeftTime();
	void NotifyCrywolfStageEffectOnOff(int state);
	void NotifyCrywolfPersonalRank();
	void NotifyCrywolfHeroList();
	void CGCrywolfInfoRecv(int aIndex);
	void CGCrywolfAltarContractRecv(PMSG_CRYWOLF_ALTAR_CONTRACT_RECV* lpMsg,int aIndex);
	void CGCrywolfChaosRateRecv(int aIndex);
	void GiveUserRewardExperience(int aIndex,int AddExperience);
	void GiveUserRewardItem(int aIndex);
	void CrywolfMonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void LoadCrywolfMapAttr(char* path,int state);
	void SetDBDataLoad(int loaded);
	void SetCrywolfMapAttr(int state);
	void SetCrywolfState(int state);
	void SetOccupationState(int state);
	void SetCrywolfCommonNPC(int state);
	void SetCrywolfStateAppliedTime(int state);
	void SetCrywolfBossIndex(int index);
	int GetCrywolfState();
	int GetOccupationState();
	int GetUserRank(int aIndex);
	int GetUserExperience(int aIndex,int rank);
	int GetUserScore(int aIndex,int bIndex,int type);
	void CheckSync();
	void StartCW();
public:
	int m_FileDataLoad;
	int m_DBDataLoadOK;
	int m_DBDataLoading;
	int m_ScheduleMode;
	CMap m_CrywolfMapAttr[MAX_CRYWOLF_OCCUPATION_STATE];
	int m_CrywolfState;
	int m_OccupationState;
	CCrywolfObjInfo m_ObjCommonNPC;
	CCrywolfObjInfo m_ObjSpecialNPC;
	CCrywolfObjInfo m_ObjCommonMonster;
	CCrywolfObjInfo m_ObjSpecialMonster;
	CCrywolfStateTimeInfo m_StartTimeInfo[MAX_CRYWOLF_STATE_TIME];
	int m_StartTimeInfoCount;
	CCrywolfStateTimeInfo m_StateTimeInfo[MAX_CRYWOLF_STATE];
	DWORD m_CrywolfNotifyMsgStartTick;
	DWORD m_CrywolfStartProcTick;
	int m_TurnUpBoss;
	int m_ChangeAI;
	int m_MonsterGroupNumberArray[MAX_CRYWOLF_MONSTER_GROUP];
	int m_MonsterGroupNumberCount;
	int m_MonsterGroupChangeAITime;
	int m_BossIndex;
	int m_BossGroupNumber;
	int m_BossTurnUpTime;
	int m_MVPScoreTable[7];
	int m_MVPRankScoreTable[5];
	int m_MVPRankExpTable[5];
	int RemainTime;
	int TargetTime;
};

extern CCrywolf gCrywolf;
