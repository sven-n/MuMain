// CashShop.h: interface for the CCashShop class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

#define MAX_CASH_SHOP_ITEM 2000
#define MAX_CASH_SHOP_PACKAGE_PRODUCT 10
#define MAX_CASH_SHOP_PAGE 5
#define MAX_CASH_SHOP_PAGE_ITEM 9

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CASH_SHOP_OPEN_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:02
	BYTE OpenType;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_BUY_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:03
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_GIF_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:04
	DWORD PackageMainIndex;
	DWORD Category;
	DWORD ProductMainIndex;
	DWORD SaleZone;
	WORD ItemIndex;
	UINT CoinIndex;
	BYTE MileageFlag;
	char GiftName[11];
	char GiftText[200];
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_NUM_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:05
	UINT InventoryPage;
	BYTE InventoryType;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_USE_RECV
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:0B
	DWORD BaseItemCode;
	DWORD MainItemCode;
	WORD ItemIndex;
	BYTE ProductType;
	#pragma pack()
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CASH_SHOP_POINT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:01
	BYTE result;
	double WCoinC[2];
	double WCoinP[2];
	double GoblinPoint;
	#pragma pack()
};

struct PMSG_CASH_SHOP_OPEN_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:02
	BYTE result;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_BUY_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:03
	BYTE result;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_GIF_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:04
	BYTE result;
	#pragma pack()
};

struct PMSG_CASH_SHOP_VERSION_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:[0C:15]
	WORD version[3];
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_NUM_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:06
	WORD ItemCount;
	WORD PageCount;
	WORD CurPage;
	WORD MaxPage;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_USE_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:0B
	BYTE result;
	#pragma pack()
};

struct PMSG_CASH_SHOP_ITEM_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:0D
	DWORD BaseItemCode;
	DWORD MainItemCode;
	DWORD PackageMainIndex;
	DWORD ProductBaseIndex;
	DWORD ProductMainIndex;
	double CoinValue;
	BYTE ProductType;
	#pragma pack()
};

struct PMSG_CASH_SHOP_GIFT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:0E
	DWORD BaseItemCode;
	DWORD MainItemCode;
	DWORD PackageMainIndex;
	DWORD ProductBaseIndex;
	DWORD ProductMainIndex;
	double CoinValue;
	BYTE ProductType;
	char GiftName[11];
	char GiftText[200];
	#pragma pack()
};

struct PMSG_CASH_SHOP_PERIODIC_ITEM_COUNT_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:11
	BYTE count;
	#pragma pack()
};

struct PMSG_CASH_SHOP_PERIODIC_ITEM_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:12
	WORD index;
	WORD slot;
	DWORD time;
	#pragma pack()
};

struct PMSG_COIN_SEND
{
	#pragma pack(1)
	PSBMSG_HEAD header; // C1:D2:01
	int  Coin1;
	int  Coin2;
	int  Coin3;
	#pragma pack()
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_CASH_SHOP_POINT_RECV
{
	PSBMSG_HEAD header; // C1:18:00
	WORD index;
	char account[11];
	BYTE result;
	DWORD WCoinC;
	DWORD WCoinP;
	DWORD GoblinPoint;
};

struct SDHP_CASH_SHOP_ITEM_BUY_RECV
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

struct SDHP_CASH_SHOP_ITEM_GIF_RECV
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

struct SDHP_CASH_SHOP_ITEM_NUM_RECV
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

struct SDHP_CASH_SHOP_ITEM_USE_RECV
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
	DWORD time;
};

struct SDHP_CASH_SHOP_RECIEVE_POINT_RECV
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
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_CASH_SHOP_POINT_SEND
{
	PSBMSG_HEAD header; // C1:18:00
	WORD index;
	char account[11];
};

struct SDHP_CASH_SHOP_ITEM_BUY_SEND
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

struct SDHP_CASH_SHOP_ITEM_GIF_SEND
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

struct SDHP_CASH_SHOP_ITEM_NUM_SEND
{
	PSBMSG_HEAD header; // C1:18:03
	WORD index;
	char account[11];
	UINT InventoryPage;
	BYTE InventoryType;
};

struct SDHP_CASH_SHOP_ITEM_USE_SEND
{
	PSBMSG_HEAD header; // C1:18:04
	WORD index;
	char account[11];
	DWORD BaseItemCode;
	DWORD MainItemCode;
	WORD ItemIndex;
	BYTE ProductType;
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
};

struct SDHP_CASH_SHOP_RECIEVE_POINT_SEND
{
	PSBMSG_HEAD header; // C1:18:06
	WORD index;
	char account[11];
	DWORD CallbackFunc;
	DWORD CallbackArg1;
	DWORD CallbackArg2;
};

struct SDHP_CASH_SHOP_ADD_POINT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:18:30
	WORD index;
	char account[11];
	char GiftAccount[11];
	DWORD AddWCoinC;
	DWORD AddWCoinP;
	DWORD AddGoblinPoint;
};

struct SDHP_CASH_SHOP_SUB_POINT_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:18:31
	WORD index;
	char account[11];
	char GiftAccount[11];
	DWORD SubWCoinC;
	DWORD SubWCoinP;
	DWORD SubGoblinPoint;
};

struct SDHP_CASH_SHOP_INSERT_ITEM_SAVE_SEND
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

struct SDHP_CASH_SHOP_DELETE_ITEM_SAVE_SEND
{
	PSBMSG_HEAD header; // C1:18:33
	WORD index;
	char account[11];
	char GiftAccount[11];
	DWORD BaseItemCode;
	DWORD MainItemCode;
};

struct SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_SEND
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
//**********************************************//
//**********************************************//

struct CASH_SHOP_INVENTORY
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
};

struct CASH_SHOP_PACKAGE_INFO
{
	int Category;
	int BaseIndex;
	int MainIndex;
	int ItemIndex;
	int CoinIndex;
	int CoinValue;
	int BonusGP;
	int ProductBaseIndex[MAX_CASH_SHOP_PACKAGE_PRODUCT];
	int ProductMainIndex[MAX_CASH_SHOP_PACKAGE_PRODUCT];
};

struct CASH_SHOP_PRODUCT_INFO
{
	int BaseIndex;
	int MainIndex;
	int CoinValue;
	int ItemIndex;
	int ItemLevel;
	int ItemOption1;
	int ItemOption2;
	int ItemOption3;
	int ItemNewOption;
	int ItemSetOption;
	int ItemHarmonyOption;
	int ItemOptionEx;
	int ItemSocketOption1;
	int ItemSocketOption2;
	int ItemSocketOption3;
	int ItemSocketOption4;
	int ItemSocketOption5;
	int ItemQuantity;
	int ItemDuration;
};

class CCashShop
{
public:
	CCashShop();
	virtual ~CCashShop();
	void LoadPackage(char* path);
	void LoadProduct(char* path);
	void SetPackageInfo(CASH_SHOP_PACKAGE_INFO info);
	void SetProductInfo(CASH_SHOP_PRODUCT_INFO info);
	CASH_SHOP_PACKAGE_INFO* GetPackageInfo(int index);
	CASH_SHOP_PRODUCT_INFO* GetProductInfo(int index);
	CASH_SHOP_PRODUCT_INFO* GetProductInfoByBaseIndex(int index);
	void MainProc();
	int GetPackageProductBaseIndexCount(CASH_SHOP_PACKAGE_INFO* lpPackageInfo);
	int GetPackageProductMainIndexCount(CASH_SHOP_PACKAGE_INFO* lpPackageInfo);
	void CGCashShopPointRecv(int aIndex);
	void CGCashShopOpenRecv(PMSG_CASH_SHOP_OPEN_RECV* lpMsg,int aIndex);
	void CGCashShopItemBuyRecv(PMSG_CASH_SHOP_ITEM_BUY_RECV* lpMsg,int aIndex);
	void CGCashShopItemGifRecv(PMSG_CASH_SHOP_ITEM_GIF_RECV* lpMsg,int aIndex);
	void CGCashShopItemNumRecv(PMSG_CASH_SHOP_ITEM_NUM_RECV* lpMsg,int aIndex);
	void CGCashShopItemUseRecv(PMSG_CASH_SHOP_ITEM_USE_RECV* lpMsg,int aIndex);
	void GCCashShopInitSend(LPOBJ lpObj);
	void GCCashShopScriptVersionSend(LPOBJ lpObj);
	void GCCashShopBannerVersionSend(LPOBJ lpObj);
	void GCCashShopItemSend(LPOBJ lpObj,int PageCount,CASH_SHOP_INVENTORY* CashInventory);
	void GCCashShopGiftSend(LPOBJ lpObj,int PageCount,CASH_SHOP_INVENTORY* CashInventory);
	void GCCashShopPeriodicItemSend(int aIndex,int index,int slot,int time);
	void DGCashShopPointRecv(SDHP_CASH_SHOP_POINT_RECV* lpMsg);
	void DGCashShopItemBuyRecv(SDHP_CASH_SHOP_ITEM_BUY_RECV* lpMsg);
	void DGCashShopItemGifRecv(SDHP_CASH_SHOP_ITEM_GIF_RECV* lpMsg);
	void DGCashShopItemNumRecv(SDHP_CASH_SHOP_ITEM_NUM_RECV* lpMsg);
	void DGCashShopItemUseRecv(SDHP_CASH_SHOP_ITEM_USE_RECV* lpMsg);
	void DGCashShopPeriodicItemRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV* lpMsg);
	void DGCashShopRecievePointRecv(SDHP_CASH_SHOP_RECIEVE_POINT_RECV* lpMsg);
	void GDCashShopPeriodicItemSend(int aIndex);
	void GDCashShopRecievePointSend(int aIndex,DWORD CallbackFunc,DWORD CallbackArg1,DWORD CallbackArg2);
	void GDCashShopAddPointSaveSend(int aIndex,char* GiftAccount,DWORD AddWCoinC,DWORD AddWCoinP,DWORD AddGoblinPoint);
	void GDCashShopSubPointSaveSend(int aIndex,char* GiftAccount,DWORD SubWCoinC,DWORD SubWCoinP,DWORD SubGoblinPoint);
	void GDCashShopInsertItemSaveSend(int aIndex,char* GiftAccount,BYTE InventoryType,DWORD PackageMainIndex,DWORD ProductBaseIndex,DWORD ProductMainIndex,double CoinValue,BYTE ProductType,char* GiftName,char* GiftText);
	void GDCashShopDeleteItemSaveSend(int aIndex,char* GiftAccount,DWORD BaseItemCode,DWORD MainItemCode);
	void GDCashShopPeriodicItemSaveSend(int aIndex);
	void GCSendCoin(LPOBJ lpObj, int coin1, int coin2, int coin3);
private:
	CASH_SHOP_PACKAGE_INFO m_CashShopPackageInfo[MAX_CASH_SHOP_ITEM];
	CASH_SHOP_PRODUCT_INFO m_CashShopProductInfo[MAX_CASH_SHOP_ITEM];
	int m_CashShopPackageCount;
	int m_CashShopProductCount;
};

extern CCashShop gCashShop;
