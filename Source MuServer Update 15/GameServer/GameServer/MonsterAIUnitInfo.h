// MonsterAIUnitInfo.h: interface for the CMonsterAIUnitInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIAutomataInfo.h"
#include "MonsterAIElementInfo.h"

class CMonsterAIUnitInfo
{
public:
	CMonsterAIUnitInfo();
	virtual ~CMonsterAIUnitInfo();
	void Reset();
	BOOL RunAIUnitInfo(int aIndex);
public:
	char m_UnitName[50];
	int m_UnitNumber;
	int m_DelayTime;
	CMonsterAIAutomataInfo* m_AutomataInfo;
	CMonsterAIElementInfo* m_AIClassNormal;
	CMonsterAIElementInfo* m_AIClassMove;
	CMonsterAIElementInfo* m_AIClassAttack;
	CMonsterAIElementInfo* m_AIClassHeal;
	CMonsterAIElementInfo* m_AIClassAvoid;
	CMonsterAIElementInfo* m_AIClassHelp;
	CMonsterAIElementInfo* m_AIClassSpecial;
	CMonsterAIElementInfo* m_AIClassEvent;
	CMonsterAIElementInfo* m_AIClassMap[8];
};
