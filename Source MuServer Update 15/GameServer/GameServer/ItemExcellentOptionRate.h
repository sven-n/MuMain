// ItemOption.h: interface for the CItemOption class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "User.h"

struct ITEM_EX_OPTION_RATE_INFO
{
	int Index;
	int OptionIndex;
	int ItemMinIndex;
	int ItemMaxIndex;
	char Name[32];
	int OptionRate;
};

class CItemExOptionRate
{
public:
	CItemExOptionRate();
	virtual ~CItemExOptionRate();
	void Load(char* path);
	bool GetExcellentOptionByRate(int ItemIndex, int NewOption, BYTE* OptionIndex);
private:
	std::map<int,ITEM_EX_OPTION_RATE_INFO> m_ItemExOptionRateInfo;
};

extern CItemExOptionRate gItemExOptionRate;
