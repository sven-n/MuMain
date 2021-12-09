// CEventRunAndCatch.h: interface for the CEventRunAndCatch class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

class CEventRunAndCatch
{
public:
	CEventRunAndCatch();
	virtual ~CEventRunAndCatch();
	void MainProc();
	void Clear();
	void CommandEventRunAndCatch(LPOBJ lpObj,char* arg);
	void EventRunAndCatchHit(int aIndex, int bIndex);
    void CommandEventRunAndCatchJoin(LPOBJ lpObj,char* arg);
	int m_EventRunAndCatchActive;
private:
	int m_GmIndex;
	int m_RemainTime;
	int m_TickCount;
	int m_TotalTime;
	int m_Map;
	int m_MapX;
	int m_MapY;
	int m_Members;
	int m_EventRunAndCatchEnter;
};

extern CEventRunAndCatch gEventRunAndCatch;
