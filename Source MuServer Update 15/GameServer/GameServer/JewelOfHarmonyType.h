// JewelOfHarmonyType.h: interface for the CJewelOfHarmonyType class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"

#define MAX_JEWEL_OF_HARMONY_TYPE 1000

struct JEWEL_OF_HARMONY_TYPE_INFO
{
	int Index;
	int Level;
};

class CJewelOfHarmonyType
{
public:
	CJewelOfHarmonyType();
	virtual ~CJewelOfHarmonyType();
	void Init();
	void Load(char* path);
	void SetInfo(JEWEL_OF_HARMONY_TYPE_INFO info);
	JEWEL_OF_HARMONY_TYPE_INFO* GetInfo(int index);
	bool CheckJewelOfHarmonyItemType(CItem* lpItem);
private:
	JEWEL_OF_HARMONY_TYPE_INFO m_JewelOfHarmonyTypeInfo[MAX_JEWEL_OF_HARMONY_TYPE];
	int m_count;
};

extern CJewelOfHarmonyType gJewelOfHarmonyType;
