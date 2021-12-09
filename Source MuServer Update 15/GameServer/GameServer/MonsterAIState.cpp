// MonsterAIState.cpp: implementation of the CMonsterAIState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIState.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIState::CMonsterAIState() // OK
{
	this->Reset();
}

void CMonsterAIState::Reset() // OK
{
	this->m_Priority = -1;
	this->m_CurrentState = -1;
	this->m_NextState = -1;
	this->m_TransitionType = -1;
	this->m_TransitionRate = -1;
	this->m_TransitionValueType = -1;
	this->m_TransitionValue = -1;
	this->m_DelayTime = -1;

	memset(this->m_StateTransitionDesc,0,sizeof(this->m_StateTransitionDesc));
}
