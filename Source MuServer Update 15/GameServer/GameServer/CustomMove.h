// Move.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_CUSTOMMOVE 20

struct CUSTOMMOVE_INFO
{
	int Index;
	char Name[32];
	int Map;
	int X;
	int Y;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int MinMReset;
	int MaxMReset;
	int AccountLevel;
	int PkMove;
};

class CCustomMove
{
public:
	CCustomMove();
	virtual ~CCustomMove();
	void Load(char* path);
	void GetMove(LPOBJ lpObj,int index);
    bool GetInfo(int index,CUSTOMMOVE_INFO* lpInfo);
    bool GetInfoByName(LPOBJ lpObj, char* message, int Npc);
private:
	std::map<int,CUSTOMMOVE_INFO> m_CustomMoveInfo;
};

extern CCustomMove gCustomMove;
