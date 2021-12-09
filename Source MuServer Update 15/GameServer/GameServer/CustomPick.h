// CustomPick.h: interface for the CCustomPick class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

#define MAX_CUSTOMPICK 20

struct CUSTOMPICK_INFO
{
	int Index;
	char Name[32];
	int Cat;
	int Item;
};

class CCustomPick
{
public:
	CCustomPick();
	virtual ~CCustomPick();
	void Load(char* path);
	void GetMove(LPOBJ lpObj,int index);
    bool GetInfo(int index,CUSTOMPICK_INFO* lpInfo);
    bool GetInfoByName(char* message,CUSTOMPICK_INFO* lpInfo);
	void ItemGet(LPOBJ lpObj);
	void PickProc(LPOBJ lpObj);
	bool CommandPick(LPOBJ lpObj,char* arg);
	void OnPickClose(LPOBJ lpObj);
private:
	std::map<int,CUSTOMPICK_INFO> m_CustomPickInfo;
};

extern CCustomPick gCustomPick;
