// ImperialGuardian.h: interface for the CImperialGuardian class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_IG_LEVEL 4
#define MAX_IG_USER 5
#define MAX_IG_MONSTER 100
#define MAX_IG_MONSTER_DIFFICULT 60

enum eImperialGuardianState
{
	IG_STATE_EMPTY = 0,
	IG_STATE_WAIT1 = 1,
	IG_STATE_PLAY1 = 2,
	IG_STATE_WAIT2 = 3,
	IG_STATE_PLAY2 = 4,
	IG_STATE_WAIT3 = 5,
	IG_STATE_PLAY3 = 6,
	IG_STATE_WAIT4 = 7,
	IG_STATE_PLAY4 = 8,
	IG_STATE_CLEAN = 9,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_IMPERIAL_GUARDIAN_ENTER_RECV
{
	PSBMSG_HEAD header; // C1:F7:01
	BYTE slot;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_IMPERIAL_GUARDIAN_STATE_SEND
{
	PSBMSG_HEAD header; // C1:F7:02
	BYTE state;
	BYTE level;
	BYTE stage;
	BYTE count;
	DWORD time;
};

struct PMSG_IMPERIAL_GUARDIAN_INTERFACE_SEND
{
	PSBMSG_HEAD header; // C1:F7:04
	BYTE state;
	BYTE level;
	BYTE stage;
	DWORD time;
	BYTE count;
};

struct PMSG_IMPERIAL_GUARDIAN_RESULT_SEND
{
	PSBMSG_HEAD header; // C1:F7:06
	BYTE result;
	DWORD experience;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct IMPERIAL_GUARDIAN_MONSTER_DIFFICULT_INFO
{
	int Index;
	int Level;
	float Life;
	float DamageMin;
	float DamageMax;
	float Defense;
	float AttackSuccessRate;
	float DefenseSuccessRate;
};

struct IMPERIAL_GUARDIAN_USER
{
	int Index;
	int RewardExperience;
};

class CImperialGuardian
{
public:
	CImperialGuardian();
	virtual ~CImperialGuardian();
	void Load(char* path);
	void MainProc();
	void ProcState_EMPTY();
	void ProcState_WAIT1();
	void ProcState_PLAY1();
	void ProcState_WAIT2();
	void ProcState_PLAY2();
	void ProcState_WAIT3();
	void ProcState_PLAY3();
	void ProcState_WAIT4();
	void ProcState_PLAY4();
	void ProcState_CLEAN();
	void SetState(int state);
	void SetState_EMPTY();
	void SetState_WAIT1();
	void SetState_PLAY1();
	void SetState_WAIT2();
	void SetState_PLAY2();
	void SetState_WAIT3();
	void SetState_PLAY3();
	void SetState_WAIT4();
	void SetState_PLAY4();
	void SetState_CLEAN();
	int GetState();
	bool AddUser(int aIndex);
	bool DelUser(int aIndex);
	IMPERIAL_GUARDIAN_USER* GetUser(int aIndex);
	void CleanUser();
	void ClearUser();
	void CheckUser();
	int GetUserCount();
	void GiveUserRewardExperience(int aIndex);
	bool AddMonster(int aIndex);
	bool DelMonster(int aIndex);
	int* GetMonster(int aIndex);
	void CleanMonster();
	void ClearMonster();
	void ClearMonsterTrap();
	int GetMonsterCount();
	void SetSideGate();
	void SetLockGate();
	void SetMainGate();
	void SetSideBoss(int MonsterClass);
	void SetMainBoss(int MonsterClass);
	void SetMonster(int MonsterClass);
	void SetMonsterTrap(int MonsterClass);
	void NpcJerryTheAdviser(LPOBJ lpNpc,LPOBJ lpObj);
	void MoveGate(LPOBJ lpObj,int gate);
	void DropSideBossItem(LPOBJ lpObj,LPOBJ lpTarget);
	void DropMainBossItem(LPOBJ lpObj,LPOBJ lpTarget);
	float GetMonsterLevel(float value);
	float GetMonsterLife(float value);
	float GetMonsterDamageMin(float value);
	float GetMonsterDamageMax(float value);
	float GetMonsterDefense(float value);
	float GetMonsterAttackSuccessRate(float value);
	float GetMonsterDefenseSuccessRate(float value);
	void SetSideGateZone();
	void SetLockGateZone();
	void SetMainGateZone();
	void DelSideGateZone();
	void DelLockGateZone();
	void DelMainGateZone();
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(int type,char* message,...);
	void DataSendToAll(BYTE* lpMsg,int size);
	void CGImperialGuardianEnterRecv(PMSG_IMPERIAL_GUARDIAN_ENTER_RECV* lpMsg,int aIndex);
	void GCImperialGuardianStateSend(int state,int aIndex);
	void GCImperialGuardianInterfaceSend();
	void GCImperialGuardianResultSend(int aIndex,int result,int experience);
private:
	int m_NotifyTime;
	int m_EventTime;
	int m_CloseTime;
	int m_DefaultMonsterAIUnitNumber;
	int m_SupportMonsterAIUnitNumber;
	int m_DayOfWeek;
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
	IMPERIAL_GUARDIAN_USER m_User[MAX_IG_USER];
	int m_SideGateIndex;
	int m_LockGateIndex;
	int m_MainGateIndex;
	int m_SideBossIndex;
	int m_MainBossIndex;
	int m_MonsterIndex[MAX_IG_MONSTER];
	IMPERIAL_GUARDIAN_MONSTER_DIFFICULT_INFO m_MonsterDifficultInfo[MAX_IG_MONSTER_DIFFICULT];
};

extern CImperialGuardian gImperialGuardian;

static const BYTE gImperialGuardianRespawnZone[MAX_IG_LEVEL][4][2] = {{{230,42},{180,25},{180,86},{0,0}},{{63,63},{38,107},{96,108},{0,0}},{{128,188},{218,140},{162,225},{0,0}},{{62,63},{33,182},{164,127},{186,22}}};

static const BYTE gImperialGuardianSideGateZone[MAX_IG_LEVEL][4][4] = {{{232,28,236,31},{191,23,194,27},{178,79,182,82},{0,0,0,0}},{{72,65,75,69},{35,93,39,96},{104,110,107,114},{0,0,0,0}},{{143,189,146,193},{220,134,224,137},{165,217,169,220},{0,0,0,0}},{{78,67,81,71},{32,176,36,179},{156,130,159,134},{211,22,214,26}}};

static const BYTE gImperialGuardianLockGateZone[MAX_IG_LEVEL][4][4] = {{{231,55,235,58},{163,24,166,28},{0,0,0,0},{0,0,0,0}},{{47,63,50,67},{39,117,43,120},{0,0,0,0},{0,0,0,0}},{{116,190,119,194},{220,160,224,163},{0,0,0,0},{0,0,0,0}},{{47,67,50,71},{52,189,55,193},{197,130,200,134},{0,0,0,0}}};

static const BYTE gImperialGuardianMainGateZone[MAX_IG_LEVEL][4][4] = {{{213,78,216,82},{152,53,156,56},{0,0,0,0},{0,0,0,0}},{{16,63,19,67},{53,154,57,157},{0,0,0,0},{0,0,0,0}},{{86,193,89,197},{221,193,225,196},{0,0,0,0},{0,0,0,0}},{{30,90,34,93},{67,163,71,166},{222,159,226,162},{0,0,0,0}}};

static const BYTE gImperialGuardianSideGatePosition[MAX_IG_LEVEL][4][3] = {{{234,28,1},{194,25,3},{180,79,1},{0,0,0}},{{75,67,3},{37,93,1},{107,112,3},{0,0,0}},{{146,191,3},{222,134,1},{167,217,1},{0,0,0}},{{81,69,3},{34,176,1},{156,132,7},{214,24,3}}};

static const BYTE gImperialGuardianLockGatePosition[MAX_IG_LEVEL][4][3] = {{{233,55,1},{166,26,3},{0,0,0},{0,0,0}},{{50,65,3},{41,117,1},{0,0,0},{0,0,0}},{{119,192,3},{222,160,1},{0,0,0},{0,0,0}},{{50,69,3},{52,191,7},{197,132,7},{0,0,0}}};

static const BYTE gImperialGuardianMainGatePosition[MAX_IG_LEVEL][4][3] = {{{216,80,3},{154,53,1},{0,0,0},{0,0,0}},{{19,65,3},{55,154,1},{0,0,0},{0,0,0}},{{89,195,3},{223,193,1},{0,0,0},{0,0,0}},{{32,90,1},{69,166,5},{224,159,1},{0,0,0}}};
