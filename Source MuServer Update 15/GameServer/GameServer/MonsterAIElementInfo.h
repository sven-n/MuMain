// MonsterAIElementInfo.h: interface for the CMonsterAIElementInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIState.h"

enum MONSTER_AI_ELEMENT_TYPE
{
	MAE_TYPE_COMMON_NORMAL = 1,
	MAE_TYPE_MOVE_NORMAL = 11,
	MAE_TYPE_MOVE_TARGET = 12,
	MAE_TYPE_GROUP_MOVE = 13,
	MAE_TYPE_GROUP_MOVE_TARGET = 14,
	MAE_TYPE_ATTACK_NORMAL = 21,
	MAE_TYPE_ATTACK_AREA = 22,
	MAE_TYPE_ATTACK_PENETRATION = 23,
	MAE_TYPE_HEAL_SELF = 31,
	MAE_TYPE_HEAL_GROUP = 32,
	MAE_TYPE_AVOID_NORMAL = 41,
	MAE_TYPE_SUPPORT_HEAL = 51,
	MAE_TYPE_SUPPORT_BUFF = 52,
	MAE_TYPE_SPECIAL = 62,
	MAE_TYPE_SPECIAL_SOMMON = 62,
	MAE_TYPE_SPECIAL_IMMUNE = 64,
	MAE_TYPE_SPECIAL_NIGHTMARE_SUMMON = 65,
	MAE_TYPE_SPECIAL_WARP = 66,
	MAE_TYPE_SPECIAL_SKILLATTACK = 67,
	MAE_TYPE_SPECIAL_CHANGEAI = 68,
	MAE_TYPE_EVENT = 71,
};

class CMonsterAIElementInfo
{
public:
	CMonsterAIElementInfo();
	virtual ~CMonsterAIElementInfo();
	void Reset();
	BOOL ForceAIElementInfo(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementCommon(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementMove(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementMoveTarget(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementGroupMove(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementAttack(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementAttackArea(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementAttackPenetration(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementAvoid(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementHealSelf(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementHealGroup(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementSupportHeal(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementSupportBuff(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementSpecialSommon(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementSpecialImmune(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementNightmareSummon(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementNightmareWarp(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementSkillAttack(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
	void ApplyElementAIChange(int aIndex,int bIndex,CMonsterAIState* lpMonsterAIState);
public:
	char m_ElementName[50];
	int m_ElementNumber;
	int m_ElementClass;
	int m_ElementType;
	int m_SuccessRate;
	int m_DelayTime;
	int m_TargetType;
	int m_X;
	int m_Y;
};
