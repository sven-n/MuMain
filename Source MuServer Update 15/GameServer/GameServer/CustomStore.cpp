// CustomStore.cpp: implementation of the CCustomStore class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomStore.h"
#include "CashShop.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "MasterSkillTree.h"
#include "Message.h"
#include "Notice.h"
#include "PcPoint.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "Util.h"

CCustomStore gCustomStore;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomStore::CCustomStore() // OK
{

}

CCustomStore::~CCustomStore() // OK
{

}

void CCustomStore::ReadCustomStoreInfo(char* section,char* path) // OK
{
	//this->m_CustomStoreMapZone = GetPrivateProfileInt(section,"CustomStoreMapZone",0,path);

	this->m_CustomStoreTime[0] = GetPrivateProfileInt(section,"CustomStoreTime_AL0",0,path);

	this->m_CustomStoreTime[1] = GetPrivateProfileInt(section,"CustomStoreTime_AL1",0,path);

	this->m_CustomStoreTime[2] = GetPrivateProfileInt(section,"CustomStoreTime_AL2",0,path);

	this->m_CustomStoreTime[3] = GetPrivateProfileInt(section,"CustomStoreTime_AL3",0,path);

	GetPrivateProfileString(section,"CustomStoreCommandJoBSyntax","",this->m_CustomStoreCommandJoBSyntax,sizeof(this->m_CustomStoreCommandJoBSyntax),path);

	GetPrivateProfileString(section,"CustomStoreCommandJoSSyntax","",this->m_CustomStoreCommandJoSSyntax,sizeof(this->m_CustomStoreCommandJoSSyntax),path);

	GetPrivateProfileString(section,"CustomStoreCommandJoCSyntax","",this->m_CustomStoreCommandJoCSyntax,sizeof(this->m_CustomStoreCommandJoCSyntax),path);

	#if(GAMESERVER_UPDATE>=501)

	GetPrivateProfileString(section,"CustomStoreCommandWCCSyntax","",this->m_CustomStoreCommandWCCSyntax,sizeof(this->m_CustomStoreCommandWCCSyntax),path);

	GetPrivateProfileString(section,"CustomStoreCommandWCPSyntax","",this->m_CustomStoreCommandWCPSyntax,sizeof(this->m_CustomStoreCommandWCPSyntax),path);

	GetPrivateProfileString(section,"CustomStoreCommandWCGSyntax","",this->m_CustomStoreCommandWCGSyntax,sizeof(this->m_CustomStoreCommandWCGSyntax),path);

	#else

	GetPrivateProfileString(section,"CustomStoreCommandWCCSyntax","",this->m_CustomStoreCommandWCCSyntax,sizeof(this->m_CustomStoreCommandWCCSyntax),path);

	GetPrivateProfileString(section,"CustomStoreCommandWCPSyntax","",this->m_CustomStoreCommandWCPSyntax,sizeof(this->m_CustomStoreCommandWCPSyntax),path);

	GetPrivateProfileString(section,"CustomStoreCommandPCPSyntax","",this->m_CustomStoreCommandWCGSyntax,sizeof(this->m_CustomStoreCommandWCGSyntax),path);

	#endif

	GetPrivateProfileString(section,"CustomStoreJoBName","",this->m_CustomStoreJoBName,sizeof(this->m_CustomStoreJoBName),path);

	GetPrivateProfileString(section,"CustomStoreJoSName","",this->m_CustomStoreJoSName,sizeof(this->m_CustomStoreJoSName),path);

	GetPrivateProfileString(section,"CustomStoreJoCName","",this->m_CustomStoreJoCName,sizeof(this->m_CustomStoreJoCName),path);

	#if(GAMESERVER_UPDATE>=501)

	GetPrivateProfileString(section,"CustomStoreWCCName","",this->m_CustomStoreWCCName,sizeof(this->m_CustomStoreWCCName),path);

	GetPrivateProfileString(section,"CustomStoreWCPName","",this->m_CustomStoreWCPName,sizeof(this->m_CustomStoreWCPName),path);

	GetPrivateProfileString(section,"CustomStoreWCGName","",this->m_CustomStoreWCGName,sizeof(this->m_CustomStoreWCGName),path);

	#else

	GetPrivateProfileString(section,"CustomStoreWCCName","",this->m_CustomStoreWCCName,sizeof(this->m_CustomStoreWCCName),path);

	GetPrivateProfileString(section,"CustomStoreWCPName","",this->m_CustomStoreWCPName,sizeof(this->m_CustomStoreWCPName),path);

	GetPrivateProfileString(section,"CustomStorePCPName","",this->m_CustomStoreWCGName,sizeof(this->m_CustomStoreWCGName),path);

	#endif

	//this->m_CustomStoreOfflineMapZone = GetPrivateProfileInt(section,"CustomStoreOfflineMapZone",0,path);
}

bool CCustomStore::CommandCustomStore(LPOBJ lpObj,char* arg) // OK
{
	char mode[32] = {0};

	gCommandManager.GetString(arg,mode,sizeof(mode),0);

	if(strcmp(mode,"") != 0)
	{
		if(strcmp(mode,this->m_CustomStoreCommandJoBSyntax) == 0)
		{
			if (this->OpenCustomStore(lpObj,0) != 0)
			{
				return 1;
			}
		}
		else if(strcmp(mode,this->m_CustomStoreCommandJoSSyntax) == 0)
		{
			if (this->OpenCustomStore(lpObj,1) != 0)
			{
				return 1;
			}
		}
		else if(strcmp(mode,this->m_CustomStoreCommandJoCSyntax) == 0)
		{
			if (this->OpenCustomStore(lpObj,2) != 0)
			{
				return 1;
			}
		}
		else if(strcmp(mode,this->m_CustomStoreCommandWCCSyntax) == 0)
		{
			if (this->OpenCustomStore(lpObj,3) != 0)
			{
				return 1;
			}
		}
		else if(strcmp(mode,this->m_CustomStoreCommandWCPSyntax) == 0)
		{
			if (this->OpenCustomStore(lpObj,4) != 0)
			{
				return 1;
			}
		}
		else if(strcmp(mode,this->m_CustomStoreCommandWCGSyntax) == 0)
		{
			if (this->OpenCustomStore(lpObj,5) != 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

bool CCustomStore::CommandCustomStoreOffline(LPOBJ lpObj,char* arg) // OK
{
	 if(lpObj->Interface.use != 0)
	 {
		return 0;
	 }

	if(lpObj->TradeDuel != 0)
	{
		return 0;
	}

	if(lpObj->PShopOpen == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(787));
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
		return 0;
	}

	if(CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
		return 0;
	}

	//if(this->m_CustomStoreOfflineMapZone != -1 && this->m_CustomStoreOfflineMapZone != lpObj->Map)
	//{
	//	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
	//	return 0;
	//}

	if (gMapManager.GetMapCustomStore(lpObj->Map) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
		return 0;
	}

	lpObj->Socket = INVALID_SOCKET;

	lpObj->PShopCustomOffline = 1;

	lpObj->PShopCustomOfflineTime = 0;

	lpObj->PShopCustomTime = this->m_CustomStoreTime[lpObj->AccountLevel]*60;

	#if(NEW_PROTOCOL_SYSTEM==0)
	closesocket(lpObj->PerSocketContext->Socket);
	#endif

	return 1;
}

bool CCustomStore::OpenCustomStore(LPOBJ lpObj,int type) // OK
{
	if(gServerInfo.m_PersonalShopSwitch == 0)
	{
		return 0;
	}

	if(lpObj->TradeDuel != 0)
	{
		return 0;
	}

	if(lpObj->Interface.use != 0)
	{
	return 0;
	}	

	if(lpObj->PShopOpen != 0)
	{
		gPersonalShop.GCPShopOpenSend(lpObj->Index,0);
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
		return 0;
	}

	if(CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
		return 0;
	}

	//if(this->m_CustomStoreMapZone != -1 && this->m_CustomStoreMapZone != lpObj->Map)
	//{
	//	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
	//	return 0;
	//}

	if (gMapManager.GetMapCustomStore(lpObj->Map) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
		return 0;
	}

	if(gPersonalShop.CheckPersonalShopOpen(lpObj->Index) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(780));
		return 0;
	}

	if(lpObj->Level <= 5)
	{
		gPersonalShop.GCPShopOpenSend(lpObj->Index,2);
		return 0;
	}

	lpObj->PShopOpen = 1;

	lpObj->PShopCustom = 1;

	lpObj->PShopCustomType = type;

	lpObj->PShopCustomOffline = 0;

	lpObj->PShopCustomOfflineTime = 0;

	lpObj->PShopCustomTime = this->m_CustomStoreTime[lpObj->AccountLevel]*60;

	if (this->m_CustomStoreTime[lpObj->AccountLevel] > 0) 
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(788),this->m_CustomStoreTime[lpObj->AccountLevel]);
	}

	switch(type)
	{
		case 0:
			memcpy(lpObj->PShopText,this->m_CustomStoreJoBName,sizeof(lpObj->PShopText));
			break;
		case 1:
			memcpy(lpObj->PShopText,this->m_CustomStoreJoSName,sizeof(lpObj->PShopText));
			break;
		case 2:
			memcpy(lpObj->PShopText,this->m_CustomStoreJoCName,sizeof(lpObj->PShopText));
			break;
		case 3:
			memcpy(lpObj->PShopText,this->m_CustomStoreWCCName,sizeof(lpObj->PShopText));
			break;
		case 4:
			memcpy(lpObj->PShopText,this->m_CustomStoreWCPName,sizeof(lpObj->PShopText));
			break;
		case 5:
			memcpy(lpObj->PShopText,this->m_CustomStoreWCGName,sizeof(lpObj->PShopText));
			break;
	}

	#if(GAMESERVER_UPDATE != 603)
		gPersonalShop.GCPShopOpenSend(lpObj->Index,1);
	#endif

	PMSG_PSHOP_TEXT_CHANGE_SEND pMsg;

	pMsg.header.set(0x3F,0x10,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(lpObj->Index);

	pMsg.index[1] = SET_NUMBERLB(lpObj->Index);

	memcpy(pMsg.text,lpObj->PShopText,sizeof(pMsg.text));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	this->GCOffActiveSend(lpObj->Index,1);

	return 1;
}

bool CCustomStore::OnPShopOpen(LPOBJ lpObj) // OK
{
	return ((lpObj->PShopCustom==0)?0:1);
}

void CCustomStore::OnPShopClose(LPOBJ lpObj) // OK
{
	if(lpObj->PShopCustom != 0)
	{
		lpObj->PShopCustom = 0;
		lpObj->PShopCustomType = 0;
	}

	if(lpObj->PShopCustomOffline == 1)
	{
		lpObj->PShopCustomOffline = 2;
		lpObj->PShopCustomOfflineTime = 5;
	}

	gPersonalShop.GCPShopCloseSend(lpObj->Index,1);
	
	lpObj->PShopCustomTime = 0;
	lpObj->PShopOpen = 0;
	memset(lpObj->PShopText,0,sizeof(lpObj->PShopText));

	//this->GCOffActiveSend(lpObj->Index,0);
}

void CCustomStore::OnPShopSecondProc(LPOBJ lpObj) // OK
{
	if(lpObj->PShopCustomOffline != 0)
	{
		if(lpObj->PShopCustomOffline == 2)
		{
			if((--lpObj->PShopCustomOfflineTime) == 0)
			{
				gObjDel(lpObj->Index);
				lpObj->PShopCustomOffline = 0;
				lpObj->PShopCustomOfflineTime = 0;
			}
		}

		lpObj->CheckSumTime = GetTickCount();
		lpObj->ConnectTickCount = GetTickCount();
		lpObj->OnlineRewardCoin1 = ((gServerInfo.m_OnlineRewardOfflineSystems==0)?GetTickCount():lpObj->OnlineRewardCoin1);
		lpObj->OnlineRewardCoin2 = ((gServerInfo.m_OnlineRewardOfflineSystems==0)?GetTickCount():lpObj->OnlineRewardCoin2);
		lpObj->OnlineRewardCoin3 = ((gServerInfo.m_OnlineRewardOfflineSystems==0)?GetTickCount():lpObj->OnlineRewardCoin3);
	}

	if (lpObj->PShopOpen == 1)
	{
		if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
		{
			this->OnPShopClose(lpObj);
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(779));
		}

		if(gMapManager.GetMapCustomStore(lpObj->Map) == 0)
		{
			this->OnPShopClose(lpObj);
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
		}
	}

	if (lpObj->PShopCustomTime > 0) 
	{
		if((--lpObj->PShopCustomTime) == 0)
		{
			LogAdd(LOG_BLACK,"[Store][%s][%s] Timeout",lpObj->Account,lpObj->Name);
			
			this->OnPShopClose(lpObj);

			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(789));
			return;
		}
	}
}

void CCustomStore::OnPShopAlreadyConnected(LPOBJ lpObj) // OK
{
	if(lpObj->PShopCustomOffline != 0)
	{
		gObjDel(lpObj->Index);
		lpObj->PShopCustomOffline = 0;
		lpObj->PShopCustomOfflineTime = 0;
	}
}

void CCustomStore::OnPShopItemList(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpTarget->PShopCustom != 0)
	{
		switch(lpTarget->PShopCustomType)
		{
			case 0:
				gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(781));
				break;
			case 1:
				gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(782));
				break;
			case 2:
				gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(783));
				break;
			case 3:
				gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(784));
				break;
			case 4:
				gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(785));
				break;
			case 5:
				gNotice.GCNoticeSend(lpObj->Index,0,0,0,0,0,0,gMessage.GetMessage(786));
				break;
		}
	}
}

bool CCustomStore::OnPShopBuyItemRecv(PMSG_PSHOP_BUY_ITEM_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return 1;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(gObjIsConnectedGP(bIndex) == 0)
	{
		return 1;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpTarget->PShopOpen == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,3);
		return 1;
	}

	if(lpTarget->PShopCustom == 0)
	{
		return 0;
	}

	if(lpTarget->PShopTransaction != 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,4);
		return 1;
	}

	if(INVENTORY_SHOP_RANGE(lpMsg->slot) == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,5);
		return 1;
	}

	gObjFixInventoryPointer(aIndex);

	if(lpObj->Transaction == 1)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return 1;
	}

	char name[11] = {0};

	memcpy(name,lpMsg->name,sizeof(lpMsg->name));
	
	if(strcmp(name,lpTarget->Name) != 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return 1;
	}

	if(lpTarget->Inventory[lpMsg->slot].IsItem() == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return 1;
	}

	if(lpTarget->Inventory[lpMsg->slot].m_PShopValue <= 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,6);
		return 1;
	}

	if(lpTarget->PShopCustomType == 3 || lpTarget->PShopCustomType == 4 || lpTarget->PShopCustomType == 5)
	{
		gCashShop.GDCashShopRecievePointSend(aIndex,(DWORD)&CCustomStore::OnPShopBuyItemCallbackRecv,(DWORD)&gObj[bIndex],lpMsg->slot);
		return 1;
	}

	int PShopJoBValue = ((lpTarget->PShopCustomType==0)?lpTarget->Inventory[lpMsg->slot].m_PShopValue:0);

	int PShopJoSValue = ((lpTarget->PShopCustomType==1)?lpTarget->Inventory[lpMsg->slot].m_PShopValue:0);

	int PShopJoCValue = ((lpTarget->PShopCustomType==2)?lpTarget->Inventory[lpMsg->slot].m_PShopValue:0);

	int RequireJewelCount[3] = {0};

	int PaymentJewelCount[3] = {0};

	int RequireJewelTable[3][4] = {0};

	int PaymentJewelTable[3][4] = {0};

	gPersonalShop.GetRequireJewelCount(lpObj,&RequireJewelCount[0],RequireJewelTable[0],0,PShopJoBValue);

	gPersonalShop.GetRequireJewelCount(lpObj,&RequireJewelCount[1],RequireJewelTable[1],1,PShopJoSValue);

	gPersonalShop.GetRequireJewelCount(lpObj,&RequireJewelCount[2],RequireJewelTable[2],2,PShopJoCValue);

	gPersonalShop.GetPaymentJewelCount(lpTarget,&PaymentJewelCount[0],PaymentJewelTable[0],0,PShopJoBValue);

	gPersonalShop.GetPaymentJewelCount(lpTarget,&PaymentJewelCount[1],PaymentJewelTable[1],1,PShopJoSValue);

	gPersonalShop.GetPaymentJewelCount(lpTarget,&PaymentJewelCount[2],PaymentJewelTable[2],2,PShopJoCValue);

	if(RequireJewelCount[0] < PShopJoBValue)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	if(RequireJewelCount[1] < PShopJoSValue)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	if(RequireJewelCount[2] < PShopJoCValue)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	if(PShopJoBValue > 0 && RequireJewelTable[0][0] == 0 && RequireJewelTable[0][1] == 0 && RequireJewelTable[0][2] == 0 && RequireJewelTable[0][3] == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	if(PShopJoSValue > 0 && RequireJewelTable[1][0] == 0 && RequireJewelTable[1][1] == 0 && RequireJewelTable[1][2] == 0 && RequireJewelTable[1][3] == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	if(PShopJoCValue > 0 && RequireJewelTable[2][0] == 0 && RequireJewelTable[2][1] == 0 && RequireJewelTable[2][2] == 0 && RequireJewelTable[2][3] == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	if(gItemManager.GetInventoryEmptySlotCount(lpTarget) < (PaymentJewelCount[0]+PaymentJewelCount[1]+PaymentJewelCount[2]))
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,7);
		return 1;
	}

	lpTarget->PShopTransaction = 1;

	BYTE result = gItemManager.InventoryInsertItem(aIndex,lpTarget->Inventory[lpMsg->slot]);

	if(result == 0xFF)
	{
		gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,0,8);
		return 1;
	}

	gLog.Output(LOG_TRADE,"[SellPesonalShopItem][%s][%s] - (Account: %s, Name: %s, Value: %d, JoBValue: %d, JoSValue: %d, JoCValue: %d, Index: %04d, Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",lpTarget->Account,lpTarget->Name,lpObj->Account,0,PShopJoBValue,PShopJoSValue,PShopJoCValue,lpTarget->Inventory[lpMsg->slot].m_Index,lpTarget->Inventory[lpMsg->slot].m_Level,lpTarget->Inventory[lpMsg->slot].m_Serial,lpTarget->Inventory[lpMsg->slot].m_Option1,lpTarget->Inventory[lpMsg->slot].m_Option2,lpTarget->Inventory[lpMsg->slot].m_Option3,lpTarget->Inventory[lpMsg->slot].m_NewOption,lpTarget->Inventory[lpMsg->slot].m_JewelOfHarmonyOption,lpTarget->Inventory[lpMsg->slot].m_ItemOptionEx,lpTarget->Inventory[lpMsg->slot].m_SocketOption[0],lpTarget->Inventory[lpMsg->slot].m_SocketOption[1],lpTarget->Inventory[lpMsg->slot].m_SocketOption[2],lpTarget->Inventory[lpMsg->slot].m_SocketOption[3],lpTarget->Inventory[lpMsg->slot].m_SocketOption[4]);

	gPersonalShop.SetRequireJewelCount(lpObj,RequireJewelTable[0],0);

	gPersonalShop.SetRequireJewelCount(lpObj,RequireJewelTable[1],1);

	gPersonalShop.SetRequireJewelCount(lpObj,RequireJewelTable[2],2);

	gPersonalShop.GCPShopBuyItemSend(aIndex,bIndex,result,1);

	GDCharacterInfoSaveSend(aIndex);

	gPersonalShop.SetPaymentJewelCount(lpTarget,PaymentJewelTable[0],0);

	gPersonalShop.SetPaymentJewelCount(lpTarget,PaymentJewelTable[1],1);

	gPersonalShop.SetPaymentJewelCount(lpTarget,PaymentJewelTable[2],2);

	gPersonalShop.GCPShopSellItemSend(bIndex,aIndex,lpMsg->slot);

	gItemManager.InventoryDelItem(bIndex,lpMsg->slot);
	gItemManager.GCItemDeleteSend(bIndex,lpMsg->slot,1);

	GDCharacterInfoSaveSend(bIndex);

	if(gPersonalShop.CheckPersonalShop(bIndex) == 0)
	{
		lpTarget->PShopItemChange = 1;
	}
	else
	{
		//lpTarget->PShopOpen = 0;
		//memset(lpTarget->PShopText,0,sizeof(lpTarget->PShopText));
		//gPersonalShop.GCPShopCloseSend(bIndex,1);
		this->OnPShopClose(lpTarget);
	}

	lpTarget->PShopTransaction = 0;

	return 1;
}

void CCustomStore::OnPShopBuyItemCallbackRecv(LPOBJ lpObj,LPOBJ lpTarget,DWORD slot,DWORD WCoinC,DWORD WCoinP,DWORD GoblinPoint) // OK
{
	if(gObjIsConnectedGP(lpTarget->Index) == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,3);
		return;
	}

	if(lpTarget->PShopOpen == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,3);
		return;
	}

	if(lpTarget->PShopTransaction != 0)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,4);
		return;
	}

	if(lpTarget->Inventory[slot].IsItem() == 0)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,6);
		return;
	}

	DWORD PShopWCCValue = ((lpTarget->PShopCustomType==3)?lpTarget->Inventory[slot].m_PShopValue:0);

	DWORD PShopWCPValue = ((lpTarget->PShopCustomType==4)?lpTarget->Inventory[slot].m_PShopValue:0);

	DWORD PShopWGPValue = ((lpTarget->PShopCustomType==5)?lpTarget->Inventory[slot].m_PShopValue:0);

	#if(GAMESERVER_UPDATE>=501)
	if(WCoinC < PShopWCCValue)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,7);
		return;
	}

	if(WCoinP < PShopWCPValue)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,7);
		return;
	}
	#else
	if(lpObj->Coin1 < PShopWCCValue)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,7);
		return;
	}

	if(lpObj->Coin2 < PShopWCPValue)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,7);
		return;
	}
	#endif

	if(GoblinPoint < PShopWGPValue)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,7);
		return;
	}

	lpTarget->PShopTransaction = 1;

	BYTE result = gItemManager.InventoryInsertItem(lpObj->Index,lpTarget->Inventory[slot]);

	if(result == 0xFF)
	{
		gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,0,8);
		return;
	}

	gLog.Output(LOG_TRADE,"[SellPesonalShopItem][%s][%s] - (Account: %s, Name: %s, Value: %d, WCCValue: %d, WCPValue: %d, WGPValue: %d, Index: %04d, Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)",lpTarget->Account,lpTarget->Name,lpObj->Account,0,PShopWCCValue,PShopWCPValue,PShopWGPValue,lpTarget->Inventory[slot].m_Index,lpTarget->Inventory[slot].m_Level,lpTarget->Inventory[slot].m_Serial,lpTarget->Inventory[slot].m_Option1,lpTarget->Inventory[slot].m_Option2,lpTarget->Inventory[slot].m_Option3,lpTarget->Inventory[slot].m_NewOption,lpTarget->Inventory[slot].m_JewelOfHarmonyOption,lpTarget->Inventory[slot].m_ItemOptionEx,lpTarget->Inventory[slot].m_SocketOption[0],lpTarget->Inventory[slot].m_SocketOption[1],lpTarget->Inventory[slot].m_SocketOption[2],lpTarget->Inventory[slot].m_SocketOption[3],lpTarget->Inventory[slot].m_SocketOption[4]);

	gCashShop.GDCashShopSubPointSaveSend(lpObj->Index,0,PShopWCCValue,PShopWCPValue,PShopWGPValue);
	gCashShop.CGCashShopPointRecv(lpObj->Index);

	gPersonalShop.GCPShopBuyItemSend(lpObj->Index,lpTarget->Index,result,1);

	GDCharacterInfoSaveSend(lpObj->Index);


	gCashShop.GDCashShopAddPointSaveSend(lpTarget->Index,0,PShopWCCValue,PShopWCPValue,PShopWGPValue);
	gCashShop.CGCashShopPointRecv(lpTarget->Index);

	gPersonalShop.GCPShopSellItemSend(lpTarget->Index,lpObj->Index,slot);

	gItemManager.InventoryDelItem(lpTarget->Index,(BYTE)slot);
	gItemManager.GCItemDeleteSend(lpTarget->Index,(BYTE)slot,1);

	GDCharacterInfoSaveSend(lpTarget->Index);

	if(gPersonalShop.CheckPersonalShop(lpTarget->Index) == 0)
	{
		lpTarget->PShopItemChange = 1;
	}
	else
	{
		//lpTarget->PShopOpen = 0;
		//memset(lpTarget->PShopText,0,sizeof(lpTarget->PShopText));
		//gPersonalShop.GCPShopCloseSend(lpTarget->Index,1);
		gCustomStore.OnPShopClose(lpTarget);
	}

	lpTarget->PShopTransaction = 0;
}

void CCustomStore::CGOffTradeRecv(PMSG_OFFTRADE_RECV* lpMsg, int aIndex)
{
	if (lpMsg->Type >= 0 && lpMsg->Type <= 5)
	{
		gCustomStore.OpenCustomStore(&gObj[aIndex], lpMsg->Type);
	}
	else if (lpMsg->Type == 6)
	{
		gCustomStore.CommandCustomStoreOffline(&gObj[aIndex], NULL);
	}
}

void CCustomStore::GCOffTradeSend(int aIndex,int bIndex)
{
	PMSG_OFFTRADE_SEND pMsg;
	pMsg.header.set(0xF3, 0xEB, sizeof(pMsg));

	pMsg.Type = -1;

	if(gObj[bIndex].PShopCustom == 1)
	{
		pMsg.Type = gObj[bIndex].PShopCustomType;
	}
	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CCustomStore::GCOffActiveSend(int aIndex, int active)
{
	PMSG_SHOPACTIVE_SEND pMsg;
	pMsg.header.set(0xF3, 0xEC, sizeof(pMsg));
	pMsg.Active = active;

	pMsg.Type = -1;
	if(gObj[aIndex].PShopCustom == 1)
	{
		pMsg.Type = gObj[aIndex].PShopCustomType;
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
}
