// MonsterAIAutomataInfo.h: interface for the CMonsterAIAutomataInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIState.h"

#define MAX_AI_STATE 8
#define MAX_AI_PRIORITY 30

class CMonsterAIAutomataInfo
{
public:
	CMonsterAIAutomataInfo();
	virtual ~CMonsterAIAutomataInfo();
	void Reset();
	CMonsterAIState* RunAutomataInfo(int aIndex);
public:
	int m_AutomataNumber;
	CMonsterAIState m_AIState[MAX_AI_STATE][MAX_AI_PRIORITY];
	int m_AIStateTransCount[MAX_AI_STATE];
};
