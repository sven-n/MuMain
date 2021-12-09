// Helper.cpp: implementation of the CHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Helper.h"
#include "QueryManager.h"
#include "SocketManager.h"

CHelper gHelper;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelper::CHelper() // OK
{

}

CHelper::~CHelper() // OK
{

}

void CHelper::GDHelperDataRecv(SDHP_HELPER_DATA_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=603)

	SDHP_HELPER_DATA_SEND pMsg;

	pMsg.header.set(0x17,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	if(gQueryManager.ExecQuery("SELECT Data FROM HelperData WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;

		memset(pMsg.data,0xFF,sizeof(pMsg.data));
	}
	else
	{
		pMsg.result = 0;

		gQueryManager.GetAsBinary("Data",pMsg.data,sizeof(pMsg.data));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CHelper::GDHelperDataSaveRecv(SDHP_HELPER_DATA_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=603)

	if(gQueryManager.ExecQuery("SELECT Name FROM HelperData WHERE Name='%s'",lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->data,sizeof(lpMsg->data));
		gQueryManager.ExecQuery("INSERT INTO HelperData (Name,Data) VALUES ('%s',?)",lpMsg->name);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1,lpMsg->data,sizeof(lpMsg->data));
		gQueryManager.ExecQuery("UPDATE HelperData SET Data=? WHERE Name='%s'",lpMsg->name);
		gQueryManager.Close();
	}

	#endif
}
