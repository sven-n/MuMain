// DevilSquare.h: interface for the CDevilSquare class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_DS_LEVEL 7
#define MAX_DS_USER 50
#define MAX_DS_MONSTER 200
#define MAX_DS_RANK 10

#define DS_LEVEL_RANGE(x) (((x)<0)?0:((x)>=MAX_DS_LEVEL)?0:1)

enum eDevilSquareState
{
	DS_STATE_BLANK = 0,
	DS_STATE_EMPTY = 1,
	DS_STATE_STAND = 2,
	DS_STATE_START = 3,
	DS_STATE_CLEAN = 4,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_DEVIL_SQUARE_ENTER_RECV
{
	PBMSG_HEAD header; // C1:90
	BYTE level;
	BYTE slot;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_DEVIL_SQUARE_ENTER_SEND
{
	PBMSG_HEAD header; // C1:90
	BYTE result;
};

struct PMSG_DEVIL_SQUARE_SCORE_SEND
{
	PBMSG_HEAD header; // C1:93
	BYTE rank;
	BYTE count;
};

struct PMSG_DEVIL_SQUARE_SCORE
{
	char name[10];
	DWORD score;
	DWORD RewardExperience;
	DWORD RewardMoney;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct DEVIL_SQUARE_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct DEVIL_SQUARE_USER
{
	int Index;
	int Score;
	int Rank;
	int RewardExperience;
	int RewardMoney;
};

struct DEVIL_SQUARE_LEVEL
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
	DEVIL_SQUARE_USER User[MAX_DS_USER];
	int MonsterIndex[MAX_DS_MONSTER];
};

class CDevilSquare
{
public:
	CDevilSquare();
	virtual ~CDevilSquare();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(DEVIL_SQUARE_LEVEL* lpLevel);
	void ProcState_EMPTY(DEVIL_SQUARE_LEVEL* lpLevel);
	void ProcState_STAND(DEVIL_SQUARE_LEVEL* lpLevel);
	void ProcState_START(DEVIL_SQUARE_LEVEL* lpLevel);
	void ProcState_CLEAN(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetState(DEVIL_SQUARE_LEVEL* lpLevel,int state);
	void SetState_BLANK(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetState_EMPTY(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetState_STAND(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetState_START(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetState_CLEAN(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetStage0(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetStage1(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetStage2(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetStage3(DEVIL_SQUARE_LEVEL* lpLevel);
	void CheckSync(DEVIL_SQUARE_LEVEL* lpLevel);
	int GetState(int level);
	int GetRemainTime(int level);
	int GetEnterEnabled(int level);
	int GetEnteredUserCount(int level);
	bool AddUser(DEVIL_SQUARE_LEVEL* lpLevel,int aIndex);
	bool DelUser(DEVIL_SQUARE_LEVEL* lpLevel,int aIndex);
	DEVIL_SQUARE_USER* GetUser(DEVIL_SQUARE_LEVEL* lpLevel,int aIndex);
	void CleanUser(DEVIL_SQUARE_LEVEL* lpLevel);
	void ClearUser(DEVIL_SQUARE_LEVEL* lpLevel);
	void CheckUser(DEVIL_SQUARE_LEVEL* lpLevel);
	int GetUserCount(DEVIL_SQUARE_LEVEL* lpLevel);
	int GetUserAbleLevel(LPOBJ lpObj);
	void CalcUserRank(DEVIL_SQUARE_LEVEL* lpLevel);
	void GiveUserRewardExperience(DEVIL_SQUARE_LEVEL* lpLevel);
	void GiveUserRewardMoney(DEVIL_SQUARE_LEVEL* lpLevel);
	bool AddMonster(DEVIL_SQUARE_LEVEL* lpLevel,int aIndex);
	bool DelMonster(DEVIL_SQUARE_LEVEL* lpLevel,int aIndex);
	int* GetMonster(DEVIL_SQUARE_LEVEL* lpLevel,int aIndex);
	void CleanMonster(DEVIL_SQUARE_LEVEL* lpLevel);
	void ClearMonster(DEVIL_SQUARE_LEVEL* lpLevel);
	int GetMonsterCount(DEVIL_SQUARE_LEVEL* lpLevel);
	void SetMonster(DEVIL_SQUARE_LEVEL* lpLevel,int MonsterClass);
	void NpcCharon(LPOBJ lpNpc,LPOBJ lpObj);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(DEVIL_SQUARE_LEVEL* lpLevel,int type,char* message,...);
	void DataSendToAll(DEVIL_SQUARE_LEVEL* lpLevel,BYTE* lpMsg,int size);
	void CGDevilSquareEnterRecv(PMSG_DEVIL_SQUARE_ENTER_RECV* lpMsg,int aIndex);
	void GCDevilSquareScoreSend(DEVIL_SQUARE_LEVEL* lpLevel);
	void StartDS();
private:
	DEVIL_SQUARE_LEVEL m_DevilSquareLevel[MAX_DS_LEVEL];
	int m_WarningTime;
	int m_NotifyTime;
	int m_EventTime;
	int m_CloseTime;
	std::vector<DEVIL_SQUARE_START_TIME> m_DevilSquareStartTime;
	int m_DevilSquareRewardExperience[MAX_DS_LEVEL][MAX_DS_RANK];
	int m_DevilSquareRewardMoney[MAX_DS_LEVEL][MAX_DS_RANK];
	int m_DevilSquareRewardValue[MAX_DS_LEVEL][3];
};

extern CDevilSquare gDevilSquare;
