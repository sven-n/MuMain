// MonsterAIElement.h: interface for the CMonsterAIElement class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Map.h"
#include "MonsterAIElementInfo.h"
#include "MonsterAIMovePath.h"

#define MAX_MONSTER_AI_ELEMENT_INFO 100

enum MONSTER_AI_ELEMENT_STATE
{
	MAE_STATE_NORMAL = 0,
	MAE_STATE_MOVE = 1,
	MAE_STATE_ATTACK = 2,
	MAE_STATE_HEAL = 3,
	MAE_STATE_AVOID = 4,
	MAE_STATE_HELP = 5,
	MAE_STATE_SPECIAL = 6,
	MAE_STATE_EVENT = 7,
};

class CMonsterAIElement
{
public:
	CMonsterAIElement();
	virtual ~CMonsterAIElement();
	static void LoadData(char* path);
	static void DelAllAIElement();
	static CMonsterAIElementInfo* FindAIElementInfo(int ElementNumber);
public:
	static BOOL m_DataLoad;
	static CMonsterAIElementInfo m_MonsterAIElementInfoArray[MAX_MONSTER_AI_ELEMENT_INFO];
	static CMonsterAIMovePath m_MonsterAIMovePath[MAX_MAP];
};
