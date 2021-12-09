// RaklionSelupan.h: interface for the CRaklionSelupan class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CRaklionSelupan
{
public:
	CRaklionSelupan();
	virtual ~CRaklionSelupan();
	void ClearData();
	void SelupanAct_FirstSkill();
	void SelupanAct_PoisonAttack();
	void SelupanAct_IceStorm();
	void SelupanAct_IceStrike();
	void SelupanAct_SummonMonster();
	void SelupanAct_Heal();
	void SelupanAct_Freeze();
	void SelupanAct_Teleport();
	void SelupanAct_Invincibility();
	void SelupanAct_BerserkCansel();
	void SelupanAct_Berserk1();
	void SelupanAct_Berserk2();
	void SelupanAct_Berserk3();
	void SelupanAct_Berserk4();
	bool CreateSelupan();
	void DeleteSelupan();
	void SetSelupanObjIndex(int index);
	void SetSelupanSkillDelay(int delay);
	void CreateSummonMonster();
	void DeleteSummonMonster();
	int GetSelupanLife();
	int GetSelupanMaxLife();
	int GetSelupanObjIndex();
	int GetSelupanTargetIndex();
	int GetSelupanSkillDelay();
	int GetBerserkIndex();
private:
	int m_SelupanObjIndex;
	int m_SelupanSkillDelay;
	int m_BerserkIndex;
	int m_BossAttackMin;
	int m_BossAttackMax;
};
