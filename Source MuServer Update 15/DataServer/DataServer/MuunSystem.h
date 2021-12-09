// MuunSystem.h: interface for the CMuunSystem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

#define MUUN_INVENTORY_SIZE 62

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_MUUN_INVENTORY_RECV
{
	PSBMSG_HEAD header; // C1:27:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_MUUN_INVENTORY_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:27:30
	WORD index;
	char account[11];
	char name[11];
	BYTE MuunInventory[MUUN_INVENTORY_SIZE][16];
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_MUUN_INVENTORY_SEND
{
	PSWMSG_HEAD header; // C2:27:00
	WORD index;
	char account[11];
	char name[11];
	BYTE MuunInventory[MUUN_INVENTORY_SIZE][16];
};

//**********************************************//
//**********************************************//
//**********************************************//

class CMuunSystem
{
public:
	CMuunSystem();
	virtual ~CMuunSystem();
	void GDMuunInventoryRecv(SDHP_MUUN_INVENTORY_RECV* lpMsg,int index);
	void GDMuunInventorySaveRecv(SDHP_MUUN_INVENTORY_SAVE_RECV* lpMsg);
};

extern CMuunSystem gMuunSystem;
