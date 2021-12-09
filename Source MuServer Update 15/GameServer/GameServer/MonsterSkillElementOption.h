// MonsterSkillElementOption.h: interface for the CMonsterSkillElementOption class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CMonsterSkillElementOption
{
public:
	CMonsterSkillElementOption();
	void Reset();
	void ResetDefense();
	void ResetAttack();
	void ResetAutoHP();
	void ResetAutoMP();
	void ResetAutoAG();
	void ResetImmune();
	void ResetResist();
	void ResetModifyStat();
	void ResetBerserk();
	BOOL CheckDefenseTime();
	BOOL CheckAttackTime();
	BOOL CheckAutoHPTime();
	BOOL CheckAutoMPTime();
	BOOL CheckAutoAGTime();
	BOOL CheckImmuneTime();
	BOOL CheckResistTime();
	BOOL CheckModifyStatTime();
	BOOL CheckBerserkTime();
	BOOL DecDefenseTime();
	BOOL DecAttackTime();
	BOOL DecAutoHPTime();
	BOOL DecAutoMPTime();
	BOOL DecAutoAGTime();
	BOOL DecImmuneTime();
	BOOL DecResistTime();
	BOOL DecModifyStatTime();
	BOOL DecBerserkTime();
	void CycleAutoHP(struct OBJECTSTRUCT* lpObj);
	void CycleAutoMP(struct OBJECTSTRUCT* lpObj);
	void CycleAutoAG(struct OBJECTSTRUCT* lpObj);
	static void CheckSkillElementOptionProc(struct OBJECTSTRUCT* lpObj);
public:
	int m_SkillElementDefense;
	int m_SkillElementDefenseTime;
	int m_SkillElementAttack;
	int m_SkillElementAttackTime;
	int m_SkillElementAutoHP;
	int m_SkillElementAutoHPCycle;
	int m_SkillElementAutoHPTime;
	int m_SkillElementAutoMP;
	int m_SkillElementAutoMPCycle;
	int m_SkillElementAutoMPTime;
	int m_SkillElementAutoAG;
	int m_SkillElementAutoAGCycle;
	int m_SkillElementAutoAGTime;
	int m_SkillElementImmuneNumber;
	int m_SkillElementImmuneTime;
	int m_SkillElementResistNumber;
	int m_SkillElementResistTime;
	int m_SkillElementModifyStat;
	int m_SkillElementModifyStatType;
	int m_SkillElementModifyStatTime;
	int m_SkillElementBerserkTime;
};
