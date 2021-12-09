// CrywolfStatue.cpp: implementation of the CCrywolfStatue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrywolfStatue.h"
#include "CrywolfAltar.h"
#include "CrywolfUtil.h"
#include "EffectManager.h"
#include "Message.h"
#include "Util.h"

CCrywolfStatue gCrywolfStatue;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrywolfStatue::CCrywolfStatue() // OK
{

}

CCrywolfStatue::~CCrywolfStatue() // OK
{

}

void CCrywolfStatue::CrywolfStatueAct(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Connected != OBJECT_ONLINE)
	{
		return;
	}

	if(lpObj->Type != OBJECT_NPC || lpObj->Class != 204)
	{
		return;
	}

	int ContractedAlterCount = gCrywolfAltar.GetContractedAltarCount();

	int PriestHPSum = gCrywolfAltar.GetPriestHPSum();

	int PriestMaxHPSum = gCrywolfAltar.GetPriestMaxHPSum();

	if(ContractedAlterCount == 0 || this->m_Shield.m_ShieldHP == 0)
	{
		if(this->m_Shield.m_ShieldState == 1)
		{
			gCrywolfUtil.SendCrywolfUserAnyMsg(0,gMessage.GetMessage(400));
			this->m_Shield.m_ShieldState = 0;
			this->m_Shield.m_ShieldHP = 0;
			this->m_Shield.m_ShieldMaxHP = 0;
			this->m_Shield.m_PriestNumber = ContractedAlterCount;
			this->SetStatueViewState(lpObj,ContractedAlterCount);
		}
		else if(this->m_Shield.m_PriestNumber != ContractedAlterCount)
		{
			this->m_Shield.m_ShieldState = 1;
			this->m_Shield.m_ShieldHP = PriestHPSum;
			this->m_Shield.m_ShieldMaxHP = PriestMaxHPSum;
			this->m_Shield.m_PriestNumber = ContractedAlterCount;
			gCrywolfUtil.SendCrywolfUserAnyMsg(0,gMessage.GetMessage(401),ContractedAlterCount,PriestHPSum);
			this->SetStatueViewState(lpObj,ContractedAlterCount);
		}

		return;
	}
	else if(this->m_Shield.m_ShieldState == 0)
	{
		gCrywolfUtil.SendCrywolfUserAnyMsg(0,gMessage.GetMessage(402));
		this->SetStatueViewState(lpObj,ContractedAlterCount);
	}
	else if(this->m_Shield.m_PriestNumber != ContractedAlterCount)
	{
		gCrywolfUtil.SendCrywolfUserAnyMsg(0,gMessage.GetMessage(401),ContractedAlterCount,PriestHPSum);
		this->SetStatueViewState(lpObj,ContractedAlterCount);
	}

	this->m_Shield.m_ShieldState = 1;
	this->m_Shield.m_ShieldHP = PriestHPSum;
	this->m_Shield.m_ShieldMaxHP = PriestMaxHPSum;
	this->m_Shield.m_PriestNumber = ContractedAlterCount;
}

void CCrywolfStatue::SetStatueViewState(LPOBJ lpObj,int PriestNumber) // OK
{
	switch(PriestNumber)
	{
		case 1:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CRYWOLF_STATE8,0,0,0,0,0);
			break;
		case 2:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CRYWOLF_STATE9,0,0,0,0,0);
			break;
		case 3:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CRYWOLF_STATE10,0,0,0,0,0);
			break;
		case 4:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CRYWOLF_STATE11,0,0,0,0,0);
			break;
		case 5:
			gEffectManager.AddEffect(lpObj,0,EFFECT_CRYWOLF_STATE12,0,0,0,0,0);
			break;
	}
}
