// ItemDrop.h: interface for the CItemDrop class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

struct ITEM_DROP_INFO
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
	int MapNumber;
	int MonsterClass;
	int MonsterLevelMin;
	int MonsterLevelMax;
	int DropRate;
};

class CItemDrop
{
public:
	CItemDrop();
	virtual ~CItemDrop();
	void Load(char* path);
	int DropItem(LPOBJ lpObj,LPOBJ lpTarget);
	int GetItemDropRate(LPOBJ lpObj,LPOBJ lpTarget,int ItemIndex,int ItemLevel,int DropRate);
private:
	std::vector<ITEM_DROP_INFO> m_ItemDropInfo;
};

extern CItemDrop gItemDrop;
