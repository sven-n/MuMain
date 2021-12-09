// CastleSiegeCrown.cpp: implementation of the CCastleSiegeCrown class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleSiegeCrown.h"
#include "CastleSiege.h"
#include "Map.h"
#include "Util.h"

CCastleSiegeCrown gCastleSiegeCrown;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleSiegeCrown::CCastleSiegeCrown() // OK
{

}

CCastleSiegeCrown::~CCastleSiegeCrown() // OK
{

}

void CCastleSiegeCrown::CastleSiegeCrownAct(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Type != OBJECT_NPC || lpObj->Class != 216)
	{
		return;
	}

	int bIndex = gCastleSiege.GetCrownUserIndex();

	if(gObjIsConnected(bIndex) == 0)
	{
		gCastleSiege.SetCrownAccessUserX(0);
		gCastleSiege.SetCrownAccessUserY(0);
		gCastleSiege.ResetCrownUserIndex();
		gCastleSiege.ResetCrownAccessTickCount();
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpTarget->Map != MAP_CASTLE_SIEGE || lpTarget->CsJoinSide < 2 || lpTarget->CsGuildInvolved == 0)
	{
		GCAnsCsAccessCrownState(bIndex,2);
		gCastleSiege.SetCrownAccessUserX(0);
		gCastleSiege.SetCrownAccessUserY(0);
		gCastleSiege.ResetCrownUserIndex();
		gCastleSiege.ResetCrownAccessTickCount();
		return;
	}

	if(lpTarget->X != gCastleSiege.GetCrownAccessUserX() || lpTarget->Y != gCastleSiege.GetCrownAccessUserY())
	{
		GCAnsCsAccessCrownState(bIndex,2);
		gCastleSiege.SetCrownAccessUserX(0);
		gCastleSiege.SetCrownAccessUserY(0);
		gCastleSiege.ResetCrownUserIndex();
		gCastleSiege.ResetCrownAccessTickCount();
		return;
	}

	if(gObjIsConnected(gCastleSiege.GetCrownSwitchUserIndex(217)) == 0 || gObjIsConnected(gCastleSiege.GetCrownSwitchUserIndex(218)) == 0)
	{
		GCAnsCsAccessCrownState(bIndex,2);
		gCastleSiege.SetCrownAccessUserX(0);
		gCastleSiege.SetCrownAccessUserY(0);
		gCastleSiege.ResetCrownUserIndex();
		gCastleSiege.ResetCrownAccessTickCount();
		return;
	}

	if(lpTarget->CsJoinSide != gObj[gCastleSiege.GetCrownSwitchUserIndex(217)].CsJoinSide || lpTarget->CsJoinSide != gObj[gCastleSiege.GetCrownSwitchUserIndex(218)].CsJoinSide)
	{
		GCAnsCsAccessCrownState(bIndex,2);
		gCastleSiege.SetCrownAccessUserX(0);
		gCastleSiege.SetCrownAccessUserY(0);
		gCastleSiege.ResetCrownUserIndex();
		gCastleSiege.ResetCrownAccessTickCount();
		LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Register Castle Crown Canceled (GUILD:%s)",lpTarget->Account,lpTarget->Name,lpTarget->GuildName);
		return;
	}

	LogAdd(LOG_BLACK,"[CastleSiege] [Reg. Accumulating] Accumulated Crown AccessTime : acc(%d) + %d [%s](%s)(%s)",lpTarget->AccumulatedCrownAccessTime,(GetTickCount()-gCastleSiege.GetCrownAccessTickCount()),lpTarget->GuildName,lpTarget->Account,lpTarget->Name);

	#endif
}
