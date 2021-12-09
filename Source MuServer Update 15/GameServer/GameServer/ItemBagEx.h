// ItemBagEx.h: interface for the CItemBagEx class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "Item.h"
#include "User.h"

struct ITEM_BAG_EX_ITEM_INFO
{
	int Index;
	int Level;
	int Grade;
	int Option0;
	int Option1;
	int Option2;
	int Option3;
	int Option4;
	int Option5;
	int Option6;
	int Duration;
};

struct ITEM_BAG_EX_DROP_INFO
{
	int Index;
	int Section;
	int SectionRate;
	int MoneyAmount;
	int OptionValue;
	int RequireClass[MAX_CLASS];
};

struct ITEM_BAG_EX_INFO
{
	int Index;
	int DropRate;
	std::vector<ITEM_BAG_EX_DROP_INFO> DropInfo;
};

class CItemBagEx
{
public:
	CItemBagEx();
	virtual ~CItemBagEx();
	void Load(char* path);
	bool GetItem(LPOBJ lpObj,CItem* lpItem);
	bool DropItem(LPOBJ lpObj,int map,int x,int y);
	bool CheckDropClass(LPOBJ lpObj,int type,int RequireClass[MAX_CLASS]);
	bool GetRandomItemDropLocation(int map,int* ox,int* oy,int tx,int ty,int count);
public:
	std::map<int,ITEM_BAG_EX_INFO> m_ItemBagInfo;
	std::map<int,std::vector<ITEM_BAG_EX_ITEM_INFO>> m_ItemBagItemInfo;
};
