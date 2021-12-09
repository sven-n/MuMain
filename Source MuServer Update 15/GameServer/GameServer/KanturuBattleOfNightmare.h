// KanturuBattleOfNightmare.h: interface for the CKanturuBattleOfNightmare class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KanturuStateInfo.h"

#define KANTURU_NIGHTMARE_STATE_INFO 6
#define KANTURU_NIGHTMARE_NONE 0
#define KANTURU_NIGHTMARE_IDLE 1
#define KANTURU_NIGHTMARE_NOTIFY 2
#define KANTURU_NIGHTMARE_START 3
#define KANTURU_NIGHTMARE_END 4
#define KANTURU_NIGHTMARE_ENDCYCLE 5

class CKanturuBattleOfNightmare
{
public:
	CKanturuBattleOfNightmare();
	virtual ~CKanturuBattleOfNightmare();
	void ResetAllData();
	void Load(char* path);
	void MainProc();
	void ProcState_NONE();
	void ProcState_IDLE();
	void ProcState_NOTIFY();
	void ProcState_START();
	void ProcState_END();
	void ProcState_ENDCYCLE();
	void SetState(int state);
	void SetState_NONE();
	void SetState_IDLE();
	void SetState_NOTIFY();
	void SetState_START();
	void SetState_END();
	void SetState_ENDCYCLE();
	void CheckStateTime();
	void CheckNightmareBattleUser();
	void SetBattleOfNightmareState(int state);
	void SetSuccessValue(int success);
	int GetBattleOfNightmareState();
	int GetSuccessValue();
	int GetRemainTime();
private:
	int m_BattleOfNightmareState;
	int m_IsSucccess;
	int m_FileDataLoad;
	CKanturuStateInfo m_StateInfo[KANTURU_NIGHTMARE_STATE_INFO];
	int m_StateInfoCount;
	int m_NightmareAIGroupNumber;
	int m_NightmareObjIndex;
};
