// CEventQuickly.h: interface for the CEventQuickly class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

class CEventQuickly
{
public:
	CEventQuickly();
	virtual ~CEventQuickly();
	int m_NpcIndex;
	void MainProc();
	void Clear();
	void CommandEventQuickly(LPOBJ lpObj,char* arg);
    void ClickNPC(LPOBJ lpObj);
private:
	int m_RemainTime;
	int m_TickCount;
	int MinutesLeft;
};

extern CEventQuickly gEventQuickly;
