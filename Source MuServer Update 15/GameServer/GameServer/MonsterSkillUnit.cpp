// MonsterSkillUnit.cpp: implementation of the CMonsterSkillUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSkillUnit.h"
#include "MemScript.h"
#include "MonsterSkillElement.h"
#include "Util.h"

BOOL CMonsterSkillUnit::m_DataLoad;
CMonsterSkillUnitInfo CMonsterSkillUnit::m_MonsterSkillUnitInfoArray[MAX_MONSTER_SKILL_UNIT_INFO_ARRAY];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSkillUnit::CMonsterSkillUnit() // OK
{
	CMonsterSkillUnit::DelAllSkillUnit();
}

CMonsterSkillUnit::~CMonsterSkillUnit() // OK
{

}

void CMonsterSkillUnit::LoadData(char* path) // OK
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

	CMonsterSkillUnit::DelAllSkillUnit();

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

					CMonsterSkillUnitInfo info;

					info.m_UnitNumber = lpMemScript->GetNumber();

					strcpy_s(info.m_UnitName,lpMemScript->GetAsString());

					info.m_UnitTargetType = lpMemScript->GetAsNumber();

					info.m_UnitScopeType = lpMemScript->GetAsNumber();

					info.m_UnitScopeValue = lpMemScript->GetAsNumber();

					info.m_Delay = lpMemScript->GetAsNumber();

					info.m_ElementsCount = 0;

					for(int n=0;n < MAX_MONSTER_SKILL_ELEMENT;n++)
					{
						info.m_ElementsSlot[n] = CMonsterSkillElement::FindSkillElementInfo(lpMemScript->GetAsNumber());

						info.m_ElementsCount = ((info.m_ElementsSlot[n]==0)?info.m_ElementsCount:(info.m_ElementsCount+1));
					}

					if(info.m_UnitNumber >= 0 && info.m_UnitNumber < MAX_MONSTER_SKILL_UNIT_INFO_ARRAY)
					{
						CMonsterSkillUnit::m_MonsterSkillUnitInfoArray[info.m_UnitNumber] = info;
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

	CMonsterSkillUnit::m_DataLoad = 1;

	delete lpMemScript;
}

void CMonsterSkillUnit::DelAllSkillUnit() // OK
{
	CMonsterSkillUnit::m_DataLoad = 0;

	for(int n=0;n < MAX_MONSTER_SKILL_UNIT_INFO_ARRAY;n++)
	{
		CMonsterSkillUnit::m_MonsterSkillUnitInfoArray[n].Reset();
	}
}

CMonsterSkillUnitInfo* CMonsterSkillUnit::FindSkillUnitInfo(int UnitNumber) // OK
{
	if(UnitNumber < 0 || UnitNumber >= MAX_MONSTER_SKILL_UNIT_INFO_ARRAY)
	{
		return 0;
	}

	if(CMonsterSkillUnit::m_MonsterSkillUnitInfoArray[UnitNumber].m_UnitNumber == UnitNumber)
	{
		return &CMonsterSkillUnit::m_MonsterSkillUnitInfoArray[UnitNumber];
	}

	return 0;
}
