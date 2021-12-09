// Skill.h: interface for the CSkill class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CSkill
{
public:
	CSkill();
	virtual ~CSkill();
	void Clear();
	bool IsSkill();
	bool IsMasterSkill();
	bool Set(int index,int level);
public:
	BYTE m_level;
	WORD m_skill;
	WORD m_index;
	int m_DamageMin;
	int m_DamageMax;
};
