// PersonalShop.h: interface for the CPersonalShop class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ItemManager.h"
#include "Protocol.h"

#if(GAMESERVER_UPDATE>=802)
#define MAX_PERSONAL_SHOP_MONEY 999999999
#else
#define MAX_PERSONAL_SHOP_MONEY 99999999
#endif

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_PSHOP_SET_ITEM_PRICE_RECV
{
	PSBMSG_HEAD header; // C1:3F:01
	BYTE slot;
	BYTE price[4];
	#if(GAMESERVER_UPDATE>=802)
	BYTE JoBPrice[2];
	BYTE JoSPrice[2];
	BYTE JoCPrice[2];
	#endif
};

struct PMSG_PSHOP_OPEN_RECV
{
	PSBMSG_HEAD header; // C1:3F:02
	char text[36];
};

struct PMSG_PSHOP_ITEM_LIST_RECV
{
	PSBMSG_HEAD header; // C1:3F:05
	BYTE index[2];
	char name[10];
};

struct PMSG_PSHOP_BUY_ITEM_RECV
{
	PSBMSG_HEAD header; // C1:3F:06
	BYTE index[2];
	char name[10];
	BYTE slot;
	#if(GAMESERVER_UPDATE>=802)
	#pragma pack(1)
	BYTE type;
	DWORD ItemIndex;
	DWORD value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
	#pragma pack()
	#endif
};

struct PMSG_PSHOP_LEAVE_RECV
{
	PSBMSG_HEAD header; // C1:3F:07
	BYTE index[2];
	char name[10];
};

struct PMSG_PSHOP_SEARCH_RECV
{
	PSBMSG_HEAD header; // C1:EC:31
	DWORD count;
	WORD ItemIndex;
};

struct PMSG_PSHOP_SEARCH_LOG_RECV
{
	PSBMSG_HEAD header; // C3:EC:33
	DWORD index;
	BYTE type;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_PSHOP_VIEWPORT_SEND
{
	PSWMSG_HEAD header; // C2:3F:00
	BYTE count;
};

struct PMSG_PSHOP_VIEWPORT
{
	BYTE index[2];
	char text[36];
};

struct PMSG_PSHOP_SET_ITEM_PRICE_SEND
{
	PSBMSG_HEAD header; // C1:3F:01
	BYTE result;
	BYTE slot;
};

struct PMSG_PSHOP_OPEN_SEND
{
	PSBMSG_HEAD header; // C1:3F:02
	BYTE result;
};

struct PMSG_PSHOP_CLOSE_SEND
{
	PSBMSG_HEAD header; // C1:3F:03
	BYTE result;
	BYTE index[2];
};

struct PMSG_PSHOP_ITEM_LIST_SEND
{
	PSWMSG_HEAD header; // C2:3F:[05:13]
	BYTE result;
	BYTE index[2];
	char name[10];
	char text[36];
	BYTE count;
};

struct PMSG_PSHOP_ITEM_LIST
{
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
	DWORD value;
	#if(GAMESERVER_UPDATE>=802)
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
	#endif
};

struct PMSG_PSHOP_BUY_ITEM_SEND
{
	PSBMSG_HEAD header; // C1:3F:06
	BYTE result;
	BYTE index[2];
	BYTE ItemInfo[MAX_ITEM_INFO];
	BYTE slot;
};

struct PMSG_PSHOP_SELL_ITEM_SEND
{
	PSBMSG_HEAD header; // C1:3F:08
	BYTE slot;
	char name[10];
};

struct PMSG_PSHOP_TEXT_CHANGE_SEND
{
	PSBMSG_HEAD header; // C1:3F:10
	BYTE index[2];
	char text[36];
	char name[10];
};

struct PMSG_PSHOP_LEAVE_SEND
{
	PSBMSG_HEAD header; // C1:3F:12
	BYTE index[2];
};

struct PMSG_PSHOP_SEARCH_SEND
{
	#pragma pack(1)
	PSWMSG_HEAD header; // C2:EC:31
	DWORD count;
	BYTE flag;
	#pragma pack()
};

struct PMSG_PSHOP_SEARCH
{
	BYTE index[2];
	char name[11];
	char text[37];
};

struct PMSG_PSHOP_ITEM_VALUE_SEND
{
	PSWMSG_HEAD header; // C2:EC:32
	BYTE MoneyCommisionRate;
	BYTE JewelCommisionRate;
	BYTE count;
};

struct PMSG_PSHOP_ITEM_VALUE
{
	#pragma pack(1)
	DWORD slot;
	DWORD serial;
	DWORD value;
	WORD JoBValue;
	WORD JoSValue;
	WORD JoCValue;
	#pragma pack()
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PSHOP_ITEM_VALUE_RECV
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
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PSHOP_ITEM_VALUE_SEND
{
	PSBMSG_HEAD header; // C1:25:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PSHOP_ITEM_VALUE_SAVE_SEND
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

struct SDHP_PSHOP_ITEM_VALUE_INSERT_SAVE_SEND
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

struct SDHP_PSHOP_ITEM_VALUE_DELETE_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:25:32
	WORD index;
	char account[11];
	char name[11];
	DWORD slot;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CPersonalShop
{
public:
	CPersonalShop();
	virtual ~CPersonalShop();
	bool CheckPersonalShop(int aIndex);
	bool CheckPersonalShopOpen(int aIndex);
	bool CheckPersonalShopViewport(int aIndex,int bIndex);
	bool CheckPersonalShopSearchItem(int aIndex,int ItemIndex);
	void GetRequireJewelCount(LPOBJ lpObj,int* count,int* table,int type,int value);
	void GetPaymentJewelCount(LPOBJ lpObj,int* count,int* table,int type,int value);
	void SetRequireJewelCount(LPOBJ lpObj,int* table,int type);
	void SetPaymentJewelCount(LPOBJ lpObj,int* table,int type);
	void CGPShopSetItemPriceRecv(PMSG_PSHOP_SET_ITEM_PRICE_RECV* lpMsg,int aIndex);
	void CGPShopOpenRecv(PMSG_PSHOP_OPEN_RECV* lpMsg,int aIndex);
	void CGPShopCloseRecv(int aIndex);
	void CGPShopItemListRecv(PMSG_PSHOP_ITEM_LIST_RECV* lpMsg,int aIndex);
	void CGPShopBuyItemRecv(PMSG_PSHOP_BUY_ITEM_RECV* lpMsg,int aIndex);
	void CGPShopLeaveRecv(PMSG_PSHOP_LEAVE_RECV* lpMsg,int aIndex);
	void CGPShopSearchRecv(PMSG_PSHOP_SEARCH_RECV* lpMsg,int aIndex);
	void CGPShopSearchLogRecv(PMSG_PSHOP_SEARCH_LOG_RECV* lpMsg,int aIndex);
	void GCPShopViewportSend(int aIndex);
	void GCPShopSetItemPriceSend(int aIndex,BYTE result,BYTE slot);
	void GCPShopOpenSend(int aIndex,BYTE result);
	void GCPShopCloseSend(int aIndex,BYTE result);
	void GCPShopItemListSend(int aIndex,int bIndex,BYTE result,BYTE type);
	void GCPShopBuyItemSend(int aIndex,int bIndex,int slot,BYTE result);
	void GCPShopSellItemSend(int aIndex,int bIndex,int slot);
	void GCPShopTextChangeSend(int aIndex);
	void GCPShopLeaveSend(int aIndex,int bIndex);
	void GCPShopItemValueSend(int aIndex);
	void DGPShopItemValueRecv(SDHP_PSHOP_ITEM_VALUE_RECV* lpMsg);
	void GDPShopItemValueSend(int aIndex);
	void GDPShopItemValueSaveSend(int aIndex);
	void GDPShopItemValueInsertSaveSend(int aIndex,int slot,CItem* lpItem);
	void GDPShopItemValueDeleteSaveSend(int aIndex,int slot);
};

extern CPersonalShop gPersonalShop;
