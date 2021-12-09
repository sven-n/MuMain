// Kanturu.h: interface for the CKanturu class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KanturuBattleOfMaya.h"
#include "KanturuBattleOfNightmare.h"
#include "KanturuBattleStanby.h"
#include "KanturuStateInfo.h"
#include "KanturuTowerOfRefinement.h"
#include "Map.h"

#define MAX_KANTURU_STATE_INFO 6
#define MAX_KANTURU_MAP_LEVEL 2
#define KANTURU_MAP_CLOSE 0
#define KANTURU_MAP_OPEN 1
#define KANTURU_STATE_NONE 0
#define KANTURU_STATE_BATTLE_STANTBY 1
#define KANTURU_STATE_BATTLE_OF_MAYA 2
#define KANTURU_STATE_BATTLE_OF_NIGHTMARE 3
#define KANTURU_STATE_TOWER_OF_REFINEMENT 4
#define KANTURU_STATE_END 5

class CKanturu
{
public:
	CKanturu();
	virtual ~CKanturu();
	void ResetAllData();
	void Load(char* path);
	void MainProc();
	void ProcState_NONE();
	void ProcState_BATTLE_STANDBY();
	void ProcState_BATTLE_OF_MAYA();
	void ProcState_BATTLE_OF_NIGHTMARE();
	void ProcState_TOWER_OF_REFINEMENT();
	void ProcState_END();
	void SetState(int state);
	void SetState_NONE();
	void SetState_BATTLE_STANDBY();
	void SetState_BATTLE_OF_MAYA();
	void SetState_BATTLE_OF_NIGHTMARE();
	void SetState_TOWER_OF_REFINEMENT();
	void SetState_END();
	void CheckStateTime();
	bool CheckEnterKanturu(int aIndex);
	bool CheckEnterKanturuRefinery(int aIndex);
	bool CheckCanEnterKanturuBattle();
	bool CheckEquipmentMoonStone(int aIndex);
	void KanturuMonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void LoadKanturuMapAttr(char* path,int level);
	void SetKanturuMapAttr(int level);
	void SetKanturuEnable(int enable);
	void SetKanturuState(int state);
	void SetEnableCheckMoonStone(int enable);
	int GetKanturuEnable();
	int GetKanturuState();
	int GetKanturuDetailState();
	int GetRemainTime();
	int GetEnableCheckMoonStone();
private:
	int m_KanturuEnable;
	int m_KanturuState;
	int m_IsSucccess;
	int m_FileDataLoad;
	CKanturuStateInfo m_StateInfo[MAX_KANTURU_STATE_INFO];
	int m_StateInfoCount;
	CKanturuBattleStanby m_BattleStanby;
	CKanturuBattleOfMaya m_BattleOfMaya;
	CKanturuBattleOfNightmare m_BattleOfNightmare;
	CKanturuTowerOfRefinement m_TowerOfRefinement;
	CMap m_KanturuMap[MAX_KANTURU_MAP_LEVEL];
	int m_EnableCheckMoonStone;
	int m_KanturuBattleCounter;
	int m_KanturuBattleDate;
};

extern CKanturu gKanturu;
