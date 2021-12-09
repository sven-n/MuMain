// MoveSummon.h: interface for the CMoveSummon class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

struct MOVE_SUMMON_INFO
{
	int Map;
	int X;
	int Y;
	int TX;
	int TY;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int AccountLevel;
	int PkMove;
};

class CMoveSummon
{
public:
	CMoveSummon();
	virtual ~CMoveSummon();
	void Load(char* path);
	bool CheckMoveSummon(LPOBJ lpObj,int map,int x,int y);
private:
	std::vector<MOVE_SUMMON_INFO> m_MoveSummonInfo;
};

extern CMoveSummon gMoveSummon;
