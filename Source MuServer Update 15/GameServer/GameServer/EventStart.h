// CEventStart.h: interface for the CEventStart class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

class CEventStart
{
public:
	CEventStart();
	virtual ~CEventStart();
	void MainProc();
	void Clear();
	void CommandEventStart(LPOBJ lpObj,char* arg);
	void CommandEventStartJoin(LPOBJ lpObj,char* arg);
private:
	int m_RemainTime;
	int m_TickCount;
	int m_EventStart;
	int m_Map;
	int m_MapX;
	int m_MapY;
};

extern CEventStart gEventStart;
