// Duel.h: interface for the CDuel class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

#define MAX_DUEL_ARENA 4
#define MAX_DUEL_ARENA_GLADIATOR 2
#define MAX_DUEL_ARENA_SPECTATOR 10

enum eDuelArenaState
{
	DA_STATE_EMPTY = 0,
	DA_STATE_START = 1,
	DA_STATE_CLEAN = 2,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_DUEL_START_RECV
{
	#if(GAMESERVER_UPDATE>=402)
	PSBMSG_HEAD header; // C1:AA:01
	BYTE index[2];
	char name[10];
	#else
	PBMSG_HEAD header; // C1:AA
	BYTE index[2];
	char name[10];
	#endif
};

struct PMSG_DUEL_OK_RECV
{
	#if(GAMESERVER_UPDATE>=402)
	PSBMSG_HEAD header; // C1:AA:02
	BYTE flag;
	BYTE index[2];
	char name[10];
	#else
	PBMSG_HEAD header; // C1:AC
	BYTE flag;
	BYTE index[2];
	char name[10];
	#endif
};

struct PMSG_DUEL_WATCH_RECV
{
	PSBMSG_HEAD header; // C1:AA:07
	BYTE number;
};

struct PMSG_DUEL_BUTTON_RECV
{
	PSBMSG_HEAD header; // C1:AA:09
	BYTE number;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_DUEL_START_SEND
{
	#if(GAMESERVER_UPDATE>=402)
	PSBMSG_HEAD header; // C1:AA:01
	BYTE result;
	BYTE index[2];
	char name[10];
	#else
	PBMSG_HEAD header; // C1:AA
	BYTE result;
	BYTE index[2];
	char name[10];
	#endif
};

struct PMSG_DUEL_OK_SEND
{
	#if(GAMESERVER_UPDATE>=402)
	PSBMSG_HEAD header; // C1:AA:02
	BYTE index[2];
	char name[10];
	#else
	PBMSG_HEAD header; // C1:AC
	BYTE index[2];
	char name[10];
	#endif
};

struct PMSG_DUEL_END_SEND
{
	#if(GAMESERVER_UPDATE>=402)
	PSBMSG_HEAD header; // C1:AA:03
	BYTE result;
	#else
	PBMSG_HEAD header; // C1:AB
	BYTE index[2];
	char name[10];
	#endif
};

struct PMSG_DUEL_SCORE_SEND
{
	#if(GAMESERVER_UPDATE>=402)
	PSBMSG_HEAD header; // C1:AA:04
	BYTE index1[2];
	BYTE index2[2];
	BYTE score[2];
	#else
	PBMSG_HEAD header; // C1:AD
	BYTE index1[2];
	BYTE index2[2];
	BYTE score[2];
	#endif
};

struct PMSG_DUEL_INTERFACE_SEND
{
	PSBMSG_HEAD header; // C1:AA:05
	BYTE index1[2];
	BYTE index2[2];
	BYTE life1;
	BYTE life2;
	BYTE shield1;
	BYTE shield2;
};

struct PMSG_DUEL_NPC_INFO_SEND
{
	PSBMSG_HEAD header; // C1:AA:06
};

struct PMSG_DUEL_NPC_INFO
{
	char name1[10];
	char name2[10];
	BYTE state;
	BYTE flag;
};

struct PMSG_DUEL_GLADIATOR_ADD_SEND
{
	PSBMSG_HEAD header; // C1:AA:07
	BYTE result;
	BYTE number;
	char name1[10];
	char name2[10];
	BYTE index1[2];
	BYTE index2[2];
};

struct PMSG_DUEL_SPECTATOR_ADD_SEND
{
	PSBMSG_HEAD header; // C1:AA:08
	char name[10];
};

struct PMSG_DUEL_GLADIATOR_DEL_SEND
{
	PSBMSG_HEAD header; // C1:AA:09
	BYTE result;
};

struct PMSG_DUEL_SPECTATOR_DEL_SEND
{
	PSBMSG_HEAD header; // C1:AA:0A
	char name[10];
};

struct PMSG_DUEL_SPECTATOR_LIST_SEND
{
	PSBMSG_HEAD header; // C1:AA:0B
	BYTE count;
};

struct PMSG_DUEL_SPECTATOR_LIST
{
	char name[10];
};

struct PMSG_DUEL_WIN_SEND
{
	PSBMSG_HEAD header; // C1:AA:0C
	char WinnerName[10];
	char LoserName[10];
};

struct PMSG_DUEL_ROUND_START_SEND
{
	PSBMSG_HEAD header; // C1:AA:0D
	BYTE flag;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct DUEL_ARENA_INFO
{
	int Index;
	int State;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int Gladiator[MAX_DUEL_ARENA_GLADIATOR];
	int Spectator[MAX_DUEL_ARENA_SPECTATOR];
};

class CDuel
{
public:
	CDuel();
	virtual ~CDuel();
	void MainProc();
	void ProcState_EMPTY(DUEL_ARENA_INFO* lpInfo);
	void ProcState_START(DUEL_ARENA_INFO* lpInfo);
	void ProcState_CLEAN(DUEL_ARENA_INFO* lpInfo);
	void SetState(DUEL_ARENA_INFO* lpInfo,int state);
	void SetState_EMPTY(DUEL_ARENA_INFO* lpInfo);
	void SetState_START(DUEL_ARENA_INFO* lpInfo);
	void SetState_CLEAN(DUEL_ARENA_INFO* lpInfo);
	bool AddDuelArena(LPOBJ lpObj,LPOBJ lpTarget);
	DUEL_ARENA_INFO* GetDuelArenaByGladiator(int aIndex);
	DUEL_ARENA_INFO* GetDuelArenaBySpectator(int aIndex);
	bool AddDuelArenaGladiator(DUEL_ARENA_INFO* lpInfo,int aIndex);
	bool DelDuelArenaGladiator(DUEL_ARENA_INFO* lpInfo,int aIndex);
	int* GetDuelArenaGladiator(DUEL_ARENA_INFO* lpInfo,int aIndex);
	bool AddDuelArenaSpectator(DUEL_ARENA_INFO* lpInfo,int aIndex);
	bool DelDuelArenaSpectator(DUEL_ARENA_INFO* lpInfo,int aIndex);
	int* GetDuelArenaSpectator(DUEL_ARENA_INFO* lpInfo,int aIndex);
	void CheckDuelArenaGladiator(DUEL_ARENA_INFO* lpInfo);
	void CheckDuelArenaSpectator(DUEL_ARENA_INFO* lpInfo);
	void ClearDuelArenaGladiator(DUEL_ARENA_INFO* lpInfo);
	void ClearDuelArenaSpectator(DUEL_ARENA_INFO* lpInfo);
	int GetDuelArenaGladiatorCount(DUEL_ARENA_INFO* lpInfo);
	int GetDuelArenaSpectatorCount(DUEL_ARENA_INFO* lpInfo);
	bool CheckDuel(LPOBJ lpObj,LPOBJ lpTarget);
	void CheckDuelScore(LPOBJ lpObj,LPOBJ lpTarget);
	void CheckDuelUser();
	void ResetDuel(LPOBJ lpObj);
	void RespawnDuelUser(LPOBJ lpObj);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void CGDuelStartRecv(PMSG_DUEL_START_RECV* lpMsg,int aIndex);
	void CGDuelOkRecv(PMSG_DUEL_OK_RECV* lpMsg,int aIndex);
	void CGDuelEndRecv(int aIndex);
	void CGDuelWatchRecv(PMSG_DUEL_WATCH_RECV* lpMsg,int aIndex);
	void CGDuelButtonRecv(PMSG_DUEL_BUTTON_RECV* lpMsg,int aIndex);
	void GCDuelStartSend(int aIndex,BYTE flag,int bIndex);
	void GCDuelOkSend(int aIndex,int bIndex);
	void GCDuelEndSend(int aIndex,BYTE result);
	void GCDuelScoreSend(int aIndex,int bIndex);
	void GCDuelScoreSend(DUEL_ARENA_INFO* lpInfo);
	void GCDuelInterfaceSend(DUEL_ARENA_INFO* lpInfo);
	void GCDuelNpcInfoSend(int aIndex);
	void GCDuelGladiatorAddSend(int aIndex,DUEL_ARENA_INFO* lpInfo);
	void GCDuelSpectatorAddSend(int aIndex,DUEL_ARENA_INFO* lpInfo);
	void GCDuelGladiatorDelSend(int aIndex,BYTE result);
	void GCDuelSpectatorDelSend(int aIndex,DUEL_ARENA_INFO* lpInfo);
	void GCDuelSpectatorListSend(int aIndex,DUEL_ARENA_INFO* lpInfo);
	void GCDuelWinSend(int aIndex,char* WinnerName,char* LoserName);
	void GCDuelWinSend(DUEL_ARENA_INFO* lpInfo,char* WinnerName,char* LoserName);
	void GCDuelRoundStartSend(DUEL_ARENA_INFO* lpInfo,BYTE flag);
	void StartDuelBit(LPOBJ lpObj, LPOBJ lpTarget);
private:
	DUEL_ARENA_INFO m_DuelArenaInfo[MAX_DUEL_ARENA];
};

extern CDuel gDuel;
