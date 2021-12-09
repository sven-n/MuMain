// ItemValue.h: interface for the CItemValue class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

struct ITEM_VALUETRADE_INFO
{
	int Index;
	int Value;
	int Coin1;
	int Coin2;
	int Coin3;
};

class CItemValueTrade
{
public:
	CItemValueTrade();
	virtual ~CItemValueTrade();
	void Load(char* path);
	bool CheckItemValueTrade(LPOBJ lpObj,LPOBJ lpTarget);
private:
	std::vector<ITEM_VALUETRADE_INFO> m_ItemValueTradeInfo;
};

extern CItemValueTrade gItemValueTrade;
