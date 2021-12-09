// NpcTalk.cpp: implementation of the CNpcTalk class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NpcTalk.h"
#include "QueryManager.h"
#include "SocketManager.h"

CNpcTalk gNpcTalk;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNpcTalk::CNpcTalk() // OK
{

}

CNpcTalk::~CNpcTalk() // OK
{

}

void CNpcTalk::GDNpcLeoTheHelperRecv(SDHP_NPC_LEO_THE_HELPER_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=202)

	SDHP_NPC_LEO_THE_HELPER_SEND pMsg;

	pMsg.header.set(0x0E,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	if(gQueryManager.ExecQuery("SELECT Status FROM EventLeoTheHelper WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO EventLeoTheHelper (Name,Status) VALUES ('%s',0)",lpMsg->name);
		gQueryManager.Close();

		pMsg.status = 0;
	}
	else
	{
		pMsg.status = (BYTE)gQueryManager.GetAsInteger("Status");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::GDNpcSantaClausRecv(SDHP_NPC_SANTA_CLAUS_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=401)

	SDHP_NPC_SANTA_CLAUS_SEND pMsg;

	pMsg.header.set(0x0E,0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	if(gQueryManager.ExecQuery("SELECT Status FROM EventSantaClaus WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO EventSantaClaus (Name,Status) VALUES ('%s',0)",lpMsg->name);
		gQueryManager.Close();

		pMsg.status = 0;
	}
	else
	{
		pMsg.status = (BYTE)gQueryManager.GetAsInteger("Status");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CNpcTalk::GDNpcLeoTheHelperSaveRecv(SDHP_NPC_LEO_THE_HELPER_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=202)

	if(gQueryManager.ExecQuery("SELECT Name FROM EventLeoTheHelper WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO EventLeoTheHelper (Name,Status) VALUES ('%s',%d)",lpMsg->name,lpMsg->status);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE EventLeoTheHelper SET Status=%d WHERE Name='%s'",lpMsg->status,lpMsg->name);
		gQueryManager.Close();
	}

	#endif
}

void CNpcTalk::GDNpcSantaClausSaveRecv(SDHP_NPC_SANTA_CLAUS_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=401)

	if(gQueryManager.ExecQuery("SELECT Name FROM EventSantaClaus WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO EventSantaClaus (Name,Status) VALUES ('%s',%d)",lpMsg->name,lpMsg->status);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE EventSantaClaus SET Status=%d WHERE Name='%s'",lpMsg->status,lpMsg->name);
		gQueryManager.Close();
	}

	#endif
}
