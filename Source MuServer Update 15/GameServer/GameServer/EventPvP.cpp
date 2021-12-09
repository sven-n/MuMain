// CEventPvP.cpp: interface for the CEventPvP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "EventPvP.h"
#include "GameMaster.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventPvP gEventPvP;

CEventPvP::CEventPvP() // OK
{
	this->m_RemainTime = 0;
	this->m_GmIndex = 0;
	this->m_TickCount = GetTickCount();
	this->m_TotalTime = 0;
	this->m_EventPvPActive = 0;
	this->m_EventPvPEnter = 0;
	this->m_Player1Index = -1;
	this->m_Player2Index = -1;
	this->m_Score1 = 0;
	this->m_Score2 = 0;
}

CEventPvP::~CEventPvP() // OK
{

}

void CEventPvP::Clear()
{
	if (gObjIsConnectedGP(this->m_Player1Index) == 1)
	{
		GCPKLevelSend(this->m_Player1Index,gObj[this->m_Player1Index].PKLevel);
	}
	if (gObjIsConnectedGP(this->m_Player2Index) == 1)
	{
		GCPKLevelSend(this->m_Player2Index,gObj[this->m_Player2Index].PKLevel);
	}

	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_GmIndex = 0;
	this->m_TotalTime = 0;
	this->m_EventPvPActive = 0;
	this->m_EventPvPEnter = 0;
	this->m_Player1Index = -1;
	this->m_Player2Index = -1;
	this->m_Score1 = 0;
	this->m_Score2 = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Type == OBJECT_USER)
		{
			if (gObj[n].PvP == 1)
			{
				gObj[n].PvP = 0;
			}			
		}
	}
}

void CEventPvP::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	if(this->m_RemainTime > 0)
	{ 

		if (this->m_EventPvPActive && gObjIsConnectedGP(this->m_Player1Index) == 0 && gObjIsConnectedGP(this->m_Player2Index) == 0) 
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(799));
			LogAdd(LOG_EVENT,"[EVENT PVP] Cancelled, no players");
			this->Clear();
		}

		if (this->m_EventPvPEnter && this->m_Player1Index != -1 && this->m_Player2Index != -1) 
		{
			this->m_EventPvPActive = 1;
			this->m_EventPvPEnter = 0;
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(801), gObj[this->m_Player1Index].Name, gObj[this->m_Player2Index].Name);

			GCActionSend(&gObj[this->m_Player1Index],ACTION_RUSH1,this->m_Player1Index,this->m_Player2Index);

			GCActionSend(&gObj[this->m_Player2Index],ACTION_RUSH1,this->m_Player2Index,this->m_Player1Index);

			GCPKLevelSend(this->m_Player1Index,6);
			GCPKLevelSend(this->m_Player2Index,6);

			LogAdd(LOG_EVENT,"[EVENT PVP] Started (%s x %s)", gObj[this->m_Player1Index].Name, gObj[this->m_Player2Index].Name);
		}
			
		this->m_RemainTime--;

		if(this->m_RemainTime <= 0)
		{
			    this->m_EventPvPActive = 0;
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(799));
				LogAdd(LOG_EVENT,"[EVENT PVP] Time is over");

				this->Clear();

		}

		if (this->m_EventPvPActive && gObjIsConnectedGP(this->m_Player1Index) == 0) 
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(803),gObj[this->m_Player2Index].Name);
			GCFireworksSend(&gObj[this->m_Player2Index],gObj[this->m_Player2Index].X,gObj[this->m_Player2Index].Y);

			if (gServerInfo.m_EventPvPAutoReward1 > 0 || gServerInfo.m_EventPvPAutoReward2 > 0 || gServerInfo.m_EventPvPAutoReward3 > 0)
			{	
				GDSetCoinSend(this->m_Player2Index, gServerInfo.m_EventPvPAutoReward1, gServerInfo.m_EventPvPAutoReward2, gServerInfo.m_EventPvPAutoReward3,"PvpEvent");
			}

			LogAdd(LOG_EVENT,"[EVENT PVP] (Winner: %s)",gObj[this->m_Player2Index].Name);
			this->Clear();
		}

		if (this->m_EventPvPActive && gObjIsConnectedGP(this->m_Player2Index) == 0) 
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(803),gObj[this->m_Player1Index].Name);
			GCFireworksSend(&gObj[this->m_Player1Index],gObj[this->m_Player1Index].X,gObj[this->m_Player1Index].Y);

			if (gServerInfo.m_EventPvPAutoReward1 > 0 || gServerInfo.m_EventPvPAutoReward2 > 0 || gServerInfo.m_EventPvPAutoReward3 > 0)
			{	
				GDSetCoinSend(this->m_Player1Index, gServerInfo.m_EventPvPAutoReward1, gServerInfo.m_EventPvPAutoReward2, gServerInfo.m_EventPvPAutoReward3,"PvpEvent");
			}

			LogAdd(LOG_EVENT,"[EVENT PVP] (Winner: %s)",gObj[this->m_Player1Index].Name);
			this->Clear();
		}

	}

}

bool CEventPvP::CommandEventPvP(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventPvPSwitch == 0)
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
		LogAdd(LOG_EVENT,"[EVENT PVP] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(800));

		return 0;
	}

	this->m_EventPvPEnter			= 1;
	this->m_GmIndex					= lpObj->Index;
	this->m_RemainTime				= gServerInfo.m_EventPvPMaxTime*60;
	this->m_TotalTime				= gServerInfo.m_EventPvPMaxTime*60;

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(804));

	LogAdd(LOG_EVENT,"[EVENT PVP] Start by %s",lpObj->Name);

	return 1;
}

bool CEventPvP::EventPvPTradeJoin(int aIndex,int bIndex) // OK 
{
	if(gServerInfo.m_EventPvPSwitch == 0)
	{
		return 0;
	}

	if (this->m_EventPvPEnter == 0) 
	{
		return 0;
	}

	if (this->m_GmIndex != aIndex) 
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[bIndex];

	if (lpObj->PvP != 0) 
	{
		return 0;
	}

	if (this->m_Player1Index != -1 && this->m_Player2Index != -1) 
	{
		return 0;
	}

	if (this->m_Player1Index == -1)
	{
		this->m_Player1Index = lpObj->Index;
	}
	else
	{
		this->m_Player2Index = lpObj->Index;
	}
		
	lpObj->PvP = 1;

	LogAdd(LOG_EVENT,"[EVENT PVP] Member add (%s)",lpObj->Name);

	return 1;
}

void CEventPvP::EventPvPDead(int aIndex,int bIndex) // OK 
{
	if(gServerInfo.m_EventPvPSwitch == 0)
	{
		return;
	}

	if (this->m_EventPvPActive == 0) 
	{
		return;
	}

	if (this->m_Player1Index != aIndex && this->m_Player1Index != bIndex) 
	{
		return;
	}

	if (this->m_Player2Index != aIndex && this->m_Player2Index != bIndex) 
	{
		return;
	}

	LPOBJ lpObj = &gObj[bIndex];

	if (lpObj->PvP != 1) 
	{
		return;
	}

	if (this->m_Player1Index == bIndex)
	{
		this->m_Score1++;
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(802),gObj[this->m_Player1Index].Name,this->m_Score1, this->m_Score2 ,gObj[this->m_Player2Index].Name);
		
	}

	if (this->m_Player2Index == bIndex)
	{
		this->m_Score2++;
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(802),gObj[this->m_Player1Index].Name,this->m_Score1, this->m_Score2 ,gObj[this->m_Player2Index].Name);
	}

	if (this->m_Score1 >= gServerInfo.m_EventPvPMaxScore)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(803),gObj[this->m_Player1Index].Name);
		GCFireworksSend(&gObj[this->m_Player1Index],gObj[this->m_Player1Index].X,gObj[this->m_Player1Index].Y);

		if (gServerInfo.m_EventPvPAutoReward1 > 0 || gServerInfo.m_EventPvPAutoReward2 > 0 || gServerInfo.m_EventPvPAutoReward3 > 0)
		{	
			GDSetCoinSend(this->m_Player1Index, gServerInfo.m_EventPvPAutoReward1, gServerInfo.m_EventPvPAutoReward2, gServerInfo.m_EventPvPAutoReward3,"PvpEvent");
		}

		LogAdd(LOG_EVENT,"[EVENT PVP] (Winner: %s)",gObj[this->m_Player1Index].Name);
		this->Clear();
	}

	if (this->m_Score2 >= gServerInfo.m_EventPvPMaxScore)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(803),gObj[this->m_Player2Index].Name);
		GCFireworksSend(&gObj[this->m_Player2Index],gObj[this->m_Player2Index].X,gObj[this->m_Player2Index].Y);

		if (gServerInfo.m_EventPvPAutoReward1 > 0 || gServerInfo.m_EventPvPAutoReward2 > 0 || gServerInfo.m_EventPvPAutoReward3 > 0)
		{	
			GDSetCoinSend(this->m_Player2Index, gServerInfo.m_EventPvPAutoReward1, gServerInfo.m_EventPvPAutoReward2, gServerInfo.m_EventPvPAutoReward3,"PvpEvent");
		}

		LogAdd(LOG_EVENT,"[EVENT PVP] (Winner: %s)",gObj[this->m_Player2Index].Name);
		this->Clear();
	}

	return;
}

