// CCustomExchangeCoin.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

struct CUSTOMEXCHANGECOIN_INFO
{
	int Index;
	char Name[32];
	int RequireCoin1;
	int RequireCoin2;
	int RequireCoin3;
	int RewardCoin1;
	int RewardCoin2;
	int RewardCoin3;
};

class CCustomExchangeCoin
{
public:
	CCustomExchangeCoin();
	virtual ~CCustomExchangeCoin();
	void Load(char* path);
    bool GetInfo(int index,CUSTOMEXCHANGECOIN_INFO* lpInfo);
    bool GetInfoByName(LPOBJ lpObj, char* message, int Npc);
private:
	std::map<int,CUSTOMEXCHANGECOIN_INFO> m_CustomExchangeCoinInfo;
};

extern CCustomExchangeCoin gCustomExchangeCoin;
