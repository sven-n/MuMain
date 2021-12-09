// CastleSiegeCrownSwitch.cpp: implementation of the CCastleSiegeCrownSwitch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleSiegeCrownSwitch.h"
#include "CastleSiege.h"
#include "Map.h"
#include "Util.h"

CCastleSiegeCrownSwitch gCastleSiegeCrownSwitch;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleSiegeCrownSwitch::CCastleSiegeCrownSwitch() // OK
{

}

CCastleSiegeCrownSwitch::~CCastleSiegeCrownSwitch() // OK
{

}

void CCastleSiegeCrownSwitch::CastleSiegeCrownSwitchAct(int aIndex) // OK
{
	#if(GAMESERVER_TYPE==1)

	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Type != OBJECT_NPC || (lpObj->Class != 217 && lpObj->Class != 218))
	{
		return;
	}

	int bIndex = gCastleSiege.GetCrownSwitchUserIndex(lpObj->Class);

	if(gObjIsConnected(bIndex) == 0)
	{
		if(gCastleSiege.GetRegCrownAvailable() != 0)
		{
			gCastleSiege.SetRegCrownAvailable(0);
			gCastleSiege.NotifyCrownState(1);
		}

		gCastleSiege.ResetCrownSwitchUserIndex(lpObj->Class);
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	gCastleSiege.NotifyCrownSwitchInfo(aIndex);

	if(lpTarget->Map != MAP_CASTLE_SIEGE || lpTarget->CsJoinSide < 2)
	{
		GCAnsCsAccessSwitchState(bIndex,aIndex,-1,0);

		if(gCastleSiege.GetRegCrownAvailable() != 0)
		{
			gCastleSiege.SetRegCrownAvailable(0);
			gCastleSiege.NotifyCrownState(1);
		}

		gCastleSiege.ResetCrownSwitchUserIndex(lpObj->Class);
		return;
	}

	if(lpObj->X < (lpTarget->X-3) || lpObj->X > (lpTarget->X+3) || lpObj->Y < (lpTarget->Y-3) || lpObj->Y > (lpTarget->Y+3))
	{
		GCAnsCsAccessSwitchState(bIndex,aIndex,-1,0);

		if(gCastleSiege.GetRegCrownAvailable() != 0)
		{
			gCastleSiege.SetRegCrownAvailable(0);
			gCastleSiege.NotifyCrownState(1);
		}

		gCastleSiege.ResetCrownSwitchUserIndex(lpObj->Class);
		LogAdd(LOG_BLACK,"[CastleSiege] [%s][%s] Push Castle Crown Switch Canceled (GUILD:%s) - CS X:%d/Y:%d",lpTarget->Account,lpTarget->Name,lpTarget->GuildName,lpObj->X,lpObj->Y);
		return;
	}

	if(gObjIsConnected(gCastleSiege.GetCrownSwitchUserIndex(217)) != 0 && gObjIsConnected(gCastleSiege.GetCrownSwitchUserIndex(218)) != 0)
	{
		if(gObj[gCastleSiege.GetCrownSwitchUserIndex(217)].CsJoinSide == gObj[gCastleSiege.GetCrownSwitchUserIndex(218)].CsJoinSide)
		{
			if(gCastleSiege.GetRegCrownAvailable() == 0)
			{
				gCastleSiege.SetRegCrownAvailable(1);
				gCastleSiege.NotifyCrownState(0);
			}
		}
	}

	#endif
}
