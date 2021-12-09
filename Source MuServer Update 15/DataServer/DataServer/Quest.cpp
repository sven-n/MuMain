// Quest.cpp: implementation of the CQuest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Quest.h"
#include "QueryManager.h"
#include "SocketManager.h"

CQuest gQuest;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuest::CQuest() // OK
{

}

CQuest::~CQuest() // OK
{

}

void CQuest::GDQuestKillCountRecv(SDHP_QUEST_KILL_COUNT_RECV* lpMsg,int index) // OK
{
	SDHP_QUEST_KILL_COUNT_SEND pMsg;

	pMsg.header.set(0x0C,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	if(gQueryManager.ExecQuery("SELECT * FROM QuestKillCount WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.QuestIndex = 0xFFFFFFFF;

		memset(pMsg.MonsterClass,0xFF,sizeof(pMsg.MonsterClass));

		memset(pMsg.KillCount,0,sizeof(pMsg.KillCount));
	}
	else
	{
		pMsg.QuestIndex = gQueryManager.GetAsInteger("QuestIndex");

		pMsg.MonsterClass[0] = gQueryManager.GetAsInteger("MonsterClass1");

		pMsg.MonsterClass[1] = gQueryManager.GetAsInteger("MonsterClass2");

		pMsg.MonsterClass[2] = gQueryManager.GetAsInteger("MonsterClass3");

		pMsg.MonsterClass[3] = gQueryManager.GetAsInteger("MonsterClass4");

		pMsg.MonsterClass[4] = gQueryManager.GetAsInteger("MonsterClass5");

		pMsg.KillCount[0] = gQueryManager.GetAsInteger("KillCount1");

		pMsg.KillCount[1] = gQueryManager.GetAsInteger("KillCount2");

		pMsg.KillCount[2] = gQueryManager.GetAsInteger("KillCount3");

		pMsg.KillCount[3] = gQueryManager.GetAsInteger("KillCount4");

		pMsg.KillCount[4] = gQueryManager.GetAsInteger("KillCount5");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}

void CQuest::GDQuestKillCountSaveRecv(SDHP_QUEST_KILL_COUNT_SAVE_RECV* lpMsg) // OK
{
	if(gQueryManager.ExecQuery("SELECT Name FROM QuestKillCount WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO QuestKillCount (Name,QuestIndex,MonsterClass1,KillCount1,MonsterClass2,KillCount2,MonsterClass3,KillCount3,MonsterClass4,KillCount4,MonsterClass5,KillCount5) VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",lpMsg->name,lpMsg->QuestIndex,lpMsg->MonsterClass[0],lpMsg->KillCount[0],lpMsg->MonsterClass[1],lpMsg->KillCount[1],lpMsg->MonsterClass[2],lpMsg->KillCount[2],lpMsg->MonsterClass[3],lpMsg->KillCount[3],lpMsg->MonsterClass[4],lpMsg->KillCount[4]);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE QuestKillCount SET QuestIndex=%d,MonsterClass1=%d,KillCount1=%d,MonsterClass2=%d,KillCount2=%d,MonsterClass3=%d,KillCount3=%d,MonsterClass4=%d,KillCount4=%d,MonsterClass5=%d,KillCount5=%d WHERE Name='%s'",lpMsg->QuestIndex,lpMsg->MonsterClass[0],lpMsg->KillCount[0],lpMsg->MonsterClass[1],lpMsg->KillCount[1],lpMsg->MonsterClass[2],lpMsg->KillCount[2],lpMsg->MonsterClass[3],lpMsg->KillCount[3],lpMsg->MonsterClass[4],lpMsg->KillCount[4],lpMsg->name);
		gQueryManager.Close();
	}
}
