// EventInventory.h: interface for the CEventInventory class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ItemManager.h"
#include "Protocol.h"

#define EVENT_INVENTORY_SIZE 32

#define EVENT_INVENTORY_RANGE(x) (((x)<0)?0:((x)>=EVENT_INVENTORY_SIZE)?0:1)

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_EVENT_ITEM_GET_RECV
{
	PSBMSG_HEAD header; // C3:4D:00
	BYTE index[2];
};

struct PMSG_EVENT_ITEM_DROP_RECV
{
	PSBMSG_HEAD header; // C3:4D:01
	BYTE x;
	BYTE y;
	BYTE slot;
};

struct PMSG_EVENT_INVENTORY_OPEN_RECV
{
	PSBMSG_HEAD header; // C3:4D:0F
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_EVENT_ITEM_GET_SEND
{
	PSBMSG_HEAD header; // C3:4D:00
	BYTE result;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_EVENT_ITEM_DROP_SEND
{
	PSBMSG_HEAD header; // C1:4D:01
	BYTE result;
	BYTE slot;
};

struct PMSG_EVENT_ITEM_LIST_SEND
{
	PSWMSG_HEAD header; // C4:4D:02
	BYTE count;
};

struct PMSG_EVENT_ITEM_LIST
{
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_EVENT_ITEM_DUR_SEND
{
	PSBMSG_HEAD header; // C1:4D:03
	BYTE slot;
	BYTE dur;
};

struct PMSG_EVENT_ITEM_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:4D:04
	BYTE slot;
	BYTE flag;
};

struct PMSG_EVENT_ITEM_MODIFY_SEND
{
	PSBMSG_HEAD header; // C1:4D:05
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_EVENT_ITEM_CHANGE_SEND
{
	PSBMSG_HEAD header; // C1:4D:06
	BYTE index[2];
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_EVENT_INVENTORY_OPEN_SEND
{
	PSBMSG_HEAD header; // C1:4D:0F
	BYTE result;
	BYTE time[4];
};

struct PMSG_EVENT_INVENTORY_ENABLE_SEND
{
	PSBMSG_HEAD header; // C1:4D:18
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_EVENT_INVENTORY_RECV
{
	PSWMSG_HEAD header; // C2:26:00
	WORD index;
	char account[11];
	char name[11];
	BYTE EventInventory[EVENT_INVENTORY_SIZE][16];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_EVENT_INVENTORY_SEND
{
	PSBMSG_HEAD header; // C1:26:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_EVENT_INVENTORY_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:26:30
	WORD index;
	char account[11];
	char name[11];
	BYTE EventInventory[EVENT_INVENTORY_SIZE][16];
};

//**********************************************//
//**********************************************//
//**********************************************//

class CEventInventory
{
public:
	CEventInventory();
	virtual ~CEventInventory();
	bool IsEventItem(int ItemIndex);
	bool IsEventItem(CItem* lpItem);
	void EventInventoryItemSet(int aIndex,int slot,BYTE type);
	BYTE EventInventoryRectCheck(int aIndex,int x,int y,int width,int height);
	BYTE EventInventoryInsertItem(int aIndex,CItem item);
	BYTE EventInventoryAddItem(int aIndex,CItem item,int slot);
	void EventInventoryDelItem(int aIndex,int slot);
	bool EventInventoryInsertItemStack(LPOBJ lpObj,CItem* lpItem);
	bool EventInventoryAddItemStack(LPOBJ lpObj,int SourceSlot,int TargetSlot);
	void CGEventItemGetRecv(PMSG_EVENT_ITEM_GET_RECV* lpMsg,int aIndex);
	void CGEventItemDropRecv(PMSG_EVENT_ITEM_DROP_RECV* lpMsg,int aIndex);
	void CGEventInventoryOpenRecv(PMSG_EVENT_INVENTORY_OPEN_RECV* lpMsg,int aIndex);
	void GCEventItemListSend(int aIndex);
	void GCEventItemDurSend(int aIndex,BYTE slot,BYTE dur);
	void GCEventItemDeleteSend(int aIndex,BYTE slot,BYTE flag);
	void GCEventItemModifySend(int aIndex,BYTE slot);
	void GCEventItemChangeSend(int aIndex,BYTE slot);
	void GCEventInventoryEnableSend(int aIndex);
	void DGEventInventoryRecv(SDHP_EVENT_INVENTORY_RECV* lpMsg);
	void GDEventInventorySend(int aIndex);
	void GDEventInventorySaveSend(int aIndex);
};

extern CEventInventory gEventInventory;
