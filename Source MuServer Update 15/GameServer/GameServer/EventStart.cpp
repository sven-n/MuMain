// CEventStart.cpp: interface for the CEventStart class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "EventStart.h"
#include "GameMaster.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventStart gEventStart;

CEventStart::CEventStart() // OK
{
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
}

CEventStart::~CEventStart() // OK
{

}

void CEventStart::Clear()
{
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_Map = 0;
	this->m_MapX = 0;
	this->m_MapY = 0;
}

void CEventStart::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	if(this->m_RemainTime > 0)
	{ 

		if(this->m_RemainTime <= 10)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(670),m_RemainTime);
		}

		this->m_RemainTime--;

		if(this->m_RemainTime <= 0)
		{
			    this->m_EventStart = 0;
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(671));
				LogAdd(LOG_EVENT,"[EVENT START] Close");

		}

	}

}

void CEventStart::CommandEventStart(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventStartSwitch == 0)
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
		LogAdd(LOG_EVENT,"[EVENT START] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(672));
		return;
	}

	this->m_EventStart = 1;
	this->m_Map = lpObj->Map;
	this->m_MapX = lpObj->X;
	this->m_MapY = lpObj->Y-1;

	this->m_RemainTime = gServerInfo.m_EventStartTime;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(673),lpObj->Name);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(674));
	LogAdd(LOG_EVENT,"[EVENT START] Start");
}

void CEventStart::CommandEventStartJoin(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_EventStartSwitch == 0)
	{
		return;
	}

	if (this->m_EventStart == 0) 
	{
		return;
	}

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(675),lpObj->Name);

	gObjTeleport(lpObj->Index,this->m_Map,this->m_MapX,this->m_MapY);

	LogAdd(LOG_EVENT,"[EVENT START] Member add (%s)",lpObj->Name);
}
