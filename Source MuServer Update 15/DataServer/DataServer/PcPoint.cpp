// PcPoint.cpp: implementation of the CPcPoint class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PcPoint.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPcPoint gPcPoint;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPcPoint::CPcPoint() // OK
{

}

CPcPoint::~CPcPoint() // OK
{

}

void CPcPoint::GDPcPointPointRecv(SDHP_PC_POINT_POINT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE<=402)

	SDHP_PC_POINT_POINT_SEND pMsg;

	pMsg.header.set(0x19,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	if(gQueryManager.ExecQuery("SELECT * FROM PcPointData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		if(gQueryManager.ExecQuery("INSERT INTO PcPointData (AccountID,PcPoint) VALUES ('%s',0)",lpMsg->account) == 0)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			gQueryManager.Close();

			pMsg.PcPoint = 0;
		}
	}
	else
	{
		pMsg.PcPoint = gQueryManager.GetAsInteger("PcPoint");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPcPoint::GDPcPointItemBuyRecv(SDHP_PC_POINT_ITEM_BUY_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE<=402)

	SDHP_PC_POINT_ITEM_BUY_SEND pMsg;

	pMsg.header.set(0x19,0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.slot = lpMsg->slot;

	if(gQueryManager.ExecQuery("SELECT * FROM PcPointData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.PcPoint = gQueryManager.GetAsInteger("PcPoint");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPcPoint::GDPcPointRecievePointRecv(SDHP_PC_POINT_RECIEVE_POINT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE<=402)

	SDHP_PC_POINT_RECIEVE_POINT_SEND pMsg;

	pMsg.header.set(0x19,0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.CallbackFunc = lpMsg->CallbackFunc;

	pMsg.CallbackArg1 = lpMsg->CallbackArg1;

	pMsg.CallbackArg2 = lpMsg->CallbackArg2;

	if(gQueryManager.ExecQuery("SELECT * FROM PcPointData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.PcPoint = 0;
	}
	else
	{
		pMsg.PcPoint = gQueryManager.GetAsInteger("PcPoint");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPcPoint::GDPcPointAddPointSaveRecv(SDHP_PC_POINT_ADD_POINT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE<=402)

	if(gQueryManager.ExecQuery("SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO PcPointData (AccountID,PcPoint) VALUES ('%s',%d)",lpMsg->account,((lpMsg->AddPcPoint>lpMsg->MaxPcPoint)?lpMsg->MaxPcPoint:lpMsg->AddPcPoint));

		gQueryManager.Close();
	}
	else
	{
		DWORD PcPoint = gQueryManager.GetAsInteger("PcPoint");

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE PcPointData SET PcPoint=%d WHERE AccountID='%s'",(((PcPoint+lpMsg->AddPcPoint)>lpMsg->MaxPcPoint)?lpMsg->MaxPcPoint:(PcPoint+lpMsg->AddPcPoint)),lpMsg->account);

		gQueryManager.Close();
	}

	#endif
}

void CPcPoint::GDPcPointSubPointSaveRecv(SDHP_PC_POINT_SUB_POINT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE<=402)

	if(gQueryManager.ExecQuery("SELECT PcPoint FROM PcPointData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO PcPointData (AccountID,PcPoint) VALUES ('%s',%d)",lpMsg->account,0,0,0);

		gQueryManager.Close();
	}
	else
	{
		DWORD PcPoint = gQueryManager.GetAsInteger("PcPoint");

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE PcPointData SET PcPoint=%d WHERE AccountID='%s'",((lpMsg->SubPcPoint>PcPoint)?0:(PcPoint-lpMsg->SubPcPoint)),lpMsg->account);

		gQueryManager.Close();
	}

	#endif
}
