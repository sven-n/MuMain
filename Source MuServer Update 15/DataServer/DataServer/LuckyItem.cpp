// LuckyItem.cpp: implementation of the CLuckyItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuckyItem.h"
#include "QueryManager.h"
#include "SocketManager.h"

CLuckyItem gLuckyItem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLuckyItem::CLuckyItem() // OK
{

}

CLuckyItem::~CLuckyItem() // OK
{

}

void CLuckyItem::GDLuckyItemRecv(SDHP_LUCKY_ITEM_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=602)

	BYTE send[4096];

	SDHP_LUCKY_ITEM_SEND pMsg;

	pMsg.header.set(0x22,0x00,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.count = 0;

	SDHP_LUCKY_ITEM2 info;

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_LUCKY_ITEM1* lpInfo = (SDHP_LUCKY_ITEM1*)(((BYTE*)lpMsg)+sizeof(SDHP_LUCKY_ITEM_RECV)+(sizeof(SDHP_LUCKY_ITEM1)*n));

		if(gQueryManager.ExecQuery("SELECT DurabilitySmall FROM LuckyItem WHERE ItemSerial=%d",lpInfo->serial) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO LuckyItem (ItemSerial,DurabilitySmall) VALUES (%d,%d)",lpInfo->serial,0);
			gQueryManager.Close();

			info.slot = lpInfo->slot;
			info.serial = lpInfo->serial;
			info.DurabilitySmall = 0;
		}
		else
		{
			info.slot = lpInfo->slot;
			info.serial = lpInfo->serial;
			info.DurabilitySmall = gQueryManager.GetAsInteger("DurabilitySmall");

			gQueryManager.Close();
		}

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CLuckyItem::GDLuckyItemSaveRecv(SDHP_LUCKY_ITEM_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=602)

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_LUCKY_ITEM_SAVE* lpInfo = (SDHP_LUCKY_ITEM_SAVE*)(((BYTE*)lpMsg)+sizeof(SDHP_LUCKY_ITEM_SAVE_RECV)+(sizeof(SDHP_LUCKY_ITEM_SAVE)*n));

		if(gQueryManager.ExecQuery("SELECT ItemSerial FROM LuckyItem WHERE ItemSerial=%d",lpInfo->serial) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO LuckyItem (ItemSerial,DurabilitySmall) VALUES (%d,%d)",lpInfo->serial,lpInfo->DurabilitySmall);
			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("UPDATE LuckyItem SET DurabilitySmall=%d WHERE ItemSerial=%d",lpInfo->DurabilitySmall,lpInfo->serial);
			gQueryManager.Close();
		}
	}

	#endif
}
