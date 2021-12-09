// QuestWorldObjective.h: interface for the CQuestWorldObjective class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "ItemManager.h"
#include "User.h"

#define MAX_QUEST_WORLD_OBJECTIVE 200
#define MAX_QUEST_WORLD_OBJECTIVE_COUNT 5

enum eQuestWorldObjectiveType
{
	QUEST_WORLD_OBJECTIVE_NONE = 0,
	QUEST_WORLD_OBJECTIVE_MONSTER = 1,
	QUEST_WORLD_OBJECTIVE_SKILL = 2,
	QUEST_WORLD_OBJECTIVE_ITEM = 4,
	QUEST_WORLD_OBJECTIVE_LEVEL = 8,
	QUEST_WORLD_OBJECTIVE_SPECIAL = 16,
};

struct QUEST_WORLD_OBJECTIVE
{
	#pragma pack(1)
	DWORD type;
	WORD index;
	DWORD value;
	DWORD count;
	BYTE ItemInfo[MAX_ITEM_INFO];
	#pragma pack()
};

struct QUEST_WORLD_OBJECTIVE_INFO
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

class CQuestWorldObjective
{
public:
	CQuestWorldObjective();
	virtual ~CQuestWorldObjective();
	void Load(char* path);
	void SetInfo(QUEST_WORLD_OBJECTIVE_INFO info);
	QUEST_WORLD_OBJECTIVE_INFO* GetInfo(int index);
	int GetQuestWorldObjectiveCount(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo);
	int GetQuestWorldObjectiveKillCount(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo);
	bool GetQuestWorldObjective(LPOBJ lpObj,int QuestIndex,int QuestGroup,QUEST_WORLD_OBJECTIVE* lpObjective,int ObjectiveNumber);
	bool CheckQuestWorldObjectiveRequisite(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo);
	bool CheckQuestWorldObjective(LPOBJ lpObj,int QuestIndex,int QuestGroup);
	void AddQuestWorldObjectiveKillCount(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo);
	void RemoveQuestWorldObjective(LPOBJ lpObj,int QuestIndex,int QuestGroup);
	void PressButton(LPOBJ lpObj,int QuestIndex,int QuestGroup);
	void MonsterKill(LPOBJ lpMonster);
	void MonsterKillParty(LPOBJ lpMonster,int PartyNumber);
private:
	QUEST_WORLD_OBJECTIVE_INFO m_QuestWorldObjectiveInfo[MAX_QUEST_WORLD_OBJECTIVE];
	int m_count;
};

extern CQuestWorldObjective gQuestWorldObjective;
