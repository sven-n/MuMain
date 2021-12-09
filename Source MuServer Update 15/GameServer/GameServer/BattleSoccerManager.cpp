#include "stdafx.h"
#include "BattleSoccer.h"
#include "BattleSoccerManager.h"
#include "Guild.h"
#include "Protocol.h"
#include "Util.h"

CBattleSoccerManager::CBattleSoccerManager()
{
	return;
}

CBattleSoccerManager::~CBattleSoccerManager()
{
	return;
}

void InitBattleSoccer()
{
	for ( int n=0;n<MAX_BATTLESOCCER_GROUND;n++)
	{
		gBSGround[n] = new CBattleSoccer;
		gBSGround[n]->m_BlueTeam = NULL;
		gBSGround[n]->m_RedTeam = NULL;
	}

	gBSGround[0]->SetBattleType(1);
	gBSGround[0]->SetGroundRect(6, 54, 141, 70, 180);
	gBSGround[0]->SetGoalLeftRect(60, 139, 64, 140);
	gBSGround[0]->SetGoalRightRect(60, 181, 64, 182);
}

void ClearBattleSoccer()
{
	for ( int n=0;n<MAX_BATTLESOCCER_GROUND;n++)
	{
		delete gBSGround[n];
	}
}

BOOL gCheckBlankBattleGround()
{
	for ( int n=0;n<MAX_BATTLESOCCER_GROUND;n++)
	{
		if ( gBSGround[n]->GetUsed() == FALSE )
		{
			return n;
		}
	}
	return 0xFF;
}

int gCheckBattleGroundTimer()
{
	int time=0;
	int  i;
	
	return 0;

	int n;

	if( gBSGround[n]->m_BlueTeam ) //HermeX Add-on for fixing registers
	{
		/*//for( i=0; i<MAX_USER_GUILD; i++)
		{
			int index = gBSGround[n]->m_RedTeam->Index[i];					
			if( index != -1 )
			{
				//GCServerMsgStringSend(szTemp, index, 1);
				GCTimeViewSend(index, time);
			}
		}*/
	}

	for( i=0; i<MAX_GUILD_USER; i++)
	{
		int index = gBSGround[n]->m_BlueTeam->Index[i];					
		if( index != -1 )
		{
			//GCServerMsgStringSend(szTemp, index, 1);
			GCTimeViewSend(index, time);
		}
	}

	if( gBSGround[n]->m_RedTeam )
	{
		for( i=0; i<MAX_GUILD_USER; i++)
		{
			int index = gBSGround[n]->m_RedTeam->Index[i];					
			if( index != -1 )
			{
				//GCServerMsgStringSend(szTemp, index, 1);
				GCTimeViewSend(index, time);
			}
		}
	}

	return time;
}

void gBattleGroundEnable(int n, BOOL enable)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(n)  == FALSE )
	{
		return;
	}

	if ( enable == TRUE )
	{
		gBSGround[n]->Enable(enable);
		gBSGround[n]->SetTimer(MAX_BATTLESOCCER_TIME);
	}
	else
	{
		gBSGround[n]->Enable(FALSE);
		gBSGround[n]->SetTeamMaster(0, "none");
		gBSGround[n]->SetTeamMaster(1, "none");
		gBSGround[n]->m_RedTeam = 0;
		gBSGround[n]->m_BlueTeam = 0;
	}
}

void gSetBattleTeamMaster(int ground, int team, char * szname, GUILD_INFO_STRUCT * lpGuild)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
	{
		return;
	}

	gBSGround[ground]->SetTeamMaster(team, szname);
	gBSGround[ground]->SetTeamName(team, lpGuild->Name );

	if ( team == 0 )
	{
		gBSGround[ground]->m_RedTeam = lpGuild;
	}
	else
	{
		gBSGround[ground]->m_BlueTeam = lpGuild;
	}
}

void gBattleGetTeamPosition(int ground, int team, short & X, short & Y)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
	{
		return;
	}


	int w = gBSGround[ground]->m_rect.right - gBSGround[ground]->m_rect.left;
	int h = gBSGround[ground]->m_rect.bottom - gBSGround[ground]->m_rect.top;

	if ( w < 1 )
	{
		w = 1;
	}

	if ( h < 1 )
	{
		h = 1;
	}

	X = (short)gBSGround[ground]->m_rect.left + (short)(GetLargeRand()%w);
	Y = (short)gBSGround[ground]->m_rect.top + (short)(GetLargeRand()%h);
}

char * GetBattleTeamName(int ground, int team)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
	{
		return NULL;
	}

	return gBSGround[ground]->GetTeamName(team);
}

void SetBattleTeamScore(int ground, int team, int score)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
		return;

	gBSGround[ground]->SetScore(team, score);
}

int GetBattleTeamScore(int ground, int team)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
		return 0;

	return gBSGround[ground]->GetScore(team);
}

void BattleSoccerGoalStart(int ground)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
		return;

	gBSGround[ground]->SetGoalMove(1);
}

void BattleSoccerGoalEnd(int ground)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
		return;

	gBSGround[ground]->SetGoalMove(0);
}

int GetBattleSoccerGoalMove(int ground)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
		return 0;

	return gBSGround[ground]->GetGoalMove();
}

BOOL gBattleSoccerScoreUpdate(int ground, int team)
{
	if ( BATTLE_SOCCER_GROUND_RANGE(ground)  == FALSE )
		return FALSE;

	int score = 20;
	int totalscore = 1;
	GUILD_INFO_STRUCT * lpGuild;
	GUILD_INFO_STRUCT * lpTargetGuild;
	bool bGoalTeam;

	if ( team == 0 )
	{
		lpGuild = gBSGround[ground]->m_BlueTeam;
		lpTargetGuild = gBSGround[ground]->m_RedTeam;
		bGoalTeam = true;
	}
	else
	{
		lpGuild = gBSGround[ground]->m_RedTeam ;
		lpTargetGuild = gBSGround[ground]->m_BlueTeam;
		bGoalTeam = FALSE;
	}

	if ( lpGuild == NULL )
	{
		return FALSE;
	}

	if (gObjGuildWarProc(lpGuild, lpTargetGuild, score) == TRUE )
	{
		if ( lpGuild->WarType == 1 )
		{
			gObjGuildWarEndSend2(lpGuild, lpTargetGuild, 1, 0);
			gBattleGroundEnable(lpGuild->BattleGroundIndex, FALSE);
			gObjGuildWarEnd(lpGuild, lpTargetGuild);
		}
	}

	return TRUE;
}

int gCheckGoal(int x, int y, int & ground)	// R:[-1:FAIL ; 0:Team_0 ; 1:Team_1]
{
	for ( int n=0;n<MAX_BATTLESOCCER_GROUND;n++)
	{
		if ( gBSGround[n]->m_Set == TRUE )
		{
			if ( x >= gBSGround[n]->m_GoalLeft.left && x <= gBSGround[n]->m_GoalLeft.right && y >= gBSGround[n]->m_GoalLeft.top && y <= gBSGround[n]->m_GoalLeft.bottom ) 
			{
				ground = n;
				return 0;
			}

			if ( x >= gBSGround[n]->m_GoalRight.left && x <= gBSGround[n]->m_GoalRight.right && y >= gBSGround[n]->m_GoalRight.top && y <= gBSGround[n]->m_GoalRight.bottom ) 
			{
				ground = n;
				return 1;
			}
		}
	}

	return -1;
}

int gCheckBattleGround(LPOBJ lpObj)	// R:[-1:FAIL else OBJGround]
{
	for ( int n=0;n<MAX_BATTLESOCCER_GROUND;n++)
	{
		if ( gBSGround[n]->m_Set == TRUE )
		{
			if ( lpObj->X >= gBSGround[n]->m_rect.left && lpObj->X <= gBSGround[n]->m_rect.right && lpObj->Y >= gBSGround[n]->m_rect.top && lpObj->Y <= gBSGround[n]->m_rect.bottom ) 
			{
				return n;
			}

			if ( lpObj->X >= gBSGround[n]->m_GoalLeft.left && lpObj->X <= gBSGround[n]->m_GoalLeft.right && lpObj->Y >= gBSGround[n]->m_GoalLeft.top && lpObj->Y <= gBSGround[n]->m_GoalLeft.bottom ) 
			{
				return n;
			}

			if ( lpObj->X >= gBSGround[n]->m_GoalRight.left && lpObj->X <= gBSGround[n]->m_GoalRight.right && lpObj->Y >= gBSGround[n]->m_GoalRight.top && lpObj->Y <= gBSGround[n]->m_GoalRight.bottom ) 
			{
				return n;
			}
		}
	}
	return -1;
}
