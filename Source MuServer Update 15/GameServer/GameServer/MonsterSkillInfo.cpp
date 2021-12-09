// MonsterSkillInfo.cpp: implementation of the CMonsterSkillInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSkillInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSkillInfo::CMonsterSkillInfo() // OK
{
	this->Reset();
}

void CMonsterSkillInfo::Reset() // OK
{
	this->m_MonsterIndex = -1;
	this->m_SkillUnitCount = 0;
	this->m_SkillUnitTypeArray[0] = -1;
	this->m_SkillUnitTypeArray[1] = -1;
	this->m_SkillUnitTypeArray[2] = -1;
	this->m_SkillUnitTypeArray[3] = -1;
	this->m_SkillUnitTypeArray[4] = -1;
	this->m_SkillUnitTypeArray[5] = -1;
	this->m_SkillUnitTypeArray[6] = -1;
	this->m_SkillUnitTypeArray[7] = -1;
	this->m_SkillUnitTypeArray[8] = -1;
	this->m_SkillUnitTypeArray[9] = -1;
	this->m_SkillUnitArray[0] = 0;
	this->m_SkillUnitArray[1] = 0;
	this->m_SkillUnitArray[2] = 0;
	this->m_SkillUnitArray[3] = 0;
	this->m_SkillUnitArray[4] = 0;
	this->m_SkillUnitArray[5] = 0;
	this->m_SkillUnitArray[6] = 0;
	this->m_SkillUnitArray[7] = 0;
	this->m_SkillUnitArray[8] = 0;
	this->m_SkillUnitArray[9] = 0;
}

BOOL CMonsterSkillInfo::IsValid() // OK
{
	if(this->m_MonsterIndex == -1 || this->m_SkillUnitCount == 0)
	{
		return 0;
	}

	return 1;
}
