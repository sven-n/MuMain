// DoubleGoer.h: interface for the CDoubleGoer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Party.h"
#include "Protocol.h"
#include "User.h"

#define MAX_DG_LEVEL 4
#define MAX_DG_USER 5
#define MAX_DG_MONSTER 100
#define MAX_DG_MONSTER_DIFFICULT 60

enum eDoubleGoerState
{
	DG_STATE_EMPTY = 0,
	DG_STATE_STAND = 1,
	DG_STATE_START = 2,
	DG_STATE_CLEAN = 3,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_DOUBLE_GOER_ENTER_RECV
{
	PSBMSG_HEAD header; // C1:BF:0E
	BYTE slot;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_DOUBLE_GOER_ENTER_SEND
{
	PSBMSG_HEAD header; // C1:BF:0E
	BYTE result;
};

struct PMSG_DOUBLE_GOER_MONSTER_POSITION_SEND
{
	PSBMSG_HEAD header; // C1:BF:0F
	BYTE position;
};

struct PMSG_DOUBLE_GOER_STATE_SEND
{
	PSBMSG_HEAD header; // C1:BF:10
	BYTE state;
};

struct PMSG_DOUBLE_GOER_BOSS_POSITION_SEND
{
	PSBMSG_HEAD header; // C1:BF:11
	BYTE type;
	BYTE position;
};

struct PMSG_DOUBLE_GOER_INTERFACE_SEND
{
	PSBMSG_HEAD header; // C1:BF:12
	WORD time;
	BYTE count;
};

struct PMSG_DOUBLE_GOER_INTERFACE
{
	WORD index;
	BYTE number;
	BYTE position;
};

struct PMSG_DOUBLE_GOER_RESULT_SEND
{
	PSBMSG_HEAD header; // C1:BF:13
	BYTE result;
};

struct PMSG_DOUBLE_GOER_MONSTER_INFO_SEND
{
	PSBMSG_HEAD header; // C1:BF:14
	BYTE MaxMonster;
	BYTE CurMonster;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct DOUBLE_GOER_MONSTER_DIFFICULT_INFO
{
	int Index;
	float Level[MAX_PARTY_USER];
	float Life[MAX_PARTY_USER];
	float Damage[MAX_PARTY_USER];
	float Defense[MAX_PARTY_USER];
};

struct DOUBLE_GOER_USER
{
	int Index;
};

class CDoubleGoer
{
public:
	CDoubleGoer();
	virtual ~CDoubleGoer();
	void Load(char* path);
	void MainProc();
	void ProcState_EMPTY();
	void ProcState_STAND();
	void ProcState_START();
	void ProcState_CLEAN();
	void SetState(int state);
	void SetState_EMPTY();
	void SetState_STAND();
	void SetState_START();
	void SetState_CLEAN();
	void SetStage0();
	void SetStage1();
	void SetStage2();
	void SetStage3();
	int GetState();
	bool AddUser(int aIndex);
	bool DelUser(int aIndex);
	DOUBLE_GOER_USER* GetUser(int aIndex);
	void CleanUser();
	void ClearUser();
	void CheckUser();
	int GetUserCount();
	bool AddMonster(int aIndex);
	bool DelMonster(int aIndex);
	int* GetMonster(int aIndex);
	void CleanMonster();
	void ClearMonster();
	int GetMonsterCount();
	void SetSlaughterer();
	void SetMadSlaughterer();
	void SetIceWalker();
	void SetSilverRewardChest(int px,int py);
	void SetGoldenRewardChest(int px,int py);
	void SetMonster(int MonsterClass);
	void NpcSartiganTheAngel(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcSilverRewardChest(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcGoldenRewardChest(LPOBJ lpNpc,LPOBJ lpObj);
	void CheckMonsterPosition();
	float GetMonsterLevel(float value);
	float GetMonsterLife(float value);
	float GetMonsterDamageMin(float value);
	float GetMonsterDamageMax(float value);
	float GetMonsterDefense(float value);
	void SetEntranceZone();
	void DelEntranceZone();
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(int type,char* message,...);
	void DataSendToAll(BYTE* lpMsg,int size);
	void CGDoubleGoerEnterRecv(PMSG_DOUBLE_GOER_ENTER_RECV* lpMsg,int aIndex);
	void GCDoubleGoerMonsterPositionSend();
	void GCDoubleGoerStateSend(int state,int aIndex);
	void GCDoubleGoerBossPositionSend();
	void GCDoubleGoerInterfaceSend();
	void GCDoubleGoerResultSend(int aIndex,int result);
	void GCDoubleGoerMonsterInfoSend();
private:
	int m_NotifyTime;
	int m_EventTime;
	int m_CloseTime;
	int m_MonsterAIUnitNumber[MAX_DG_LEVEL];
	int m_Level;
	int m_State;
	int m_Map;
	int m_RemainTime;
	int m_TargetTime;
	int m_TickCount;
	int m_TimeCount;
	int m_Stage;
	int m_CurMonster;
	int m_MaxMonster;
	int m_PartyNumber;
	int m_PartyCount;
	int m_HighestLevel;
	int m_CreateMonsterTime;
	DOUBLE_GOER_USER m_User[MAX_DG_USER];
	int m_SlaughtererIndex;
	int m_MadSlaughtererIndex;
	int m_IceWalkerIndex;
	int m_SilverRewardChestIndex;
	int m_GoldenRewardChestIndex;
	int m_MonsterIndex[MAX_DG_MONSTER];
	DOUBLE_GOER_MONSTER_DIFFICULT_INFO m_MonsterDifficultInfo[MAX_DG_MONSTER_DIFFICULT];
};

extern CDoubleGoer gDoubleGoer;

static const BYTE gDoubleGoerEntranceZone[MAX_DG_LEVEL][4] = {{195,33,199,36},{135,76,140,79},{106,65,112,68},{93,24,97,27}};

static const BYTE gDoubleGoerRespawnZone[MAX_DG_LEVEL][2] = {{225,102},{114,180},{110,150},{44,108}};

static const BYTE gDoubleGoerStartPoint[MAX_DG_LEVEL] = {25,67,56,11};

static const BYTE gDoubleGoerFinalPoint[MAX_DG_LEVEL] = {105,184,153,111};

static const float gDoubleGoerPointValue[MAX_DG_LEVEL] = {3.5f,5.1f,4.3f,4.4f};
