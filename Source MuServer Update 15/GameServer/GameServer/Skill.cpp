// Skill.cpp: implementation of the CSkill class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Skill.h"
#include "SkillManager.h"
#include "MasterSkillTree.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkill::CSkill() // OK
{
	this->Clear();
}

CSkill::~CSkill() // OK
{

}

void CSkill::Clear() // OK
{
	this->m_level = 0;
	this->m_skill = 0xFFFF;
	this->m_index = 0xFFFF;
	this->m_DamageMin = 0;
	this->m_DamageMax = 0;
}

bool CSkill::IsSkill() // OK
{
	if(this->m_index == 0xFFFF)
	{
		return 0;
	}

	return 1;
}

bool CSkill::IsMasterSkill() // OK
{
	if(this->m_index == 0xFFFF || this->m_index < 0x12C)
	{
		return 0;
	}

	return 1;
}

bool CSkill::Set(int index,int level) // OK
{
	if(this->IsSkill() != 0 && index < 300)
	{
		return 0;
	}

	this->m_level = level;

	#if(GAMESERVER_UPDATE>=401)

	this->m_skill = (((this->m_skill=gMasterSkillTree.GetMasterSkillRelated(index))==0)?index:this->m_skill);

	#else

	this->m_skill = index;

	#endif

	this->m_index = index;

	this->m_DamageMin = gSkillManager.GetSkillDamage(index);

	this->m_DamageMax = this->m_DamageMin+(this->m_DamageMin/2);

	return 1;
}
