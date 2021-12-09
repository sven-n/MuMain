// CrywolfAltar.cpp: implementation of the CCrywolfAltar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrywolfAltar.h"
#include "CrywolfUtil.h"
#include "EffectManager.h"
#include "Map.h"
#include "Message.h"
#include "Notice.h"
#include "User.h"
#include "Util.h"

CCrywolfAltar gCrywolfAltar;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrywolfAltar::CCrywolfAltar() // OK
{

}

CCrywolfAltar::~CCrywolfAltar() // OK
{

}

void CCrywolfAltar::CrywolfAltarAct(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	if(OBJECT_RANGE(this->GetAltarUserIndex(lpObj->Class)) == 0)
	{
		return;
	}

	if(lpObj->Type != OBJECT_NPC || (lpObj->Class < 205 || lpObj->Class > 209))
	{
		return;
	}

	int AltarUserIndex = this->GetAltarUserIndex(lpObj->Class);

	if(gObjIsConnectedGP(AltarUserIndex) == 0)
	{
		this->ResetAltarUserIndex(aIndex,lpObj->Class);
		return;
	}

	if(gObj[AltarUserIndex].Live == 0 || gObj[AltarUserIndex].Map != MAP_CRYWOLF)
	{
		this->ResetAltarUserIndex(aIndex,lpObj->Class);
		return;
	}

	if(abs(lpObj->X-gObj[AltarUserIndex].X) > 0 || abs(lpObj->Y-gObj[AltarUserIndex].Y) > 0)
	{
		this->ResetAltarUserIndex(aIndex,lpObj->Class);
		return;
	}

	this->SetAltarValidContract(aIndex,lpObj->Class);
}

void CCrywolfAltar::ResetAllAltar() // OK
{
	this->ResetAltar(205);
	this->ResetAltar(206);
	this->ResetAltar(207);
	this->ResetAltar(208);
	this->ResetAltar(209);
}

void CCrywolfAltar::ResetAltar(int Class) // OK
{
	int AltarNumber = this->GetAltarNumber(Class);

	CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[AltarNumber];

	if(OBJECT_RANGE(lpAltarInfo->m_AltarIndex) == 0)
	{
		return;
	}

	gEffectManager.ClearAllEffect(&gObj[lpAltarInfo->m_AltarIndex]);

	lpAltarInfo->Reset();
}

void CCrywolfAltar::ResetAltarUserIndex(int AltarObjIndex,int Class) // OK
{
	int AltarNumber = this->GetAltarNumber(Class);

	CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[AltarNumber];

	if(gObj[lpAltarInfo->m_UserIndex].Connected == OBJECT_ONLINE)
	{
		gNotice.GCNoticeSend(lpAltarInfo->m_UserIndex,1,0,0,0,0,0,gMessage.GetMessage(399),AltarNumber);
		LogAdd(LOG_BLACK,"[ Crywolf ][Altar Op.] [%s][%s] Remove contract Altar[%d]",gObj[lpAltarInfo->m_UserIndex].Account,gObj[lpAltarInfo->m_UserIndex].Name,AltarNumber);
	}

	if(lpAltarInfo->m_ContractCount >= MAX_CRYWOLF_ALTAR_CONTRACT)
	{
		lpAltarInfo->SetAltarState(3);
		gEffectManager.AddEffect(&gObj[AltarObjIndex],0,EFFECT_CRYWOLF_STATE2,0,0,0,0,0);
	}
	else
	{
		lpAltarInfo->SetAltarState(0);
		gEffectManager.AddEffect(&gObj[AltarObjIndex],0,EFFECT_CRYWOLF_STATE1,0,0,0,0,0);
	}

	lpAltarInfo->m_UserIndex = -1;
	lpAltarInfo->m_AppliedContractTime = 0;
	lpAltarInfo->m_ValidContractTime = 0;
	lpAltarInfo->m_LastValidContractTime = GetTickCount();
}

void CCrywolfAltar::SetAllAltarObjectIndex() // OK
{
	for(int n=0;n < MAX_OBJECT_MONSTER;n++)
	{
		if(gObjIsConnected(n) != 0 && gObj[n].Map == MAP_CRYWOLF && (gObj[n].Class >= 205 && gObj[n].Class <= 209))
		{
			this->SetAltarObjIndex(gObj[n].Class,n);
		}
	}
}

void CCrywolfAltar::SetAllAltarViewState(int AltarState) // OK
{
	this->SetAltarViewState(205,AltarState);
	this->SetAltarViewState(206,AltarState);
	this->SetAltarViewState(207,AltarState);
	this->SetAltarViewState(208,AltarState);
	this->SetAltarViewState(209,AltarState);
}

void CCrywolfAltar::SetAltarObjIndex(int Class,int ObjIndex) // OK
{
	int AltarNumber = this->GetAltarNumber(Class);

	this->ResetAltar(Class);

	CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[AltarNumber];

	lpAltarInfo->SetAltarIndex(ObjIndex);
}

bool CCrywolfAltar::SetAltarUserIndex(int AltarObjIndex,int Class,int UserIndex) // OK
{
	int AltarNumber = this->GetAltarNumber(Class);

	CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[AltarNumber];

	DWORD CurrentTime = GetTickCount();

	if(gObj[lpAltarInfo->m_UserIndex].Connected == OBJECT_ONLINE)
	{
		gNotice.GCNoticeSend(UserIndex,1,0,0,0,0,0,gMessage.GetMessage(392),AltarNumber);
		return 0;
	}

	if((CurrentTime-lpAltarInfo->m_LastValidContractTime) < 10000)
	{
		gNotice.GCNoticeSend(UserIndex,1,0,0,0,0,0,gMessage.GetMessage(393),AltarNumber);
		return 0;
	}

	if(lpAltarInfo->m_ContractCount > MAX_CRYWOLF_ALTAR_CONTRACT) 
	{
		gNotice.GCNoticeSend(UserIndex,1,0,0,0,0,0,gMessage.GetMessage(394),AltarNumber);
		return 0;
	}

	if(abs(gObj[AltarObjIndex].X-gObj[UserIndex].X) > 0 || abs(gObj[AltarObjIndex].Y-gObj[UserIndex].Y) > 0)
	{
		gNotice.GCNoticeSend(UserIndex,1,0,0,0,0,0,gMessage.GetMessage(395),AltarNumber);
		return 0;
	}

	lpAltarInfo->SetAltarState(2);
	lpAltarInfo->m_UserIndex = UserIndex;
	lpAltarInfo->m_AppliedContractTime = CurrentTime;
	lpAltarInfo->m_ValidContractTime = CurrentTime+5000;
	lpAltarInfo->m_LastValidContractTime = CurrentTime;
	lpAltarInfo->m_ContractCount++;

	gEffectManager.AddEffect(&gObj[AltarObjIndex],0,EFFECT_CRYWOLF_STATE4,0,0,0,0,0);

	gCrywolfUtil.SendCrywolfUserAnyMsg(0,gMessage.GetMessage(396),gObj[lpAltarInfo->m_UserIndex].Name,AltarNumber);

	LogAdd(LOG_BLACK,"[ Crywolf ][Altar Op.] [%s][%s] Attempt to contract Altar[%d]",gObj[lpAltarInfo->m_UserIndex].Account,gObj[lpAltarInfo->m_UserIndex].Name,AltarNumber);

	gNotice.GCNoticeSend(UserIndex,1,0,0,0,0,0,gMessage.GetMessage(397));
	gNotice.GCNoticeSend(UserIndex,1,0,0,0,0,0,gMessage.GetMessage(398));
	return 1;
}

void CCrywolfAltar::SetAltarViewState(int Class,int AltarState) // OK
{
	int AltarNumber = this->GetAltarNumber(Class);

	CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[AltarNumber];

	if(OBJECT_RANGE(lpAltarInfo->m_AltarIndex) == 0)
	{
		return;
	}

	gEffectManager.AddEffect(&gObj[lpAltarInfo->m_AltarIndex],0,EFFECT_CRYWOLF_STATE1,0,0,0,0,0);
}

void CCrywolfAltar::SetAltarValidContract(int AltarObjIndex,int Class) // OK
{
	int AltarNumber = this->GetAltarNumber(Class);

	CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[AltarNumber];

	if(lpAltarInfo->GetAltarState() != 2)
	{
		return;
	}

	if(((DWORD)lpAltarInfo->m_ValidContractTime) < GetTickCount())
	{
		gNotice.GCNoticeSend(lpAltarInfo->m_UserIndex,1,0,0,0,0,0,gMessage.GetMessage(390),AltarNumber);

		gCrywolfUtil.SendCrywolfUserAnyMsg(0,gMessage.GetMessage(391),gObj[lpAltarInfo->m_UserIndex].Name,AltarNumber);

		LogAdd(LOG_BLACK,"[ Crywolf ][Altar Op.] [%s][%s] Set Valid Contract Altar[%d]",gObj[lpAltarInfo->m_UserIndex].Account,gObj[lpAltarInfo->m_UserIndex].Name,AltarNumber);

		lpAltarInfo->SetAltarState(1);

		gEffectManager.AddEffect(&gObj[AltarObjIndex],0,EFFECT_CRYWOLF_STATE3,0,0,0,0,0);
	}
}

int CCrywolfAltar::GetAltarState(int Class) // OK
{
	if(Class < 205 || Class > 209)
	{
		return 0;
	}

	return this->m_AltarInfo[(Class-205)].GetAltarState();
}

int CCrywolfAltar::GetAltarNumber(int Class) // OK
{
	if(Class < 205 || Class > 209)
	{
		return 0;
	}

	return (Class-205);
}

int CCrywolfAltar::GetAltarUserIndex(int Class) // OK
{
	if(Class < 205 || Class > 209)
	{
		return -1;
	}

	return this->m_AltarInfo[(Class-205)].m_UserIndex;
}

int CCrywolfAltar::GetAltarRemainContractCount(int Class) // OK
{
	if(Class < 205 || Class > 209)
	{
		return 0;
	}

	return this->m_AltarInfo[(Class-205)].GetRemainContractCount();
}

int CCrywolfAltar::GetPriestHPSum() // OK
{
	int PriestHPSum = 0;

	for(int n=0;n < MAX_CRYWOLF_ALTAR;n++)
	{
		CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[n];

		if(lpAltarInfo->GetAltarState() != 1)
		{
			continue;
		}

		if(gObj[lpAltarInfo->m_UserIndex].Connected != OBJECT_ONLINE)
		{
			continue;
		}

		PriestHPSum += (int)gObj[lpAltarInfo->m_UserIndex].Life;
	}

	return PriestHPSum;
}

int CCrywolfAltar::GetPriestMaxHPSum() // OK
{
	int PriestMaxHPSum = 0;

	for(int n=0;n < MAX_CRYWOLF_ALTAR;n++)
	{
		CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[n];

		if(lpAltarInfo->GetAltarState() != 1)
		{
			continue;
		}

		if(gObj[lpAltarInfo->m_UserIndex].Connected != OBJECT_ONLINE)
		{
			continue;
		}

		PriestMaxHPSum += (int)(gObj[lpAltarInfo->m_UserIndex].MaxLife+gObj[lpAltarInfo->m_UserIndex].AddLife);
	}

	return PriestMaxHPSum;
}

int CCrywolfAltar::GetContractedAltarCount() // OK
{
	int ContractedAltarCount = 0;

	for(int n=0;n < MAX_CRYWOLF_ALTAR;n++)
	{
		CCrywolfAltarInfo* lpAltarInfo = &this->m_AltarInfo[n];

		if(lpAltarInfo->GetAltarState() != 1)
		{
			continue;
		}

		ContractedAltarCount++;
	}

	return ContractedAltarCount;
}
