// KanturuBattleStanby.h: interface for the CKanturuBattleStanby class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KanturuStateInfo.h"

#define KANTURU_STANBY_STATE_INFO 4
#define KANTURU_STANBY_NONE 0
#define KANTURU_STANBY_START 1
#define KANTURU_STANBY_NOTIFY 2
#define KANTURU_STANBY_END 3

class CKanturuBattleStanby
{
public:
	CKanturuBattleStanby();
	virtual ~CKanturuBattleStanby();
	void ResetAllData();
	void Load(char* path);
	void MainProc();
	void ProcState_NONE();
	void ProcState_START();
	void ProcState_NOTIFY();
	void ProcState_END();
	void SetState(int state);
	void SetState_NONE();
	void SetState_START();
	void SetState_NOTIFY();
	void SetState_END();
	void CheckStateTime();
	void SetBattleStanbyState(int state);
	void SetSuccessValue(int success);
	int GetBattleStanbyState();
	int GetRemainTime();
private:
	int m_BattleStanbyState;
	int m_IsSucccess;
	int m_FileDataLoad;
	CKanturuStateInfo m_StateInfo[KANTURU_STANBY_STATE_INFO];
	int m_StateInfoCount;
};
