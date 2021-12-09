// QuestWorldReward.h: interface for the CQuestWorldReward class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "ItemManager.h"
#include "User.h"

#define MAX_QUEST_WORLD_REWARD 200

enum eQuestWorldRewardType
{
	QUEST_WORLD_REWARD_NONE = 0,
	QUEST_WORLD_REWARD_EXPERIENCE = 1,
	QUEST_WORLD_REWARD_MONEY = 2,
	QUEST_WORLD_REWARD_ITEM = 4,
	QUEST_WORLD_REWARD_EFFECT = 8,
	QUEST_WORLD_REWARD_CONTRIBUTION = 16,
};

struct QUEST_WORLD_REWARD
{
	#pragma pack(1)
	DWORD type;
	WORD index;
	DWORD value;
	BYTE ItemInfo[MAX_ITEM_INFO];
	#pragma pack()
};

struct QUEST_WORLD_REWARD_INFO
{
	int Number;
	int Type;
	int Index;
	int Value;
	int Level;
	int Option1;
	int Option2;
	int Option3;
	int NewOption;
	int RequireIndex;
	int RequireGroup;
	int RequireState;
	int RequireClass[MAX_CLASS];
};

class CQuestWorldReward
{
public:
	CQuestWorldReward();
	virtual ~CQuestWorldReward();
	void Load(char* path);
	void SetInfo(QUEST_WORLD_REWARD_INFO info);
	QUEST_WORLD_REWARD_INFO* GetInfo(int index);
	bool GetQuestWorldReward(LPOBJ lpObj,int QuestIndex,int QuestGroup,QUEST_WORLD_REWARD* lpReward,int RewardNumber);
	bool CheckQuestWorldRewardRequisite(LPOBJ lpObj,QUEST_WORLD_REWARD_INFO* lpInfo);
	void InsertQuestWorldReward(LPOBJ lpObj,int QuestIndex,int QuestGroup);
private:
	QUEST_WORLD_REWARD_INFO m_QuestWorldRewardInfo[MAX_QUEST_WORLD_REWARD];
	int m_count;
};

extern CQuestWorldReward gQuestWorldReward;
