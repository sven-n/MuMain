// MonsterAIElementInfo.cpp: implementation of the CMonsterAIElementInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIElementInfo.h"
#include "Attack.h"
#include "EffectManager.h"
#include "KanturuMonsterMng.h"
#include "Monster.h"
#include "MonsterAIElement.h"
#include "MonsterAIGroup.h"
#include "MonsterAIUtil.h"
#include "MonsterSkillManager.h"
#include "SkillManager.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIElementInfo::CMonsterAIElementInfo() // OK
{
	this->Reset();
}

CMonsterAIElementInfo::~CMonsterAIElementInfo() // OK
{

}

void CMonsterAIElementInfo::Reset() // OK
{
	memset(this->m_ElementName,0,sizeof(this->m_ElementName));

	this->m_ElementNumber = -1;
	this->m_ElementClass = -1;
	this->m_ElementType = -1;
	this->m_SuccessRate = -1;
	this->m_DelayTime = -1;
	this->m_TargetType = -1;
	this->m_X = -1;
	this->m_Y = -1;
}

BOOL CMonsterAIElementInfo::ForceAIElementInfo(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if((GetLargeRand()%100) > this->m_SuccessRate)
	{
		return 0;
	}

	switch(this->m_ElementType)
	{
		case MAE_TYPE_COMMON_NORMAL:
			this->ApplyElementCommon(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_MOVE_NORMAL:
			this->ApplyElementMove(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_MOVE_TARGET:
			this->ApplyElementMoveTarget(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_GROUP_MOVE:
			this->ApplyElementGroupMove(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_ATTACK_NORMAL:
			this->ApplyElementAttack(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_ATTACK_AREA:
			this->ApplyElementAttackArea(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_ATTACK_PENETRATION:
			this->ApplyElementAttackPenetration(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_HEAL_SELF:
			this->ApplyElementHealSelf(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_HEAL_GROUP:
			this->ApplyElementHealGroup(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_AVOID_NORMAL:
			this->ApplyElementAvoid(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SUPPORT_HEAL:
			this->ApplyElementSupportHeal(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SUPPORT_BUFF:
			this->ApplyElementSupportBuff(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SPECIAL_SOMMON:
			this->ApplyElementSpecialSommon(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SPECIAL_IMMUNE:
			this->ApplyElementSpecialImmune(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SPECIAL_NIGHTMARE_SUMMON:
			this->ApplyElementNightmareSummon(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SPECIAL_WARP:
			this->ApplyElementNightmareWarp(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SPECIAL_SKILLATTACK:
			this->ApplyElementSkillAttack(aIndex,bIndex,lpMonsterAIState);
			break;
		case MAE_TYPE_SPECIAL_CHANGEAI:
			this->ApplyElementAIChange(aIndex,bIndex,lpMonsterAIState);
			break;
	}

	return 1;
}

void CMonsterAIElementInfo::ApplyElementCommon(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{

}

void CMonsterAIElementInfo::ApplyElementMove(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->PathStartEnd != 0)
	{
		return;
	}

	if(lpMonsterAIState->m_TransitionType == 2 && CMonsterAIUtil::GetXYToChase(lpObj) != 0)
	{
		CMonsterAIUtil::FindPathToMoveMonster(lpObj,lpObj->MTX,lpObj->MTY,5,1);
		return;
	}

	if(lpMonsterAIState->m_TransitionType != 2 && CMonsterAIUtil::GetXYToPatrol(lpObj) != 0)
	{
		CMonsterAIUtil::FindPathToMoveMonster(lpObj,lpObj->MTX,lpObj->MTY,5,1);
		return;
	}
}

void CMonsterAIElementInfo::ApplyElementMoveTarget(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->PathStartEnd != 0)
	{
		return;
	}

	if(lpObj->X == this->m_X && lpObj->Y == this->m_Y)
	{
		this->ApplyElementMove(aIndex,bIndex,lpMonsterAIState);
		return;
	}

	int PathX = this->m_X;
	int PathY = this->m_Y;

	if(CMonsterAIElement::m_MonsterAIMovePath[lpObj->Map].m_DataLoad == 0)
	{
		lpObj->PathStartEnd = CMonsterAIUtil::FindPathToMoveMonster(lpObj,PathX,PathY,5,0);
		return;
	}

	CMonsterAIElement::m_MonsterAIMovePath[lpObj->Map].GetMonsterNearestPath(lpObj->X,lpObj->Y,PathX,PathY,&PathX,&PathY,(int)sqrt(pow(((float)lpObj->X-(float)PathX),2)+pow(((float)lpObj->Y-(float)PathY),2)));

	lpObj->PathStartEnd = CMonsterAIUtil::FindPathToMoveMonster(lpObj,PathX,PathY,7,0);
}

void CMonsterAIElementInfo::ApplyElementGroupMove(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->PathStartEnd != 0)
	{
		return;
	}

	int LeaderIndex = CMonsterAIGroup::FindGroupLeader(lpObj->GroupNumber);

	if(OBJECT_RANGE(LeaderIndex) != 0 && gObj[LeaderIndex].Live != 0 && gObjCalcDistance(lpObj,&gObj[LeaderIndex]) > 6)
	{
		lpObj->TargetNumber = LeaderIndex;

		if(CMonsterAIUtil::GetXYToChase(lpObj) != 0)
		{
			CMonsterAIUtil::FindPathToMoveMonster(lpObj,lpObj->MTX,lpObj->MTY,5,1);
		}
	}
	else
	{
		if(lpMonsterAIState->m_TransitionType == 2 && CMonsterAIUtil::GetXYToChase(lpObj) != 0)
		{
			CMonsterAIUtil::FindPathToMoveMonster(lpObj,lpObj->MTX,lpObj->MTY,5,1);
			return;
		}

		if(lpMonsterAIState->m_TransitionType != 2 && CMonsterAIUtil::GetXYToPatrol(lpObj) != 0)
		{
			CMonsterAIUtil::FindPathToMoveMonster(lpObj,lpObj->MTX,lpObj->MTY,5,1);
			return;
		}
	}
}

void CMonsterAIElementInfo::ApplyElementAttack(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(CMonsterSkillManager::CheckMonsterSkill(lpObj->Class,0) == 0)
	{
		if(OBJECT_RANGE(lpObj->TargetNumber) == 0 || gObj[lpObj->TargetNumber].Connected != OBJECT_ONLINE || gObj[lpObj->TargetNumber].CloseCount > 0 || gObj[lpObj->TargetNumber].Live == 0 || gObj[lpObj->TargetNumber].Teleport != 0)
		{
			lpObj->TargetNumber = -1;
			lpObj->LastAttackerID = -1;
			lpObj->NextActionTime = 1000;
			lpObj->ActionState.Attack = 0;
			lpObj->ActionState.Move = 0;
			lpObj->ActionState.Emotion = 0;
			return;
		}

		lpObj->Dir = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-lpObj->X),(gObj[lpObj->TargetNumber].Y-lpObj->Y));

		gObjMonsterAttack(lpObj,&gObj[lpObj->TargetNumber]);

		lpObj->ActionState.Attack = 0;
	}
	else
	{
		if(OBJECT_RANGE(lpObj->TargetNumber) == 0 || gObj[lpObj->TargetNumber].Connected != OBJECT_ONLINE || gObj[lpObj->TargetNumber].CloseCount > 0 || gObj[lpObj->TargetNumber].Live == 0 || gObj[lpObj->TargetNumber].Teleport != 0)
		{
			lpObj->TargetNumber = -1;
			lpObj->LastAttackerID = -1;
			lpObj->NextActionTime = 1000;
			lpObj->ActionState.Attack = 0;
			lpObj->ActionState.Move = 0;
			lpObj->ActionState.Emotion = 0;
			return;
		}

		if((GetLargeRand()%4) == 0)
		{
			lpObj->Dir = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-lpObj->X),(gObj[lpObj->TargetNumber].Y-lpObj->Y));

			PMSG_ATTACK_RECV pMsg;

			pMsg.index[0] = SET_NUMBERHB(lpObj->TargetNumber);

			pMsg.index[1] = SET_NUMBERLB(lpObj->TargetNumber);

			pMsg.action = ACTION_ATTACK1;

			pMsg.dir = lpObj->Dir;

			gAttack.CGAttackRecv(&pMsg,aIndex);

			lpObj->ActionState.Attack = 0;
		}
		else
		{
			lpObj->Dir = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-lpObj->X),(gObj[lpObj->TargetNumber].Y-lpObj->Y));

			CMonsterSkillManager::UseMonsterSkill(aIndex,lpObj->TargetNumber,0,-1,0);

			lpObj->ActionState.Attack = 0;
		}
	}
}

void CMonsterAIElementInfo::ApplyElementAttackArea(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	int TargetX = this->m_X+((GetLargeRand()%5)*(((GetLargeRand()%2)==0)?1:-1));
	int TargetY = this->m_Y+((GetLargeRand()%5)*(((GetLargeRand()%2)==0)?1:-1));

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		if(gObj[n].Live == 0 || gObj[n].Map != lpObj->Map)
		{
			continue;
		}

		if(sqrt(pow(((float)gObj[n].X-(float)TargetX),2)+pow(((float)gObj[n].Y-(float)TargetY),2)) < 6)
		{
			gAttack.Attack(lpObj,&gObj[n],0,0,0,0,0,0);
		}

		if(sqrt(pow(((float)gObj[n].X-(float)TargetX),2)+pow(((float)gObj[n].Y-(float)TargetY),2)) < 10)
		{
			GCMonsterAreaSkillSend(n,lpObj->Class,lpObj->X,lpObj->Y,TargetX,TargetY);
		}
	}
}

void CMonsterAIElementInfo::ApplyElementAttackPenetration(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(OBJECT_RANGE(lpObj->TargetNumber) == 0)
	{
		return;
	}

	if(gObj[lpObj->TargetNumber].Live == 0)
	{
		return;
	}

	CMonsterSkillManager::UseMonsterSkill(aIndex,lpObj->TargetNumber,2,-1,0);
}

void CMonsterAIElementInfo::ApplyElementAvoid(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(CMonsterAIUtil::GetXYToEascape(lpObj) == 0)
	{
		return;
	}

	CMonsterAIUtil::FindPathToMoveMonster(lpObj,lpObj->MTX,lpObj->MTY,5,1);
}

void CMonsterAIElementInfo::ApplyElementHealSelf(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo = CMonsterSkillManager::FindMonsterSkillUnitInfo(aIndex,21);

	if(lpMonsterSkillUnitInfo == 0)
	{
		return;
	}

	lpMonsterSkillUnitInfo->RunSkill(aIndex,aIndex);

	GCMonsterSkillSend(lpObj,&gObj[aIndex],lpMonsterSkillUnitInfo->m_UnitNumber);
}

void CMonsterAIElementInfo::ApplyElementHealGroup(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	CMonsterAIGroupMember* lpMonsterAIGroupMember = CMonsterAIGroup::FindGroupMemberToHeal(aIndex,lpObj->GroupNumber,lpObj->GroupMemberGuid,6,lpMonsterAIState->m_TransitionValue);

	if(lpMonsterAIGroupMember == 0)
	{
		return;
	}

	if(gObj[lpMonsterAIGroupMember->m_ObjIndex].Live == 0)
	{
		return;
	}

	CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo = CMonsterSkillManager::FindMonsterSkillUnitInfo(aIndex,21);

	if(lpMonsterSkillUnitInfo == 0)
	{
		return;
	}

	lpMonsterSkillUnitInfo->RunSkill(aIndex,lpMonsterAIGroupMember->m_ObjIndex);

	GCMonsterSkillSend(lpObj,&gObj[lpMonsterAIGroupMember->m_ObjIndex],lpMonsterSkillUnitInfo->m_UnitNumber);
}

void CMonsterAIElementInfo::ApplyElementSupportHeal(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	bIndex = CMonsterAIUtil::FindMonsterToHeal(&gObj[aIndex],lpMonsterAIState->m_TransitionValue,6);

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo = CMonsterSkillManager::FindMonsterSkillUnitInfo(aIndex,21);

	if(lpMonsterSkillUnitInfo == 0)
	{
		return;
	}

	lpMonsterSkillUnitInfo->RunSkill(aIndex,bIndex);

	GCMonsterSkillSend(lpObj,&gObj[bIndex],lpMonsterSkillUnitInfo->m_UnitNumber);
}

void CMonsterAIElementInfo::ApplyElementSupportBuff(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{

}

void CMonsterAIElementInfo::ApplyElementSpecialSommon(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	CMonsterAIGroupMember* lpMonsterAIGroupMember = CMonsterAIGroup::FindGroupMemberToSommon(aIndex,lpObj->GroupNumber,lpObj->GroupMemberGuid);

	if(lpMonsterAIGroupMember == 0)
	{
		return;
	}

	if(gObj[lpMonsterAIGroupMember->m_ObjIndex].Live != 0)
	{
		return;
	}

	CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo = CMonsterSkillManager::FindMonsterSkillUnitInfo(aIndex,30);

	if(lpMonsterSkillUnitInfo == 0)
	{
		return;
	}

	lpMonsterSkillUnitInfo->RunSkill(aIndex,lpMonsterAIGroupMember->m_ObjIndex);

	GCMonsterSkillSend(lpObj,&gObj[lpMonsterAIGroupMember->m_ObjIndex],lpMonsterSkillUnitInfo->m_UnitNumber);
}

void CMonsterAIElementInfo::ApplyElementSpecialImmune(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	this->m_X = ((this->m_X<1)?10:this->m_X);

	gEffectManager.AddEffect(lpObj,0,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,this->m_X,0,0,0,0);

	this->m_Y = ((this->m_Y<1)?10:this->m_Y);

	gEffectManager.AddEffect(lpObj,0,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY,this->m_Y,0,0,0,0);
}

void CMonsterAIElementInfo::ApplyElementNightmareSummon(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo = CMonsterSkillManager::FindMonsterSkillUnitInfo(aIndex,30);

	if(lpMonsterSkillUnitInfo == 0)
	{
		return;
	}

	GCMonsterSkillSend(lpObj,((OBJECT_RANGE(lpObj->TargetNumber)==0)?lpObj:&gObj[lpObj->TargetNumber]),lpMonsterSkillUnitInfo->m_UnitNumber);

	gKanturuMonsterMng.SetKanturuMonster(6);
}

void CMonsterAIElementInfo::ApplyElementNightmareWarp(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	gSkillManager.GCSkillAttackSend(lpObj,SKILL_TELEPORT,aIndex,1);

	gObjTeleportMagicUse(aIndex,this->m_X,this->m_Y);

	lpObj->TargetNumber = -1;
}

void CMonsterAIElementInfo::ApplyElementSkillAttack(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(OBJECT_RANGE(lpObj->TargetNumber) == 0 || gObj[lpObj->TargetNumber].Connected != OBJECT_ONLINE || gObj[lpObj->TargetNumber].CloseCount > 0 || gObj[lpObj->TargetNumber].Live == 0 || gObj[lpObj->TargetNumber].Teleport != 0)
	{
		lpObj->TargetNumber = -1;
		lpObj->LastAttackerID = -1;
		lpObj->NextActionTime = 1000;
		lpObj->ActionState.Attack = 0;
		lpObj->ActionState.Move = 0;
		lpObj->ActionState.Emotion = 0;
		return;
	}

	if(CMonsterSkillManager::CheckMonsterSkill(lpObj->Class,0) != 0 && (GetLargeRand()%100) < this->m_TargetType)
	{
		lpObj->Dir = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-lpObj->X),(gObj[lpObj->TargetNumber].Y-lpObj->Y));
		CMonsterSkillManager::UseMonsterSkill(aIndex,lpObj->TargetNumber,0,-1,0);
		lpObj->ActionState.Attack = 0;
		return;
	}

	if(CMonsterSkillManager::CheckMonsterSkill(lpObj->Class,1) != 0 && (GetLargeRand()%100) < this->m_X)
	{
		lpObj->Dir = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-lpObj->X),(gObj[lpObj->TargetNumber].Y-lpObj->Y));
		CMonsterSkillManager::UseMonsterSkill(aIndex,lpObj->TargetNumber,1,-1,0);
		lpObj->ActionState.Attack = 0;
		return;
	}

	if(CMonsterSkillManager::CheckMonsterSkill(lpObj->Class,2) != 0 && (GetLargeRand()%100) < this->m_Y)
	{
		lpObj->Dir = GetPathPacketDirPos((gObj[lpObj->TargetNumber].X-lpObj->X),(gObj[lpObj->TargetNumber].Y-lpObj->Y));
		CMonsterSkillManager::UseMonsterSkill(aIndex,lpObj->TargetNumber,2,-1,0);
		lpObj->ActionState.Attack = 0;
		return;
	}
}

void CMonsterAIElementInfo::ApplyElementAIChange(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState) // OK
{
	CMonsterAIGroup::ChangeAIOrder(this->m_TargetType,this->m_X);
}
