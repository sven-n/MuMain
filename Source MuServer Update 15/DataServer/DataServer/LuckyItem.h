// LuckyItem.h: interface for the CLuckyItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_LUCKY_ITEM_RECV
{
	PSWMSG_HEAD header; // C2:22:00
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_LUCKY_ITEM1
{
	BYTE slot;
	DWORD serial;
};

struct SDHP_LUCKY_ITEM_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:22:30
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_LUCKY_ITEM_SAVE
{
	DWORD serial;
	DWORD DurabilitySmall;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_LUCKY_ITEM_SEND
{
	PSWMSG_HEAD header; // C2:22:00
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_LUCKY_ITEM2
{
	BYTE slot;
	DWORD serial;
	DWORD DurabilitySmall;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CLuckyItem
{
public:
	CLuckyItem();
	virtual ~CLuckyItem();
	void GDLuckyItemRecv(SDHP_LUCKY_ITEM_RECV* lpMsg,int index);
	void GDLuckyItemSaveRecv(SDHP_LUCKY_ITEM_SAVE_RECV* lpMsg);
};

extern CLuckyItem gLuckyItem;
