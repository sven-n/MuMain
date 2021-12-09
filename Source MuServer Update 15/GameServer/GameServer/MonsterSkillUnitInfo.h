// MonsterSkillUnitInfo.h: interface for the CMonsterSkillUnitInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterSkillElementInfo.h"

#define MAX_MONSTER_SKILL_ELEMENT 5

enum MONSTER_SKILL_UNIT_TARGET_TYPE
{
	MSU_TARGET_TYPE_NONE = 0,
	MSU_TARGET_TYPE_COMMON = 1,
	MSU_TARGET_TYPE_SELF = 2,
};

enum MONSTER_SKILL_UNIT_SCOPE_TYPE
{
	MSU_SCOPE_TYPE_NONE = -1,
	MSU_SCOPE_TYPE_AREA = 0,
	MSU_SCOPE_TYPE_HITBOX = 1,
};

class CMonsterSkillUnitInfo
{
public:
	CMonsterSkillUnitInfo();
	virtual ~CMonsterSkillUnitInfo();
	void Reset();
	void RunSkill(int aIndex,int bIndex);
public:
	char m_UnitName[50];
	int m_UnitNumber;
	int m_UnitTargetType;
	int m_UnitScopeType;
	int m_UnitScopeValue;
	int m_Delay;
	int m_ElementsCount;
	CMonsterSkillElementInfo* m_ElementsSlot[MAX_MONSTER_SKILL_ELEMENT];
};
