// LuckyCoin.cpp: implementation of the CLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuckyCoin.h"
#include "QueryManager.h"
#include "SocketManager.h"

CLuckyCoin gLuckyCoin;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLuckyCoin::CLuckyCoin() // OK
{

}

CLuckyCoin::~CLuckyCoin() // OK
{

}

void CLuckyCoin::GDLuckyCoinCountRecv(SDHP_LUCKY_COIN_COUNT_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=402)

	SDHP_LUCKY_COIN_COUNT_SEND pMsg;

	pMsg.header.set(0x1A,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	if(gQueryManager.ExecQuery("SELECT * FROM LuckyCoin WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		if(gQueryManager.ExecQuery("INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',0)",lpMsg->account) == 0)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			gQueryManager.Close();

			pMsg.LuckyCoin = 0;
		}
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CLuckyCoin::GDLuckyCoinRegisterRecv(SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=402)

	SDHP_LUCKY_COIN_REGISTER_SEND pMsg;

	pMsg.header.set(0x1A,0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.slot = lpMsg->slot;

	if(gQueryManager.ExecQuery("SELECT * FROM LuckyCoin WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CLuckyCoin::GDLuckyCoinExchangeRecv(SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=402)

	SDHP_LUCKY_COIN_EXCHANGE_SEND pMsg;

	pMsg.header.set(0x1A,0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.TradeCoin = lpMsg->TradeCoin;

	if(gQueryManager.ExecQuery("SELECT * FROM LuckyCoin WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CLuckyCoin::GDLuckyCoinAddCountSaveRecv(SDHP_LUCKY_COIN_ADD_COUNT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=402)

	if(gQueryManager.ExecQuery("SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',%d)",lpMsg->account,lpMsg->AddLuckyCoin);

		gQueryManager.Close();
	}
	else
	{
		DWORD LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE LuckyCoin SET LuckyCoin=%d WHERE AccountID='%s'",(((LuckyCoin+lpMsg->AddLuckyCoin)>0x7FFFFFFF)?0x7FFFFFFF:(LuckyCoin+lpMsg->AddLuckyCoin)),lpMsg->account);

		gQueryManager.Close();
	}

	#endif
}

void CLuckyCoin::GDLuckyCoinSubCountSaveRecv(SDHP_LUCKY_COIN_SUB_COUNT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=402)

	if(gQueryManager.ExecQuery("SELECT LuckyCoin FROM LuckyCoin WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO LuckyCoin (AccountID,LuckyCoin) VALUES ('%s',%d)",lpMsg->account,0,0,0);

		gQueryManager.Close();
	}
	else
	{
		DWORD LuckyCoin = gQueryManager.GetAsInteger("LuckyCoin");

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE LuckyCoin SET LuckyCoin=%d WHERE AccountID='%s'",((lpMsg->SubLuckyCoin>LuckyCoin)?0:(LuckyCoin-lpMsg->SubLuckyCoin)),lpMsg->account);

		gQueryManager.Close();
	}

	#endif
}
