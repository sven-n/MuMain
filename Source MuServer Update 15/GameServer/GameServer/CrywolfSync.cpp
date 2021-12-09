// CrywolfSync.cpp: implementation of the CCrywolfSync class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrywolfSync.h"

CCrywolfSync gCrywolfSync;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrywolfSync::CCrywolfSync() // OK
{
	this->m_CrywolfEnable = 0;
	this->m_ApplyBenefit = 0;
	this->m_ApplyPenalty = 0;
	this->m_CrywolfState = 0;
	this->m_OccupationState = 1;
	this->m_PlusChaosRate = 0;
	this->m_GemDropPenaltyRate = 100;
	this->m_GettingExpPenaltyRate = 100;
	this->m_MinusMonHPBenefitRate = 100;
	this->m_KundunHPRefillState = 1;
}

CCrywolfSync::~CCrywolfSync() // OK
{

}

BOOL CCrywolfSync::CheckEnableCrywolf() // OK
{
	return this->m_CrywolfEnable;
}

BOOL CCrywolfSync::CheckApplyBenefit() // OK
{
	return this->m_ApplyBenefit;
}

BOOL CCrywolfSync::CheckApplyPenalty() // OK
{
	return this->m_ApplyPenalty;
}

void CCrywolfSync::SetEnableCrywolf(int EnableCrywolf) // OK
{
	this->m_CrywolfEnable = EnableCrywolf;
}

void CCrywolfSync::SetApplyBenefit(int ApplyBenefit) // OK
{
	this->m_ApplyBenefit = ApplyBenefit;
}

void CCrywolfSync::SetApplyPenalty(int ApplyPenalty) // OK
{
	this->m_ApplyPenalty = ApplyPenalty;
}

void CCrywolfSync::SetCrywolfState(int CrywolfState) // OK
{
	this->m_CrywolfState = CrywolfState;
}

void CCrywolfSync::SetOccupationState(int OccupationState) // OK
{
	this->m_OccupationState = OccupationState;
}

void CCrywolfSync::SetPlusChaosRate(int PlusChaosRate) // OK
{
	this->m_PlusChaosRate = PlusChaosRate;
}

void CCrywolfSync::SetGemDropPenaltiyRate(int GemDropPenaltyRate) // OK
{
	this->m_GemDropPenaltyRate = GemDropPenaltyRate;
}

void CCrywolfSync::SetGettingExpPenaltyRate(int GettingExpPenaltyRate) // OK
{
	this->m_GettingExpPenaltyRate = GettingExpPenaltyRate;
}

void CCrywolfSync::SetMonHPBenefitRate(int MinusMonHPBenefitRate) // OK
{
	this->m_MinusMonHPBenefitRate = MinusMonHPBenefitRate;
}

void CCrywolfSync::SetKundunHPRefillState(int KundunHPRefillState) // OK
{
	this->m_KundunHPRefillState = KundunHPRefillState;
}

int CCrywolfSync::GetCrywolfState() // OK
{
	return this->m_CrywolfState;
}

int CCrywolfSync::GetOccupationState() // OK
{
	return this->m_OccupationState;
}

int CCrywolfSync::GetPlusChaosRate() // OK
{
	return this->m_PlusChaosRate;
}

int CCrywolfSync::GetGemDropPenaltiyRate() // OK
{
	return this->m_GemDropPenaltyRate;
}

int CCrywolfSync::GetGettingExpPenaltyRate() // OK
{
	return this->m_GettingExpPenaltyRate;
}

int CCrywolfSync::GetMonHPBenefitRate() // OK
{
	return this->m_MinusMonHPBenefitRate;
}

int CCrywolfSync::GetKundunHPRefillState() // OK
{
	return this->m_KundunHPRefillState;
}
