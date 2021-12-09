// MonsterAIUtil.h: interface for the CMonsterAIUtil class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

class CMonsterAIUtil
{
public:
	CMonsterAIUtil();
	virtual ~CMonsterAIUtil();
	static BOOL FindPathToMoveMonster(LPOBJ lpObj,int tx,int ty,int MaxPathCount,bool PreventOverMoving);
	static void SendMonsterMoveMsg(LPOBJ lpObj);
	static BOOL CheckMovingCondition(LPOBJ lpObj);
	static BOOL GetXYToPatrol(LPOBJ lpObj);
	static BOOL GetXYToEascape(LPOBJ lpObj);
	static BOOL GetXYToChase(LPOBJ lpObj);
	static BOOL FindMonViewportObj(int aIndex,int bIndex);
	static BOOL FindMonViewportObj2(int aIndex,int bIndex);
	static long FindMonsterToHeal(LPOBJ lpObj,int rate,int distance);
};
