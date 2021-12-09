// MonsterAIState.h: interface for the CMonsterAIState class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

enum MONSTER_AI_STATE_TRANSITION_TYPE
{
	MAI_STATE_TRANS_NO_ENEMY = 0,
	MAI_STATE_TRANS_IN_ENEMY = 1,
	MAI_STATE_TRANS_OUT_ENEMY = 2,
	MAI_STATE_TRANS_DIE_ENEMY = 3,
	MAI_STATE_TRANS_DEC_HP = 4,
	MAI_STATE_TRANS_DEC_HP_PER = 5,
	MAI_STATE_TRANS_IMMEDIATELY = 6,
	MAI_STATE_TRANS_AGRO_UP = 7,
	MAI_STATE_TRANS_AGRO_DOWN = 8,
	MAI_STATE_TRANS_GROUP_SOMMON = 10,
	MAI_STATE_TRANS_GROUP_HEAL = 11,
	MAI_STATE_TRANS_SUPPORT_HEAL = 12,
};

enum MONSTER_AI_STATE_TRANSITION_VALUE_TYPE
{
	MAI_STATE_TRANS_VALUE = 0,
};

class CMonsterAIState
{
public:
	CMonsterAIState();
	void Reset();
public:
	int m_Priority;
	int m_CurrentState;
	int m_NextState;
	int m_TransitionType;
	int m_TransitionRate;
	int m_TransitionValueType;
	int m_TransitionValue;
	int m_DelayTime;
	char m_StateTransitionDesc[100];
};
