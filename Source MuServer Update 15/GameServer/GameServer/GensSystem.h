// GensSystem.h: interface for the CGensSystem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_GENS_SYSTEM_SYMBOL 15
#define MAX_GENS_SYSTEM_VICTIM 20

enum eGensSystemFamily
{
	GENS_FAMILY_NONE = 0,
	GENS_FAMILY_VARNERT = 1,
	GENS_FAMILY_DUPRIAN = 2,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_GENS_SYSTEM_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:F8:01
	BYTE GensFamily;
};

struct PMSG_GENS_SYSTEM_DELETE_RECV
{
	PSBMSG_HEAD header; // C1:F8:03
};

struct PMSG_GENS_SYSTEM_REWARD_RECV
{
	PSBMSG_HEAD header; // C1:F8:09
	BYTE GensFamily;
};

struct PMSG_GENS_SYSTEM_MEMBER_RECV
{
	PSBMSG_HEAD header; // C1:F8:0B
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_GENS_SYSTEM_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:F8:02
	BYTE result;
	BYTE GensFamily;
};

struct PMSG_GENS_SYSTEM_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:F8:04
	BYTE result;
};

struct PMSG_GENS_SYSTEM_VIEWPORT_SEND
{
	PSWMSG_HEAD header; // C1:F8:05
	BYTE count;
};

struct PMSG_GENS_SYSTEM_VIEWPORT
{
	BYTE GensFamily;
	BYTE index[2];
	DWORD GensRank;
	DWORD GensSymbol;
	DWORD GensContribution;
};

struct PMSG_GENS_SYSTEM_MEMBER_SEND
{
	PSBMSG_HEAD header; // C1:F8:07
	DWORD GensFamily;
	DWORD GensRank;
	DWORD GensSymbol;
	DWORD GensContribution;
	DWORD GensNextContribution;
};

struct PMSG_GENS_SYSTEM_DISABLE_MOVE_SEND
{
	PSBMSG_HEAD header; // C1:F8:08
};

struct PMSG_GENS_SYSTEM_REWARD_SEND
{
	PSBMSG_HEAD header; // C1:F8:0A
	BYTE result;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GENS_SYSTEM_INSERT_RECV
{
	PSBMSG_HEAD header; // C1:11:00
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_DELETE_RECV
{
	PSBMSG_HEAD header; // C1:11:01
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
};

struct SDHP_GENS_SYSTEM_MEMBER_RECV
{
	PSBMSG_HEAD header; // C1:11:02
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_UPDATE_RECV
{
	PSBMSG_HEAD header; // C1:11:03
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_REWARD_RECV
{
	PSBMSG_HEAD header; // C1:11:04
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensRewardStatus;
};

struct SDHP_GENS_SYSTEM_CREATE_RECV
{
	PSBMSG_HEAD header; // C1:11:70
	DWORD GensFamily;
	DWORD GensRankStart;
	DWORD GensRankFinal;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GENS_SYSTEM_INSERT_SEND
{
	PSBMSG_HEAD header; // C1:11:00
	WORD index;
	char account[11];
	char name[11];
	BYTE GensFamily;
};

struct SDHP_GENS_SYSTEM_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:11:01
	WORD index;
	char account[11];
	char name[11];
	BYTE GensFamily;
	DWORD GensRank;
};

struct SDHP_GENS_SYSTEM_MEMBER_SEND
{
	PSBMSG_HEAD header; // C1:11:02
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_GENS_SYSTEM_UPDATE_SEND
{
	PSBMSG_HEAD header; // C1:11:03
	WORD index;
	char account[11];
	char name[11];
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensContribution;
};

struct SDHP_GENS_SYSTEM_REWARD_SEND
{
	PSBMSG_HEAD header; // C1:11:04
	WORD index;
	char account[11];
	char name[11];
	BYTE GensFamily;
};

struct SDHP_GENS_SYSTEM_REWARD_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:11:30
	WORD index;
	char account[11];
	char name[11];
	BYTE GensFamily;
	DWORD GensRank;
	DWORD GensRewardStatus;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct GENS_SYSTEM_VICTIM_LIST
{
	void Reset() // OK
	{
		this->Time = 0;

		this->KillCount = 0;

		memset(this->Name,0,sizeof(this->Name));
	}

	void Set(char* name) // OK
	{
		this->Time = GetTickCount();

		this->KillCount = 1;

		memcpy(this->Name,name,sizeof(this->Name));
	}

	UINT Time;
	char Name[11];
	UINT KillCount;
};

class CGensSystem
{
public:
	CGensSystem();
	virtual ~CGensSystem();
	void CalcGensMemberInfo(LPOBJ lpObj);
	int GetGensRewardSymbol(int GensRank);
	void NpcGensDuprianSteward(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcGensVarnertSteward(LPOBJ lpNpc,LPOBJ lpObj);
	bool VerifyGensVictimList(LPOBJ lpObj,LPOBJ lpTarget);
	bool InsertGensVictimList(LPOBJ lpObj,LPOBJ lpTarget);
	bool SearchGensVictimList(LPOBJ lpObj,LPOBJ lpTarget,GENS_SYSTEM_VICTIM_LIST** lpVictimList);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckGens(LPOBJ lpObj,LPOBJ lpTarget);
	BYTE GetGensBattleMap(BYTE* GensBattleMap);
	BYTE GetGensMoveIndex(BYTE* GensMoveIndex);
	void CGGensSystemInsertRecv(PMSG_GENS_SYSTEM_INSERT_RECV* lpMsg,int aIndex);
	void CGGensSystemDeleteRecv(PMSG_GENS_SYSTEM_DELETE_RECV* lpMsg,int aIndex);
	void CGGensSystemRewardRecv(PMSG_GENS_SYSTEM_REWARD_RECV* lpMsg,int aIndex);
	void CGGensSystemMemberRecv(PMSG_GENS_SYSTEM_MEMBER_RECV* lpMsg,int aIndex);
	void GCGensSystemInsertSend(int aIndex,BYTE result,BYTE GensFamily);
	void GCGensSystemDeleteSend(int aIndex,BYTE result);
	void GCGensSystemMemberSend(int aIndex);
	void GCGensSystemDisableMoveSend(int aIndex);
	void GCGensSystemRewardSend(int aIndex,BYTE result);
	void DGGensSystemInsertRecv(SDHP_GENS_SYSTEM_INSERT_RECV* lpMsg);
	void DGGensSystemDeleteRecv(SDHP_GENS_SYSTEM_DELETE_RECV* lpMsg);
	void DGGensSystemMemberRecv(SDHP_GENS_SYSTEM_MEMBER_RECV* lpMsg);
	void DGGensSystemUpdateRecv(SDHP_GENS_SYSTEM_UPDATE_RECV* lpMsg);
	void DGGensSystemRewardRecv(SDHP_GENS_SYSTEM_REWARD_RECV* lpMsg);
	void DGGensSystemCreateRecv(SDHP_GENS_SYSTEM_CREATE_RECV* lpMsg);
	void GDGensSystemInsertSend(int aIndex,BYTE GensFamily);
	void GDGensSystemDeleteSend(int aIndex,BYTE GensFamily);
	void GDGensSystemMemberSend(int aIndex);
	void GDGensSystemUpdateSend(int aIndex);
	void GDGensSystemRewardSend(int aIndex);
	void GDGensSystemRewardSaveSend(int aIndex,DWORD GensRank,DWORD GensRewardStatus);
};

extern CGensSystem gGensSystem;

static const int gGensSystemContributionTable[MAX_GENS_SYSTEM_SYMBOL] = {0,0,0,0,0,0,0,0,0,10000,6000,3000,1500,500,0};

static const int gGensSystemRewardAmountTable[MAX_GENS_SYSTEM_SYMBOL] = {0,30,20,20,10,10,5,5,3,0,0,0,0,0,0};
