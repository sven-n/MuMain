// KanturuTowerOfRefinement.h: interface for the CKanturuTowerOfRefinement class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KanturuStateInfo.h"

#define KANTURU_TOWER_OF_REFINEMENT_STATE_INFO 6
#define KANTURU_TOWER_OF_REFINEMENT_NONE 0
#define KANTURU_TOWER_OF_REFINEMENT_REVITALIZATION 1
#define KANTURU_TOWER_OF_REFINEMENT_NOTIFY1 2
#define KANTURU_TOWER_OF_REFINEMENT_CLOSE 3
#define KANTURU_TOWER_OF_REFINEMENT_NOTIFY2 4
#define KANTURU_TOWER_OF_REFINEMENT_END 5

class CKanturuTowerOfRefinement
{
public:
	CKanturuTowerOfRefinement();
	virtual ~CKanturuTowerOfRefinement();
	void ResetAllData();
	void Load(char* path);
	void MainProc();
	void ProcState_NONE();
	void ProcState_REVITALIZATION();
	void ProcState_NOTIFY1();
	void ProcState_CLOSE();
	void ProcState_NOTIFY2();
	void ProcState_END();
	void SetState(int state);
	void SetState_NONE();
	void SetState_REVITALIZATION();
	void SetState_NOTIFY1();
	void SetState_CLOSE();
	void SetState_NOTIFY2();
	void SetState_END();
	void CheckStateTime();
	void SetTowerOfRefinementState(int state);
	void SetEntrancePermit(int permit);
	void SetUseTowerOfRefinement(int use);
	int GetTowerOfRefinementState();
	int GetEntrancePermit();
	int GetUseTowerOfRefinement();
	int GetRemainTime();
private:
	int m_TowerOfRefinementState;
	int m_EntrancePermit;
	int m_IsUseTowerOfRefinement;
	int m_FileDataLoad;
	CKanturuStateInfo m_StateInfo[KANTURU_TOWER_OF_REFINEMENT_STATE_INFO];
	int m_StateInfoCount;
};
