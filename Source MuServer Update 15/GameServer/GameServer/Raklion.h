// Raklion.h: interface for the CRaklion class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "RaklionBattleOfSelupan.h"
#include "User.h"

#define RAKLION_STATE_IDLE 0
#define RAKLION_STATE_NOTIFY1 1
#define RAKLION_STATE_STANDBY 2
#define RAKLION_STATE_NOTIFY2 3
#define RAKLION_STATE_READY 4
#define RAKLION_STATE_START_BATTLE 5
#define RAKLION_STATE_NOTIFY3 6
#define RAKLION_STATE_CLOSE_DOOR 7
#define RAKLION_STATE_ALL_USER_DIE 8
#define RAKLION_STATE_NOTIFY4 9
#define RAKLION_STATE_END 10

class CRaklion
{
public:
	CRaklion();
	virtual ~CRaklion();
	void ClearData();
	void Load(char* path);
	void MainProc();
	void ProcState_IDLE();
	void ProcState_NOTIFY1();
	void ProcState_STANDBY();
	void ProcState_NOTIFY2();
	void ProcState_READY();
	void ProcState_START_BATTLE();
	void ProcState_NOTIFY3();
	void ProcState_CLOSE_DOOR();
	void ProcState_ALL_USER_DIE();
	void ProcState_NOTIFY4();
	void ProcState_END();
	void SetState(int state);
	void SetState_IDLE();
	void SetState_NOTIFY1();
	void SetState_STANDBY();
	void SetState_NOTIFY2();
	void SetState_READY();
	void SetState_START_BATTLE();
	void SetState_NOTIFY3();
	void SetState_CLOSE_DOOR();
	void SetState_ALL_USER_DIE();
	void SetState_NOTIFY4();
	void SetState_END();
	void SetRaklionState(int state);
	void SetRaklionDetailState(int state);
	void SetRaklionEnable(int enable);
	void SelupanDie(int index);
	void RaklionMonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	bool BossEggRegen();
	bool BossEggDie();
	void BossEggRegenAll();
	void BossEggDeleteAll();
	int IsRaklionEnable();
	int GetRaklionState();
	int GetRaklionDetailState();
	int GetBossEggCount();
private:
	int m_RaklionState;
	int m_IsSucccess;
	int m_IsComplete;
	int m_AppearanceMSec;
	int m_BossZoneCloseMSec;
	int m_BossZoneOpenMSec;
	int m_FileDataLoad;
	int m_AppearanceDelay;
	int m_BossZoneCloseDelay;
	int m_BossZoneOpenDelay;
	int m_BossEggHalf;
	int m_BossEggMax;
	CRaklionBattleOfSelupan m_RaklionBattleOfSelupan;
	int m_BossEggHalfChecked;
	int m_BossEggCount;
	int m_BossEggCountTotal;
};

extern CRaklion gRaklion;
