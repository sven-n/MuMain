// MonsterAI.cpp: implementation of the CMonsterAI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAI.h"
#include "Attack.h"
#include "EffectManager.h"
#include "Map.h"
#include "Monster.h"
#include "MonsterAIRule.h"
#include "MonsterAIUnit.h"
#include "MonsterAIUtil.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAI::CMonsterAI() // OK
{

}

CMonsterAI::~CMonsterAI() // OK
{

}

void CMonsterAI::MonsterAIProc() // OK
{
	CMonsterAIRule::MonsterAIRuleProc();

	for(int n=0;n < MAX_OBJECT_MONSTER;n++)
	{
		if(gObjIsConnected(n) == 0 || gObj[n].Live == 0 || gObj[n].CurrentAI == 0)
		{
			if(gObj[n].CurrentAI != 0)
			{
				CMonsterAI::MonsterStateMsgProc(n);
			}
		}
		else
		{
			CMonsterAI::UpdateCurrentAIUnit(n);
			CMonsterAI::RunAI(n,gObj[n].Class);
		}
	}
}

BOOL CMonsterAI::RunAI(int aIndex,int MonsterClass) // OK
{
	CMonsterAI::MonsterStateMsgProc(aIndex);

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->CurrentAI == 0)
	{
		return 0;
	}

	CMonsterAIUnitInfo* lpMonsterAIUnitInfo = CMonsterAIUnit::FindAIUnitInfo(lpObj->CurrentAI);

	if(lpMonsterAIUnitInfo == 0)
	{
		return 0;
	}

	if((GetTickCount()-lpObj->LastAIRunTime) < ((DWORD)lpMonsterAIUnitInfo->m_DelayTime))
	{
		return 0;
	}

	if(lpMonsterAIUnitInfo->RunAIUnitInfo(aIndex) == 0)
	{
		return 0;
	}

	lpObj->LastAIRunTime = GetTickCount();

	return 1;
}

void CMonsterAI::MonsterStateMsgProc(int aIndex) // OK
{
	for(int n=0;n < MAX_MONSTER_SEND_MSG;n++)
	{
		if(gSMMsg[aIndex][n].MsgCode != -1 && GetTickCount() > ((DWORD)gSMMsg[aIndex][n].MsgTime))
		{
			CMonsterAI::ProcessStateMsg(&gObj[aIndex],gSMMsg[aIndex][n].MsgCode,gSMMsg[aIndex][n].SendUser,gSMMsg[aIndex][n].SubCode);
			gSMMsg[aIndex][n].Clear();
		}
	}
}

void CMonsterAI::ProcessStateMsg(LPOBJ lpObj,int code,int aIndex,int SubCode) // OK
{
	switch(code)
	{
		case 1:
			gObjMonsterDieGiveItem(lpObj,&gObj[aIndex]);
			break;
		case 2:
			if(gObj[aIndex].Live != 0)
			{
				gObjBackSpring(lpObj,&gObj[aIndex]);
			}
			break;
		case 3:
			if(OBJECT_RANGE(lpObj->TargetNumber) != 0)
			{
				lpObj->Agro.DelAgro(lpObj->TargetNumber);
			}

			lpObj->TargetNumber = -1;
			lpObj->LastAttackerID = -1;
			lpObj->NextActionTime = 1000;
			lpObj->ActionState.Attack = 0;
			lpObj->ActionState.Move = 0;
			lpObj->ActionState.Emotion = 0;
			break;
		case 4:
			lpObj->ActionState.Emotion = 3;
			lpObj->ActionState.EmotionCount = 1;
			break;
		case 55:
			if(gObj[aIndex].Live != 0)
			{
				gAttack.Attack(lpObj,&gObj[aIndex],0,0,0,0,0,0);
			}
			break;
		case 56:
			if(gObjCheckResistance(&gObj[aIndex],1) == 0)
			{
				gEffectManager.AddEffect(&gObj[aIndex],0,EFFECT_POISON,SubCode,3,2,lpObj->Index,0);
			}
			break;
		case 57:
			if(gObj[aIndex].Live != 0)
			{
				gObjBackSpring2(&gObj[aIndex],lpObj,SubCode);
			}
			break;
	}
}

BOOL CMonsterAI::UpdateCurrentAIUnit(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Live == 0)
	{
		return 0;
	}

	if(CMonsterAIRule::GetCurrentAIUnit(lpObj->Class) == 0 && lpObj->BasicAI != 0)
	{
		lpObj->CurrentAI = lpObj->BasicAI;
	}
	else
	{
		lpObj->CurrentAI = CMonsterAIRule::GetCurrentAIUnit(lpObj->Class);
	}

	return 1;
}

void CMonsterAI::MonsterMoveProc() // OK
{
	for(int n=0;n < MAX_OBJECT_MONSTER;n++)
	{
		if(gObj[n].CurrentAI != 0)
		{
			CMonsterAI::MonsterMove(n);
		}
	}
}

void CMonsterAI::MonsterMove(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->PathCount == 0 || CMonsterAIUtil::CheckMovingCondition(lpObj) == 0)
	{
		lpObj->PathCur = 0;
		lpObj->PathCount = 0;
		lpObj->PathStartEnd = 0;
		memset(lpObj->PathX,0,sizeof(lpObj->PathX));
		memset(lpObj->PathY,0,sizeof(lpObj->PathY));
		memset(lpObj->PathDir,0,sizeof(lpObj->PathDir));
		return;
	}

	DWORD MoveTime = 0;

	if((lpObj->PathDir[lpObj->PathCur]%2) == 0)
	{
		MoveTime = (DWORD)((lpObj->MoveSpeed+((lpObj->DelayLevel==0)?0:300))*(double)1.3);
	}
	else
	{
		MoveTime = (DWORD)((lpObj->MoveSpeed+((lpObj->DelayLevel==0)?0:300))*(double)1.0);
	}

	if((GetTickCount()-lpObj->PathTime) > MoveTime && lpObj->PathCur < (MAX_ROAD_PATH_TABLE-1))
	{
		if(gMap[lpObj->Map].CheckAttr(lpObj->PathX[lpObj->PathCur],lpObj->PathY[lpObj->PathCur],4) != 0 || gMap[lpObj->Map].CheckAttr(lpObj->PathX[lpObj->PathCur],lpObj->PathY[lpObj->PathCur],8) != 0)
		{
			lpObj->PathCur = 0;
			lpObj->PathCount = 0;
			lpObj->PathTime = GetTickCount();
			lpObj->PathStartEnd = 0;

			memset(lpObj->PathX,0,sizeof(lpObj->PathX));

			memset(lpObj->PathY,0,sizeof(lpObj->PathY));

			memset(lpObj->PathOri,0,sizeof(lpObj->PathOri));

			gObjSetPosition(lpObj->Index,lpObj->X,lpObj->Y);
		}
		else
		{
			lpObj->X = lpObj->PathX[lpObj->PathCur];
			lpObj->Y = lpObj->PathY[lpObj->PathCur];
			lpObj->Dir = lpObj->PathDir[lpObj->PathCur];
			lpObj->PathTime = GetTickCount();

			if((++lpObj->PathCur) >= lpObj->PathCount)
			{
				lpObj->PathCur = 0;
				lpObj->PathCount = 0;
				lpObj->PathStartEnd = 0;
			}
		}
	}
}
