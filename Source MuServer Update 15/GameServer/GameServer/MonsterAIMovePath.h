// MonsterAIMovePath.h: interface for the CMonsterAIMovePath class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIMovePathInfo.h"

#define MAX_MONSTER_AI_MOVE_PATH_INFO 300

class CMonsterAIMovePath
{
public:
	CMonsterAIMovePath();
	virtual ~CMonsterAIMovePath();
	void LoadData(char* path);
	void DelAllAIMonsterMovePath();
	void GetMonsterNearestPath(int sx,int sy,int tx,int ty,int* ox,int* oy,int NearestDistance);
public:
	BOOL m_DataLoad;
	CMonsterAIMovePathInfo m_MonsterAIMovePathInfoArray[MAX_MONSTER_AI_MOVE_PATH_INFO];
	int m_MonsterAIMovePathInfoCount;
};
