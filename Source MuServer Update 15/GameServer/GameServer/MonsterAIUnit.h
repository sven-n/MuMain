// MonsterAIUnit.h: interface for the CMonsterAIUnit class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIUnitInfo.h"

#define MAX_MONSTER_AI_UNIT_INFO 100

class CMonsterAIUnit
{
public:
	CMonsterAIUnit();
	virtual ~CMonsterAIUnit();
	static void LoadData(char* path);
	static void DelAllAIUnit();
	static CMonsterAIUnitInfo* FindAIUnitInfo(int UnitNumber);
public:
	static BOOL m_DataLoad;
	static CMonsterAIUnitInfo m_MonsterAIUnitInfoArray[MAX_MONSTER_AI_UNIT_INFO];
};
