// CrywolfUtil.h: interface for the CCrywolfUtil class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

class CCrywolfUtil
{
public:
	CCrywolfUtil();
	virtual ~CCrywolfUtil();
	void SendMapServerGroupMsg(char* lpMsg,...);
	void SendAllUserAnyData(BYTE* lpMsg,int size);
	void SendAllUserAnyMsg(int type,char* lpMsg,...);
	void SendCrywolfUserAnyData(BYTE* lpMsg,int size);
	void SendCrywolfUserAnyMsg(int type,char* lpMsg,...);
	static bool CrywolfAllUserScoreSort(LPOBJ const &lpObj,LPOBJ const &lpTarget);
};

extern CCrywolfUtil gCrywolfUtil;
