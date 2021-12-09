// MonsterAIUnitInfo.cpp: implementation of the CMonsterAIUnitInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIUnitInfo.h"
#include "User.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIUnitInfo::CMonsterAIUnitInfo() // OK
{
	this->Reset();
}

CMonsterAIUnitInfo::~CMonsterAIUnitInfo() // OK
{

}

void CMonsterAIUnitInfo::Reset() // OK
{
	memset(this->m_UnitName,0,sizeof(this->m_UnitName));

	this->m_UnitNumber = -1;
	this->m_DelayTime = 0;
	this->m_AutomataInfo = 0;
	this->m_AIClassNormal = 0;
	this->m_AIClassMove = 0;
	this->m_AIClassAttack = 0;
	this->m_AIClassHeal = 0;
	this->m_AIClassAvoid = 0;
	this->m_AIClassHelp = 0;
	this->m_AIClassSpecial = 0;
	this->m_AIClassEvent = 0;
	this->m_AIClassMap[0] = 0;
	this->m_AIClassMap[1] = 0;
	this->m_AIClassMap[2] = 0;
	this->m_AIClassMap[3] = 0;
	this->m_AIClassMap[4] = 0;
	this->m_AIClassMap[5] = 0;
	this->m_AIClassMap[6] = 0;
	this->m_AIClassMap[7] = 0;
}

BOOL CMonsterAIUnitInfo::RunAIUnitInfo(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(this->m_AutomataInfo == 0)
	{
		return 0;
	}

	if((GetTickCount()-lpObj->LastAutomataRuntime) < ((DWORD)lpObj->LastAutomataDelay))
	{
		return 0;
	}

	CMonsterAIState* lpMonsterAIState = this->m_AutomataInfo->RunAutomataInfo(aIndex);

	if(lpMonsterAIState == 0)
	{
		return 0;
	}

	lpObj->LastAutomataRuntime = GetTickCount();

	if(this->m_AIClassMap[lpMonsterAIState->m_NextState] == 0)
	{
		return 0;
	}

	if(this->m_AIClassMap[lpMonsterAIState->m_NextState]->ForceAIElementInfo(aIndex,0,lpMonsterAIState) == 0)
	{
		return 0;
	}

	lpObj->CurrentAIState = lpMonsterAIState->m_NextState;

	return 1;
}
