// CashShop.h: interface for the CCashShop class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

#define MAX_CASH_SHOP_PAGE 5
#define MAX_CASH_SHOP_PAGE_ITEM 9

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_CASH_SHOP_POINT_RECV
{
	PSBMSG_HEAD header; // C1:18:00
	WORD index;
	char account[11];
};

struct SDHP_CASH_SHOP_ITEM_BUY_RECV
{
	PSBMSG_HEAD header; // C1:18:01
	WORD index;
	char account[11];
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
};

struct SDHP_CASH_SHOP_ITEM_GIF_RECV
{
	PSWMSG_HEAD header; // C2:18:02
	WORD index;
	char account[11];
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	DWORD SaleZone;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	char GiftName[11];
	char GiftText[200];
};

struct SDHP_CASH_SHOP_ITEM_NUM_RECV
{
	PSBMSG_HEAD header; // C1:18:03
	WORD index;
	char account[11];
	UINT InventoryPage;
	BYTE InventoryType;
};

struct SDHP_CASH_SHOP_ITEM_USE_RECV
{
	PSBMSG_HEAD header; // C1:18:04
	WORD index;
	char account[11];
	DWORD BaseItemCode;
	DWORD MainItemCode;
	WORD ItemIndex;
	BYTE ProductType;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_RECV
{
	PSWMSG_HEAD header; // C2:18:05
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM1
{
	BYTE slot;
	DWORD serial;
};

struct SDHP_CASH_SHOP_RECIEVE_POINT_RECV
{
	PSBMSG_HEAD header; // C1:18:06
	WORD index;
	char account[11];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
};

struct SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:18:30
	WORD index;
	char account[11];
	char GiftAccount[11];
	DWORD AddWCoinC;
	DWORD AddWCoinP;
	DWORD AddGoblinPoint;
};

struct SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:18:31
	WORD index;
	char account[11];
	char GiftAccount[11];
	DWORD SubWCoinC;
	DWORD SubWCoinP;
	DWORD SubGoblinPoint;
};

struct SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:18:32
	WORD index;
	char account[11];
	char GiftAccount[11];
	BYTE InventoryType;
	DWORD PackageMainIndex;
	DWORD ProductBaseIndex;
	DWORD ProductMainIndex;
	double CoinValue;
	BYTE ProductType;
	char GiftName[11];
	char GiftText[200];
};

struct SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:18:33
	WORD index;
	char account[11];
	char GiftAccount[11];
	DWORD BaseItemCode;
	DWORD MainItemCode;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:18:34
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE
{
	DWORD serial;
	DWORD time;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_CASH_SHOP_POINT_SEND
{
	PSBMSG_HEAD header; // C1:18:00
	WORD index;
	char account[11];
	BYTE result;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
};

struct SDHP_CASH_SHOP_ITEM_BUY_SEND
{
	PSBMSG_HEAD header; // C1:18:01
	WORD index;
	char account[11];
	BYTE result;
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
	DWORD ItemCount;
};

struct SDHP_CASH_SHOP_ITEM_GIF_SEND
{
	PSWMSG_HEAD header; // C2:18:02
	WORD index;
	char account[11];
	BYTE result;
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	DWORD SaleZone;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	char GiftName[11];
	char GiftText[200];
	char GiftAccount[11];
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
	DWORD ItemCount;
};

struct SDHP_CASH_SHOP_ITEM_NUM_SEND
{
	PSWMSG_HEAD header; // C2:18:03
	WORD index;
	char account[11];
	BYTE result;
	UINT InventoryPage;
	BYTE InventoryType;
	DWORD ItemCount;
	DWORD PageCount;
	struct
	{
		DWORD BaseItemCode;
		DWORD MainItemCode;
		DWORD PackageMainIndex;
		DWORD ProductBaseIndex;
		DWORD ProductMainIndex;
		double CoinValue;
		BYTE ProductType;
		char GiftName[11];
		char GiftText[200];
	}ProductInfo[MAX_CASH_SHOP_PAGE_ITEM];
};

struct SDHP_CASH_SHOP_ITEM_USE_SEND
{
	PSWMSG_HEAD header; // C2:18:04
	WORD index;
	char account[11];
	BYTE result;
	DWORD BaseItemCode;
	DWORD MainItemCode;
	WORD ItemIndex;
	BYTE ProductType;
	struct
	{
		DWORD BaseItemCode;
		DWORD MainItemCode;
		DWORD PackageMainIndex;
		DWORD ProductBaseIndex;
		DWORD ProductMainIndex;
		double CoinValue;
		BYTE ProductType;
		char GiftName[11];
		char GiftText[200];
	}ProductInfo;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SEND
{
	PSWMSG_HEAD header; // C2:18:05
	WORD index;
	char account[11];
	BYTE count;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM2
{
	BYTE slot;
	DWORD serial;
	DWORD time;
};

struct SDHP_CASH_SHOP_RECIEVE_POINT_SEND
{
	PSBMSG_HEAD header; // C1:18:06
	WORD index;
	char account[11];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CCashShop
{
public:
	CCashShop();
	virtual ~CCashShop();
	void GDCashShopPointRecv(SDHP_CASH_SHOP_POINT_RECV* lpMsg,int index);
	void GDCashShopItemBuyRecv(SDHP_CASH_SHOP_ITEM_BUY_RECV* lpMsg,int index);
	void GDCashShopItemGifRecv(SDHP_CASH_SHOP_ITEM_GIF_RECV* lpMsg,int index);
	void GDCashShopItemNumRecv(SDHP_CASH_SHOP_ITEM_NUM_RECV* lpMsg,int index);
	void GDCashShopItemUseRecv(SDHP_CASH_SHOP_ITEM_USE_RECV* lpMsg,int index);
	void GDCashShopPeriodicItemRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV* lpMsg,int index);
	void GDCashShopRecievePointRecv(SDHP_CASH_SHOP_RECIEVE_POINT_RECV* lpMsg,int index);
	void GDCashShopAddPointSaveRecv(SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV* lpMsg);
	void GDCashShopSubPointSaveRecv(SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV* lpMsg);
	void GDCashShopInsertItemSaveRecv(SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV* lpMsg);
	void GDCashShopDeleteItemSaveRecv(SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV* lpMsg);
	void GDCashShopPeriodicItemSaveRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV* lpMsg);
};

extern CCashShop gCashShop;
