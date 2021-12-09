// 380ItemOption.h: interface for the C380ItemOption class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "User.h"

#define MAX_380_ITEM_OPTION 10

enum e380ItemOption
{
	ITEM_380_OPTION_NONE = 0,
	ITEM_380_OPTION_ADD_ATTACK_SUCCESS_RATE_PVP = 1,
	ITEM_380_OPTION_ADD_DAMAGE_PVP = 2,
	ITEM_380_OPTION_ADD_DEFENSE_SUCCESS_RATE_PVP = 3,
	ITEM_380_OPTION_ADD_DEFENSE_PVP = 4,
	ITEM_380_OPTION_ADD_MAX_HP = 5,
	ITEM_380_OPTION_ADD_MAX_SD = 6,
	ITEM_380_OPTION_SET_SD_RECOVERY_TYPE = 7,
	ITEM_380_OPTION_ADD_SD_RECOVERY_RATE = 8,
};

struct ITEM_380_OPTION_INFO
{
	int Index;
	char Name[32];
	int Value;
};

class C380ItemOption
{
public:
	C380ItemOption();
	virtual ~C380ItemOption();
	void Init();
	void Load(char* path);
	void SetInfo(ITEM_380_OPTION_INFO info);
	ITEM_380_OPTION_INFO* GetInfo(int index);
	bool Is380Item(CItem* lpItem);
	void Calc380ItemOption(LPOBJ lpObj,bool flag);
	void InsertOption(LPOBJ lpObj,int index,int value,bool flag);
private:
	ITEM_380_OPTION_INFO m_380ItemOptionInfo[MAX_380_ITEM_OPTION];
};

extern C380ItemOption g380ItemOption;
