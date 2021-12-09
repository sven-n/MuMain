// CEventRussianRoulette.h: interface for the CEventRussianRoulette class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

class CEventRussianRoulette
{
public:
	CEventRussianRoulette();
	virtual ~CEventRussianRoulette();
	void MainProc();
	void Clear();
	void CommandEventRussianRoulette(LPOBJ lpObj,char* arg);
	int EventRussianRouletteTrade(int aIndex, int bIndex);
    void CommandEventRussianRouletteJoin(LPOBJ lpObj,char* arg);
	int m_EventRussianRouletteActive;
private:
	int m_GmIndex;
	int m_PlayerIndex;
	int m_RemainTime;
	int m_RouletteRemainTime;
	int m_TickCount;
	int m_TotalTime;
	int m_Map;
	int m_MapX;
	int m_MapY;
	int m_Members;
	int m_EventRussianRouletteEnter;
};

extern CEventRussianRoulette gEventRussianRoulette;
