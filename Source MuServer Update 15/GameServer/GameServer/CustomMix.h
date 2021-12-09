// CustomMix.h: interface for the CCustomJewel class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "User.h"

struct CUSTOM_MIX_INFO
{
	int Index;
	int MixMoney;
	int SuccessRate[4];
};

struct CUSTOM_MIX_ITEM_INFO
{
	int Index;
	int Group;
	int Count;
	int ItemIndex;
	int Level;
	int Option1;
	int Option2;
	int Option3;
	int NewOption;
	int SetOption;
};

struct CUSTOM_MIX_REWARD_INFO
{
	int Index;
	int Group;
	int BagSpecial;
};

class CCustomMix
{
public:
	CCustomMix();
	virtual ~CCustomMix();
	void Init();
	void Load(char* path);
	bool CheckCustomMix(int index);
	int GetCustomMixSuccessRate(int index,int AccountLevel);
	int GetCustomMixMoney(int index);
	int GetCustomMixReward(int index);
	bool CheckItem(LPOBJ lpObj,int index);
public:
	std::vector<CUSTOM_MIX_INFO>		m_CustomMixInfo;
	std::vector<CUSTOM_MIX_ITEM_INFO>	m_CustomMixItemInfo;
	std::vector<CUSTOM_MIX_REWARD_INFO>	m_CustomMixRewardInfo;
};

extern CCustomMix gCustomMix;
