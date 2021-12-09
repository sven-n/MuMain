// MiningSystem.h: interface for the CMiningSystem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

#define MAX_MINING_STAGE 5
#define MAX_MINING_JEWEL_PACK 40

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_MINING_START_RECV
{
	PSBMSG_HEAD header; // C1:4C:00
	WORD index;
	WORD MonsterIndex;
	WORD MonsterClass;
	BYTE map;
	BYTE stage;
};

struct PMSG_MINING_SUCCESS_RECV
{
	PSBMSG_HEAD header; // C1:4C:01
	WORD index;
	WORD MonsterIndex;
	WORD MonsterClass;
	BYTE map;
	BYTE stage;
	BYTE result;
};

struct PMSG_MINING_FAILURE_RECV
{
	PSBMSG_HEAD header; // C1:4C:03
	WORD index;
	WORD MonsterIndex;
	BYTE map;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_MINING_START_SEND
{
	PSBMSG_HEAD header; // C1:4C:00
	WORD index;
	WORD MonsterIndex;
	WORD MonsterClass;
	BYTE map;
	BYTE stage;
	DWORD value;
	DWORD result;
};

struct PMSG_MINING_SUCCESS_SEND
{
	PSBMSG_HEAD header; // C1:4C:01
	WORD index;
	WORD MonsterIndex;
	WORD MonsterClass;
	BYTE map;
	BYTE stage;
	BYTE result;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct MINING_INFO
{
	int Index;
	int Stage;
	int Value;
	int ItemRate;
	int ItemIndex;
	int ItemCount;
	int BonusItemRate;
	int BonusItemIndex;
	int BonusItemCount;
};

class CMiningSystem
{
public:
	CMiningSystem();
	virtual ~CMiningSystem();
	void Load(char* path);
	bool GetMiningInfo(int index,int stage,MINING_INFO* lpInfo);
	void CreateMiningReward(int aIndex,int ItemIndex,int PackIndex,int ItemCount);
	void CGMiningStartRecv(PMSG_MINING_START_RECV* lpMsg,int aIndex);
	void CGMiningSuccessRecv(PMSG_MINING_SUCCESS_RECV* lpMsg,int aIndex);
	void CGMiningFailureRecv(PMSG_MINING_FAILURE_RECV* lpMsg,int aIndex);
	void GCMiningStartSend(int aIndex,int MonsterIndex,int MonsterClass,int map,int stage,int value,int result);
	void GCMiningSuccessSend(int aIndex,int MonsterIndex,int MonsterClass,int map,int stage,int result);
private:
	std::vector<MINING_INFO> m_MiningInfo;
};

extern CMiningSystem gMiningSystem;
