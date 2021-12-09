// ItemValue.h: interface for the CItemValue class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"

struct ITEM_VALUE_INFO
{
	int Index;
	int Level;
	int Grade;
	int Value;
	int Coin1;
	int Coin2;
	int Coin3;
	int Sell;
};

class CItemValue
{
public:
	CItemValue();
	virtual ~CItemValue();
	void Load(char* path);
	bool GetItemValue(CItem* lpItem,int* value);
	bool GetItemValueNew(CItem* lpItem,int* value,int* coin1,int* coin2,int* coin3,int* sell);
private:
	std::vector<ITEM_VALUE_INFO> m_ItemValueInfo;
};

extern CItemValue gItemValue;
