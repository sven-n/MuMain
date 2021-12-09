// CEventRussianRoulette.cpp: interface for the CEventRussianRoulette class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "EventRussianRoulette.h"
#include "GameMaster.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventRussianRoulette gEventRussianRoulette;

CEventRussianRoulette::CEventRussianRoulette() // OK
{
	this->m_RemainTime = 0;
	this->m_GmIndex = 0;
	this->m_TickCount = GetTickCount();
	this->m_TotalTime = 0;
	this->m_EventRussianRouletteActive = 0;
	this->m_EventRussianRouletteEnter = 0;
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
	this->m_Members = 0;
}

CEventRussianRoulette::~CEventRussianRoulette() // OK
{

}

void CEventRussianRoulette::Clear()
{
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_GmIndex = 0;
	this->m_TotalTime = 0;
	this->m_EventRussianRouletteActive = 0;
	this->m_EventRussianRouletteEnter = 0;
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
	this->m_Members = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Type == OBJECT_USER)
		{
			if (gObj[n].RussianRoulette == 1)
			{
				gObj[n].RussianRoulette = 0;
			}			
		}
	}
}

void CEventRussianRoulette::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	if(this->m_RemainTime > 0)
	{ 
		if(this->m_RemainTime <= (this->m_TotalTime-gServerInfo.m_EventRussianRouletteTimeToEnter) && this->m_EventRussianRouletteEnter == 1)
		{
			if (this->m_Members < 2)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(707));
				LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] Canceled");

				this->Clear();
			}
			else
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(704));
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(705));
				this->m_EventRussianRouletteActive = 1;
				this->m_EventRussianRouletteEnter = 0;
			}
		}

		this->m_RemainTime--;

		if(this->m_RemainTime <= 0 && (this->m_EventRussianRouletteEnter != 0 || this->m_EventRussianRouletteActive != 0))
		{
			    this->m_EventRussianRouletteActive = 0;
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(706));
				LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] Time is over");

				this->Clear();

		}

	}

	if(this->m_RouletteRemainTime > 0 && this->m_EventRussianRouletteActive == 1)
	{ 
		if(this->m_RouletteRemainTime == 8)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(712));
		}

		if(this->m_RouletteRemainTime == 6)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(713));
		}

		if(this->m_RouletteRemainTime == 4)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(714));
		}
		if(this->m_RouletteRemainTime == 2)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(715));
		}

		this->m_RouletteRemainTime--;

		if(this->m_RouletteRemainTime <= 0)
		{
				LPOBJ lpObj = &gObj[this->m_PlayerIndex];

				if((GetLargeRand()%100) <= 25)
				{
					lpObj->RussianRoulette = 0;
					//gObjMoveGate(lpObj->Index,1);

					lpObj->Live = 0;
					lpObj->State = OBJECT_DYING;
					lpObj->RegenTime = GetTickCount();
					lpObj->DieRegen = 1;
					lpObj->PathCount = 0;
					lpObj->Teleport = ((lpObj->Teleport==1)?0:lpObj->Teleport);

					GCUserDieSend(lpObj,lpObj->Index,0,0);

					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(716),lpObj->Name);
					LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] %s Removed",lpObj->Name);
				}
				else 
				{
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(717),lpObj->Name);
					LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] %s Saved",lpObj->Name);
				}

						this->m_Members = 0;

						for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
						{
							if(gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER)
							{
								if (gObj[n].RussianRoulette == 1)
								{
									this->m_Members++;
									this->m_PlayerIndex = n;
								}			
							}
						}

						if (this->m_Members <= 0)
						{
							this->Clear();
							LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] Canceled");
							gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(707));

							return;
						}

						if (this->m_Members == 1)
						{
							this->Clear();

							LPOBJ lpObj = &gObj[this->m_PlayerIndex];

							gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(711),lpObj->Name);
							GCFireworksSend(lpObj,lpObj->X,lpObj->Y);

							if (gServerInfo.m_EventRussianRouletteAutoReward1 > 0 || gServerInfo.m_EventRussianRouletteAutoReward2 > 0 || gServerInfo.m_EventRussianRouletteAutoReward3 > 0)
							{	
								GDSetCoinSend(lpObj->Index, gServerInfo.m_EventRussianRouletteAutoReward1, gServerInfo.m_EventRussianRouletteAutoReward2, gServerInfo.m_EventRussianRouletteAutoReward3,"RussianR.");
							}

							LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] (Winner: %s)",lpObj->Name);
							
						}

		}

	}

}

void CEventRussianRoulette::CommandEventRussianRoulette(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventRussianRouletteSwitch == 0)
	{
		return;
	}

	if(gGameMaster.CheckGameMasterLevel(lpObj,1) == 0)
	{
		return;
	}

	if (this->m_RemainTime > 0) 
	{
		this->Clear();
		LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(707));

		return;
	}

	this->m_EventRussianRouletteEnter	= 1;
	this->m_GmIndex					= lpObj->Index;
	this->m_RemainTime				= gServerInfo.m_EventRussianRouletteMaxTime*60;
	this->m_TotalTime				= gServerInfo.m_EventRussianRouletteMaxTime*60;
	this->m_Map						= lpObj->Map;
	this->m_MapX					= lpObj->X;
	this->m_MapY					= lpObj->Y-1;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(708),gServerInfo.m_EventRussianRouletteMaxPlayer);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(709));

	LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] Start by %s",lpObj->Name);
}

void CEventRussianRoulette::CommandEventRussianRouletteJoin(LPOBJ lpObj,char* arg) // OK 
{
	if(gServerInfo.m_EventRussianRouletteSwitch == 0)
	{
		return;
	}

	if (this->m_EventRussianRouletteEnter == 0) 
	{
		return;
	}

	if (lpObj->Level <= 10) 
	{
		return;
	}

	if (lpObj->RussianRoulette != 0) 
	{
		return;
	}

	if (this->m_Members > gServerInfo.m_EventRussianRouletteMaxPlayer)
	{
		return;
	}

	this->m_Members++;

	lpObj->RussianRoulette = 1;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(710),this->m_Members, lpObj->Name);

	gObjTeleport(lpObj->Index,this->m_Map,this->m_MapX,this->m_MapY);

	LogAdd(LOG_EVENT,"[EVENT RUSSIAN ROULETTE] Member add (%s)",lpObj->Name);
}

int CEventRussianRoulette::EventRussianRouletteTrade(int aIndex,int bIndex) // OK 
{
	if(gServerInfo.m_EventRussianRouletteSwitch == 0)
	{
		return 0;
	}

	if (this->m_EventRussianRouletteActive == 0) 
	{
		return 0;
	}

	if (this->m_GmIndex != aIndex) 
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[bIndex];

	if (lpObj->RussianRoulette != 1) 
	{
		return 0;
	}

	this->m_PlayerIndex = bIndex;
	this->m_RouletteRemainTime = 8;

	return 1;
}

