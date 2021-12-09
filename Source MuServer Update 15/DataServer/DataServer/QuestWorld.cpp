// QuestWorld.cpp: implementation of the CQuestWorld class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QuestWorld.h"
#include "QueryManager.h"
#include "SocketManager.h"

CQuestWorld gQuestWorld;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuestWorld::CQuestWorld() // OK
{

}

CQuestWorld::~CQuestWorld() // OK
{

}

void CQuestWorld::GDQuestWorldRecv(SDHP_QUEST_WORLD_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=501)

	SDHP_QUEST_WORLD_SEND pMsg;

	pMsg.header.set(0x10,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	if(gQueryManager.ExecQuery("SELECT QuestWorldList FROM QuestWorld WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		memset(pMsg.QuestWorldList,0xFF,sizeof(pMsg.QuestWorldList));
	}
	else
	{
		gQueryManager.GetAsBinary("QuestWorldList",pMsg.QuestWorldList[0],sizeof(pMsg.QuestWorldList));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CQuestWorld::GDQuestWorldSaveRecv(SDHP_QUEST_WORLD_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=501)

	if(gQueryManager.ExecQuery("SELECT Name FROM QuestWorld WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->QuestWorldList[0],sizeof(lpMsg->QuestWorldList));
		gQueryManager.ExecQuery("INSERT INTO QuestWorld (Name,QuestWorldList) VALUES ('%s',?)",lpMsg->name);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->QuestWorldList[0],sizeof(lpMsg->QuestWorldList));
		gQueryManager.ExecQuery("UPDATE QuestWorld SET QuestWorldList=? WHERE Name='%s'",lpMsg->name);
		gQueryManager.Close();
	}

	#endif
}
