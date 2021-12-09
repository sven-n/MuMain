// MonsterAIRuleInfo.h: interface for the CMonsterAIRuleInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CMonsterAIRuleInfo
{
public:
	CMonsterAIRuleInfo();
	virtual ~CMonsterAIRuleInfo();
	void Reset();
	BOOL IsValid();
	BOOL CheckConditionSpecificDate();
	BOOL CheckConditionCrywolfStart();
	BOOL CheckConditionCrywolfEnd();
public:
	int m_RuleNumber;
	int m_MonsterClass;
	int m_MonsterAIUnit;
	int m_RuleCondition;
	int m_WaitTime;
	int m_ContinuanceTime;
	int m_Month;
	int m_Day;
	int m_WeekDay;
	int m_Hour;
	int m_Minute;
	int m_AppliedTime;
	int m_ApplyingTime;
	char m_RuleDesc[100];
};
