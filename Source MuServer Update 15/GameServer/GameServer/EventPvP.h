// CEventPvP.h: interface for the CEventPvP class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

class CEventPvP
{
public:
	CEventPvP();
	virtual ~CEventPvP();
	void MainProc();
	void Clear();
	bool CommandEventPvP(LPOBJ lpObj,char* arg);
	void EventPvPDead(int aIndex, int bIndex);
    bool EventPvPTradeJoin(int aIndex,int bIndex);
	int m_EventPvPActive;
private:
	int m_GmIndex;
	int m_Player1Index;
	int m_Player2Index;
	int m_Score1;
	int m_Score2;
	int m_RemainTime;
	int m_TickCount;
	int m_TotalTime;
	int m_EventPvPEnter;
};

extern CEventPvP gEventPvP;
