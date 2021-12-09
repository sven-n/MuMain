// Warehouse.h: interface for the CWarehouse class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Warehouse.h"
#include "Protocol.h"
#include "User.h"

#define MAX_WAREHOUSE_MONEY 100000000

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_WAREHOUSE_MONEY_RECV
{
	PBMSG_HEAD header; // C1:81
	BYTE type;
	DWORD money;
};

struct PMSG_WAREHOUSE_PASSWORD_RECV
{
	PBMSG_HEAD header; // C1:83
	BYTE type;
	WORD password;
	char PersonalCode[10];
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_WAREHOUSE_MONEY_SEND
{
	PBMSG_HEAD header; // C1:81
	BYTE result;
	DWORD WarehouseMoney;
	DWORD InventoryMoney;
};

struct PMSG_WAREHOUSE_STATE_SEND
{
	PBMSG_HEAD header; // C1:83
	BYTE state;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_WAREHOUSE_ITEM_RECV
{
	PSWMSG_HEAD header; // C2:05:00
	WORD index;
	char account[11];
	BYTE WarehouseItem[WAREHOUSE_SIZE][16];
	UINT WarehouseMoney;
	WORD WarehousePassword;
};

struct SDHP_WAREHOUSE_FREE_RECV
{
	PSBMSG_HEAD header; // C1:05:01
	WORD index;
	char account[11];
};

struct SDHP_WAREHOUSEGUILD_CLOSE_RECV
{
	PSBMSG_HEAD header; // C1:05:01
	WORD index;
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_WAREHOUSE_ITEM_SEND
{
	PSBMSG_HEAD header; // C1:05:00
	WORD index;
	char account[11];
	UINT WarehouseNumber;
};

struct SDHP_WAREHOUSE_ITEM_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:05:30
	WORD index;
	char account[11];
	BYTE WarehouseItem[WAREHOUSE_SIZE][16];
	UINT WarehouseMoney;
	UINT WarehousePassword;
	UINT WarehouseNumber;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CWarehouse
{
public:
	CWarehouse();
	virtual ~CWarehouse();
	int GetWarehouseTaxMoney(int level,int lock);
	void CGWarehouseMoneyRecv(PMSG_WAREHOUSE_MONEY_RECV* lpMsg,int aIndex);
	void CGWarehouseClose(int aIndex);
	void CGWarehousePasswordRecv(PMSG_WAREHOUSE_PASSWORD_RECV* lpMsg,int aIndex);
	void GCWarehouseListSend(LPOBJ lpObj);
	void GCWarehouseMoneySend(int aIndex,BYTE result,int InventoryMoney,int WarehouseMoney);
	void GCWarehouseStateSend(int aIndex,BYTE state);
	void DGWarehouseItemRecv(SDHP_WAREHOUSE_ITEM_RECV* lpMsg);
	void DGWarehouseFreeRecv(SDHP_WAREHOUSE_FREE_RECV* lpMsg);
	void GDWarehouseItemSend(int aIndex,char* account);
	void GDWarehouseItemSaveSend(int aIndex);
	void DGWarehouseGuildItemRecv(SDHP_WAREHOUSE_ITEM_RECV* lpMsg);
	void DGWarehouseGuildFreeRecv(SDHP_WAREHOUSE_FREE_RECV* lpMsg);
	void GDWarehouseGuildItemSend(int aIndex,char* account);
	void GDWarehouseGuildItemSaveSend(int aIndex);
	void GDWarehouseGuildClose(SDHP_WAREHOUSEGUILD_CLOSE_RECV* lpMsg);
};

extern CWarehouse gWarehouse;
