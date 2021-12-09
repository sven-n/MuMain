// LuckyCoin.cpp: implementation of the CLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuckyCoin.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "ItemBagManager.h"
#include "ItemManager.h"
#include "Util.h"

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

void CLuckyCoin::CGLuckyCoinCountRecv(PMSG_LUCKY_COIN_COUNT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(((lpObj->LuckyCoinTransaction[0]==0)?(lpObj->LuckyCoinTransaction[0]++):lpObj->LuckyCoinTransaction[0]) != 0)
	{
		return;
	}

	SDHP_LUCKY_COIN_COUNT_SEND pMsg;

	pMsg.header.set(0x1A,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CLuckyCoin::CGLuckyCoinRegisterRecv(PMSG_LUCKY_COIN_REGISTER_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(((lpObj->LuckyCoinTransaction[1]==0)?(lpObj->LuckyCoinTransaction[1]++):lpObj->LuckyCoinTransaction[1]) != 0)
	{
		return;
	}

	SDHP_LUCKY_COIN_REGISTER_SEND pMsg;

	pMsg.header.set(0x1A,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.slot = gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,100),-1);

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CLuckyCoin::CGLuckyCoinExchangeRecv(PMSG_LUCKY_COIN_EXCHANGE_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(((lpObj->LuckyCoinTransaction[1]==0)?(lpObj->LuckyCoinTransaction[1]++):lpObj->LuckyCoinTransaction[1]) != 0)
	{
		return;
	}

	SDHP_LUCKY_COIN_EXCHANGE_SEND pMsg;

	pMsg.header.set(0x1A,0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.TradeCoin = lpMsg->TradeCoin;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CLuckyCoin::DGLuckyCoinCountRecv(SDHP_LUCKY_COIN_COUNT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGLuckyCoinCountRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->LuckyCoinTransaction[0] = 0;

	PMSG_LUCKY_COIN_COUNT_SEND pMsg;

	pMsg.header.set(0xBF,0x0B,sizeof(pMsg));

	pMsg.LuckyCoin = lpMsg->LuckyCoin;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CLuckyCoin::DGLuckyCoinRegisterRecv(SDHP_LUCKY_COIN_REGISTER_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGLuckyCoinRegisterRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->LuckyCoinTransaction[1] = 0;

	PMSG_LUCKY_COIN_REGISTER_SEND pMsg;

	pMsg.header.set(0xBF,0x0C,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.LuckyCoin = 0;

	if(lpMsg->result != 0)
	{
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpMsg->slot == 0xFF)
	{
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.result = 1;

	pMsg.LuckyCoin = lpMsg->LuckyCoin+1;

	gItemManager.DecreaseItemDur(lpObj,lpMsg->slot,1);

	this->GDLuckyCoinAddCountSaveSend(lpObj->Index,1);

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CLuckyCoin::DGLuckyCoinExchangeRecv(SDHP_LUCKY_COIN_EXCHANGE_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGLuckyCoinExchangeRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->LuckyCoinTransaction[1] = 0;

	PMSG_LUCKY_COIN_EXCHANGE_SEND pMsg;

	pMsg.header.set(0xBF,0x0D,sizeof(pMsg));

	pMsg.result = 1;

	if(lpMsg->result != 0)
	{
		pMsg.result = 0;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpMsg->LuckyCoin < lpMsg->TradeCoin)
	{
		pMsg.result = 0;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gItemManager.CheckItemInventorySpace(lpObj,2,4) == 0)
	{
		pMsg.result = 2;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CItem item;

	switch(lpMsg->TradeCoin)
	{
		case 10:
			if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_LUCKY_COIN1,lpObj,&item) == 0)
			{
				pMsg.result = 4;
				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				return;
			}
			break;
		case 20:
			if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_LUCKY_COIN2,lpObj,&item) == 0)
			{
				pMsg.result = 4;
				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				return;
			}
			break;
		case 30:
			if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_LUCKY_COIN3,lpObj,&item) == 0)
			{
				pMsg.result = 4;
				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				return;
			}
			break;
		default:
			pMsg.result = 4;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return;
	}

	GDCreateItemSend(lpObj->Index,0xEB,0,0,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,-1,item.m_NewOption,0,0,0,item.m_SocketOption,item.m_SocketOptionBonus,0);

	this->GDLuckyCoinSubCountSaveSend(lpObj->Index,lpMsg->TradeCoin);

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CLuckyCoin::GDLuckyCoinAddCountSaveSend(int aIndex,DWORD AddLuckyCoin) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	SDHP_LUCKY_COIN_ADD_COUNT_SAVE_SEND pMsg;

	pMsg.header.set(0x1A,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.AddLuckyCoin = AddLuckyCoin;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CLuckyCoin::GDLuckyCoinSubCountSaveSend(int aIndex,DWORD SubLuckyCoin) // OK
{
	#if(GAMESERVER_UPDATE>=402)

	SDHP_LUCKY_COIN_SUB_COUNT_SAVE_SEND pMsg;

	pMsg.header.set(0x1A,0x31,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.SubLuckyCoin = SubLuckyCoin;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}
