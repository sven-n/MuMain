#pragma once

#include "Protocol.h"
#include "User.h"


struct CUSTOMDEATHMESSAGE_INFO
{
	int Index;
	char Text[128];
};


class CustomDeathMessage
{
public:
	CustomDeathMessage();
	virtual ~CustomDeathMessage();
	void Load(char* path);
	void GetDeathText(LPOBJ lpTarget, LPOBJ lpObj, int index);
	bool GetInfo(int index,CUSTOMDEATHMESSAGE_INFO* lpInfo);
	private:
	std::map<int,CUSTOMDEATHMESSAGE_INFO> m_CustomDeathMessage;
};


extern CustomDeathMessage gCustomDeathMessage; 