// KanturuBattleOfMaya.h: interface for the CKanturuBattleOfMaya class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "KanturuStateInfo.h"
#include "KanturuMaya.h"

#define KANTURU_MAYA_AI_CHANGE_TIME 4
#define KANTURU_MAYA_TIME_INFO 3
#define KANTURU_MAYA_STATE_INFO 19
#define KANTURU_MAYA_GROUP_NUMBER 3
#define KANTURU_MAYA_NONE 0
#define KANTURU_MAYA_STANBY1 1
#define KANTURU_MAYA_NOTIFY1 2
#define KANTURU_MAYA_START1 3
#define KANTURU_MAYA1 4
#define KANTURU_MAYA_END1 5
#define KANTURU_MAYA_ENDCYCLE1 6
#define KANTURU_MAYA_STANBY2 7
#define KANTURU_MAYA_START2 8
#define KANTURU_MAYA2 9
#define KANTURU_MAYA_END2 10
#define KANTURU_MAYA_ENDCYCLE2 11
#define KANTURU_MAYA_STANBY3 12
#define KANTURU_MAYA_START3 13
#define KANTURU_MAYA3 14
#define KANTURU_MAYA_END3 15
#define KANTURU_MAYA_ENDCYCLE3 16
#define KANTURU_MAYA_END 17
#define KANTURU_MAYA_ENDCYCLE 18

class CKanturuBattleOfMaya
{
public:
	CKanturuBattleOfMaya();
	virtual ~CKanturuBattleOfMaya();
	void ResetAllData();
	void Load(char* path);
	void MainProc();
	void ProcState_NONE();
	void ProcState_STANBY1();
	void ProcState_NOTIFY1();
	void ProcState_START1();
	void ProcState_MAYA1();
	void ProcState_END_MAYA1();
	void ProcState_ENDCYCLE_MAYA1();
	void ProcState_STANBY2();
	void ProcState_START2();
	void ProcState_MAYA2();
	void ProcState_END_MAYA2();
	void ProcState_ENDCYCLE_MAYA2();
	void ProcState_STANBY3();
	void ProcState_START3();
	void ProcState_MAYA3();
	void ProcState_END_MAYA3();
	void ProcState_ENDCYCLE_MAYA3();
	void ProcState_END();
	void ProcState_ENDCYCLE();
	void SetState(int state);
	void SetState_NONE();
	void SetState_STANBY1();
	void SetState_NOTIFY1();
	void SetState_START1();
	void SetState_MAYA1();
	void SetState_END_MAYA1();
	void SetState_ENDCYCLE_MAYA1();
	void SetState_STANBY2();
	void SetState_START2();
	void SetState_MAYA2();
	void SetState_END_MAYA2();
	void SetState_ENDCYCLE_MAYA2();
	void SetState_STANBY3();
	void SetState_START3();
	void SetState_MAYA3();
	void SetState_END_MAYA3();
	void SetState_ENDCYCLE_MAYA3();
	void SetState_END();
	void SetState_ENDCYCLE();
	void CheckStateTime();
	void CheckMayaBattleTime();
	void CheckMayaBattleUser();
	void ResetAIMonster();
	void SetAIMonsterGroup(int GroupNumber);
	void ChangeAI(int GroupNumber);
	void SetBattleOfMayaState(int state);
	void SetBattleSceneState(int scene);
	void SetEntrancePermit(int permit);
	void SetSuccessValue(int success);
	void SetSceneSuccessValue(int success);
	int GetBattleOfMayaState();
	int GetBattleSceneState();
	int GetRemainTime();
	int GetEntrancePermit();
	int GetSuccessValue();
	int GetSceneSuccessValue();
private:
	int m_FileDataLoad;
	int m_BattleOfMayaState;
	int m_SceneState;
	CKanturuStateInfo m_BattleTimeInfo[KANTURU_MAYA_TIME_INFO];
	int m_BattleTimeInfoCount;
	CKanturuStateInfo m_StateInfo[KANTURU_MAYA_STATE_INFO];
	int m_StateInfoCount;
	int m_EntrancePermit;
	int m_IsSucccess;
	int m_IsSceneSuccess;
	CKanturuMaya m_KanturuMaya;
	int m_MayaHandDieTimeCounter;
	int m_MayaLeftHandObjIndex;
	int m_MayaRightHandObjIndex;
	int m_MayaHandAIChangeTime[KANTURU_MAYA_AI_CHANGE_TIME];
	int m_MayaHandAIChangeTimeCount;
	int m_MayaHandAIAppliedTime;
	int m_MayaHandCurrentAI;
	int m_MayaHandGroupNumber[KANTURU_MAYA_GROUP_NUMBER];
	int m_MayaHAndGroupNumberCount;
	int m_MayaIceStormUsingRate;
};
