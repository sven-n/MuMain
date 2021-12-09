// MonsterSkillManager.h: interface for the CMonsterSkillManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterManager.h"
#include "MonsterSkillInfo.h"
#include "MonsterSkillUnitInfo.h"
#include "Skill.h"
#include "User.h"

#define MAX_MONSTER_SKILL_INFO MAX_MONSTER_INFO

class CMonsterSkillManager
{
public:
	CMonsterSkillManager();
	virtual ~CMonsterSkillManager();
	static void LoadData(char* path);
	static void DelAllSkillManagerInfo();
	static BOOL CheckMonsterSkill(int MonsterClass,int MonsterSkillUnitType);
	static CMonsterSkillUnitInfo* FindMonsterSkillUnitInfo(int aIndex,int MonsterSkillUnitType);
	static void UseMonsterSkill(int aIndex,int bIndex,int MonsterSkillUnitType,int MonsterSkillUnit,CSkill* lpSkill);
	static void SpecialMonsterSkillAttack(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo);
public:
	static BOOL m_DataLoad;
	static CMonsterSkillInfo m_MonsterSkillInfoArray[MAX_MONSTER_SKILL_INFO];
};
