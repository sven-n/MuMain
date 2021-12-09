// QuestReward.h: interface for the CQuestReward class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "User.h"

#define MAX_QUEST_REWARD 100

enum eQuestRewardType
{
	QUEST_REWARD_NONE = 0,
	QUEST_REWARD_POINT = 1,
	QUEST_REWARD_CHANGE1 = 2,
	QUEST_REWARD_HERO = 4,
	QUEST_REWARD_COMBO = 8,
	QUEST_REWARD_CHANGE2 = 16,
};

struct QUEST_REWARD_INFO
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
	int RequireState;
	int RequireClass[MAX_CLASS];
};

class CQuestReward
{
public:
	CQuestReward();
	virtual ~CQuestReward();
	void Load(char* path);
	void SetInfo(QUEST_REWARD_INFO info);
	QUEST_REWARD_INFO* GetInfo(int index);
	bool CheckQuestRewardRequisite(LPOBJ lpObj,QUEST_REWARD_INFO* lpInfo);
	void InsertQuestReward(LPOBJ lpObj,int QuestIndex);
	long GetQuestRewardPoint(LPOBJ lpObj,int QuestIndex);
private:
	QUEST_REWARD_INFO m_QuestObjectiveInfo[MAX_QUEST_REWARD];
	int m_count;
};

extern CQuestReward gQuestReward;
