// BloodCastle.h: interface for the CBloodCastle class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MapItem.h"
#include "Protocol.h"
#include "User.h"

#define MAX_BC_LEVEL 8
#define MAX_BC_USER 50
#define MAX_BC_MONSTER 200

#define GET_BC_LEVEL(x) (((x)<MAP_BLOOD_CASTLE1)?-1:((x)>MAP_BLOOD_CASTLE7)?(((x)==MAP_BLOOD_CASTLE8)?(MAX_BC_LEVEL-1):-1):((x)-MAP_BLOOD_CASTLE1))

#define BC_LEVEL_RANGE(x) (((x)<0)?0:((x)>=MAX_BC_LEVEL)?0:1)

enum eBloodCastleState
{
	BC_STATE_BLANK = 0,
	BC_STATE_EMPTY = 1,
	BC_STATE_STAND = 2,
	BC_STATE_START = 3,
	BC_STATE_CLEAN = 4,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_BLOOD_CASTLE_ENTER_RECV
{
	PBMSG_HEAD header; // C1:9A
	BYTE level;
	BYTE slot;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_BLOOD_CASTLE_SCORE_SEND
{
	#pragma pack(1)
	PBMSG_HEAD header; // C1:93
	BYTE type;
	BYTE flag;
	char name[10];
	WORD none;
	DWORD Score;
	DWORD RewardExperience;
	DWORD RewardMoney;
	#pragma pack()
};

struct PMSG_BLOOD_CASTLE_ENTER_SEND
{
	PBMSG_HEAD header; // C1:9A
	BYTE result;
};

struct PMSG_BLOOD_CASTLE_STATE_SEND
{
	PBMSG_HEAD header; // C1:9B
	BYTE state;
	WORD time;
	WORD MaxMonster;
	WORD CurMonster;
	WORD EventItemOwner;
	BYTE EventItemLevel;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct BLOOD_CASTLE_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct BLOOD_CASTLE_DELAY_SCORE
{
	int Delay;
	int Index;
	PMSG_BLOOD_CASTLE_SCORE_SEND Score;
};

struct BLOOD_CASTLE_USER
{
	int Index;
	int Score;
	int RewardExperience;
	int RewardMoney;
};

struct BLOOD_CASTLE_LEVEL
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
	int CurMonster;
	int MaxMonster;
	int BridgeOpenTime;
	int EventItemSerial;
	int EventItemNumber;
	int EventItemOwner;
	int EventItemLevel;
	int WinnerIndex;
	BLOOD_CASTLE_USER User[MAX_BC_USER];
	int GateIndex;
	int SaintStatueIndex;
	int MonsterIndex[MAX_BC_MONSTER];
	std::vector<BLOOD_CASTLE_DELAY_SCORE> DelayScore;
};

class CBloodCastle
{
public:
	CBloodCastle();
	virtual ~CBloodCastle();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(BLOOD_CASTLE_LEVEL* lpLevel);
	void ProcState_EMPTY(BLOOD_CASTLE_LEVEL* lpLevel);
	void ProcState_STAND(BLOOD_CASTLE_LEVEL* lpLevel);
	void ProcState_START(BLOOD_CASTLE_LEVEL* lpLevel);
	void ProcState_CLEAN(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetState(BLOOD_CASTLE_LEVEL* lpLevel,int state);
	void SetState_BLANK(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetState_EMPTY(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetState_STAND(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetState_START(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetState_CLEAN(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetStage0(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetStage1(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetStage2(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetStage3(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetStage4(BLOOD_CASTLE_LEVEL* lpLevel);
	void CheckSync(BLOOD_CASTLE_LEVEL* lpLevel);
	int GetState(int level);
	int GetRemainTime(int level);
	int GetEnterEnabled(int level);
	int GetEnteredUserCount(int level);
	bool AddUser(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	bool DelUser(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	BLOOD_CASTLE_USER* GetUser(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	void CleanUser(BLOOD_CASTLE_LEVEL* lpLevel);
	void ClearUser(BLOOD_CASTLE_LEVEL* lpLevel);
	void CheckUser(BLOOD_CASTLE_LEVEL* lpLevel);
	int GetUserCount(BLOOD_CASTLE_LEVEL* lpLevel);
	int GetUserAbleLevel(LPOBJ lpObj);
	void GiveUserRewardExperience(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	void GiveUserRewardMoney(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	void GiveUserRewardItem(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	void SearchUserDropEventItem(int aIndex);
	void SearchUserDeleteEventItem(int aIndex);
	bool AddMonster(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	bool DelMonster(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	int* GetMonster(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex);
	void CleanMonster(BLOOD_CASTLE_LEVEL* lpLevel);
	void ClearMonster(BLOOD_CASTLE_LEVEL* lpLevel);
	int GetMonsterCount(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetGate(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetSaintStatue(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetMonster(BLOOD_CASTLE_LEVEL* lpLevel,int MonsterClass);
	void NpcAngelKing(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcAngelMessenger(LPOBJ lpNpc,LPOBJ lpObj);
	bool CheckEventItemSerial(int map,CMapItem* lpItem);
	void SetEventItemSerial(int map,int index,int serial);
	void DropEventItem(int map,int number,int aIndex);
	void GetEventItem(int map,int aIndex,CMapItem* lpItem);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void SetEntranceZone(BLOOD_CASTLE_LEVEL* lpLevel);
	void DelEntranceZone(BLOOD_CASTLE_LEVEL* lpLevel);
	void SetBridgeZone(BLOOD_CASTLE_LEVEL* lpLevel,bool send);
	void DelBridgeZone(BLOOD_CASTLE_LEVEL* lpLevel,bool send);
	void SetGateZone(BLOOD_CASTLE_LEVEL* lpLevel);
	void DelGateZone(BLOOD_CASTLE_LEVEL* lpLevel);
	void CheckDelayScore(BLOOD_CASTLE_LEVEL* lpLevel);
	void NoticeSendToAll(BLOOD_CASTLE_LEVEL* lpLevel,int type,char* message,...);
	void DataSendToAll(BLOOD_CASTLE_LEVEL* lpLevel,BYTE* lpMsg,int size);
	void CGBloodCastleEnterRecv(PMSG_BLOOD_CASTLE_ENTER_RECV* lpMsg,int aIndex);
	void GCBloodCastleScoreSend(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex,int type);
	void GCBloodCastleDelayScoreSend(BLOOD_CASTLE_LEVEL* lpLevel,int aIndex,int type);
	void GCBloodCastleStateSend(BLOOD_CASTLE_LEVEL* lpLevel,int state);
	void GCBloodCastleSimpleStateSend(BLOOD_CASTLE_LEVEL* lpLevel,int state,int aIndex);
	void StartBC();
private:
	BLOOD_CASTLE_LEVEL m_BloodCastleLevel[MAX_BC_LEVEL];
	int m_WarningTime;
	int m_NotifyTime;
	int m_EventTime;
	int m_CloseTime;
	std::vector<BLOOD_CASTLE_START_TIME> m_BloodCastleStartTime;
	int m_BloodCastleExperienceTable[MAX_BC_LEVEL][2];
	int m_BloodCastleMoneyTable[MAX_BC_LEVEL][2];
	int m_BloodCastleNpcLife[MAX_BC_LEVEL][2];
	int m_BloodCastleRewardValue[MAX_BC_LEVEL][3];
};

extern CBloodCastle gBloodCastle;

static const BYTE gBloodCastleEntranceZone[4] = {13,15,15,23};

static const BYTE gBloocCastleBridgeZone[4] = {13,70,15,75};

static const BYTE gBloodCastleGateZone[3][4] = {{13,76,15,79},{11,80,25,89},{8,80,10,83}};
