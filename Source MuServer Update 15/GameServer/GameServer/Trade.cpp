// Trade.cpp: implementation of the CTrade class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Trade.h"
#include "Duel.h"
#include "DSProtocol.h"
#include "EventHideAndSeek.h"
#include "EventPvP.h"
#include "EventRussianRoulette.h"
#include "EventInventory.h"
#include "ItemValueTrade.h"
#include "Log.h"
#include "Map.h"
#include "Message.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "PentagramSystem.h"
#include "ServerInfo.h"
#include "User.h"
#include "Util.h"
//MC
#include "BotBuffer.h"

CTrade gTrade;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrade::CTrade() // OK
{

}

CTrade::~CTrade() // OK
{

}

void CTrade::ClearTrade(LPOBJ lpObj) // OK
{
	for(int n=0;n < TRADE_SIZE;n++)
	{
		lpObj->Trade[n].Clear();
	}

	memset(lpObj->TradeMap,0xFF,TRADE_SIZE);
}

void CTrade::ResetTrade(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0)
	{
		return;
	}

	gObjInventoryRollback(aIndex);

	gItemManager.GCItemListSend(aIndex);

	#if(GAMESERVER_UPDATE>=802)

	gEventInventory.GCEventItemListSend(aIndex);

	#endif

	GCMoneySend(aIndex,lpObj->Money);

	gObjectManager.CharacterMakePreviewCharSet(aIndex);

	gItemManager.GCItemEquipmentSend(aIndex);

	this->ClearTrade(lpObj);

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->TargetNumber = -1;
	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = 0;
	lpObj->TradeMoney = 0;
}

bool CTrade::ExchangeTradeItem(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	for(int n=0;n < TRADE_SIZE;n++)
	{
		if(lpObj->Trade[n].IsItem() != 0)
		{
			if(lpObj->Trade[n].IsEventItem() == 0)
			{
				if((lpObj->Trade[n].m_Slot=gItemManager.InventoryInsertItem(lpTarget->Index,lpObj->Trade[n])) == 0xFF)
				{
					return 0;
				}
			}
			else
			{
				if((lpObj->Trade[n].m_Slot=gEventInventory.EventInventoryInsertItem(lpTarget->Index,lpObj->Trade[n])) == 0xFF)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}

void CTrade::ExchangeTradeItemLog(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	for(int n=0;n < TRADE_SIZE;n++)
	{
		if(lpObj->Trade[n].IsItem() != 0)
		{
			gLog.Output(LOG_TRADE,"[ExchangeTradeItem][%s][%s] - (Account: %s, Name: %s, Index: %04d, Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",lpObj->Account,lpObj->Name,lpTarget->Account,lpTarget->Name,lpObj->Trade[n].m_Index,lpObj->Trade[n].m_Level,lpObj->Trade[n].m_Serial,lpObj->Trade[n].m_Option1,lpObj->Trade[n].m_Option2,lpObj->Trade[n].m_Option3,lpObj->Trade[n].m_NewOption,lpObj->Trade[n].m_JewelOfHarmonyOption,lpObj->Trade[n].m_ItemOptionEx,lpObj->Trade[n].m_SocketOption[0],lpObj->Trade[n].m_SocketOption[1],lpObj->Trade[n].m_SocketOption[2],lpObj->Trade[n].m_SocketOption[3],lpObj->Trade[n].m_SocketOption[4]);
		}
	}
}

void CTrade::CGTradeRequestRecv(PMSG_TRADE_REQUEST_RECV* lpMsg,int aIndex) // OK
{
	if(gServerInfo.m_TradeSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use != 0 || lpTarget->Interface.use != 0)
	{
		return;
	}

	if(lpObj->TradeDuel != 0)
	{
		return;
	}

	if(lpTarget->TradeDuel != 0)
	{
		return;
	}

	if(CA_MAP_RANGE(lpTarget->Map) != 0 || DS_MAP_RANGE(lpTarget->Map) != 0 || BC_MAP_RANGE(lpTarget->Map) != 0 || CC_MAP_RANGE(lpTarget->Map) != 0 || IT_MAP_RANGE(lpTarget->Map) != 0 || DA_MAP_RANGE(lpTarget->Map) != 0 || DG_MAP_RANGE(lpTarget->Map) != 0 || IG_MAP_RANGE(lpTarget->Map) != 0)
	{
		this->GCTradeResponseSend(aIndex,0,lpTarget->Name,0,0);
		return;
	}

	if(lpObj->X < (lpTarget->X-2) || lpObj->X > (lpTarget->X+2) || lpObj->Y < (lpTarget->Y-2) || lpObj->Y > (lpTarget->Y+2))
	{
		this->GCTradeResponseSend(aIndex,0,lpTarget->Name,0,0);
		return;
	}

	if(lpObj->PShopOpen != 0 || lpTarget->PShopOpen != 0)
	{
		this->GCTradeResponseSend(aIndex,0,lpTarget->Name,0,0);
		return;
	}

	if(gServerInfo.m_PKDisableTrade == 1 && lpObj->PKLevel >= 5)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(861));
		return;
	}

	if(gServerInfo.m_PKDisableTrade == 1 && lpTarget->PKLevel >= 5)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(862));
		return;
	}

	if(gEventHideAndSeek.EventHideAndSeekTrade(aIndex,bIndex) == 1)
	{
		return;
	}

	if(gEventPvP.EventPvPTradeJoin(aIndex,bIndex) == 1)
	{
		return;
	}

	if(gEventRussianRoulette.EventRussianRouletteTrade(aIndex,bIndex) == 1)
	{
		return;
	}

	if((lpTarget->Option & 1) == 0)
	{
		this->GCTradeResponseSend(aIndex,0,lpTarget->Name,0,0);
		return;
	}

	//MC bot
	// SCF BOTS
	
	int	number = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if ( OBJMAX_RANGE(number) == FALSE )
	{
		LogAdd(LOG_BLACK,"Error : %s %d (%d)", __FILE__, __LINE__, number);
		return;
	}
	if ( number == aIndex )
	{
		return;
	}

	if(ObjBotBuff.TradeOpen(aIndex,number) == 1)
	{
		return;
	}

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_TRADE;
	lpObj->Interface.state = 0;
	lpObj->InterfaceTime = GetTickCount();
	lpObj->TargetNumber = bIndex;

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_TRADE;
	lpTarget->Interface.state = 0;
	lpTarget->InterfaceTime = GetTickCount();
	lpTarget->TargetNumber = aIndex;

	this->GCTradeRequestSend(bIndex,lpObj->Name);
}

void CTrade::CGTradeResponseRecv(PMSG_TRADE_RESPONSE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = lpObj->TargetNumber;

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state != 0)
	{
		return;
	}

	if(lpTarget->Interface.use == 0 || lpTarget->Interface.type != INTERFACE_TRADE || lpTarget->Interface.state != 0)
	{
		return;
	}

	if(CA_MAP_RANGE(lpTarget->Map) != 0 || DS_MAP_RANGE(lpTarget->Map) != 0 || BC_MAP_RANGE(lpTarget->Map) != 0 || CC_MAP_RANGE(lpTarget->Map) != 0 || IT_MAP_RANGE(lpTarget->Map) != 0 || DA_MAP_RANGE(lpTarget->Map) != 0 || DG_MAP_RANGE(lpTarget->Map) != 0 || IG_MAP_RANGE(lpTarget->Map) != 0)
	{
		goto CLEAR_JUMP;
	}

	if(lpObj->X < (lpTarget->X-2) || lpObj->X > (lpTarget->X+2) || lpObj->Y < (lpTarget->Y-2) || lpObj->Y > (lpTarget->Y+2))
	{
		goto CLEAR_JUMP;
	}

	if(lpObj->PShopOpen != 0 || lpTarget->PShopOpen != 0)
	{
		goto CLEAR_JUMP;
	}

	if(lpMsg->response == 0)
	{
		goto CLEAR_JUMP;
	}

	gObjFixInventoryPointer(aIndex);

	if(lpObj->Transaction == 1)
	{
		goto CLEAR_JUMP;
	}

	if(gObjInventoryTransaction(aIndex) == 0 || gObjInventoryTransaction(bIndex) == 0)
	{
		goto CLEAR_JUMP;
	}

	this->ClearTrade(lpObj);

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_TRADE;
	lpObj->Interface.state = 1;
	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = 0;
	lpObj->TradeMoney = 0;

	this->GCTradeResponseSend(aIndex,1,lpTarget->Name,lpTarget->Level,lpTarget->GuildNumber);

	this->ClearTrade(lpTarget);

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_TRADE;
	lpTarget->Interface.state = 1;
	lpTarget->TradeOk = 0;
	lpTarget->TradeOkTime = 0;
	lpTarget->TradeMoney = 0;

	this->GCTradeResponseSend(bIndex,1,lpObj->Name,lpObj->Level,lpObj->GuildNumber);

	return;

	CLEAR_JUMP:

	gObjInventoryCommit(aIndex);

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->TargetNumber = -1;

	this->GCTradeResponseSend(aIndex,0,lpTarget->Name,0,0);

	gObjInventoryCommit(bIndex);

	lpTarget->Interface.use = 0;
	lpTarget->Interface.type = INTERFACE_NONE;
	lpTarget->Interface.state = 0;
	lpTarget->TargetNumber = -1;

	this->GCTradeResponseSend(bIndex,0,lpObj->Name,0,0);
}

void CTrade::CGTradeMoneyRecv(PMSG_TRADE_MONEY_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = lpObj->TargetNumber;

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0)
	{
		return;
	}

	if(lpTarget->Interface.use == 0 || lpTarget->Interface.type != INTERFACE_TRADE || lpTarget->Interface.state == 0)
	{
		return;
	}

	if(lpMsg->money >= MAX_TRADE_MONEY || lpObj->Money < lpMsg->money)
	{
		return;
	}

	GCMoneySend(aIndex,(lpObj->Money-lpMsg->money));

	PMSG_TRADE_RESULT_SEND pMsg;

	pMsg.header.set(0x3A,sizeof(pMsg));

	pMsg.result = 1;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = GetTickCount();
	this->GCTradeOkButtonSend(aIndex,0);

	lpTarget->TradeOk = 0;
	lpTarget->TradeOkTime = GetTickCount();
	this->GCTradeOkButtonSend(bIndex,2);

	lpObj->TradeMoney = lpMsg->money;
	this->GCTradeMoneySend(bIndex,lpMsg->money);
}

void CTrade::CGTradeOkButtonRecv(PMSG_TRADE_OK_BUTTON_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];


	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = lpObj->TargetNumber;

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0)
	{
		return;
	}

	if(lpTarget->Interface.use == 0 || lpTarget->Interface.type != INTERFACE_TRADE || lpTarget->Interface.state == 0)
	{
		return;
	}

	if((GetTickCount()-lpObj->TradeOkTime) < 6000)
	{
		return;
	}

	if(lpMsg->flag == 0 && lpObj->TradeOk != 0)
	{
		lpObj->TradeOk = 0;
	}

	if(lpMsg->flag == 1 && lpObj->TradeOk == 0)
	{
		lpObj->TradeOk = 1;
	}

	this->GCTradeOkButtonSend(bIndex,lpObj->TradeOk);

	if(lpObj->TradeOk == 0 || lpTarget->TradeOk == 0)
	{
		return;
	}

	if (lpObj->TradeDuel == 1 && lpTarget->TradeDuel == 1)
	{
		this->GCTradeResultSend(aIndex,1);
		this->GCTradeResultSend(bIndex,1);

		gDuel.StartDuelBit(lpObj,lpTarget);
		return;
	}


	#if(GAMESERVER_UPDATE>=701)

	if(gPentagramSystem.CheckExchangePentagramItem(lpObj) == 0 || gPentagramSystem.CheckExchangePentagramItem(lpTarget) == 0)
	{
		this->ResetTrade(aIndex);
		this->GCTradeResultSend(aIndex,6);
		this->ResetTrade(bIndex);
		this->GCTradeResultSend(bIndex,6);
		return;
	}

	#endif

	if(gObjCheckMaxMoney(aIndex,lpTarget->TradeMoney) == 0 || gObjCheckMaxMoney(bIndex,lpObj->TradeMoney) == 0)
	{
		this->ResetTrade(aIndex);
		this->GCTradeResultSend(aIndex,5);
		this->ResetTrade(bIndex);
		this->GCTradeResultSend(bIndex,5);
		return;
	}

	if(this->ExchangeTradeItem(lpObj,lpTarget) == 0 || this->ExchangeTradeItem(lpTarget,lpObj) == 0)
	{
		this->ResetTrade(aIndex);
		this->GCTradeResultSend(aIndex,2);
		this->ResetTrade(bIndex);
		this->GCTradeResultSend(bIndex,2);
		return;
	}

	if(gItemValueTrade.CheckItemValueTrade(lpObj,lpTarget) == 0)
	{
		this->ResetTrade(aIndex);
		this->GCTradeResultSend(aIndex,2);
		this->ResetTrade(bIndex);
		this->GCTradeResultSend(bIndex,2);
		return;
	}

	#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.ExchangePentagramItem(lpObj,lpTarget);

	gPentagramSystem.ExchangePentagramItem(lpTarget,lpObj);

	#endif

	gObjInventoryCommit(aIndex);

	gItemManager.GCItemListSend(aIndex);

	#if(GAMESERVER_UPDATE>=802)

	gEventInventory.GCEventItemListSend(aIndex);

	#endif

	gObjectManager.CharacterMakePreviewCharSet(aIndex);

	lpObj->Money -= lpObj->TradeMoney;

	lpObj->Money += lpTarget->TradeMoney;

	GCMoneySend(aIndex,lpObj->Money);

	GDCharacterInfoSaveSend(aIndex);

	this->ExchangeTradeItemLog(lpObj,lpTarget);

	this->ClearTrade(lpObj);

	gObjInventoryCommit(bIndex);

	gItemManager.GCItemListSend(bIndex);

	#if(GAMESERVER_UPDATE>=802)

	gEventInventory.GCEventItemListSend(bIndex);

	#endif

	gObjectManager.CharacterMakePreviewCharSet(bIndex);

	lpTarget->Money -= lpTarget->TradeMoney;

	lpTarget->Money += lpObj->TradeMoney;

	GCMoneySend(bIndex,lpTarget->Money);

	GDCharacterInfoSaveSend(bIndex);

	this->ExchangeTradeItemLog(lpTarget,lpObj);

	this->ClearTrade(lpTarget);

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->TargetNumber = -1;
	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = 0;
	lpObj->TradeMoney = 0;

	this->GCTradeResultSend(aIndex,1);

	lpTarget->Interface.use = 0;
	lpTarget->Interface.type = INTERFACE_NONE;
	lpTarget->Interface.state = 0;
	lpTarget->TargetNumber = -1;
	lpTarget->TradeOk = 0;
	lpTarget->TradeOkTime = 0;
	lpTarget->TradeMoney = 0;

	this->GCTradeResultSend(bIndex,1);
}

void CTrade::CGTradeCancelButtonRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	int bIndex = lpObj->TargetNumber;

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0)
	{
		return;
	}

	if(lpTarget->Interface.use == 0 || lpTarget->Interface.type != INTERFACE_TRADE || lpTarget->Interface.state == 0)
	{
		return;
	}

	this->ResetTrade(aIndex);
	this->GCTradeResultSend(aIndex,0);

	this->ResetTrade(bIndex);
	this->GCTradeResultSend(bIndex,0);
}

void CTrade::GCTradeRequestSend(int aIndex,char* name) // OK
{
	PMSG_TRADE_REQUEST_SEND pMsg;

	pMsg.header.set(0x36,sizeof(pMsg));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CTrade::GCTradeResponseSend(int aIndex,BYTE response,char* name,int level,int GuildNumber) // OK
{
	PMSG_TRADE_RESPONSE_SEND pMsg;

	pMsg.header.set(0x37,sizeof(pMsg));

	pMsg.response = response;

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	pMsg.level = level;

	pMsg.GuildNumber = GuildNumber;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CTrade::GCTradeItemDelSend(int aIndex,BYTE slot) // OK
{
	PMSG_TRADE_ITEM_DEL_SEND pMsg;

	pMsg.header.set(0x38,sizeof(pMsg));

	pMsg.slot = slot;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CTrade::GCTradeItemAddSend(int aIndex,BYTE slot,BYTE* ItemInfo) // OK
{
	PMSG_TRADE_ITEM_ADD_SEND pMsg;

	pMsg.header.set(0x39,sizeof(pMsg));

	pMsg.slot = slot;

	memcpy(pMsg.ItemInfo,ItemInfo,sizeof(pMsg.ItemInfo));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CTrade::GCTradeMoneySend(int aIndex,DWORD money) // OK
{
	PMSG_TRADE_MONEY_SEND pMsg;

	pMsg.header.set(0x3B,sizeof(pMsg));

	pMsg.money = money;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);	
}

void CTrade::GCTradeOkButtonSend(int aIndex,BYTE flag) // OK
{
	PMSG_TRADE_OK_BUTTON_SEND pMsg;

	pMsg.header.set(0x3C,sizeof(pMsg));

	pMsg.flag = flag;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CTrade::GCTradeResultSend(int aIndex,BYTE result) // OK
{
	PMSG_TRADE_RESULT_SEND pMsg;

	pMsg.header.set(0x3D,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CTrade::TradeDuel(LPOBJ lpObj, LPOBJ lpTarget) // OK
{
	if(gObjInventoryTransaction(lpObj->Index) == 0 || gObjInventoryTransaction(lpTarget->Index) == 0)
	{
		gObjInventoryCommit(lpObj->Index);

		lpObj->Interface.use = 0;
		lpObj->Interface.type = INTERFACE_NONE;
		lpObj->Interface.state = 0;
		lpObj->TargetNumber = -1;
		lpObj->TradeDuel = 0;

		this->GCTradeResponseSend(lpObj->Index,0,lpTarget->Name,0,0);

		gObjInventoryCommit(lpTarget->Index);

		lpTarget->Interface.use = 0;
		lpTarget->Interface.type = INTERFACE_NONE;
		lpTarget->Interface.state = 0;
		lpTarget->TargetNumber = -1;
		lpTarget->TradeDuel = 0;

		this->GCTradeResponseSend(lpTarget->Index,0,lpObj->Name,0,0);
		return;
	}

	this->ClearTrade(lpObj);

	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_TRADE;
	lpObj->Interface.state = 1;
	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = 0;
	lpObj->TradeMoney = 0;
	lpObj->InterfaceTime = GetTickCount();
	lpObj->TargetNumber = lpTarget->Index;
	lpObj->TradeDuel = 1;

	this->GCTradeResponseSend(lpObj->Index,1,lpTarget->Name,lpTarget->Level,lpTarget->GuildNumber);

	this->ClearTrade(lpTarget);

	lpTarget->Interface.use = 1;
	lpTarget->Interface.type = INTERFACE_TRADE;
	lpTarget->Interface.state = 1;
	lpTarget->TradeOk = 0;
	lpTarget->TradeOkTime = 0;
	lpTarget->TradeMoney = 0;
	lpTarget->InterfaceTime = GetTickCount();
	lpTarget->TargetNumber = lpObj->Index;
	lpTarget->TradeDuel = 1;

	this->GCTradeResponseSend(lpTarget->Index,1,lpObj->Name,lpObj->Level,lpObj->GuildNumber);

}

void CTrade::TradeDuelEnd(LPOBJ lpObj, LPOBJ lpTarget)
{

	#if(GAMESERVER_UPDATE>=701)

	if(gPentagramSystem.CheckExchangePentagramItem(lpObj) == 0 || gPentagramSystem.CheckExchangePentagramItem(lpTarget) == 0)
	{
		this->ResetTrade(lpObj->Index);
		this->ResetTrade(lpTarget->Index);
		return;
	}

	#endif

	if(gObjCheckMaxMoney(lpObj->Index,lpTarget->TradeMoney) != 0)
	{
		lpObj->Money += lpTarget->TradeMoney;
		GCMoneySend(lpObj->Index,lpObj->Money);
	}

	for(int n=0;n < TRADE_SIZE;n++)
	{
		if(lpObj->Trade[n].IsItem() != 0)
		{
			if((lpObj->Trade[n].m_Slot=gItemManager.InventoryInsertItem(lpObj->Index,lpObj->Trade[n])) == 0xFF)
			{
				//return 0;
			}
		}
	}

	for(int n=0;n < TRADE_SIZE;n++)
	{
		if(lpTarget->Trade[n].IsItem() != 0)
		{
			if((lpTarget->Trade[n].m_Slot=gItemManager.InventoryInsertItem(lpObj->Index,lpTarget->Trade[n])) == 0xFF)
			{
				//return 0;
			}
		}
	}



	#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.ExchangePentagramItem(lpObj,lpTarget);

	gPentagramSystem.ExchangePentagramItem(lpTarget,lpObj);

	#endif

	gObjInventoryCommit(lpObj->Index);

	gItemManager.GCItemListSend(lpObj->Index);

	#if(GAMESERVER_UPDATE>=802)

	gEventInventory.GCEventItemListSend(lpObj->Index);

	#endif

	//gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);

	GDCharacterInfoSaveSend(lpObj->Index);

	this->ClearTrade(lpObj);

	gObjInventoryCommit(lpTarget->Index);

	gItemManager.GCItemListSend(lpTarget->Index);

	#if(GAMESERVER_UPDATE>=802)

	gEventInventory.GCEventItemListSend(lpTarget->Index);

	#endif

	//gObjectManager.CharacterMakePreviewCharSet(lpTarget->Index);

	GCMoneySend(lpTarget->Index,lpTarget->Money);

	GDCharacterInfoSaveSend(lpTarget->Index);

	this->ClearTrade(lpTarget);

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->TargetNumber = -1;
	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = 0;
	lpObj->TradeMoney = 0;
	lpTarget->TradeDuel = 0;

	lpTarget->Interface.use = 0;
	lpTarget->Interface.type = INTERFACE_NONE;
	lpTarget->Interface.state = 0;
	lpTarget->TargetNumber = -1;
	lpTarget->TradeOk = 0;
	lpTarget->TradeOkTime = 0;
	lpTarget->TradeMoney = 0;
	lpTarget->TradeDuel = 0;
}

void CTrade::ResetDuelTrade(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	gObjInventoryRollback(aIndex);

	gItemManager.GCItemListSend(aIndex);

	#if(GAMESERVER_UPDATE>=802)

	gEventInventory.GCEventItemListSend(aIndex);

	#endif

	GCMoneySend(aIndex,lpObj->Money);

	gObjectManager.CharacterMakePreviewCharSet(aIndex);

	gItemManager.GCItemEquipmentSend(aIndex);

	this->ClearTrade(lpObj);

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;
	lpObj->TargetNumber = -1;
	lpObj->TradeOk = 0;
	lpObj->TradeOkTime = 0;
	lpObj->TradeMoney = 0;
	lpObj->TradeDuel = 0;
}