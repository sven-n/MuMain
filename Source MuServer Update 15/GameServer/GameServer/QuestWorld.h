// QuestWorld.h: interface for the CQuestWorld class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "QuestWorldObjective.h"
#include "QuestWorldReward.h"
#include "User.h"

#define MAX_QUEST_WORLD 10000
#define MAX_QUEST_WORLD_LIST 20
#define MAX_QUEST_WORLD_DIALOG_OPTION 4

enum eQuestWorldState
{
	QUEST_WORLD_NORMAL = 0,
	QUEST_WORLD_ACCEPT = 1,
	QUEST_WORLD_REJECT = 2,
	QUEST_WORLD_FINISH = 3,
	QUEST_WORLD_CANCEL = 4,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_QUEST_WORLD_DIALOG_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:F6:0A
	WORD QuestIndex;
	WORD QuestGroup;
	BYTE QuestOption;
	#pragma pack()
};

struct PMSG_QUEST_WORLD_ACCEPT_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:F6:0B
	WORD QuestIndex;
	WORD QuestGroup;
	BYTE QuestOption;
	#pragma pack()
};

struct PMSG_QUEST_WORLD_FINISH_RECV
{
	PSBMSG_HEAD header; // C1:F6:0D
	WORD QuestIndex;
	WORD QuestGroup;
};

struct PMSG_QUEST_WORLD_CANCEL_RECV
{
	PSBMSG_HEAD header; // C1:F6:0F
	WORD QuestIndex;
	WORD QuestGroup;
};

struct PMSG_QUEST_WORLD_BUTTON_RECV
{
	PSBMSG_HEAD header; // C1:F6:10
	WORD QuestIndex;
	WORD QuestGroup;
};

struct PMSG_QUEST_WORLD_DETAIL_RECV
{
	PSBMSG_HEAD header; // C1:F6:1B
	WORD QuestIndex;
	WORD QuestGroup;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_QUEST_WORLD_BUTTON_SEND
{
	PSBMSG_HEAD header; // C1:F6:03
	WORD QuestIndex1;
	WORD QuestGroup1;
	WORD QuestIndex2;
	WORD QuestGroup2;
};

struct PMSG_QUEST_WORLD_LIST_SEND
{
	PSBMSG_HEAD header; // C1:F6:0A
	WORD MonsterClass;
	BYTE count;
	BYTE result;
};

struct PMSG_QUEST_WORLD_LIST
{
	WORD QuestIndex;
	WORD QuestGroup;
};

struct PMSG_QUEST_WORLD_ACCEPT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:F6:0B
	WORD QuestIndex;
	WORD QuestGroup;
	BYTE QuestOption;
	#pragma pack()
};

struct PMSG_QUEST_WORLD_DETAIL_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:F6:[0C:1B]
	WORD QuestIndex;
	WORD QuestGroup;
	BYTE ObjectiveCount;
	BYTE RewardCount;
	BYTE result;
	QUEST_WORLD_OBJECTIVE QuestObjective[MAX_QUEST_WORLD_OBJECTIVE_COUNT];
	QUEST_WORLD_REWARD QuestReward[MAX_QUEST_WORLD_OBJECTIVE_COUNT];
	#pragma pack()
};

struct PMSG_QUEST_WORLD_FINISH_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:F6:0D
	WORD QuestIndex;
	WORD QuestGroup;
	BYTE QuestResult;
	#pragma pack()
};

struct PMSG_QUEST_WORLD_CANCEL_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:F6:0F
	WORD QuestIndex;
	WORD QuestGroup;
	BYTE QuestResult;
	#pragma pack()
};

struct PMSG_QUEST_WORLD_INTERFACE_LIST_SEND
{
	PSBMSG_HEAD header; // C1:F6:1A
	BYTE count;
};

struct PMSG_QUEST_WORLD_INTERFACE_LIST
{
	WORD QuestIndex;
	WORD QuestGroup;
};

struct PMSG_QUEST_WORLD_NPC_TALK_SEND
{
	PSBMSG_HEAD header; // C1:F9:01
	WORD MonsterClass;
	DWORD contribution;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_QUEST_WORLD_RECV
{
	PSWMSG_HEAD header; // C1:10:00
	WORD index;
	char account[11];
	char name[11];
	BYTE QuestWorldList[MAX_QUEST_WORLD_LIST][20];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_QUEST_WORLD_SEND
{
	PSBMSG_HEAD header; // C1:10:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_QUEST_WORLD_SAVE_SEND
{
	PSWMSG_HEAD header; // C1:10:30
	WORD index;
	char account[11];
	char name[11];
	BYTE QuestWorldList[MAX_QUEST_WORLD_LIST][20];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct QUEST_WORLD_LIST
{
	WORD QuestIndex;
	WORD QuestStart;
	WORD QuestNumber;
	BYTE QuestState;
	BYTE CooldownDay;
	BYTE CooldownMonth;
	BYTE CooldownYear;
	WORD ObjectiveCount[MAX_QUEST_WORLD_OBJECTIVE_COUNT];
};

struct QUEST_WORLD_INFO
{
	int Index;
	int Group;
	int Number;
	int StartType;
	int MonsterClass;
	int CurrentState;
	int NextIndex[MAX_QUEST_WORLD_DIALOG_OPTION];
	int NextState[MAX_QUEST_WORLD_DIALOG_OPTION];
	int RequireIndex;
	int RequireGroup;
	int RequireState;
	int RequireMinLevel;
	int RequireMaxLevel;
	int RequireFamily;
	int RequireCooldown;
	int RequireClass[MAX_CLASS];
};

class CQuestWorld
{
public:
	CQuestWorld();
	virtual ~CQuestWorld();
	void Load(char* path);
	void SetInfo(QUEST_WORLD_INFO info);
	QUEST_WORLD_INFO* GetInfo(int index);
	QUEST_WORLD_INFO* GetInfoByIndex(LPOBJ lpObj,int QuestIndex,int QuestGroup);
	bool AddQuestWorldList(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestNumber,int QuestState);
	QUEST_WORLD_LIST* GetQuestWorldList(LPOBJ lpObj,int QuestIndex,int QuestGroup);
	bool CheckQuestWorldRequisite(LPOBJ lpObj,QUEST_WORLD_INFO* lpInfo);
	bool CheckQuestWorldListState(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestState);
	void NpcElfBuffer(LPOBJ lpNpc,LPOBJ lpObj);
	void CGQuestWorldDialogRecv(PMSG_QUEST_WORLD_DIALOG_RECV* lpMsg,int aIndex);
	void CGQuestWorldAcceptRecv(PMSG_QUEST_WORLD_ACCEPT_RECV* lpMsg,int aIndex);
	void CGQuestWorldFinishRecv(PMSG_QUEST_WORLD_FINISH_RECV* lpMsg,int aIndex);
	void CGQuestWorldCancelRecv(PMSG_QUEST_WORLD_CANCEL_RECV* lpMsg,int aIndex);
	void CGQuestWorldButtonRecv(PMSG_QUEST_WORLD_BUTTON_RECV* lpMsg,int aIndex);
	void CGQuestWorldDetailRecv(PMSG_QUEST_WORLD_DETAIL_RECV* lpMsg,int aIndex);
	void CGQuestWorldListRecv(int aIndex);
	void CGQuestWorldNpcElfBufferRecv(int aIndex);
	void GCQuestWorldButtonSend(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestOption);
	void GCQuestWorldAcceptSend(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestOption);
	void GCQuestWorldDetailSend(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestOption);
	void GCQuestWorldFinishSend(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestResult);
	void GCQuestWorldCancelSend(LPOBJ lpObj,int QuestIndex,int QuestGroup,int QuestResult);
	void GCQuestWorldInterfaceListSend(LPOBJ lpObj);
	void DGQuestWorldRecv(SDHP_QUEST_WORLD_RECV* lpMsg);
	void GDQuestWorldSend(int aIndex);
	void GDQuestWorldSaveSend(int aIndex);
private:
	QUEST_WORLD_INFO m_QuestWorldInfo[MAX_QUEST_WORLD];
	int m_count;
};

extern CQuestWorld gQuestWorld;
