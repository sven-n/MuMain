// PersonalShop.cpp: implementation of the CPersonalShop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PersonalShop.h"
#include "CustomStore.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Log.h"
#include "Map.h"
#include "PentagramSystem.h"
#include "ServerInfo.h"
#include "User.h"
#include "Util.h"
#include "Viewport.h"

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

bool CPersonalShop::CheckPersonalShop(int aIndex) // OK
{
	for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
	{
		if(gObj[aIndex].Inventory[n].IsItem() != 0)
		{
			return 0;
		}
	}

	return 1;
}

bool CPersonalShop::CheckPersonalShopOpen(int aIndex) // OK
{
	int count = 0;

	for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
	{
		if(gObj[aIndex].Inventory[n].IsItem() != 0)
		{
			#if(GAMESERVER_UPDATE>=802)
			if(gObj[aIndex].Inventory[n].m_PShopValue <= 0 && gObj[aIndex].Inventory[n].m_PShopJoBValue <= 0 && gObj[aIndex].Inventory[n].m_PShopJoSValue <= 0 && gObj[aIndex].Inventory[n].m_PShopJoCValue <= 0)
			#else
			if(gObj[aIndex].Inventory[n].m_PShopValue <= 0)
			#endif
			{
				return 0;
			}
			else
			{
				count++;
			}
		}
	}

	return ((count==0)?0:1);
}

bool CPersonalShop::CheckPersonalShopViewport(int aIndex,int bIndex) // OK
{
	for(int n=0;n < gObj[aIndex].VpPShopPlayerCount;n++)
	{
		if(gObj[aIndex].VpPShopPlayer[n] == bIndex)
		{
			return 0;
		}
	}

	return 1;
}

bool CPersonalShop::CheckPersonalShopSearchItem(int aIndex,int ItemIndex) // OK
{
	for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
	{
		if(gObj[aIndex].Inventory[n].IsItem() != 0)
		{
			if(gObj[aIndex].Inventory[n].m_Index == ItemIndex)
			{
				return 1;
			}
		}
	}

	return 0;
}

void CPersonalShop::GetRequireJewelCount(LPOBJ lpObj,int* count,int* table,int type,int value) // OK
{
	int require[4] = {0,0,0,0};

	switch(type)
	{
		case 0:
			require[0] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(14,13),0);
			require[1] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,30),0);
			require[2] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,30),1);
			require[3] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,30),2);
			break;
		case 1:
			require[0] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(14,14),0);
			require[1] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,31),0);
			require[2] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,31),1);
			require[3] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,31),2);
			break;
		case 2:
			require[0] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,15),0);
			require[1] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,141),0);
			require[2] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,141),1);
			require[3] = gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(12,141),2);
			break;
	}

	(*count) = require[0]+(require[1]*10)+(require[2]*20)+(require[3]*30);

	if(require[3] > 0)
	{
		require[3] = (((value/30)>require[3])?require[3]:(value/30));

		value -= require[3]*30;
	}

	if(require[2] > 0)
	{
		require[2] = (((value/20)>require[2])?require[2]:(value/20));

		value -= require[2]*20;
	}

	if(require[1] > 0)
	{
		require[1] = (((value/10)>require[1])?require[1]:(value/10));

		value -= require[1]*10;
	}

	if(require[0] > 0)
	{
		require[0] = ((value>require[0])?require[0]:value);

		value -= require[0];
	}

	if(value == 0)
	{
		table[0] = require[0];
		table[1] = require[1];
		table[2] = require[2];
		table[3] = require[3];
	}
}

void CPersonalShop::GetPaymentJewelCount(LPOBJ lpObj,int* count,int* table,int type,int value) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	value = (int)GetRoundValue((((float)value*(100-gServerInfo.m_PersonalShopJewelCommisionRate))/100));

	#endif

	table[3] = value/30;
	value = value%30;

	table[2] = value/20;
	value = value%20;

	table[1] = value/10;
	value = value%10;

	table[0] = value;
	value = 0;

	(*count) += table[0]+table[1]+table[2]+table[3];
}

void CPersonalShop::SetRequireJewelCount(LPOBJ lpObj,int* table,int type) // OK
{
	switch(type)
	{
		case 0:
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(14,13),0,table[0]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,30),0,table[1]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,30),1,table[2]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,30),2,table[3]);
			break;
		case 1:
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(14,14),0,table[0]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,31),0,table[1]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,31),1,table[2]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,31),2,table[3]);
			break;
		case 2:
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,15),0,table[0]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,141),0,table[1]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,141),1,table[2]);
			gItemManager.DeleteInventoryItemCount(lpObj,GET_ITEM(12,141),2,table[3]);
			break;
	}
}

void CPersonalShop::SetPaymentJewelCount(LPOBJ lpObj,int* table,int type) // OK
{
	for(int n=0;n < table[0];n++)
	{
		switch(type)
		{
			case 0:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(14,13),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 1:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(14,14),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 2:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,15),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
		}
	}

	for(int n=0;n < table[1];n++)
	{
		switch(type)
		{
			case 0:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,30),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 1:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,31),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 2:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,141),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
		}
	}

	for(int n=0;n < table[2];n++)
	{
		switch(type)
		{
			case 0:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,30),1,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 1:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,31),1,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 2:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,141),1,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
		}
	}

	for(int n=0;n < table[3];n++)
	{
		switch(type)
		{
			case 0:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,30),2,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 1:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,31),2,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
			case 2:
				GDCreateItemSend(lpObj->Index,0xEB,0,0,GET_ITEM(12,141),2,0,0,0,0,-1,0,0,0,0,0,0xFF,0);
				break;
		}
	}
}

void CPersonalShop::CGPShopSetItemPriceRecv(PMSG_PSHOP_SET_ITEM_PRICE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(INVENTORY_RANGE(lpMsg->slot) == 0)
	{
		this->GCPShopSetItemPriceSend(aIndex,2,lpMsg->slot);
		return;
	}

	if(lpObj->Inventory[lpMsg->slot].IsItem() == 0)
	{
		this->GCPShopSetItemPriceSend(aIndex,3,lpMsg->slot);
		return;
	}

	int price = MAKE_NUMBERDW(MAKE_NUMBERW(lpMsg->price[3],lpMsg->price[2]),MAKE_NUMBERW(lpMsg->price[1],lpMsg->price[0]));

	#if(GAMESERVER_UPDATE>=802)

	int JoBPrice = MAKE_NUMBERW(lpMsg->JoBPrice[1],lpMsg->JoBPrice[0]);

	int JoSPrice = MAKE_NUMBERW(lpMsg->JoSPrice[1],lpMsg->JoSPrice[0]);

	int JoCPrice = MAKE_NUMBERW(lpMsg->JoCPrice[1],lpMsg->JoCPrice[0]);

	#endif

	#if(GAMESERVER_UPDATE>=802)

	if(price <= 0 && JoBPrice <= 0 && JoSPrice <= 0 && JoCPrice <= 0)
	{
		this->GCPShopSetItemPriceSend(aIndex,4,lpMsg->slot);
		return;
	}

	#else

	if(price <= 0)
	{
		this->GCPShopSetItemPriceSend(aIndex,4,lpMsg->slot);
		return;
	}

	#endif

	if(lpObj->Level <= 5)
	{
		this->GCPShopSetItemPriceSend(aIndex,5,lpMsg->slot);
		return;
	}

	lpObj->Inventory[lpMsg->slot].m_PShopValue = price;

	#if(GAMESERVER_UPDATE>=802)

	lpObj->Inventory[lpMsg->slot].m_PShopJoBValue = JoBPrice;

	lpObj->Inventory[lpMsg->slot].m_PShopJoSValue = JoSPrice;

	lpObj->Inventory[lpMsg->slot].m_PShopJoCValue = JoCPrice;

	#endif

	this->GCPShopSetItemPriceSend(aIndex,1,lpMsg->slot);
}

void CPersonalShop::CGPShopOpenRecv(PMSG_PSHOP_OPEN_RECV* lpMsg,int aIndex) // OK
{
	if(gServerInfo.m_PersonalShopSwitch == 0)
	{
		return;
	}

	if(gCustomStore.OnPShopOpen(&gObj[aIndex]) != 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_PERSONAL_SHOP)
	{
		return;
	}

	if(lpObj->TradeDuel != 0)
	{
		return;
	}

	if(CA_MAP_RANGE(lpObj->Map) != 0 || CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0)
	{
		this->GCPShopOpenSend(aIndex,0);
		return;
	}

	if(this->CheckPersonalShopOpen(aIndex) == 0)
	{
		this->GCPShopOpenSend(aIndex,0);
		return;
	}

	if(lpObj->Level <= 5)
	{
		this->GCPShopOpenSend(aIndex,2);
		return;
	}

	if(lpObj->PShopOpen == 0)
	{
		lpObj->PShopOpen = 1;
		memcpy(lpObj->PShopText,lpMsg->text,sizeof(lpMsg->text));
	}
	else
	{
		memcpy(lpObj->PShopText,lpMsg->text,sizeof(lpMsg->text));
		this->GCPShopTextChangeSend(aIndex);
	}

	this->GCPShopOpenSend(aIndex,1);
}

void CPersonalShop::CGPShopCloseRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->PShopOpen == 0)
	{
		return;
	}

	//lpObj->PShopOpen = 0;
	//memset(lpObj->PShopText,0,sizeof(lpObj->PShopText));
	//this->GCPShopCloseSend(aIndex,1);
	gCustomStore.OnPShopClose(lpObj);
}

void CPersonalShop::CGPShopItemListRecv(PMSG_PSHOP_ITEM_LIST_RECV* lpMsg,int aIndex) // OK
{
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

	if(lpTarget->PShopOpen == 0)
	{
		this->GCPShopItemListSend(aIndex,-1,3,0);
		return;
	}

	char name[11] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));

	if(strcmp(name,lpTarget->Name) != 0)
	{
		this->GCPShopItemListSend(aIndex,-1,2,0);
		return;
	}

	gCustomStore.GCOffTradeSend(lpObj->Index,lpTarget->Index);

	lpObj->PShopWantDeal = 1;
	lpObj->PShopDealerIndex = bIndex;
	memcpy(lpObj->PShopDealerName,lpMsg->name,sizeof(lpObj->PShopDealerName));
	this->GCPShopItemListSend(aIndex,bIndex,1,0);

	gCustomStore.OnPShopItemList(lpObj,lpTarget);

	
}

void CPersonalShop::CGPShopBuyItemRecv(PMSG_PSHOP_BUY_ITEM_RECV* lpMsg,int aIndex) // OK
{
	if(gCustomStore.OnPShopBuyItemRecv(lpMsg,aIndex) != 0)
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

	if(lpTarget->PShopOpen == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,3);
		return;
	}

	if(lpTarget->PShopTransaction != 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,4);
		return;
	}

	if(INVENTORY_SHOP_RANGE(lpMsg->slot) == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,5);
		return;
	}

	gObjFixInventoryPointer(aIndex);

	if(lpObj->Transaction == 1)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return;
	}

	char name[11] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));
	
	if(strcmp(name,lpTarget->Name) != 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return;
	}

	if(lpTarget->Inventory[lpMsg->slot].IsItem() == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return;
	}

	#if(GAMESERVER_UPDATE>=802)

	if(lpTarget->Inventory[lpMsg->slot].m_PShopValue <= 0 && lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue <= 0 && lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue <= 0 && lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue <= 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return;
	}

	#else

	if(lpTarget->Inventory[lpMsg->slot].m_PShopValue <= 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return;
	}

	#endif

	if(lpObj->Money < ((DWORD)lpTarget->Inventory[lpMsg->slot].m_PShopValue))
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return;
	}

	if(gObjCheckMaxMoney(bIndex,lpTarget->Inventory[lpMsg->slot].m_PShopValue) == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,8);
		return;
	}

	#if(GAMESERVER_UPDATE>=701)

	if(gPentagramSystem.CheckExchangePentagramItem(lpTarget,&lpTarget->Inventory[lpMsg->slot]) == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,8);
		return;
	}

	#endif

	#if(GAMESERVER_UPDATE>=802)

	int RequireJewelCount[3] = {0};

	int PaymentJewelCount[3] = {0};

	int RequireJewelTable[3][4] = {0};

	int PaymentJewelTable[3][4] = {0};

	this->GetRequireJewelCount(lpObj,&RequireJewelCount[0],RequireJewelTable[0],0,lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue);

	this->GetRequireJewelCount(lpObj,&RequireJewelCount[1],RequireJewelTable[1],1,lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue);

	this->GetRequireJewelCount(lpObj,&RequireJewelCount[2],RequireJewelTable[2],2,lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue);

	this->GetPaymentJewelCount(lpTarget,&PaymentJewelCount[0],PaymentJewelTable[0],0,lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue);

	this->GetPaymentJewelCount(lpTarget,&PaymentJewelCount[1],PaymentJewelTable[1],1,lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue);

	this->GetPaymentJewelCount(lpTarget,&PaymentJewelCount[2],PaymentJewelTable[2],2,lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue);

	if(RequireJewelCount[0] < lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,11);
		return;
	}

	if(RequireJewelCount[1] < lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,12);
		return;
	}

	if(RequireJewelCount[2] < lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,13);
		return;
	}

	if(lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue > 0 && RequireJewelTable[0][0] == 0 && RequireJewelTable[0][1] == 0 && RequireJewelTable[0][2] == 0 && RequireJewelTable[0][3] == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,14);
		return;
	}

	if(lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue > 0 && RequireJewelTable[1][0] == 0 && RequireJewelTable[1][1] == 0 && RequireJewelTable[1][2] == 0 && RequireJewelTable[1][3] == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,15);
		return;
	}

	if(lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue > 0 && RequireJewelTable[2][0] == 0 && RequireJewelTable[2][1] == 0 && RequireJewelTable[2][2] == 0 && RequireJewelTable[2][3] == 0)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,16);
		return;
	}

	if(gItemManager.GetInventoryEmptySlotCount(lpTarget) < (PaymentJewelCount[0]+PaymentJewelCount[1]+PaymentJewelCount[2]))
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,17);
		return;
	}

	if(lpTarget->Inventory[lpMsg->slot].m_Index != lpMsg->ItemIndex || lpTarget->Inventory[lpMsg->slot].m_PShopValue != lpMsg->value || lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue != lpMsg->JoBValue || lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue != lpMsg->JoSValue || lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue != lpMsg->JoCValue)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,18);
		return;
	}

	#endif

	lpTarget->PShopTransaction = 1;

	BYTE result = gItemManager.InventoryInsertItem(aIndex,lpTarget->Inventory[lpMsg->slot]);

	if(result == 0xFF)
	{
		this->GCPShopBuyItemSend(aIndex,bIndex,0,8);
		return;
	}

	#if(GAMESERVER_UPDATE>=802)

	gLog.Output(LOG_TRADE,"[SellPesonalShopItem][%s][%s] - (Account: %s, Name: %s, Value: %d, JoBValue: %d, JoSValue: %d, JoCValue: %d, Index: %04d, Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",lpTarget->Account,lpTarget->Name,lpObj->Account,lpObj->Name,lpTarget->Inventory[lpMsg->slot].m_PShopValue,lpTarget->Inventory[lpMsg->slot].m_PShopJoBValue,lpTarget->Inventory[lpMsg->slot].m_PShopJoSValue,lpTarget->Inventory[lpMsg->slot].m_PShopJoCValue,lpTarget->Inventory[lpMsg->slot].m_Index,lpTarget->Inventory[lpMsg->slot].m_Level,lpTarget->Inventory[lpMsg->slot].m_Serial,lpTarget->Inventory[lpMsg->slot].m_Option1,lpTarget->Inventory[lpMsg->slot].m_Option2,lpTarget->Inventory[lpMsg->slot].m_Option3,lpTarget->Inventory[lpMsg->slot].m_NewOption,lpTarget->Inventory[lpMsg->slot].m_JewelOfHarmonyOption,lpTarget->Inventory[lpMsg->slot].m_ItemOptionEx,lpTarget->Inventory[lpMsg->slot].m_SocketOption[0],lpTarget->Inventory[lpMsg->slot].m_SocketOption[1],lpTarget->Inventory[lpMsg->slot].m_SocketOption[2],lpTarget->Inventory[lpMsg->slot].m_SocketOption[3],lpTarget->Inventory[lpMsg->slot].m_SocketOption[4]);

	#else

	gLog.Output(LOG_TRADE,"[SellPesonalShopItem][%s][%s] - (Account: %s, Name: %s, Value: %d, Index: %04d, Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",lpTarget->Account,lpTarget->Name,lpObj->Account,lpObj->Name,lpTarget->Inventory[lpMsg->slot].m_PShopValue,lpTarget->Inventory[lpMsg->slot].m_Index,lpTarget->Inventory[lpMsg->slot].m_Level,lpTarget->Inventory[lpMsg->slot].m_Serial,lpTarget->Inventory[lpMsg->slot].m_Option1,lpTarget->Inventory[lpMsg->slot].m_Option2,lpTarget->Inventory[lpMsg->slot].m_Option3,lpTarget->Inventory[lpMsg->slot].m_NewOption,lpTarget->Inventory[lpMsg->slot].m_JewelOfHarmonyOption,lpTarget->Inventory[lpMsg->slot].m_ItemOptionEx,lpTarget->Inventory[lpMsg->slot].m_SocketOption[0],lpTarget->Inventory[lpMsg->slot].m_SocketOption[1],lpTarget->Inventory[lpMsg->slot].m_SocketOption[2],lpTarget->Inventory[lpMsg->slot].m_SocketOption[3],lpTarget->Inventory[lpMsg->slot].m_SocketOption[4]);

	#endif

	lpObj->Money -= lpTarget->Inventory[lpMsg->slot].m_PShopValue;

	GCMoneySend(aIndex,lpObj->Money);

	#if(GAMESERVER_UPDATE>=802)

	this->SetRequireJewelCount(lpObj,RequireJewelTable[0],0);

	this->SetRequireJewelCount(lpObj,RequireJewelTable[1],1);

	this->SetRequireJewelCount(lpObj,RequireJewelTable[2],2);

	#endif

	#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.ExchangePentagramItem(lpTarget,lpObj,&lpObj->Inventory[result]);

	#endif

	this->GCPShopBuyItemSend(aIndex,bIndex,result,1);

	GDCharacterInfoSaveSend(aIndex);

	#if(GAMESERVER_UPDATE>=802)

	lpTarget->Money += (int)GetRoundValue((((float)lpTarget->Inventory[lpMsg->slot].m_PShopValue*(100-gServerInfo.m_PersonalShopMoneyCommisionRate))/100));

	#else

	lpTarget->Money += lpTarget->Inventory[lpMsg->slot].m_PShopValue;

	#endif

	GCMoneySend(bIndex,lpTarget->Money);

	#if(GAMESERVER_UPDATE>=802)

	this->SetPaymentJewelCount(lpTarget,PaymentJewelTable[0],0);

	this->SetPaymentJewelCount(lpTarget,PaymentJewelTable[1],1);

	this->SetPaymentJewelCount(lpTarget,PaymentJewelTable[2],2);

	this->GDPShopItemValueDeleteSaveSend(bIndex,lpMsg->slot);

	#endif

	this->GCPShopSellItemSend(bIndex,aIndex,lpMsg->slot);

	gItemManager.InventoryDelItem(bIndex,lpMsg->slot);
	gItemManager.GCItemDeleteSend(bIndex,lpMsg->slot,1);

	GDCharacterInfoSaveSend(bIndex);

	if(this->CheckPersonalShop(bIndex) == 0)
	{
		lpTarget->PShopItemChange = 1;
	}
	else
	{
		//lpTarget->PShopOpen = 0;
		//memset(lpTarget->PShopText,0,sizeof(lpTarget->PShopText));
		//this->GCPShopCloseSend(bIndex,1);
		gCustomStore.OnPShopClose(lpTarget);
	}

	lpTarget->PShopTransaction = 0;
}

void CPersonalShop::CGPShopLeaveRecv(PMSG_PSHOP_LEAVE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	lpObj->PShopWantDeal = 0;

	lpObj->PShopDealerIndex = -1;

	memset(lpObj->PShopDealerName,0,sizeof(lpObj->PShopDealerName));
}

void CPersonalShop::CGPShopSearchRecv(PMSG_PSHOP_SEARCH_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	BYTE send[4096];

	PMSG_PSHOP_SEARCH_SEND pMsg;

	pMsg.header.set(0xEC,0x31,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.flag = 0;

	PMSG_PSHOP_SEARCH info;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		LPOBJ lpTarget = &gObj[n];

		if(lpTarget->PShopOpen == 0)
		{
			continue;
		}

		if(lpMsg->ItemIndex != 0xFFFF && this->CheckPersonalShopSearchItem(lpTarget->Index,lpMsg->ItemIndex) == 0)
		{
			continue;
		}

		if((pMsg.count++) >= lpMsg->count)
		{
			info.index[0] = SET_NUMBERHB(lpTarget->Index);

			info.index[1] = SET_NUMBERLB(lpTarget->Index);

			memcpy(info.name,lpTarget->Name,sizeof(lpTarget->Name));

			memcpy(info.text,lpTarget->PShopText,sizeof(lpTarget->PShopText));

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			if((pMsg.count-lpMsg->count) >= 50)
			{
				pMsg.flag = 1;
				break;
			}
		}
	}

	pMsg.count = ((pMsg.count==0)?0xFFFFFFFF:pMsg.count);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(aIndex,send,size);

	#endif
}

void CPersonalShop::CGPShopSearchLogRecv(PMSG_PSHOP_SEARCH_LOG_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(gObjIsConnectedGP(lpMsg->index) == 0)
	{
		return;
	}

	gLog.Output(LOG_TRADE,"[ContactPesonalShopOwner][%s][%s] - (Account: %s, Name: %s, Type: %d)",lpObj->Account,lpObj->Name,gObj[lpMsg->index].Account,gObj[lpMsg->index].Name,lpMsg->type);

	#endif
}

void CPersonalShop::GCPShopViewportSend(int aIndex) // OK
{
	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->RegenOk > 0)
	{
		return;
	}

	if(lpObj->PShopRedrawAbs != 0)
	{
		memset(lpObj->VpPShopPlayer,0,sizeof(lpObj->VpPShopPlayer));
		lpObj->VpPShopPlayerCount = 0;
		lpObj->PShopRedrawAbs = 0;
	}

	int IndexTable[MAX_VIEWPORT];
	int IndexCount = 0;

	BYTE send[4096];

	PMSG_PSHOP_VIEWPORT_SEND pMsg;

	pMsg.header.set(0x3F,0x00,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	PMSG_PSHOP_VIEWPORT info;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer[n].state != VIEWPORT_WAIT)
		{
			continue;
		}

		if(lpObj->VpPlayer[n].type != OBJECT_USER)
		{
			continue;
		}

		if(OBJECT_RANGE(lpObj->VpPlayer[n].index) == 0)
		{
			continue;
		}

		LPOBJ lpTarget = &gObj[lpObj->VpPlayer[n].index];

		if(lpTarget->PShopOpen == 0)
		{
			continue;
		}

		IndexTable[IndexCount] = lpTarget->Index;
		IndexCount++;

		if(lpObj->VpPShopPlayerCount == 0)
		{
			info.index[0] = SET_NUMBERHB(lpTarget->Index);
			info.index[1] = SET_NUMBERLB(lpTarget->Index);

			memcpy(info.text,lpTarget->PShopText,sizeof(info.text));

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
		else
		{
			if(this->CheckPersonalShopViewport(aIndex,lpTarget->Index) != 0)
			{
				info.index[0] = SET_NUMBERHB(lpTarget->Index);
				info.index[1] = SET_NUMBERLB(lpTarget->Index);

				memcpy(info.text,lpTarget->PShopText,sizeof(info.text));

				memcpy(&send[size],&info,sizeof(info));
				size += sizeof(info);

				pMsg.count++;
			}
		}
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		DataSend(aIndex,send,size);
	}

	if(IndexCount == 0)
	{
		memset(lpObj->VpPShopPlayer,-1,sizeof(lpObj->VpPShopPlayer));

		lpObj->VpPShopPlayerCount = 0;
	}
	else
	{
		for(int n=0;n < IndexCount;n++)
		{
			lpObj->VpPShopPlayer[n] = IndexTable[n];
		}

		lpObj->VpPShopPlayerCount = IndexCount;
	}

	if(lpObj->PShopWantDeal != 0)
	{
		if(gObjIsConnectedGP(lpObj->PShopDealerIndex) == 0 || gObj[lpObj->PShopDealerIndex].PShopOpen == 0)
		{
			lpObj->PShopWantDeal = 0;
			lpObj->PShopDealerIndex = -1;
			memset(lpObj->PShopDealerName,0,sizeof(lpObj->PShopDealerName));
			this->GCPShopLeaveSend(aIndex,lpObj->PShopDealerIndex);
		}
		else if(gObj[lpObj->PShopDealerIndex].PShopItemChange != 0)
		{
			this->GCPShopItemListSend(aIndex,lpObj->PShopDealerIndex,1,1);
		}
	}
}

void CPersonalShop::GCPShopSetItemPriceSend(int aIndex,BYTE result,BYTE slot) // OK
{
	PMSG_PSHOP_SET_ITEM_PRICE_SEND pMsg;

	pMsg.header.set(0x3F,0x01,sizeof(pMsg));

	pMsg.result = result;

	pMsg.slot = slot;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CPersonalShop::GCPShopOpenSend(int aIndex,BYTE result) // OK
{
	PMSG_PSHOP_OPEN_SEND pMsg;

	pMsg.header.set(0x3F,0x02,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	gCustomStore.GCOffActiveSend(aIndex,1);
}

void CPersonalShop::GCPShopCloseSend(int aIndex,BYTE result) // OK
{
	PMSG_PSHOP_CLOSE_SEND pMsg;

	pMsg.header.set(0x3F,0x03,sizeof(pMsg));

	pMsg.result = result;

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	if(pMsg.result != 0)
	{
		MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
	}

	gCustomStore.GCOffActiveSend(aIndex,0);
}

void CPersonalShop::GCPShopItemListSend(int aIndex,int bIndex,BYTE result,BYTE type) // OK
{
	LPOBJ lpObj = &gObj[bIndex];

	BYTE send[4096];

	PMSG_PSHOP_ITEM_LIST_SEND pMsg;

	pMsg.header.set(0x3F,((type==0)?0x05:0x13),0);

	int size = sizeof(pMsg);

	pMsg.result = result;

	pMsg.index[0] = SET_NUMBERHB(bIndex);
	pMsg.index[1] = SET_NUMBERLB(bIndex);

	memset(pMsg.name,0,sizeof(pMsg.name));

	memset(pMsg.text,0,sizeof(pMsg.text));

	pMsg.count = 0;

	if(result == 1)
	{
		memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

		memcpy(pMsg.text,lpObj->PShopText,sizeof(pMsg.text));

		PMSG_PSHOP_ITEM_LIST info;

		for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
		{
			if(lpObj->Inventory[n].IsItem() == 0)
			{
				continue;
			}

			info.slot = n;

			gItemManager.ItemByteConvert(info.ItemInfo,lpObj->Inventory[n]);

			info.value = lpObj->Inventory[n].m_PShopValue;

			#if(GAMESERVER_UPDATE>=802)

			info.JoBValue = lpObj->Inventory[n].m_PShopJoBValue;

			info.JoSValue = lpObj->Inventory[n].m_PShopJoSValue;

			info.JoCValue = lpObj->Inventory[n].m_PShopJoCValue;

			#endif

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(aIndex,send,size);

	#if(GAMESERVER_UPDATE>=701)

	if(result == 1){gPentagramSystem.GCPentagramJewelPShopInfoSend(aIndex,bIndex);}

	#endif
}

void CPersonalShop::GCPShopBuyItemSend(int aIndex,int bIndex,int slot,BYTE result) // OK
{
	PMSG_PSHOP_BUY_ITEM_SEND pMsg;

	pMsg.header.set(0x3F,0x06,sizeof(pMsg));

	pMsg.result = result;

	pMsg.index[0] = SET_NUMBERHB(bIndex);

	pMsg.index[1] = SET_NUMBERLB(bIndex);

	gItemManager.ItemByteConvert(pMsg.ItemInfo,gObj[aIndex].Inventory[slot]);

	pMsg.slot = slot;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CPersonalShop::GCPShopSellItemSend(int aIndex,int bIndex,int slot) // OK
{
	PMSG_PSHOP_SELL_ITEM_SEND pMsg;

	pMsg.header.set(0x3F,0x08,sizeof(pMsg));

	pMsg.slot = slot;

	memcpy(pMsg.name,gObj[bIndex].Name,sizeof(pMsg.name));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CPersonalShop::GCPShopTextChangeSend(int aIndex) // OK
{
	PMSG_PSHOP_TEXT_CHANGE_SEND pMsg;

	pMsg.header.set(0x3F,0x10,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	memcpy(pMsg.text,gObj[aIndex].PShopText,sizeof(pMsg.text));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
}

void CPersonalShop::GCPShopLeaveSend(int aIndex,int bIndex) // OK
{
	PMSG_PSHOP_LEAVE_SEND pMsg;

	pMsg.header.set(0x3F,0x12,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(bIndex);

	pMsg.index[1] = SET_NUMBERLB(bIndex);
	
	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CPersonalShop::GCPShopItemValueSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[1024];

	PMSG_PSHOP_ITEM_VALUE_SEND pMsg;

	pMsg.header.set(0xEC,0x32,0);

	int size = sizeof(pMsg);

	pMsg.MoneyCommisionRate = gServerInfo.m_PersonalShopMoneyCommisionRate;

	pMsg.JewelCommisionRate = gServerInfo.m_PersonalShopJewelCommisionRate;

	pMsg.count = 0;

	PMSG_PSHOP_ITEM_VALUE info;

	for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() == 0)
		{
			continue;
		}

		info.slot = n;

		info.serial = lpObj->Inventory[n].m_Serial;

		info.value = lpObj->Inventory[n].m_PShopValue;

		info.JoBValue = lpObj->Inventory[n].m_PShopJoBValue;

		info.JoSValue = lpObj->Inventory[n].m_PShopJoSValue;

		info.JoCValue = lpObj->Inventory[n].m_PShopJoCValue;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(aIndex,send,size);

	#endif
}

void CPersonalShop::DGPShopItemValueRecv(SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPShopItemValueRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_PSHOP_ITEM_VALUE* lpInfo = (SDHP_PSHOP_ITEM_VALUE*)(((BYTE*)lpMsg)+sizeof(SDHP_PSHOP_ITEM_VALUE_RECV)+(sizeof(SDHP_PSHOP_ITEM_VALUE)*n));

		if(INVENTORY_SHOP_RANGE(lpInfo->slot) == 0)
		{
			continue;
		}

		if(lpObj->Inventory[lpInfo->slot].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->Inventory[lpInfo->slot].m_Serial != lpInfo->serial)
		{
			continue;
		}

		lpObj->Inventory[lpInfo->slot].m_PShopValue = lpInfo->value;

		lpObj->Inventory[lpInfo->slot].m_PShopJoBValue = lpInfo->JoBValue;

		lpObj->Inventory[lpInfo->slot].m_PShopJoSValue = lpInfo->JoSValue;

		lpObj->Inventory[lpInfo->slot].m_PShopJoCValue = lpInfo->JoCValue;
	}

	gPersonalShop.GCPShopItemValueSend(lpObj->Index);

	#endif
}

void CPersonalShop::GDPShopItemValueSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_PSHOP_ITEM_VALUE_SEND pMsg;

	pMsg.header.set(0x25,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPersonalShop::GDPShopItemValueSaveSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[1024];

	SDHP_PSHOP_ITEM_VALUE_SAVE_SEND pMsg;

	pMsg.header.set(0x25,0x30,0);

	int size = sizeof(pMsg);

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.count = 0;

	SDHP_PSHOP_ITEM_VALUE_SAVE info;

	for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			info.slot = n;

			info.serial = lpObj->Inventory[n].m_Serial;

			info.value = lpObj->Inventory[n].m_PShopValue;

			info.JoBValue = lpObj->Inventory[n].m_PShopJoBValue;

			info.JoSValue = lpObj->Inventory[n].m_PShopJoSValue;

			info.JoCValue = lpObj->Inventory[n].m_PShopJoCValue;

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		gDataServerConnection.DataSend(send,size);
	}

	#endif
}

void CPersonalShop::GDPShopItemValueInsertSaveSend(int aIndex,int slot,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_SEND pMsg;

	pMsg.header.set(0x25,0x31,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.account));

	pMsg.slot = slot;

	pMsg.serial = lpItem->m_Serial;

	pMsg.value = lpItem->m_PShopValue;

	pMsg.JoBValue = lpItem->m_PShopJoBValue;

	pMsg.JoSValue = lpItem->m_PShopJoSValue;

	pMsg.JoCValue = lpItem->m_PShopJoCValue;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPersonalShop::GDPShopItemValueDeleteSaveSend(int aIndex,int slot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_SEND pMsg;

	pMsg.header.set(0x25,0x32,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.account));

	pMsg.slot = slot;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}
