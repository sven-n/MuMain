// EventInventory.h: interface for the CEventInventory class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

#define EVENT_INVENTORY_SIZE 32

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_EVENT_INVENTORY_RECV
{
	PSBMSG_HEAD header; // C1:26:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_EVENT_INVENTORY_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:26:30
	WORD index;
	char account[11];
	char name[11];
	BYTE EventInventory[EVENT_INVENTORY_SIZE][16];
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_EVENT_INVENTORY_SEND
{
	PSWMSG_HEAD header; // C2:26:00
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
	void GDEventInventoryRecv(SDHP_EVENT_INVENTORY_RECV* lpMsg,int index);
	void GDEventInventorySaveRecv(SDHP_EVENT_INVENTORY_SAVE_RECV* lpMsg);
};

extern CEventInventory gEventInventory;
