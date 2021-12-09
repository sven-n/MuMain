// ChaosCastle.h: interface for the CChaosCastle class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_CC_LEVEL 7
#define MAX_CC_USER 70
#define MAX_CC_MONSTER 100
#define MAX_CC_HOLLOW_ZONE 4
#define MAX_CC_CENTER_HOLLOW_ZONE 2

#define GET_CC_LEVEL(x) (((x)<MAP_CHAOS_CASTLE1)?-1:((x)>MAP_CHAOS_CASTLE6)?(((x)==MAP_CHAOS_CASTLE7)?(MAX_CC_LEVEL-1):-1):((x)-MAP_CHAOS_CASTLE1))

#define CC_LEVEL_RANGE(x) (((x)<0)?0:((x)>=MAX_CC_LEVEL)?0:1)

enum eChaosCastleState
{
	CC_STATE_BLANK = 0,
	CC_STATE_EMPTY = 1,
	CC_STATE_STAND = 2,
	CC_STATE_START = 3,
	CC_STATE_CLEAN = 4,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CHAOS_CASTLE_ENTER_RECV
{
	PSBMSG_HEAD header; // C1:AF:01
	BYTE level;
	BYTE slot;
};

struct PMSG_CHAOS_CASTLE_POSITION_RECV
{
	PSBMSG_HEAD header; // C1:AF:02
	BYTE x;
	BYTE y;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CHAOS_CASTLE_SCORE_SEND
{
	#pragma pack(1)
	PBMSG_HEAD header; // C1:93
	BYTE type;
	BYTE flag;
	char name[10];
	WORD none;
	DWORD MonsterKillCount;
	DWORD RewardExperience;
	DWORD UserKillCount;
	#pragma pack()
};

struct PMSG_CHAOS_CASTLE_STATE_SEND
{
	PBMSG_HEAD header; // C1:9B
	BYTE state;
	WORD time;
	WORD MaxMonster;
	WORD CurMonster;
	WORD EventItemOwner;
	BYTE EventItemLevel;
};

struct PMSG_CHAOS_CASTLE_ENTER_SEND
{
	PSBMSG_HEAD header; // C1:AF:01
	BYTE result;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct CHAOS_CASTLE_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct CHAOS_CASTLE_DELAY_SCORE
{
	int Delay;
	int Index;
	PMSG_CHAOS_CASTLE_SCORE_SEND Score;
};

struct CHAOS_CASTLE_USER
{
	int Index;
	int Score;
	int UserKillCount;
	int MonsterKillCount;
	int RewardExperience;
};

struct CHAOS_CASTLE_LEVEL
{
	int Level;
	int State;
	int Map;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int EnterEnabled;
	int MinutesLeft;
	int TimeCount;
	int Stage;
	int MaxMonster;
	int WinnerIndex;
	CHAOS_CASTLE_USER User[MAX_CC_USER];
	int MonsterIndex[MAX_CC_MONSTER];
	std::vector<CHAOS_CASTLE_DELAY_SCORE> DelayScore;
};

class CChaosCastle
{
public:
	CChaosCastle();
	virtual ~CChaosCastle();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(CHAOS_CASTLE_LEVEL* lpLevel);
	void ProcState_EMPTY(CHAOS_CASTLE_LEVEL* lpLevel);
	void ProcState_STAND(CHAOS_CASTLE_LEVEL* lpLevel);
	void ProcState_START(CHAOS_CASTLE_LEVEL* lpLevel);
	void ProcState_CLEAN(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetState(CHAOS_CASTLE_LEVEL* lpLevel,int state);
	void SetState_BLANK(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetState_EMPTY(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetState_STAND(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetState_START(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetState_CLEAN(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetStage0(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetStage1(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetStage2(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetStage3(CHAOS_CASTLE_LEVEL* lpLevel);
	void CheckSync(CHAOS_CASTLE_LEVEL* lpLevel);
	int GetState(int level);
	int GetRemainTime(int level);
	int GetEnterEnabled(int level);
	int GetEnteredUserCount(int level);
	bool AddUser(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	bool DelUser(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	CHAOS_CASTLE_USER* GetUser(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	void CleanUser(CHAOS_CASTLE_LEVEL* lpLevel);
	void ClearUser(CHAOS_CASTLE_LEVEL* lpLevel);
	void CheckUser(CHAOS_CASTLE_LEVEL* lpLevel);
	int GetUserCount(CHAOS_CASTLE_LEVEL* lpLevel);
	int GetUserAbleLevel(LPOBJ lpObj);
	int GetUserWinner(CHAOS_CASTLE_LEVEL* lpLevel);
	void GiveUserRewardExperience(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	void GiveUserRewardItem(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	bool AddMonster(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	bool DelMonster(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	int* GetMonster(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex);
	void CleanMonster(CHAOS_CASTLE_LEVEL* lpLevel);
	void ClearMonster(CHAOS_CASTLE_LEVEL* lpLevel);
	int GetMonsterCount(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetMonster(CHAOS_CASTLE_LEVEL* lpLevel,int MonsterClass);
	void SetSafeZone(CHAOS_CASTLE_LEVEL* lpLevel);
	void DelSafeZone(CHAOS_CASTLE_LEVEL* lpLevel);
	void SetHollowZone(CHAOS_CASTLE_LEVEL* lpLevel,int stage);
	void DelHollowZone(CHAOS_CASTLE_LEVEL* lpLevel,int stage);
	void SetCenterHollowZone(CHAOS_CASTLE_LEVEL* lpLevel);
	void DelCenterHollowZone(CHAOS_CASTLE_LEVEL* lpLevel);
	void CheckUserPosition(CHAOS_CASTLE_LEVEL* lpLevel);
	void CheckMonsterPosition(CHAOS_CASTLE_LEVEL* lpLevel);
	void CheckItemPosition(CHAOS_CASTLE_LEVEL* lpLevel);
	void SearchUserAndBlow(CHAOS_CASTLE_LEVEL* lpLevel,int x,int y);
	void BlowUserFromPoint(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex,int x,int y);
	bool GetFreePosition(CHAOS_CASTLE_LEVEL* lpLevel,int* ox,int* oy);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void CheckDelayScore(CHAOS_CASTLE_LEVEL* lpLevel);
	void NoticeSendToAll(CHAOS_CASTLE_LEVEL* lpLevel,int type,char* message,...);
	void DataSendToAll(CHAOS_CASTLE_LEVEL* lpLevel,BYTE* lpMsg,int size);
	void CGChaosCastleEnterRecv(PMSG_CHAOS_CASTLE_ENTER_RECV* lpMsg,int aIndex);
	void CGChaosCastlePositionRecv(PMSG_CHAOS_CASTLE_POSITION_RECV* lpMsg,int aIndex);
	void GCChaosCastleScoreSend(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex,int type);
	void GCChaosCastleDelayScoreSend(CHAOS_CASTLE_LEVEL* lpLevel,int aIndex,int type);
	void GCChaosCastleStateSend(CHAOS_CASTLE_LEVEL* lpLevel,int state);
	void GCChaosCastleSimpleStateSend(CHAOS_CASTLE_LEVEL* lpLevel,int state,int aIndex);
	void StartCC();
private:
	CHAOS_CASTLE_LEVEL m_ChaosCastleLevel[MAX_CC_LEVEL];
	int m_WarningTime;
	int m_NotifyTime;
	int m_EventTime;
	int m_CloseTime;
	std::vector<CHAOS_CASTLE_START_TIME> m_ChaosCastleStartTime;
	int m_ChaosCastleExperienceTable[MAX_CC_LEVEL][2];
	int m_ChaosCastleRewardValue[MAX_CC_LEVEL][3];
};

extern CChaosCastle gChaosCastle;

static const BYTE gChaosCastleSafeZone[4] = {23,75,44,108};

static const BYTE gChaosCastleHollowZone[4][MAX_CC_HOLLOW_ZONE][4] = {{{23,75,44,76},{43,77,44,108},{23,107,42,108},{23,77,24,106}},{{23,75,44,76},{43,77,44,108},{23,107,42,108},{23,77,24,106}},{{25,77,42,78},{41,79,42,106},{25,105,40,106},{25,79,26,104}},{{27,79,40,80},{39,81,40,104},{27,103,38,104},{27,81,28,102}}};

static const BYTE gChaosCastleCenterHollowZone[MAX_CC_CENTER_HOLLOW_ZONE][4] = {{32,84,35,87},{32,96,35,99}};

static const int gChaosCastleEnterMoney[MAX_CC_LEVEL] = {25000,80000,150000,250000,400000,650000,1000000};
