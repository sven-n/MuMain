// MonsterAIRule.cpp: implementation of the CMonsterAIRule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIRule.h"
#include "MemScript.h"
#include "Util.h"

BOOL CMonsterAIRule::m_DataLoad;
int CMonsterAIRule::m_MonsterCurrentAIUnitTable[MAX_MONSTER_AI_RULE_TABLE];
CMonsterAIRuleInfo CMonsterAIRule::m_MonsterAIRuleInfoArray[MAX_MONSTER_AI_RULE_INFO];
int CMonsterAIRule::m_MonsterAIRuleInfoCount;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIRule::CMonsterAIRule() // OK
{
	CMonsterAIRule::DelAllAIRule();
}

CMonsterAIRule::~CMonsterAIRule() // OK
{

}

void CMonsterAIRule::LoadData(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	CMonsterAIRule::DelAllAIRule();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CMonsterAIRuleInfo info;

					info.m_RuleNumber = lpMemScript->GetNumber();

					info.m_MonsterClass = lpMemScript->GetAsNumber();

					strcpy_s(info.m_RuleDesc,lpMemScript->GetAsString());

					info.m_MonsterAIUnit = lpMemScript->GetAsNumber();

					info.m_RuleCondition = lpMemScript->GetAsNumber();

					info.m_WaitTime = lpMemScript->GetAsNumber();

					info.m_ContinuanceTime = lpMemScript->GetAsNumber();

					info.m_Month = lpMemScript->GetAsNumber();

					info.m_Day = lpMemScript->GetAsNumber();

					info.m_WeekDay = lpMemScript->GetAsNumber();

					info.m_Hour = lpMemScript->GetAsNumber();

					info.m_Minute = lpMemScript->GetAsNumber();

					if(CMonsterAIRule::m_MonsterAIRuleInfoCount >= 0 && CMonsterAIRule::m_MonsterAIRuleInfoCount < MAX_MONSTER_AI_RULE_INFO)
					{
						CMonsterAIRule::m_MonsterAIRuleInfoArray[CMonsterAIRule::m_MonsterAIRuleInfoCount++] = info;
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	CMonsterAIRule::m_DataLoad = 1;

	delete lpMemScript;
}

void CMonsterAIRule::DelAllAIRule()
{
	CMonsterAIRule::m_DataLoad = 0;

	for(int n=0;n < MAX_MONSTER_AI_RULE_TABLE;n++)
	{
		CMonsterAIRule::m_MonsterCurrentAIUnitTable[n] = 0;
	}

	for(int n=0;n < MAX_MONSTER_AI_RULE_INFO;n++)
	{
		CMonsterAIRule::m_MonsterAIRuleInfoArray[n].Reset();
	}

	CMonsterAIRule::m_MonsterAIRuleInfoCount = 0;
}

int CMonsterAIRule::GetCurrentAIUnit(int MonsterClass) // OK
{
	if(MonsterClass < 0 || MonsterClass >= MAX_MONSTER_AI_RULE_TABLE)
	{
		return 0;
	}

	return CMonsterAIRule::m_MonsterCurrentAIUnitTable[MonsterClass];
}

void CMonsterAIRule::MonsterAIRuleProc() // OK
{
	for(int n=0;n < CMonsterAIRule::m_MonsterAIRuleInfoCount;n++)
	{
		CMonsterAIRuleInfo* lpMonsterAIRuleInfo = &CMonsterAIRule::m_MonsterAIRuleInfoArray[n];

		if(lpMonsterAIRuleInfo->m_MonsterClass < 0 || lpMonsterAIRuleInfo->m_MonsterClass >= MAX_MONSTER_AI_RULE_TABLE)
		{
			continue;
		}

		if(lpMonsterAIRuleInfo->IsValid() == 0)
		{
			CMonsterAIRule::m_MonsterCurrentAIUnitTable[lpMonsterAIRuleInfo->m_MonsterClass] = 0;
		}
		else
		{
			CMonsterAIRule::m_MonsterCurrentAIUnitTable[lpMonsterAIRuleInfo->m_MonsterClass] = lpMonsterAIRuleInfo->m_MonsterAIUnit;
		}
	}
}
