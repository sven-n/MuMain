// MonsterSkillUnitInfo.cpp: implementation of the CMonsterSkillUnitInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSkillUnitInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSkillUnitInfo::CMonsterSkillUnitInfo() // OK
{
	this->Reset();
}

CMonsterSkillUnitInfo::~CMonsterSkillUnitInfo() // OK
{

}

void CMonsterSkillUnitInfo::Reset() // OK
{
	memset(this->m_UnitName,0,sizeof(this->m_UnitName));

	this->m_UnitNumber = -1;
	this->m_UnitTargetType = -1;
	this->m_UnitScopeType = -1;
	this->m_UnitScopeValue = -1;
	this->m_Delay = -1;
	this->m_ElementsCount = -1;
	this->m_ElementsSlot[0] = 0;
	this->m_ElementsSlot[1] = 0;
	this->m_ElementsSlot[2] = 0;
	this->m_ElementsSlot[3] = 0;
	this->m_ElementsSlot[4] = 0;
}

void CMonsterSkillUnitInfo::RunSkill(int aIndex,int bIndex) // OK
{
	for(int n=0;n < MAX_MONSTER_SKILL_ELEMENT;n++)
	{
		if(this->m_ElementsSlot[n] != 0)
		{
			this->m_ElementsSlot[n]->ForceSkillElementInfo(aIndex,bIndex);
		}
	}
}
