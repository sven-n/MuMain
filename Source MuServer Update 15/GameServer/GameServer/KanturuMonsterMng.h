// KanturuMonsterMng.h: interface for the CKanturuMonsterMng class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KanturuObjInfo.h"

#define MAX_KANTURU_MONSTER 200

struct KANTURU_MONSTER_SETBASE_INFO
{
	BYTE Group;
	WORD Type;
	BYTE Map;
	BYTE Dis;
	BYTE X;
	BYTE Y;
	BYTE Dir;
};

class CKanturuMonsterMng
{
public:
	CKanturuMonsterMng();
	virtual ~CKanturuMonsterMng();
	void ResetLoadData();
	void Load(char* path);
	void SetMonsterSetBaseInfo(KANTURU_MONSTER_SETBASE_INFO info);
	void MonsterDie(int aIndex);
	void ResetRegenMonsterObjData();
	int SetKanturuMonster(int GroupNumber);
	int GetPosition(int index,short map,short* x,short* y);
	int GetMayaObjIndex();
	int GetAliveMonsterCount();
	int IsExistAliveMonster();
private:
	int m_MaxMonsterCount;
	KANTURU_MONSTER_SETBASE_INFO m_SetBaseInfo[MAX_KANTURU_MONSTER];
	int m_MayaObjIndex;
	int m_FileDataLoad;
	CKanturuObjInfo m_KanturuMonster;
	int m_MaxRegenMonsterCount;
	int m_AliveMonsterCount;
};

extern CKanturuMonsterMng gKanturuMonsterMng;
