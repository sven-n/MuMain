// ShopManager.h: interface for the CShopManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Shop.h"

#define MAX_SHOP 100

#define SHOP_RANGE(x) (((x)<0)?0:((x)>=MAX_SHOP)?0:1)

struct SHOP_MANAGER_INFO
{
	int Index;
	int MonsterClass;
	int Map;
	int X;
	int Y;
	CShop Shop;
};

class CShopManager
{
public:
	CShopManager();
	virtual ~CShopManager();
	void Load(char* path);
	void LoadShop();
	void ReloadShopInterface();
	long GetShopNumber(int MonsterClass,int Map,int X,int Y);
	bool GetItemByIndex(int index,CItem* lpItem,int slot);
	bool GetItemByMonsterClass(int MonsterClass,CItem* lpItem,int slot);
	long GetItemCountByIndex(int index);
	long GetItemCountByMonsterClass(int MonsterClass);
	bool GCShopItemListSendByIndex(int index,int aIndex);
	bool GCShopItemListSendByMonsterClass(int MonsterClass,int Map,int X,int Y,int aIndex);
private:
	std::map<int,SHOP_MANAGER_INFO> m_ShopManagerInfo;
};

extern CShopManager gShopManager;
