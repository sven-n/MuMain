// Quest.h: interface for the CQuest class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "Protocol.h"
#include "QuestObjective.h"
#include "User.h"

#define MAX_QUEST 500
#define MAX_QUEST_LIST 200
#define MAX_QUEST_BIT_MASK 8
#define MAX_QUEST_KILL_COUNT 5

enum eQuestState
{
	QUEST_NORMAL = 0,
	QUEST_ACCEPT = 1,
	QUEST_FINISH = 2,
	QUEST_CANCEL = 3,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_QUEST_STATE_RECV
{
	PBMSG_HEAD header; // C1:A1
	BYTE QuestIndex;
	BYTE QuestState;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_QUEST_INFO_SEND
{
	PBMSG_HEAD header; // C1:A0
	BYTE count;
	BYTE QuestInfo[50];
};

struct PMSG_QUEST_STATE_SEND
{
	PBMSG_HEAD header; // C1:A1
	BYTE QuestIndex;
	BYTE QuestState;
};

struct PMSG_QUEST_RESULT_SEND
{
	PBMSG_HEAD header; // C1:A2
	BYTE QuestIndex;
	BYTE QuestResult;
	BYTE QuestState;
};

struct PMSG_QUEST_REWARD_SEND
{
	PBMSG_HEAD header; // C1:A3
	BYTE index[2];
	BYTE QuestReward;
	BYTE QuestAmount;
	#if(GAMESERVER_EXTRA==1)
	DWORD ViewPoint;
	#endif
};

struct PMSG_QUEST_KILL_COUNT_SEND
{
	PBMSG_HEAD header; // C1:A4
	BYTE QuestResult;
	BYTE QuestIndex;
	QUEST_KILL_COUNT QuestKillCount[MAX_QUEST_KILL_COUNT];
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_QUEST_KILL_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:0C:00
	WORD index;
	char account[11];
	char name[11];
	DWORD QuestIndex;
	DWORD MonsterClass[5];
	DWORD KillCount[5];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_QUEST_KILL_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:0C:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_QUEST_KILL_COUNT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:0C:30
	WORD index;
	char account[11];
	char name[11];
	DWORD QuestIndex;
	DWORD MonsterClass[5];
	DWORD KillCount[5];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct QUEST_INFO
{
	int Index;
	int StartType;
	int MonsterClass;
	int CurrentState;
	int RequireIndex;
	int RequireState;
	int RequireMinLevel;
	int RequireMaxLevel;
	int RequireClass[MAX_CLASS];
};

class CQuest
{
public:
	CQuest();
	virtual ~CQuest();
	void Load(char* path);
	void SetInfo(QUEST_INFO info);
	QUEST_INFO* GetInfo(int index);
	QUEST_INFO* GetInfoByIndex(LPOBJ lpObj,int QuestIndex);
	bool AddQuestList(LPOBJ lpObj,int QuestIndex,int QuestState);
	BYTE GetQuestList(LPOBJ lpObj,int QuestIndex);
	bool CheckQuestRequisite(LPOBJ lpObj,QUEST_INFO* lpInfo);
	bool CheckQuestListState(LPOBJ lpObj,int QuestIndex,int QuestState);
	long GetQuestRewardLevelUpPoint(LPOBJ lpObj);
	bool NpcTalk(LPOBJ lpNpc,LPOBJ lpObj);
	void CGQuestInfoRecv(int aIndex);
	void CGQuestStateRecv(PMSG_QUEST_STATE_RECV* lpMsg,int aIndex);
	void CGQuestNpcWarewolfRecv(int aIndex);
	void CGQuestNpcKeeperRecv(int aIndex);
	void GCQuestInfoSend(int aIndex);
	void GCQuestStateSend(int aIndex,int QuestIndex);
	void GCQuestResultSend(int aIndex,int QuestIndex,int QuestResult,int QuestState);
	void GCQuestRewardSend(int aIndex,int QuestReward,int QuestAmount);
	void GCQuestKillCountSend(int aIndex,int QuestIndex);
	void DGQuestKillCountRecv(SDHP_QUEST_KILL_COUNT_RECV* lpMsg);
	void GDQuestKillCountSend(int aIndex);
	void GDQuestKillCountSaveSend(int aIndex);
private:
	QUEST_INFO m_QuestInfo[MAX_QUEST];
	int m_count;
};

extern CQuest gQuest;

static const BYTE gQuestBitMask[MAX_QUEST_BIT_MASK] = {0xFC,0xFC,0xF3,0xF3,0xCF,0xCF,0x3F,0x3F};
