// Quest.h: interface for the CQuest class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_QUEST_KILL_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:0C:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_QUEST_KILL_COUNT_SAVE_RECV
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
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_QUEST_KILL_COUNT_SEND
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
//**********************************************//
//**********************************************//

class CQuest
{
public:
	CQuest();
	virtual ~CQuest();
	void GDQuestKillCountRecv(SDHP_QUEST_KILL_COUNT_RECV* lpMsg,int index);
	void GDQuestKillCountSaveRecv(SDHP_QUEST_KILL_COUNT_SAVE_RECV* lpMsg);
};

extern CQuest gQuest;
