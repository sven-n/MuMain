// CustomCommandDescription.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

struct CUSTOMCOMMANDDESCRIPTION_INFO
{
	int Index;
	char Commmand[32];
	char Description[128];
};

class CCustomCommandDescription
{
public:
	CCustomCommandDescription();
	virtual ~CCustomCommandDescription();
	void Load(char* path);
	void GetMove(LPOBJ lpObj,int index);
    bool GetInfo(int index,CUSTOMCOMMANDDESCRIPTION_INFO* lpInfo);
    bool GetInfoByName(LPOBJ lpObj, char* message);
private:
	std::map<int,CUSTOMCOMMANDDESCRIPTION_INFO> m_CustomCommandDescriptionInfo;
};

extern CCustomCommandDescription gCustomCommandDescription;
