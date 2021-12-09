// Move.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

//#define MAX_CUSTOMTOP 20

struct CUSTOMTOP_INFO
{
	int Index;
	char Name[10];
	char Title[32];
};

class CCustomTop
{
public:
	CCustomTop();
	virtual ~CCustomTop();
	void Load(char* path);
	void GetTop(LPOBJ lpObj,int index);
    bool GetInfo(int index,CUSTOMTOP_INFO* lpInfo);
    bool GetInfoByName(char* message,CUSTOMTOP_INFO* lpInfo);
private:
	std::map<int,CUSTOMTOP_INFO> m_CustomTopInfo;
};

extern CCustomTop gCustomTop;
