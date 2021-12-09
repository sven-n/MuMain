// MonsterAIAutomataInfo.cpp: implementation of the CMonsterAIAutomataInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIAutomataInfo.h"
#include "MonsterAIGroup.h"
#include "MonsterAIUtil.h"
#include "User.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIAutomataInfo::CMonsterAIAutomataInfo() // OK
{
	this->Reset();
}

CMonsterAIAutomataInfo::~CMonsterAIAutomataInfo() // OK
{

}

void CMonsterAIAutomataInfo::Reset() // OK
{
	this->m_AutomataNumber = -1;

	for(int n=0;n < MAX_AI_STATE;n++)for(int i=0;i < MAX_AI_PRIORITY;i++)
	{
		this->m_AIState[n][i].Reset();
	}

	memset(this->m_AIStateTransCount,0,sizeof(this->m_AIStateTransCount));
}

CMonsterAIState* CMonsterAIAutomataInfo::RunAutomataInfo(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->CurrentAIState < 0 || lpObj->CurrentAIState >= MAX_AI_STATE)
	{
		return 0;
	}

	lpObj->TargetNumber = lpObj->Agro.GetMaxAgroUserIndex(aIndex);

	if(OBJECT_RANGE(lpObj->TargetNumber) != 0 && CMonsterAIUtil::FindMonViewportObj(aIndex,lpObj->TargetNumber) == 0)
	{
		lpObj->TargetNumber = -1;
	}

	for(int n=0;n < this->m_AIStateTransCount[lpObj->CurrentAIState];n++)
	{
		CMonsterAIState* lpMonsterAIState = &this->m_AIState[lpObj->CurrentAIState][n];

		switch(lpMonsterAIState->m_TransitionType)
		{
			case MAI_STATE_TRANS_NO_ENEMY:
				if(OBJECT_RANGE(lpObj->TargetNumber) == 0)
				{
					if((GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_IN_ENEMY:
				if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
				{
					if(gObjCalcDistance(lpObj,&gObj[lpObj->TargetNumber]) <= lpObj->AttackRange && (GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_OUT_ENEMY:
				if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
				{
					if(gObjCalcDistance(lpObj,&gObj[lpObj->TargetNumber]) >= lpObj->AttackRange && (GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_DEC_HP:
				if(lpObj->Life <= lpMonsterAIState->m_TransitionValue)
				{
					if((GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_DEC_HP_PER:
				if(lpObj->Life <= (((lpObj->MaxLife+lpObj->AddLife)*lpMonsterAIState->m_TransitionValue)/100))
				{
					if((GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_IMMEDIATELY:
				if((GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
				{
					lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
					return lpMonsterAIState;
				}
				break;
			case MAI_STATE_TRANS_AGRO_UP:
				if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
				{
					if(lpObj->Agro.GetAgro(lpObj->TargetNumber) >= lpMonsterAIState->m_TransitionValue && (GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_AGRO_DOWN:
				if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
				{
					if(lpObj->Agro.GetAgro(lpObj->TargetNumber) <= lpMonsterAIState->m_TransitionValue && (GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_GROUP_SOMMON:
				if(lpObj->GroupNumber >= 0 && lpObj->GroupNumber < MAX_MONSTER_AI_GROUP)
				{
					if(CMonsterAIGroup::FindGroupMemberToSommon(aIndex,lpObj->GroupNumber,lpObj->GroupMemberGuid) != 0 && (GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_GROUP_HEAL:
				if(lpObj->GroupNumber >= 0 && lpObj->GroupNumber < MAX_MONSTER_AI_GROUP)
				{
					if(CMonsterAIGroup::FindGroupMemberToHeal(aIndex,lpObj->GroupNumber,lpObj->GroupMemberGuid,6,lpMonsterAIState->m_TransitionValue) != 0 && (GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
			case MAI_STATE_TRANS_SUPPORT_HEAL:
				if(OBJECT_RANGE(CMonsterAIUtil::FindMonsterToHeal(lpObj,lpMonsterAIState->m_TransitionValue,6)) != 0)
				{
					if((GetLargeRand()%100) < lpMonsterAIState->m_TransitionRate)
					{
						lpObj->LastAutomataDelay = lpMonsterAIState->m_DelayTime;
						return lpMonsterAIState;
					}
				}
				break;
		}
	}

	return 0;
}
