// MonsterSkillElementOption.cpp: implementation of the CMonsterSkillElementOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSkillElementOption.h"
#include "MonsterSkillElement.h"
#include "Protocol.h"
#include "User.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSkillElementOption::CMonsterSkillElementOption() // OK
{
	this->Reset();
}

void CMonsterSkillElementOption::Reset() // OK
{
	this->ResetDefense();
	this->ResetAttack();
	this->ResetAutoHP();
	this->ResetAutoMP();
	this->ResetAutoAG();
	this->ResetImmune();
	this->ResetResist();
	this->ResetModifyStat();
	this->ResetBerserk();
}

void CMonsterSkillElementOption::ResetDefense() // OK
{
	this->m_SkillElementDefense = 0;
	this->m_SkillElementDefenseTime = 0;
}

void CMonsterSkillElementOption::ResetAttack() // OK
{
	this->m_SkillElementAttack = 0;
	this->m_SkillElementAttackTime = 0;
}

void CMonsterSkillElementOption::ResetAutoHP() // OK
{
	this->m_SkillElementAutoHP = 0;
	this->m_SkillElementAutoHPCycle = 0;
	this->m_SkillElementAutoHPTime = 0;
}

void CMonsterSkillElementOption::ResetAutoMP() // OK
{
	this->m_SkillElementAutoMP = 0;
	this->m_SkillElementAutoMPCycle = 0;
	this->m_SkillElementAutoMPTime = 0;
}

void CMonsterSkillElementOption::ResetAutoAG() // OK
{
	this->m_SkillElementAutoAG = 0;
	this->m_SkillElementAutoAGCycle = 0;
	this->m_SkillElementAutoAGTime = 0;
}

void CMonsterSkillElementOption::ResetImmune() // OK
{
	this->m_SkillElementImmuneNumber = 0;
	this->m_SkillElementImmuneTime = 0;
}

void CMonsterSkillElementOption::ResetResist() // OK
{
	this->m_SkillElementResistNumber = 0;
	this->m_SkillElementResistTime = 0;
}

void CMonsterSkillElementOption::ResetModifyStat() // OK
{
	this->m_SkillElementModifyStat = 0;
	this->m_SkillElementModifyStatType = 0;
	this->m_SkillElementModifyStatTime = 0;
}

void CMonsterSkillElementOption::ResetBerserk() // OK
{
	this->m_SkillElementBerserkTime = 0;
}

BOOL CMonsterSkillElementOption::CheckDefenseTime() // OK
{
	return ((this->m_SkillElementDefenseTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckAttackTime() // OK
{
	return ((this->m_SkillElementAttackTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckAutoHPTime() // OK
{
	return ((this->m_SkillElementAutoHPTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckAutoMPTime() // OK
{
	return ((this->m_SkillElementAutoMPTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckAutoAGTime() // OK
{
	return ((this->m_SkillElementAutoAGTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckImmuneTime() // OK
{
	return ((this->m_SkillElementImmuneTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckResistTime() // OK
{
	return ((this->m_SkillElementResistTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckModifyStatTime() // OK
{
	return ((this->m_SkillElementModifyStatTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::CheckBerserkTime() // OK
{
	return ((this->m_SkillElementBerserkTime<=0)?0:1);
}

BOOL CMonsterSkillElementOption::DecDefenseTime() // OK
{
	return (((--this->m_SkillElementDefense)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecAttackTime() // OK
{
	return (((--this->m_SkillElementAttack)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecAutoHPTime() // OK
{
	return (((--this->m_SkillElementAutoHPTime)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecAutoMPTime() // OK
{
	return (((--this->m_SkillElementAutoMPTime)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecAutoAGTime() // OK
{
	return (((--this->m_SkillElementAutoAGTime)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecImmuneTime() // OK
{
	return (((--this->m_SkillElementImmuneTime)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecResistTime() // OK
{
	return (((--this->m_SkillElementResistTime)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecModifyStatTime() // OK
{
	return (((--this->m_SkillElementModifyStatTime)>0)?0:1);
}

BOOL CMonsterSkillElementOption::DecBerserkTime() // OK
{
	return (((--this->m_SkillElementBerserkTime)>0)?0:1);
}

void CMonsterSkillElementOption::CycleAutoHP(LPOBJ lpObj) // OK
{
	if(lpObj->MonsterSkillElementOption.m_SkillElementAutoHPCycle == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		lpObj->Life += lpObj->MonsterSkillElementOption.m_SkillElementAutoHP;
	}
	else
	{
		lpObj->Life += (lpObj->Life*lpObj->MonsterSkillElementOption.m_SkillElementAutoHP)/100;
	}

	lpObj->Life = ((lpObj->Life<0)?0:lpObj->Life);

	lpObj->Life = ((lpObj->Life>(lpObj->MaxLife+lpObj->AddLife))?(lpObj->MaxLife+lpObj->AddLife):lpObj->Life);

	GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
}

void CMonsterSkillElementOption::CycleAutoMP(LPOBJ lpObj) // OK
{
	if(lpObj->MonsterSkillElementOption.m_SkillElementAutoMPCycle == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		lpObj->Mana += lpObj->MonsterSkillElementOption.m_SkillElementAutoMP;
	}
	else
	{
		lpObj->Mana += (lpObj->Mana*lpObj->MonsterSkillElementOption.m_SkillElementAutoMP)/100;
	}

	lpObj->Mana = ((lpObj->Mana<0)?0:lpObj->Mana);

	lpObj->Mana = ((lpObj->Mana>(lpObj->MaxMana+lpObj->AddMana))?(lpObj->MaxMana+lpObj->AddMana):lpObj->Mana);

	GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
}

void CMonsterSkillElementOption::CycleAutoAG(LPOBJ lpObj) // OK
{
	if(lpObj->MonsterSkillElementOption.m_SkillElementAutoAGCycle == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		lpObj->BP += lpObj->MonsterSkillElementOption.m_SkillElementAutoAG;
	}
	else
	{
		lpObj->BP += (lpObj->BP*lpObj->MonsterSkillElementOption.m_SkillElementAutoAG)/100;
	}

	lpObj->BP = ((lpObj->BP<0)?0:lpObj->BP);

	lpObj->BP = ((lpObj->BP>(lpObj->MaxBP+lpObj->AddBP))?(lpObj->MaxBP+lpObj->AddBP):lpObj->BP);

	GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
}

void CMonsterSkillElementOption::CheckSkillElementOptionProc(LPOBJ lpObj) // OK
{
	((lpObj->MonsterSkillElementOption.CheckDefenseTime()==0)?0:((lpObj->MonsterSkillElementOption.DecDefenseTime()==0)?0:lpObj->MonsterSkillElementOption.ResetDefense()));

	((lpObj->MonsterSkillElementOption.CheckAttackTime()==0)?0:((lpObj->MonsterSkillElementOption.DecAttackTime()==0)?0:lpObj->MonsterSkillElementOption.ResetAttack()));

	((lpObj->MonsterSkillElementOption.CheckAutoHPTime()==0)?0:((lpObj->MonsterSkillElementOption.DecAutoHPTime()==0)?lpObj->MonsterSkillElementOption.CycleAutoHP(lpObj):lpObj->MonsterSkillElementOption.ResetAutoHP()));

	((lpObj->MonsterSkillElementOption.CheckAutoMPTime()==0)?0:((lpObj->MonsterSkillElementOption.DecAutoMPTime()==0)?lpObj->MonsterSkillElementOption.CycleAutoMP(lpObj):lpObj->MonsterSkillElementOption.ResetAutoMP()));

	((lpObj->MonsterSkillElementOption.CheckAutoAGTime()==0)?0:((lpObj->MonsterSkillElementOption.DecAutoAGTime()==0)?lpObj->MonsterSkillElementOption.CycleAutoAG(lpObj):lpObj->MonsterSkillElementOption.ResetAutoAG()));

	((lpObj->MonsterSkillElementOption.CheckImmuneTime()==0)?0:((lpObj->MonsterSkillElementOption.DecImmuneTime()==0)?0:lpObj->MonsterSkillElementOption.ResetImmune()));

	((lpObj->MonsterSkillElementOption.CheckResistTime()==0)?0:((lpObj->MonsterSkillElementOption.DecResistTime()==0)?0:lpObj->MonsterSkillElementOption.ResetResist()));

	((lpObj->MonsterSkillElementOption.CheckModifyStatTime()==0)?0:((lpObj->MonsterSkillElementOption.DecModifyStatTime()==0)?0:lpObj->MonsterSkillElementOption.ResetModifyStat()));

	((lpObj->MonsterSkillElementOption.CheckBerserkTime()==0)?0:((lpObj->MonsterSkillElementOption.DecBerserkTime()==0)?0:lpObj->MonsterSkillElementOption.ResetBerserk()));
}
