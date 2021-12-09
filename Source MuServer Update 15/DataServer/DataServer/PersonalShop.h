// PersonalShop.h: interface for the CPersonalShop class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PSHOP_ITEM_VALUE_RECV
{
	PSBMSG_HEAD header; // C1:25:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PSHOP_ITEM_VALUE_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:25:30
	WORD index;
	char account[11];
	char name[11];
	BYTE count;
};

struct SDHP_PSHOP_ITEM_VALUE_SAVE
{
	DWORD slot;
	DWORD serial;
	DWORD value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
};

struct SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:25:31
	WORD index;
	char account[11];
	char name[11];
	DWORD slot;
	DWORD serial;
	DWORD value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
};

struct SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:25:32
	WORD index;
	char account[11];
	char name[11];
	DWORD slot;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PSHOP_ITEM_VALUE_SEND
{
	PSWMSG_HEAD header; // C2:25:00
	WORD index;
	char account[11];
	char name[11];
	BYTE count;
};

struct SDHP_PSHOP_ITEM_VALUE
{
	DWORD slot;
	DWORD serial;
	DWORD value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CPersonalShop
{
public:
	CPersonalShop();
	virtual ~CPersonalShop();
	void GDPShopItemValueRecv(SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg,int index);
	void GDPShopItemValueSaveRecv(SDHP_PSHOP_ITEM_VALUE_SAVE_RECV* lpMsg);
	void GDPShopItemValueInsertSaveRecv(SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_RECV* lpMsg);
	void GDPShopItemValueDeleteSaveRecv(SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_RECV* lpMsg);
};

extern CPersonalShop gPersonalShop;
