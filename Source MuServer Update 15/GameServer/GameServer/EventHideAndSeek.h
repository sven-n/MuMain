// CEventHideAndSeek.h: interface for the CEventHideAndSeek class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

class CEventHideAndSeek
{
public:
	CEventHideAndSeek();
	virtual ~CEventHideAndSeek();
	void MainProc();
	void Clear();
	void CommandEventHideAndSeek(LPOBJ lpObj,char* arg);
	int EventHideAndSeekTrade(int aIndex, int bIndex);
private:
	int m_GmIndex;
	int m_RemainTime;
	int m_TickCount;
	int m_EventHideAndSeek;
	int MinutesLeft;
};

extern CEventHideAndSeek gEventHideAndSeek;
