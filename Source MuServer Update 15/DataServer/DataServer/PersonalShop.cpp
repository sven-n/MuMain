// PersonalShop.cpp: implementation of the CPersonalShop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PersonalShop.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPersonalShop gPersonalShop;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPersonalShop::CPersonalShop() // OK
{

}

CPersonalShop::~CPersonalShop() // OK
{

}

void CPersonalShop::GDPShopItemValueRecv(SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=802)

	BYTE send[1024];

	SDHP_PSHOP_ITEM_VALUE_SEND pMsg;

	pMsg.header.set(0x25,0x00,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.count = 0;

	SDHP_PSHOP_ITEM_VALUE info;

	if(gQueryManager.ExecQuery("SELECT * FROM PShopItemValue WHERE Name='%s'",lpMsg->name) != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			info.slot = gQueryManager.GetAsInteger("Slot");
			info.serial = gQueryManager.GetAsInteger("Serial");
			info.value = gQueryManager.GetAsInteger("Value");
			info.JoBValue = gQueryManager.GetAsInteger("JoBValue");
			info.JoSValue = gQueryManager.GetAsInteger("JoSValue");
			info.JoCValue = gQueryManager.GetAsInteger("JoCValue");

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	gQueryManager.Close();

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CPersonalShop::GDPShopItemValueSaveRecv(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_PSHOP_ITEM_VALUE_SAVE* lpInfo = (SDHP_PSHOP_ITEM_VALUE_SAVE*)(((BYTE*)lpMsg)+sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV)+(sizeof(SDHP_PSHOP_ITEM_VALUE_SAVE)*n));

		if(gQueryManager.ExecQuery("SELECT Name FROM PShopItemValue WHERE Name='%s' AND Slot=%d",lpMsg->name,lpInfo->slot) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO PShopItemValue (Name,Slot,Serial,Value,JoBValue,JoSValue,JoCValue) VALUES ('%s',%d,%d,%d,%d,%d,%d)",lpMsg->name,lpInfo->slot,lpInfo->serial,lpInfo->value,lpInfo->JoBValue,lpInfo->JoSValue,lpInfo->JoCValue);
			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("UPDATE PShopItemValue SET Serial=%d,Value=%d,JoBValue=%d,JoSValue=%d,JoCValue=%d WHERE Name='%s' AND Slot=%d",lpInfo->serial,lpInfo->value,lpInfo->JoBValue,lpInfo->JoSValue,lpInfo->JoCValue,lpMsg->name,lpInfo->slot);
			gQueryManager.Close();
		}
	}

	#endif
}

void CPersonalShop::GDPShopItemValueInsertSaveRecv(SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	if(gQueryManager.ExecQuery("SELECT Name FROM PShopItemValue WHERE Name='%s' AND Slot=%d",lpMsg->name,lpMsg->slot) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO PShopItemValue (Name,Slot,Serial,Value,JoBValue,JoSValue,JoCValue) VALUES ('%s',%d,%d,%d,%d,%d,%d)",lpMsg->name,lpMsg->slot,lpMsg->serial,lpMsg->value,lpMsg->JoBValue,lpMsg->JoSValue,lpMsg->JoCValue);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE PShopItemValue SET Serial=%d,Value=%d,JoBValue=%d,JoSValue=%d,JoCValue=%d WHERE Name='%s' AND Slot=%d",lpMsg->serial,lpMsg->value,lpMsg->JoBValue,lpMsg->JoSValue,lpMsg->JoCValue,lpMsg->name,lpMsg->slot);
		gQueryManager.Close();
	}

	#endif
}

void CPersonalShop::GDPShopItemValueDeleteSaveRecv(SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	gQueryManager.ExecQuery("DELETE FROM PShopItemValue WHERE Name='%s' AND Slot=%d",lpMsg->name,lpMsg->slot);
	gQueryManager.Close();

	#endif
}
