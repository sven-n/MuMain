// SetItemType.h: interface for the CSetItemType class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_SET_ITEM_TYPE 1000
#define MAX_SET_ITEM_OPTION_INDEX 2

struct SET_ITEM_TYPE_INFO
{
	int Index;
	int StatType;
	int OptionIndex[MAX_SET_ITEM_OPTION_INDEX];
};

class CSetItemType
{
public:
	CSetItemType();
	virtual ~CSetItemType();
	void Init();
	void Load(char* path);
	void SetInfo(SET_ITEM_TYPE_INFO info);
	SET_ITEM_TYPE_INFO* GetInfo(int index);
	bool CheckSetItemType(int index);
	int GetSetItemOptionIndex(int index,int number);
	int GetSetItemStatType(int index);
	int GetRandomSetItem();
	void MakeRandomSetItem(int aIndex,int map,int x,int y);
private:
	SET_ITEM_TYPE_INFO m_SetItemTypeInfo[MAX_SET_ITEM_TYPE];
	int m_count;
};

extern CSetItemType gSetItemType;
