// MonsterSkillUnit.h: interface for the CMonsterSkillUnit class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterSkillUnitInfo.h"

#define MAX_MONSTER_SKILL_UNIT_INFO_ARRAY 200

class CMonsterSkillUnit
{
public:
	CMonsterSkillUnit();
	virtual ~CMonsterSkillUnit();
	static void LoadData(char* path);
	static void DelAllSkillUnit();
	static CMonsterSkillUnitInfo* FindSkillUnitInfo(int UnitNumber);
public:
	static BOOL m_DataLoad;
	static CMonsterSkillUnitInfo m_MonsterSkillUnitInfoArray[MAX_MONSTER_SKILL_UNIT_INFO_ARRAY];
};
