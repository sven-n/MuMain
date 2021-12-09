// MonsterAIUnit.cpp: implementation of the CMonsterAIUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIUnit.h"
#include "MemScript.h"
#include "MonsterAIAutomata.h"
#include "MonsterAIElement.h"
#include "Util.h"

BOOL CMonsterAIUnit::m_DataLoad;
CMonsterAIUnitInfo CMonsterAIUnit::m_MonsterAIUnitInfoArray[MAX_MONSTER_AI_UNIT_INFO];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIUnit::CMonsterAIUnit() // OK
{
	CMonsterAIUnit::DelAllAIUnit();
}

CMonsterAIUnit::~CMonsterAIUnit() // OK
{

}

void CMonsterAIUnit::LoadData(char* path) // OK
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

	CMonsterAIUnit::DelAllAIUnit();

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

					CMonsterAIUnitInfo info;

					info.m_UnitNumber = lpMemScript->GetNumber();

					strcpy_s(info.m_UnitName,lpMemScript->GetAsString());

					info.m_DelayTime = lpMemScript->GetAsNumber();

					info.m_AutomataInfo = CMonsterAIAutomata::FindAutomataInfo(lpMemScript->GetAsNumber());

					info.m_AIClassNormal = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassMove = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassAttack = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassHeal = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassAvoid = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassHelp = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassSpecial = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassEvent = CMonsterAIElement::FindAIElementInfo(lpMemScript->GetAsNumber());

					info.m_AIClassMap[0] = info.m_AIClassNormal;

					info.m_AIClassMap[1] = info.m_AIClassMove;

					info.m_AIClassMap[2] = info.m_AIClassAttack;

					info.m_AIClassMap[3] = info.m_AIClassHeal;

					info.m_AIClassMap[4] = info.m_AIClassAvoid;

					info.m_AIClassMap[5] = info.m_AIClassHelp;

					info.m_AIClassMap[6] = info.m_AIClassSpecial;

					info.m_AIClassMap[7] = info.m_AIClassEvent;

					if(info.m_UnitNumber >= 0 && info.m_UnitNumber < MAX_MONSTER_AI_UNIT_INFO)
					{
						CMonsterAIUnit::m_MonsterAIUnitInfoArray[info.m_UnitNumber] = info;
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

	CMonsterAIUnit::m_DataLoad = 1;

	delete lpMemScript;
}

void CMonsterAIUnit::DelAllAIUnit() // OK
{
	CMonsterAIUnit::m_DataLoad = 0;

	for(int n=0;n < MAX_MONSTER_AI_UNIT_INFO;n++)
	{
		CMonsterAIUnit::m_MonsterAIUnitInfoArray[n].Reset();
	}
}

CMonsterAIUnitInfo* CMonsterAIUnit::FindAIUnitInfo(int UnitNumber) // OK
{
	if(UnitNumber < 0 || UnitNumber >= MAX_MONSTER_AI_UNIT_INFO)
	{
		return 0;
	}

	if(CMonsterAIUnit::m_MonsterAIUnitInfoArray[UnitNumber].m_UnitNumber == UnitNumber)
	{
		return &CMonsterAIUnit::m_MonsterAIUnitInfoArray[UnitNumber];
	}

	return 0;
}
