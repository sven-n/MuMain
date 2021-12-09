// 380ItemType.h: interface for the C380ItemType class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_380_ITEM_OPTION_INDEX 2

struct ITEM_380_TYPE_INFO
{
	int Index;
	int OptionIndex[MAX_380_ITEM_OPTION_INDEX];
	int OptionValue[MAX_380_ITEM_OPTION_INDEX];
};

class C380ItemType
{
public:
	C380ItemType();
	virtual ~C380ItemType();
	void Load(char* path);
	bool Check380ItemType(int index);
	int Get380ItemOptionIndex(int index,int number);
	int Get380ItemOptionValue(int index,int number);
private:
	std::map<int,ITEM_380_TYPE_INFO> m_380ItemTypeInfo;
};

extern C380ItemType g380ItemType;
