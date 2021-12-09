// EventInventory.cpp: implementation of the CEventInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EventInventory.h"
#include "DSProtocol.h"
#include "Duel.h"
#include "GameMain.h"
#include "ItemBagManager.h"
#include "ItemMove.h"
#include "ItemStack.h"
#include "Map.h"
#include "MuRummy.h"
#include "ServerInfo.h"
#include "Util.h"

CEventInventory gEventInventory;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEventInventory::CEventInventory() // OK
{

}

CEventInventory::~CEventInventory() // OK
{

}

bool CEventInventory::IsEventItem(int ItemIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(ItemIndex == GET_ITEM(13,210) || ItemIndex == GET_ITEM(13,220) || ItemIndex == GET_ITEM(13,221) || ItemIndex == GET_ITEM(13,222) || ItemIndex == GET_ITEM(14,215) || ItemIndex == GET_ITEM(14,216))
	{
		return 1;
	}
	else
	{
		return 0;
	}

	#else

	return 0;

	#endif
}

bool CEventInventory::IsEventItem(CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(lpItem->m_Index == GET_ITEM(13,210) || lpItem->m_Index == GET_ITEM(13,220) || lpItem->m_Index == GET_ITEM(13,221) || lpItem->m_Index == GET_ITEM(13,222) || lpItem->m_Index == GET_ITEM(14,215) || lpItem->m_Index == GET_ITEM(14,216))
	{
		return 1;
	}
	else
	{
		return 0;
	}

	#else

	return 0;

	#endif
}

void CEventInventory::EventInventoryItemSet(int aIndex,int slot,BYTE type) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(EVENT_INVENTORY_RANGE(slot) == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(gObj[aIndex].EventInventory[slot].m_Index,&ItemInfo) == 0)
	{
		return;
	}

	int x = slot%8;
	int y = slot/8;

	if((x+ItemInfo.Width) > 8 || (y+ItemInfo.Height) > 4)
	{
		return;
	}

	for(int sy=0;sy < ItemInfo.Height;sy++)
	{
		for(int sx=0;sx < ItemInfo.Width;sx++)
		{
			gObj[aIndex].EventInventoryMap[(((sy+y)*8)+(sx+x))] = type;
		}
	}

	#endif
}

BYTE CEventInventory::EventInventoryRectCheck(int aIndex,int x,int y,int width,int height) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if((x+width) > 8 || (y+height) > 4)
	{
		return 0xFF;
	}

	for(int sy=0;sy < height;sy++)
	{
		for(int sx=0;sx < width;sx++)
		{
			if(gObj[aIndex].EventInventoryMap[(((sy+y)*8)+(sx+x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return ((y*8)+x);

	#else

	return 0xFF;

	#endif
}

BYTE CEventInventory::EventInventoryInsertItem(int aIndex,CItem item) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(item.m_Index,&ItemInfo) == 0)
	{
		return 0xFF;
	}

	for(int y=0;y < 4;y++)
	{
		for(int x=0;x < 8;x++)
		{
			if(gObj[aIndex].EventInventoryMap[((y*8)+x)] == 0xFF)
			{
				BYTE slot = this->EventInventoryRectCheck(aIndex,x,y,ItemInfo.Width,ItemInfo.Height);

				if(slot != 0xFF)
				{
					gObj[aIndex].EventInventory[slot] = item;
					this->EventInventoryItemSet(aIndex,slot,1);
					return slot;
				}
			}
		}
	}

	return 0xFF;

	#else

	return 0xFF;

	#endif
}

BYTE CEventInventory::EventInventoryAddItem(int aIndex,CItem item,int slot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(EVENT_INVENTORY_RANGE(slot) == 0)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->EventInventory[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(item.m_Index,&ItemInfo) == 0)
	{
		return 0xFF;
	}

	int x = slot%8;
	int y = slot/8;

	if(this->EventInventoryRectCheck(aIndex,x,y,ItemInfo.Width,ItemInfo.Height) == 0xFF)
	{
		return 0xFF;
	}

	lpObj->EventInventory[slot] = item;
	this->EventInventoryItemSet(aIndex,slot,1);
	return slot;

	#else

	return 0xFF;

	#endif
}

void CEventInventory::EventInventoryDelItem(int aIndex,int slot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	this->EventInventoryItemSet(aIndex,slot,0xFF);
	gObj[aIndex].EventInventory[slot].Clear();

	#endif
}

bool CEventInventory::EventInventoryInsertItemStack(LPOBJ lpObj,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int MaxStack,CreateItemIndex;

	if(lpItem->IsItem() == 0)
	{
		return 0;
	}

	if((MaxStack=gItemStack.GetItemMaxStack(lpItem->m_Index)) <= 0)
	{
		return 0;
	}

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		if(lpObj->EventInventory[n].IsItem() != 0)
		{
			if(lpObj->EventInventory[n].m_Index == lpItem->m_Index && lpObj->EventInventory[n].m_Level == lpItem->m_Level && lpObj->EventInventory[n].m_SocketOptionBonus == lpItem->m_SocketOptionBonus)
			{
				if(lpObj->EventInventory[n].m_Durability < MaxStack)
				{
					int AddDur = (int)(MaxStack-lpObj->EventInventory[n].m_Durability);

					AddDur = ((AddDur>lpItem->m_Durability)?(int)lpItem->m_Durability:AddDur);

					lpItem->m_Durability -= AddDur;

					lpObj->EventInventory[n].m_Durability += AddDur;

					if(lpObj->EventInventory[n].m_Durability == MaxStack && (CreateItemIndex=gItemStack.GetCreateItemIndex(lpItem->m_Index)) != -1)
					{
						this->EventInventoryDelItem(lpObj->Index,n);
						this->GCEventItemDeleteSend(lpObj->Index,n,1);
						GDCreateItemSend(lpObj->Index,0xEB,0,0,CreateItemIndex,(BYTE)lpItem->m_Level,1,0,0,0,lpObj->Index,0,0,0,0,0,lpItem->m_SocketOptionBonus,0);
						if(lpItem->m_Durability < 1){return 1;}
					}
					else
					{
						this->GCEventItemDurSend(lpObj->Index,n,(BYTE)lpObj->EventInventory[n].m_Durability);
						if(lpItem->m_Durability < 1){return 1;}
					}
				}
			}
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CEventInventory::EventInventoryAddItemStack(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int MaxStack,CreateItemIndex;

	if(lpObj->EventInventory[SourceSlot].IsItem() == 0 || lpObj->EventInventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->EventInventory[SourceSlot].m_Index != lpObj->EventInventory[TargetSlot].m_Index || lpObj->EventInventory[SourceSlot].m_Level != lpObj->EventInventory[TargetSlot].m_Level || lpObj->EventInventory[SourceSlot].m_SocketOptionBonus != lpObj->EventInventory[TargetSlot].m_SocketOptionBonus)
	{
		return 0;
	}

	if((MaxStack=gItemStack.GetItemMaxStack(lpObj->EventInventory[SourceSlot].m_Index)) <= 0)
	{
		return 0;
	}

	if(lpObj->EventInventory[TargetSlot].m_Durability >= MaxStack)
	{
		return 0;
	}

	int AddDur = (int)(MaxStack-lpObj->EventInventory[TargetSlot].m_Durability);

	AddDur = ((AddDur>lpObj->EventInventory[SourceSlot].m_Durability)?(int)lpObj->EventInventory[SourceSlot].m_Durability:AddDur);

	lpObj->EventInventory[SourceSlot].m_Durability -= AddDur;

	lpObj->EventInventory[TargetSlot].m_Durability += AddDur;

	if(lpObj->EventInventory[TargetSlot].m_Durability == MaxStack && (CreateItemIndex=gItemStack.GetCreateItemIndex(lpObj->EventInventory[SourceSlot].m_Index)) != -1)
	{
		this->EventInventoryDelItem(lpObj->Index,TargetSlot);
		this->GCEventItemDeleteSend(lpObj->Index,TargetSlot,1);
		GDCreateItemSend(lpObj->Index,0xEB,0,0,CreateItemIndex,(BYTE)lpObj->EventInventory[SourceSlot].m_Level,0,0,0,0,lpObj->Index,0,0,0,0,0,lpObj->EventInventory[SourceSlot].m_SocketOptionBonus,0);
	}
	else
	{
		this->GCEventItemDurSend(lpObj->Index,TargetSlot,(BYTE)lpObj->EventInventory[TargetSlot].m_Durability);
	}

	if(lpObj->EventInventory[SourceSlot].m_Durability < 1)
	{
		this->EventInventoryDelItem(lpObj->Index,SourceSlot);
		this->GCEventItemDeleteSend(lpObj->Index,SourceSlot,1);
	}

	return 1;

	#else

	return 0;

	#endif
}

void CEventInventory::CGEventItemGetRecv(PMSG_EVENT_ITEM_GET_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_EVENT_ITEM_GET_SEND pMsg;

	pMsg.header.set(0x4D,0x00,sizeof(pMsg));

	pMsg.result = 0xFF;

	memset(pMsg.ItemInfo,0,sizeof(pMsg.ItemInfo));

	if(lpObj->DieRegen != 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gServerInfo.m_EventInventorySwitch == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gDuel.GetDuelArenaBySpectator(aIndex) != 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gObjFixEventInventoryPointer(aIndex);

	if(lpObj->Transaction == 1)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	int index = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(MAP_ITEM_RANGE(index) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(MAP_RANGE(lpObj->Map) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gMap[lpObj->Map].CheckItemGive(aIndex,index) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CMapItem* lpItem = &gMap[lpObj->Map].m_Item[index];

	if(lpItem->IsEventItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CItem item = (*lpItem);

	if(this->EventInventoryInsertItemStack(lpObj,&item) == 0)
	{
		if((pMsg.result=this->EventInventoryInsertItem(aIndex,item)) == 0xFF)
		{
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gMap[lpObj->Map].ItemGive(aIndex,index);

		gItemManager.ItemByteConvert(pMsg.ItemInfo,item);

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

		GCPartyItemInfoSend(aIndex,&item);
	}
	else
	{
		gMap[lpObj->Map].ItemGive(aIndex,index);

		pMsg.result = 0xFD;

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	}

	#endif
}

void CEventInventory::CGEventItemDropRecv(PMSG_EVENT_ITEM_DROP_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_EVENT_ITEM_DROP_SEND pMsg;

	pMsg.header.set(0x4D,0x01,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.slot = lpMsg->slot;

	if(lpObj->DieRegen != 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gServerInfo.m_EventInventorySwitch == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gDuel.GetDuelArenaBySpectator(aIndex) != 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gObjFixEventInventoryPointer(aIndex);

	if(lpObj->Transaction == 1)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(EVENT_INVENTORY_RANGE(lpMsg->slot) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CItem* lpItem = &lpObj->EventInventory[lpMsg->slot];

	if(lpItem->IsItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gItemMove.CheckItemMoveAllowDrop(lpItem->m_Index) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gItemBagManager.DropItemByItemIndex(lpItem->m_Index,lpItem->m_Level,lpObj,lpObj->Map,lpMsg->x,lpMsg->y) != 0)
	{
		this->EventInventoryDelItem(aIndex,lpMsg->slot);
	}
	else
	{
		if(gMap[lpObj->Map].ItemDrop(lpItem->m_Index,lpItem->m_Level,lpItem->m_Durability,lpMsg->x,lpMsg->y,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_Serial,aIndex,lpItem->m_PetItemLevel,lpItem->m_PetItemExp,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus,lpItem->m_PeriodicItemTime) == 0)
		{
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		this->EventInventoryDelItem(aIndex,lpMsg->slot);
	}

	pMsg.result = 1;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::CGEventInventoryOpenRecv(PMSG_EVENT_INVENTORY_OPEN_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(gServerInfo.m_EventInventorySwitch == 0)
	{
		return;
	}

	if(gMuRummy.IsMuRummyEventOn() != 0)
	{
		gMuRummy.CGReqMuRummyEventOpen((_tagPMSG_REQ_MURUMMY_EVENT_OPEN*)lpMsg,aIndex);
		return;
	}

	CTime ExpireTime(gServerInfo.m_EventInventoryExpireYear,gServerInfo.m_EventInventoryExpireMonth,gServerInfo.m_EventInventoryExpireDay,0,0,0,-1);

	DWORD time = (DWORD)(ExpireTime.GetTime()-CTime::GetTickCount().GetTime());

	PMSG_EVENT_INVENTORY_OPEN_SEND pMsg;

	pMsg.header.set(0x4D,0x0F,sizeof(pMsg));

	pMsg.result = 3;

	pMsg.time[0] = SET_NUMBERHB(SET_NUMBERHW(time));

	pMsg.time[1] = SET_NUMBERLB(SET_NUMBERHW(time));

	pMsg.time[2] = SET_NUMBERHB(SET_NUMBERLW(time));

	pMsg.time[3] = SET_NUMBERLB(SET_NUMBERLW(time));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::GCEventItemListSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	PMSG_EVENT_ITEM_LIST_SEND pMsg;

	pMsg.header.set(0x4D,0x02,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	PMSG_EVENT_ITEM_LIST info;

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		if(lpObj->EventInventory[n].IsItem() == 0 || lpObj->EventInventory[n].m_ItemExist == 0)
		{
			continue;
		}

		info.slot = n;

		gItemManager.ItemByteConvert(info.ItemInfo,lpObj->EventInventory[n]);

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

void CEventInventory::GCEventItemDurSend(int aIndex,BYTE slot,BYTE dur) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	PMSG_EVENT_ITEM_DUR_SEND pMsg;

	pMsg.header.set(0x4D,0x03,sizeof(pMsg));

	pMsg.slot = slot;

	pMsg.dur = dur;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::GCEventItemDeleteSend(int aIndex,BYTE slot,BYTE flag) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	PMSG_EVENT_ITEM_DELETE_SEND pMsg;

	pMsg.header.set(0x4D,0x04,sizeof(pMsg));

	pMsg.slot = slot;

	pMsg.flag = flag;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::GCEventItemModifySend(int aIndex,BYTE slot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->EventInventory[slot].IsItem() == 0)
	{
		return;
	}

	PMSG_EVENT_ITEM_MODIFY_SEND pMsg;

	pMsg.header.set(0x4D,0x05,sizeof(pMsg));

	pMsg.slot = slot;

	gItemManager.ItemByteConvert(pMsg.ItemInfo,lpObj->EventInventory[slot]);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::GCEventItemChangeSend(int aIndex,BYTE slot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	PMSG_EVENT_ITEM_CHANGE_SEND pMsg;

	pMsg.header.set(0x4D,0x06,sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);
	pMsg.index[1] = SET_NUMBERLB(aIndex);

	gItemManager.ItemByteConvert(pMsg.ItemInfo,lpObj->EventInventory[slot]);

	pMsg.ItemInfo[1] = slot*16;
	pMsg.ItemInfo[1] |= ((lpObj->EventInventory[slot].m_Level-1)/2) & 0x0F;

	MsgSendV2(lpObj,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::GCEventInventoryEnableSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(gServerInfo.m_EventInventorySwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	PMSG_EVENT_INVENTORY_ENABLE_SEND pMsg;

	pMsg.header.set(0x4D,0x18,sizeof(pMsg));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::DGEventInventoryRecv(SDHP_EVENT_INVENTORY_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGEventInventoryRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(lpObj->LoadEventInventory != 0)
	{
		return;
	}

	lpObj->LoadEventInventory = 1;

	memset(lpObj->EventInventory,0xFF,EVENT_INVENTORY_SIZE);

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++)
	{
		CItem item;

		lpObj->EventInventory[n].Clear();

		if(gItemManager.ConvertItemByte(&item,lpMsg->EventInventory[n]) != 0){this->EventInventoryAddItem(lpObj->Index,item,n);}
	}

	this->GCEventItemListSend(lpObj->Index);

	this->GCEventInventoryEnableSend(lpObj->Index);

	#endif
}

void CEventInventory::GDEventInventorySend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(gObj[aIndex].LoadEventInventory != 0)
	{
		return;
	}

	SDHP_EVENT_INVENTORY_SEND pMsg;

	pMsg.header.set(0x26,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CEventInventory::GDEventInventorySaveSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->LoadEventInventory == 0)
	{
		return;
	}

	SDHP_EVENT_INVENTORY_SAVE_SEND pMsg;

	pMsg.header.set(0x26,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	for(int n=0;n < EVENT_INVENTORY_SIZE;n++){gItemManager.DBItemByteConvert(pMsg.EventInventory[n],&lpObj->EventInventory[n]);}

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}
