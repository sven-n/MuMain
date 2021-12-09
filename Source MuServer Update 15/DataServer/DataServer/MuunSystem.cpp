// MuunSystem.cpp: implementation of the CMuunSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MuunSystem.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMuunSystem gMuunSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMuunSystem::CMuunSystem() // OK
{

}

CMuunSystem::~CMuunSystem() // OK
{

}

void CMuunSystem::GDMuunInventoryRecv(SDHP_MUUN_INVENTORY_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=803)

	SDHP_MUUN_INVENTORY_SEND pMsg;

	pMsg.header.set(0x27,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	if(gQueryManager.ExecQuery("SELECT Items FROM MuunInventory WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		memset(pMsg.MuunInventory,0xFF,sizeof(pMsg.MuunInventory));
	}
	else
	{
		gQueryManager.GetAsBinary("Items",pMsg.MuunInventory[0],sizeof(pMsg.MuunInventory));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CMuunSystem::GDMuunInventorySaveRecv(SDHP_MUUN_INVENTORY_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=803)

	if(gQueryManager.ExecQuery("SELECT Name FROM MuunInventory WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->MuunInventory[0],sizeof(lpMsg->MuunInventory));
		gQueryManager.ExecQuery("INSERT INTO MuunInventory (Name,Items) VALUES ('%s',?)",lpMsg->name);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->MuunInventory[0],sizeof(lpMsg->MuunInventory));
		gQueryManager.ExecQuery("UPDATE MuunInventory SET Items=? WHERE Name='%s'",lpMsg->name);
		gQueryManager.Close();
	}

	#endif
}
