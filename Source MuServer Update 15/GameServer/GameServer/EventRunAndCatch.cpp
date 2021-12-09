// CEventRunAndCatch.cpp: interface for the CEventRunAndCatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "EventRunAndCatch.h"
#include "GameMaster.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventRunAndCatch gEventRunAndCatch;

CEventRunAndCatch::CEventRunAndCatch() // OK
{
	this->m_RemainTime = 0;
	this->m_GmIndex = 0;
	this->m_TickCount = GetTickCount();
	this->m_TotalTime = 0;
	this->m_EventRunAndCatchActive = 0;
	this->m_EventRunAndCatchEnter = 0;
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
	this->m_Members = 0;
}

CEventRunAndCatch::~CEventRunAndCatch() // OK
{

}

void CEventRunAndCatch::Clear()
{
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_GmIndex = 0;
	this->m_TotalTime = 0;
	this->m_EventRunAndCatchActive = 0;
	this->m_EventRunAndCatchEnter = 0;
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
	this->m_Members = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObj[n].Type == OBJECT_USER)
		{
			if (gObj[n].RunAndCatch == 1)
			{
				gObj[n].RunAndCatch = 0;
			}			
		}
	}
}

void CEventRunAndCatch::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	if(this->m_RemainTime > 0)
	{ 
		if(this->m_RemainTime <= (this->m_TotalTime-gServerInfo.m_EventRunAndCatchTimeToEnter) && this->m_EventRunAndCatchEnter == 1)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(683));
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(684));
			this->m_EventRunAndCatchActive = 1;
			this->m_EventRunAndCatchEnter = 0;
		}

		this->m_RemainTime--;

		if(this->m_RemainTime <= 0)
		{
			    this->m_EventRunAndCatchActive = 0;
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(685));
				LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] Time is over");

				this->Clear();

		}

	}

}

void CEventRunAndCatch::CommandEventRunAndCatch(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventRunAndCatchSwitch == 0)
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
		LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(686));

		return;
	}

	this->m_EventRunAndCatchEnter	= 1;
	this->m_GmIndex					= lpObj->Index;
	this->m_RemainTime				= gServerInfo.m_EventRunAndCatchMaxTime*60;
	this->m_TotalTime				= gServerInfo.m_EventRunAndCatchMaxTime*60;
	this->m_Map						= lpObj->Map;
	this->m_MapX					= lpObj->X;
	this->m_MapY					= lpObj->Y-1;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(687));
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(688));

	LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] Start by %s",lpObj->Name);
}

void CEventRunAndCatch::CommandEventRunAndCatchJoin(LPOBJ lpObj,char* arg) // OK 
{
	if(gServerInfo.m_EventRunAndCatchSwitch == 0)
	{
		return;
	}

	if (this->m_EventRunAndCatchEnter == 0) 
	{
		return;
	}

	if (lpObj->Level <= 10) 
	{
		return;
	}

	//if (lpObj->Authority == 32) 
	//{
	//	return;
	//}

	if (lpObj->RunAndCatch != 0) 
	{
		return;
	}

	lpObj->RunAndCatch = 1;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(689),lpObj->Name);

	gObjTeleport(lpObj->Index,this->m_Map,this->m_MapX,this->m_MapY);

	LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] Member add (%s)",lpObj->Name);
}

void CEventRunAndCatch::EventRunAndCatchHit(int aIndex,int bIndex) // OK 
{
	if(gServerInfo.m_EventRunAndCatchSwitch == 0)
	{
		return;
	}

	if (this->m_EventRunAndCatchActive == 0) 
	{
		return;
	}

	if (this->m_GmIndex != aIndex) 
	{
		return;
	}

	LPOBJ lpObj = &gObj[bIndex];

	if (lpObj->RunAndCatch != 1) 
	{
		return;
	}

	this->m_Members = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER)
		{
			if (gObj[n].RunAndCatch == 1)
			{
				this->m_Members++;
			}			
		}
	}

	if (this->m_Members <= 0)
	{
		this->Clear();
		LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] Canceled");
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(686));

		return;
	}

	if (this->m_Members == 1)
	{
		this->Clear();

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(690),lpObj->Name);
		GCFireworksSend(lpObj,lpObj->X,lpObj->Y);

		if (gServerInfo.m_EventRunAndCatchAutoReward1 > 0 || gServerInfo.m_EventRunAndCatchAutoReward2 > 0 || gServerInfo.m_EventRunAndCatchAutoReward3 > 0)
		{	
			GDSetCoinSend(lpObj->Index, gServerInfo.m_EventRunAndCatchAutoReward1, gServerInfo.m_EventRunAndCatchAutoReward2, gServerInfo.m_EventRunAndCatchAutoReward3,"RunAndCatch");
		}

		LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] (Winner: %s)",lpObj->Name);
		lpObj->RunAndCatch = 0;
	}
	else 
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(691),lpObj->Name);
		LogAdd(LOG_EVENT,"[EVENT RUN AND CATCH] (Catched: %s)",lpObj->Name);
		lpObj->RunAndCatch = 0;
		gObjMoveGate(lpObj->Index,17);
	}

	return;
}

