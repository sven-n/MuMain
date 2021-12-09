// MonsterAIAutomata.h: interface for the CMonsterAIAutomata class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIAutomataInfo.h"

#define MAX_MONSTER_AI_AUTOMATA_INFO 300

class CMonsterAIAutomata
{
public:
	CMonsterAIAutomata();
	virtual ~CMonsterAIAutomata();
	static void LoadData(char* path);
	static void DelAllAutomata();
	static CMonsterAIAutomataInfo* FindAutomataInfo(int AutomataNumber);
public:
	static BOOL m_DataLoad;
	static CMonsterAIAutomataInfo m_MonsterAIAutomataInfoArray[MAX_MONSTER_AI_AUTOMATA_INFO];
};
