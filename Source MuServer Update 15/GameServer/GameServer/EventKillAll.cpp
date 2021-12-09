// CEventKillAll.cpp: interface for the CEventKillAll class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "EventKillAll.h"
#include "GameMaster.h"
#include "Map.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventKillAll gEventKillAll;

CEventKillAll::CEventKillAll() // OK
{
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TotalTime = 0;
	this->m_EventKillAllActive = 0;
	this->m_EventKillAllEnter = 0;
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
	this->m_Members = 0;

	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		this->User[n].Reset();
	}

}

CEventKillAll::~CEventKillAll() // OK
{

}

void CEventKillAll::Clear()
{

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Type == OBJECT_USER)
		{
			if (gObj[n].KillAll == 1)
			{
				gObj[n].KillAll = 0;
			}			
		}
	}

	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			gObjMoveGate(this->User[n].Index,17);
		}
	}

	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TotalTime = 0;
	this->m_EventKillAllActive = 0;
	this->m_EventKillAllEnter = 0;
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
	this->m_Members = 0;

	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		this->User[n].Reset();
	}
}

void CEventKillAll::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	if(this->m_RemainTime > 0)
	{ 
		this->CheckUser();

		if(this->m_RemainTime <= (this->m_TotalTime-gServerInfo.m_EventKillAllTimeToEnter) && this->m_EventKillAllEnter == 1)
		{
			if (this->m_Members < 2)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(808));
				LogAdd(LOG_EVENT,"[EVENT KILL ALL] Canceled");

				this->Clear();
			}
			else
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(805));
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(806));
				this->m_EventKillAllActive = 1;
				this->m_EventKillAllEnter = 0;
			}
		}

		this->m_RemainTime--;

		if(this->m_RemainTime <= 0 && (this->m_EventKillAllEnter != 0 || this->m_EventKillAllActive != 0))
		{
			    this->m_EventKillAllActive = 0;
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(807));
				LogAdd(LOG_EVENT,"[EVENT KILL ALL] Time is over");

				this->Clear();

		}

		if(this->m_RemainTime <= 2 && this->m_EventKillAllActive != 0)
		{
			this->CalcUserRank();

			for(int n=0;n < MAX_KILLALL_USER;n++)
			{
				if(OBJECT_RANGE(this->User[n].Index) != 0)
				{
					if(this->User[n].Rank == 1)
					{
						//GDRankingCustomArenaSaveSend(this->User[n].Index,this->Index,this->User[n].Score,this->User[n].Rank);
					
						if (gServerInfo.m_EventKillAllAutoReward1[0] > 0 || gServerInfo.m_EventKillAllAutoReward2[0] > 0 || gServerInfo.m_EventKillAllAutoReward3[0] > 0)
						{	
							GDSetCoinSend(this->User[n].Index, gServerInfo.m_EventKillAllAutoReward1[0], gServerInfo.m_EventKillAllAutoReward2[0], gServerInfo.m_EventKillAllAutoReward3[0],"KillAll");
						}

						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(812),gObj[this->User[n].Index].Name,this->User[n].Score);

						LogAdd(LOG_EVENT,"[EVENT PVP] (Winner1: %s)",gObj[this->User[n].Index].Name);
					}
					if(this->User[n].Rank == 2)
					{
						//GDRankingCustomArenaSaveSend(this->User[n].Index,this->Index,this->User[n].Score,this->User[n].Rank);
					
						if (gServerInfo.m_EventKillAllAutoReward1[1] > 0 || gServerInfo.m_EventKillAllAutoReward2[1] > 0 || gServerInfo.m_EventKillAllAutoReward3[1] > 0)
						{	
							GDSetCoinSend(this->User[n].Index, gServerInfo.m_EventKillAllAutoReward1[1], gServerInfo.m_EventKillAllAutoReward2[1], gServerInfo.m_EventKillAllAutoReward3[1],"KillAll");
						}

						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(813),gObj[this->User[n].Index].Name,this->User[n].Score);

						LogAdd(LOG_EVENT,"[EVENT PVP] (Winner2: %s)",gObj[this->User[n].Index].Name);
					}
					if(this->User[n].Rank == 3)
					{
						//GDRankingCustomArenaSaveSend(this->User[n].Index,this->Index,this->User[n].Score,this->User[n].Rank);
					
						if (gServerInfo.m_EventKillAllAutoReward1[2] > 0 || gServerInfo.m_EventKillAllAutoReward2[2] > 0 || gServerInfo.m_EventKillAllAutoReward3[2] > 0)
						{	
							GDSetCoinSend(this->User[n].Index, gServerInfo.m_EventKillAllAutoReward1[2], gServerInfo.m_EventKillAllAutoReward2[2], gServerInfo.m_EventKillAllAutoReward3[2],"KillAll");
						}

						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(814),gObj[this->User[n].Index].Name,this->User[n].Score);

						LogAdd(LOG_EVENT,"[EVENT PVP] (Winner3: %s)",gObj[this->User[n].Index].Name);
					}

					gNotice.GCNoticeSend(this->User[n].Index,1,0,0,0,0,0,gMessage.GetMessage(815),this->User[n].Rank,this->User[n].Score);
					
				}

			}

			LogAdd(LOG_EVENT,"[EVENT PVP] Finish");
			this->Clear();

		}
	}

}

bool CEventKillAll::CommandEventKillAll(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventKillAllSwitch == 0)
	{
		return 0;
	}

	if(gGameMaster.CheckGameMasterLevel(lpObj,1) == 0)
	{
		return 0;
	}

	if (this->m_RemainTime > 0) 
	{
		this->Clear();
		LogAdd(LOG_EVENT,"[EVENT KILL ALL] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(808));

		return 0;
	}

	this->m_EventKillAllEnter	= 1;
	this->m_RemainTime				= gServerInfo.m_EventKillAllMaxTime*60;
	this->m_TotalTime				= gServerInfo.m_EventKillAllMaxTime*60;
	this->m_Map						= lpObj->Map;
	this->m_MapX					= lpObj->X;
	this->m_MapY					= lpObj->Y-1;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(809),gServerInfo.m_EventKillAllMaxPlayers);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(810));

	LogAdd(LOG_EVENT,"[EVENT KILL ALL] Start by %s",lpObj->Name);

	return 1;
}

void CEventKillAll::CommandEventKillAllJoin(LPOBJ lpObj,char* arg) // OK 
{
	if(gServerInfo.m_EventKillAllSwitch == 0)
	{
		return;
	}

	if (this->m_EventKillAllEnter == 0) 
	{
		return;
	}

	if (lpObj->Level <= 10) 
	{
		return;
	}

	if (lpObj->KillAll != 0) 
	{
		return;
	}

	if (this->m_Members > gServerInfo.m_EventKillAllMaxPlayers)
	{
		return;
	}

	if (this->AddUser(lpObj->Index) == 0)
	{
		return;
	}

	this->m_Members++;

	lpObj->KillAll = 1;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(811),this->m_Members, lpObj->Name);

	gObjTeleport(lpObj->Index,this->m_Map,this->m_MapX,this->m_MapY);

	LogAdd(LOG_EVENT,"[EVENT KILL ALL] Member add (%s)",lpObj->Name);
}

bool CEventKillAll::AddUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		if(this->User[n].Index == aIndex)
		{
			return 0;
		}
	}

	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			continue;
		}

		this->User[n].Index = aIndex;
		return 1;
	}

	return 0;
}

void CEventKillAll::CalcUserRank() // OK
{
	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) == 0)
		{
			continue;
		}

		int rank = MAX_KILLALL_USER;

		for(int i=0;i < MAX_KILLALL_USER;i++)
		{
			if(OBJECT_RANGE(this->User[i].Index) == 0)
			{
				rank--;
				continue;
			}

			if(this->User[n].Index == this->User[i].Index)
			{
				rank--;
				continue;
			}

			if(this->User[n].Score > this->User[i].Score)
			{
				rank--;
				continue;
			}

			if(this->User[n].Score == this->User[i].Score && n < i)
			{
				rank--;
				continue;
			}
		}

		this->User[n].Rank = (rank+1);
	}
}

bool CEventKillAll::CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(this->m_EventKillAllActive != 0)
	{
		return 1;
	}

	return 0;
}

void CEventKillAll::CheckUser() // OK
{
	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) == 0)
		{
			continue;
		}

		if(gObjIsConnected(this->User[n].Index) == 0)
		{
			this->User[n].Reset();
			gObj[this->User[n].Index].KillAll = 0;
			continue;
		}

		if(gObj[this->User[n].Index].Map != this->m_Map)
		{
			this->User[n].Reset();
			gObj[this->User[n].Index].KillAll = 0;
			continue;
		}

		if(gObj[this->User[n].Index].KillAll == 0)
		{
			this->User[n].Reset();
			continue;
		}
	}
}


void CEventKillAll::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(gServerInfo.m_EventKillAllSwitch == 0)
	{
		return;
	}

	if(this->m_EventKillAllActive != 1)
	{
		return;
	}

	if (lpObj->KillAll != 1 && lpTarget->KillAll != 1)
	{
		return;
	}

	for(int n=0;n < MAX_KILLALL_USER;n++)
	{
		if(this->User[n].Index == lpObj->Index)
		{
			this->User[n].Score -= ((this->User[n].Score>gServerInfo.m_EventKillAllDeadDecrease)?gServerInfo.m_EventKillAllDeadDecrease:this->User[n].Score);
			gNotice.GCNoticeSend(this->User[n].Index,1,0,0,0,0,0,gMessage.GetMessage(816),this->User[n].Score);
		}

		if(this->User[n].Index == lpTarget->Index)
		{
			this->User[n].Score += gServerInfo.m_EventKillAllKillIncrease;
			gNotice.GCNoticeSend(this->User[n].Index,1,0,0,0,0,0,gMessage.GetMessage(816),this->User[n].Score);
		}
	}
}

bool CEventKillAll::Respawn(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level) // OK
{
	if(gServerInfo.m_EventKillAllSwitch == 0)
	{
		return 0;
	}

	if(this->m_EventKillAllActive != 1)
	{
		return 0;
	}

	if (lpObj->KillAll != 1)
	{
		return 0;
	}

	int px,py;

	for(int n=0;n < 50;n++)
	{

		px = this->m_MapX+(GetLargeRand()%20-GetLargeRand()%20);

		py = this->m_MapY+(GetLargeRand()%20-GetLargeRand()%20);

		if(gMap[this->m_Map].CheckAttr(px,py,1) == 0 && gMap[this->m_Map].CheckAttr(px,py,2) == 0 && gMap[this->m_Map].CheckAttr(px,py,4) == 0 && gMap[this->m_Map].CheckAttr(px,py,8) == 0)
		{
			(*gate) = 0;
			(*map) = this->m_Map;
			(*x) = px;
			(*y) = py;
			(*dir) = 3;
			(*level) = 0;
			return 1;
		}
	}

	return 0;
}

