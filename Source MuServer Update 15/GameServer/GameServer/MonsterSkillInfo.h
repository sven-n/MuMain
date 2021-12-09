// MonsterSkillInfo.h: interface for the CMonsterSkillInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterSkillUnitInfo.h"

#define MAX_MONSTER_SKILL_UNIT 10

class CMonsterSkillInfo
{
public:
	CMonsterSkillInfo();
	void Reset();
	BOOL IsValid();
public:
	int m_MonsterIndex;
	int m_SkillUnitCount;
	int m_SkillUnitTypeArray[MAX_MONSTER_SKILL_UNIT];
	CMonsterSkillUnitInfo* m_SkillUnitArray[MAX_MONSTER_SKILL_UNIT];
};
