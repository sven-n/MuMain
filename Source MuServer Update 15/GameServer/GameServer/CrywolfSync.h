// CrywolfSync.h: interface for the CCrywolfSync class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CCrywolfSync
{
public:
	CCrywolfSync();
	virtual ~CCrywolfSync();
	BOOL CheckEnableCrywolf();
	BOOL CheckApplyBenefit();
	BOOL CheckApplyPenalty();
	void SetEnableCrywolf(int EnableCrywolf);
	void SetApplyBenefit(int ApplyBenefit);
	void SetApplyPenalty(int ApplyPenalty);
	void SetCrywolfState(int CrywolfState);
	void SetOccupationState(int OccupationState);
	void SetPlusChaosRate(int PlusChaosRate);
	void SetGemDropPenaltiyRate(int GemDropPenaltyRate);
	void SetGettingExpPenaltyRate(int GettingExpPenaltyRate);
	void SetMonHPBenefitRate(int MinusMonHPBenefitRate);
	void SetKundunHPRefillState(int KundunHPRefillState);
	int GetCrywolfState();
	int GetOccupationState();
	int GetPlusChaosRate();
	int GetGemDropPenaltiyRate();
	int GetGettingExpPenaltyRate();
	int GetMonHPBenefitRate();
	int GetKundunHPRefillState();
private:
	BOOL m_CrywolfEnable;
	BOOL m_ApplyBenefit;
	BOOL m_ApplyPenalty;
	int m_CrywolfState;
	int m_OccupationState;
	int m_PlusChaosRate;
	int m_GemDropPenaltyRate;
	int m_GettingExpPenaltyRate;
	int m_MinusMonHPBenefitRate;
	int m_KundunHPRefillState;
};

extern CCrywolfSync gCrywolfSync;
