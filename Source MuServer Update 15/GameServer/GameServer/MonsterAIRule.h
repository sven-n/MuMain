// MonsterAIRule.h: interface for the CMonsterAIRule class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIRuleInfo.h"
#include "MonsterManager.h"

#define MAX_MONSTER_AI_RULE_TABLE MAX_MONSTER_INFO
#define MAX_MONSTER_AI_RULE_INFO 200

class CMonsterAIRule
{
public:
	CMonsterAIRule();
	virtual ~CMonsterAIRule();
	static void LoadData(char* path);
	static void DelAllAIRule();
	static int GetCurrentAIUnit(int MonsterClass);
	static void MonsterAIRuleProc();
public:
	static BOOL m_DataLoad;
	static int m_MonsterCurrentAIUnitTable[MAX_MONSTER_AI_RULE_TABLE];
	static CMonsterAIRuleInfo m_MonsterAIRuleInfoArray[MAX_MONSTER_AI_RULE_INFO];
	static int m_MonsterAIRuleInfoCount;
};
