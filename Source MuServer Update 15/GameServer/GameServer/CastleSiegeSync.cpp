// CastleSiegeSync.cpp: implementation of the CCastleSiegeSync class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleSiegeSync.h"
#include "DSProtocol.h"
#include "Guild.h"
#include "MapServerManager.h"
#include "User.h"
#include "Union.h"
#include "Util.h"

CCastleSiegeSync gCastleSiegeSync;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleSiegeSync::CCastleSiegeSync() // OK
{
	this->Clear();
}

CCastleSiegeSync::~CCastleSiegeSync() // OK
{

}

void CCastleSiegeSync::Clear() // OK
{
	this->m_CurCastleState = -1;
	this->m_CurTaxRateChaos = 0;
	this->m_CurTaxRateStore = 0;
	this->m_CastleTributeMoney = 0;
	this->m_CsTributeMoneyTimer = 0;
	memset(this->m_CastleOwnerGuild,0,sizeof(this->m_CastleOwnerGuild));
}

int CCastleSiegeSync::GetCastleState() // OK
{
	return this->m_CurCastleState;
}

int CCastleSiegeSync::GetTaxRateChaos(int aIndex) // OK
{
	int CurTaxRateChaos = this->m_CurTaxRateChaos;

	if(this->CheckCastleOwnerMember(aIndex) != 0 || this->CheckCastleOwnerUnionMember(aIndex) != 0)
	{
		CurTaxRateChaos = 0;
	}

	(CurTaxRateChaos > 3) ? 3 : CurTaxRateChaos;
	(CurTaxRateChaos < 0) ? 0 : CurTaxRateChaos;

	return CurTaxRateChaos;
}

int CCastleSiegeSync::GetTaxRateStore(int aIndex) // OK
{
	int CurTaxRateStore = this->m_CurTaxRateStore;

	if(this->CheckCastleOwnerMember(aIndex) != 0 || this->CheckCastleOwnerUnionMember(aIndex) != 0)
	{
		CurTaxRateStore = 0;
	}

	(CurTaxRateStore > 3) ? 3 : CurTaxRateStore;
	(CurTaxRateStore < 0) ? 0 : CurTaxRateStore;

	return CurTaxRateStore;
}

int CCastleSiegeSync::GetTaxHuntZone(int aIndex,bool CheckOwnerGuild) // OK
{
	int CurTaxHuntZone = this->m_CurTaxHuntZone;

	if(CheckOwnerGuild != 0)
	{
		if(this->CheckCastleOwnerMember(aIndex) != 0 || this->CheckCastleOwnerUnionMember(aIndex) != 0)
		{
			CurTaxHuntZone = 0;
		}
	}

	(CurTaxHuntZone > 3) ? 3 : CurTaxHuntZone;
	(CurTaxHuntZone < 0) ? 0 : CurTaxHuntZone;

	return CurTaxHuntZone;
}

char* CCastleSiegeSync::GetCastleOwnerGuild() // OK
{
	return this->m_CastleOwnerGuild;
}

bool CCastleSiegeSync::CheckCastleOwnerMember(int aIndex) // OK
{
	int iIndex=aIndex;
	if ( gObjIsConnected(iIndex) == FALSE )
	{
		return FALSE;
	}

	if ( strcmp(this->m_CastleOwnerGuild, "") == 0 )
	{
		return FALSE;
	}

	if ( strcmp(gObj[iIndex].GuildName, this->m_CastleOwnerGuild) != 0 )
	{
		return FALSE;
	}

	return TRUE;
}

bool CCastleSiegeSync::CheckCastleOwnerUnionMember(int aIndex) // OK
{
	int iIndex=aIndex;
	if ( gObjIsConnected(iIndex) == FALSE )
	{
		return FALSE;
	}

	if ( strcmp(this->m_CastleOwnerGuild, "") == 0 )
	{
		return FALSE;
	}

	GUILD_INFO_STRUCT * lpGuildInfo = gObj[iIndex].Guild;
	
	if ( lpGuildInfo == NULL )
	{
		return FALSE;
	}
	
	CUnionInfo * pUnionInfo = gUnionManager.SearchUnion(lpGuildInfo->GuildUnion);

	if ( pUnionInfo == NULL )
	{
		return FALSE;
	}

	if ( strcmp( pUnionInfo->m_szMasterGuild, this->m_CastleOwnerGuild) == 0 )
	{
		return TRUE;
	}

	return FALSE;
}

void CCastleSiegeSync::ResetTributeMoney() // OK
{
	InterlockedExchange((long*)&this->m_CastleTributeMoney,0);
}

void CCastleSiegeSync::AddTributeMoney(int money) // OK
{
	if(this->m_CastleTributeMoney < 0)
	{
		InterlockedExchange((long*)&this->m_CastleTributeMoney,0);
	}

	if(money <= 0)
	{
		return;
	}

	if((this->m_CastleTributeMoney+money) > MAX_TRIBUTE_MONEY)
	{
		return;
	}

	InterlockedExchangeAdd((long*)&this->m_CastleTributeMoney,money);
}

void CCastleSiegeSync::AdjustTributeMoney()
{
	if ( this->m_CastleTributeMoney < 0 )
	{
		InterlockedExchange((LPLONG)&this->m_CastleTributeMoney, 0);
	}

	if ( this->m_CastleTributeMoney == 0 )
	{
		return;
	}

	this->m_CsTributeMoneyTimer++;

	this->m_CsTributeMoneyTimer %= 180;

	if ( this->m_CsTributeMoneyTimer != 0 )
	{
		return;
	}

	GS_GDReqCastleTributeMoney(gMapServerManager.GetMapServerGroup(), this->m_CastleTributeMoney);
}

void CCastleSiegeSync::SetCastleOwnerGuild(char* GuildName) // OK
{
	memset(this->m_CastleOwnerGuild,0,sizeof(this->m_CastleOwnerGuild));
	memcpy(this->m_CastleOwnerGuild,GuildName,(sizeof(this->m_CastleOwnerGuild)/2));
}

void CCastleSiegeSync::SetCastleState(int state) // OK
{
	this->m_CurCastleState = state;
}

void CCastleSiegeSync::SetTaxRateChaos(int rate) // OK
{
	this->m_CurTaxRateChaos = rate;
}

void CCastleSiegeSync::SetTaxRateStore(int rate) // OK
{
	this->m_CurTaxRateStore = rate;
}

void CCastleSiegeSync::SetTaxHuntZone(int rate) // OK
{
	this->m_CurTaxHuntZone = rate;
}

int CCastleSiegeSync::GetTributeMoney() // OK
{
	return this->m_CastleTributeMoney;
}
