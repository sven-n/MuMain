// Message.h: interface for the CCommand class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

struct COMMAND_LIST
{
	int Index;
	char Command[128];
	int Enable[MAX_ACCOUNT_LEVEL];
	int Money[MAX_ACCOUNT_LEVEL];
	int MinLevel[MAX_ACCOUNT_LEVEL];
	int MaxLevel[MAX_ACCOUNT_LEVEL];
	int MinReset[MAX_ACCOUNT_LEVEL];
	int MaxReset[MAX_ACCOUNT_LEVEL];
	int Delay;
	int GameMaster;
	int Coin1;
	int Coin2;
	int Coin3;
};

class CCommand
{
public:
	CCommand();
	virtual ~CCommand();
	void Load(char* path);
	char* GetCommand(int index);
	bool GetInfo(int index,COMMAND_LIST* lpInfo);
	bool GetInfoByName(char* label,COMMAND_LIST* lpInfo);
private:
	char m_DefaultCommand[128];
	std::map<int,COMMAND_LIST> m_CommandInfo;
};

extern CCommand gCommand;
