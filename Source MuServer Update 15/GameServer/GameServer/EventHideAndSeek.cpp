// CEventHideAndSeek.cpp: interface for the CEventHideAndSeek class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "EventHideAndSeek.h"
#include "GameMaster.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventHideAndSeek gEventHideAndSeek;

CEventHideAndSeek::CEventHideAndSeek() // OK
{
	this->m_RemainTime = 0;
	this->m_GmIndex = 0;
	this->m_TickCount = GetTickCount();
	this->m_EventHideAndSeek = 0;
}

CEventHideAndSeek::~CEventHideAndSeek() // OK
{

}

void CEventHideAndSeek::Clear()
{
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_GmIndex = 0;
	this->MinutesLeft = -1;
	this->m_EventHideAndSeek = 0;
}

void CEventHideAndSeek::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	if(this->m_RemainTime > 0)
	{ 
		int minutes = this->m_RemainTime/60;

		if((this->m_RemainTime%60) == 0)
		{
			minutes--;
		}

		if(this->MinutesLeft != minutes)
		{
			this->MinutesLeft = minutes;

				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(682),(MinutesLeft+1));
		}

		if(this->m_RemainTime <= 10)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(676),m_RemainTime);
		}

		this->m_RemainTime--;

		if(this->m_RemainTime <= 0)
		{
			    this->m_EventHideAndSeek = 0;
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(677));
				LogAdd(LOG_EVENT,"[EVENT HIDE AND SEEK] Close");

		}

	}

}

void CEventHideAndSeek::CommandEventHideAndSeek(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventHideAndSeekSwitch == 0)
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
		LogAdd(LOG_EVENT,"[EVENT HIDE AND SEEK] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(678));
		return;
	}

	this->m_EventHideAndSeek = 1;
	this->m_GmIndex = lpObj->Index;

	this->m_RemainTime = gServerInfo.m_EventHideAndSeekMaxTime*60;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(679),lpObj->Name);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(680));
	LogAdd(LOG_EVENT,"[EVENT HIDE AND SEEK] Start");
}

int CEventHideAndSeek::EventHideAndSeekTrade(int aIndex,int bIndex) // OK 
{

	if(gServerInfo.m_EventHideAndSeekSwitch == 0)
	{
		return 0;
	}

	if (this->m_EventHideAndSeek == 0) 
	{
		return 0;
	}

	if (this->m_GmIndex != bIndex) 
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[aIndex];

	this->Clear();

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(681),lpObj->Name);
	GCFireworksSend(lpObj,lpObj->X,lpObj->Y);

	if (gServerInfo.m_EventHideAndSeekAutoReward1 > 0 || gServerInfo.m_EventHideAndSeekAutoReward2 > 0 || gServerInfo.m_EventHideAndSeekAutoReward3 > 0)
	{	
		GDSetCoinSend(lpObj->Index, gServerInfo.m_EventHideAndSeekAutoReward1, gServerInfo.m_EventHideAndSeekAutoReward2, gServerInfo.m_EventHideAndSeekAutoReward3,"HideAndSeek");
	}

	LogAdd(LOG_EVENT,"[EVENT HIDE AND SEEK] Finish (Winner: %s)",lpObj->Name);

	return 1;
}
