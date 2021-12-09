// CEventQuickly.cpp: interface for the CEventQuickly class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "EventQuickly.h"
#include "GameMaster.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CEventQuickly gEventQuickly;

CEventQuickly::CEventQuickly() // OK
{
	this->m_NpcIndex = -1;
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->MinutesLeft = -1;
}

CEventQuickly::~CEventQuickly() // OK
{

}

void CEventQuickly::Clear()
{
	this->m_NpcIndex = -1;
	this->m_RemainTime = 0;
	this->m_TickCount = GetTickCount();
	this->MinutesLeft = -1;
}

void CEventQuickly::MainProc() // OK
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

				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(661),(MinutesLeft+1));
		}

		if(this->m_RemainTime <= 5)
		{
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(662),m_RemainTime);
		}

	
		this->m_RemainTime--;

		if(this->m_RemainTime <= 0)
		{
				gObjDel(this->m_NpcIndex);
			    this->Clear();
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(664));
				LogAdd(LOG_EVENT,"[QUICKLY EVENT] Finish");

		}

	}

}

void CEventQuickly::CommandEventQuickly(LPOBJ lpObj,char* arg) // OK 
{

	if(gServerInfo.m_QuicklyEvent == 0)
	{
		return;
	}

	if(gGameMaster.CheckGameMasterLevel(lpObj,1) == 0)
	{
		return;
	}

	if (this->m_RemainTime > 0) 
	{
		gObjDel(this->m_NpcIndex);
		this->Clear();
		LogAdd(LOG_EVENT,"[QUICKLY EVENT] Canceled");

		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(666));
		return;
	}

	int map  = gCommandManager.GetNumber(arg,0);
	int x    = gCommandManager.GetNumber(arg,1);
	int y    = gCommandManager.GetNumber(arg,2);
	int time = gCommandManager.GetNumber(arg,3);

	

	if(map < 0 || x <= 0 || y <= 0)
	{

		//gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(665));
		//return;

		map = rand() % 4;

			map = (map==1) ? 0:map;
	
			if(gObjGetRandomFreeLocation(map,&x,&y,255,255,50) == 0)
			{
				return;
			}
	}

	char MapName[15];

	switch(map) 
	{
		case 0:
			wsprintf(MapName,"Lorencia");
			break;
		case 2:
			wsprintf(MapName,"Devias");
			break;
		case 3:
			wsprintf(MapName,"Noria");
			break;
		default:
			wsprintf(MapName,"Secreto");
			break;

	}

	time = (time <= 0 || time > 5) ? 2:time;

	int index = gObjAddMonster(map);
	
	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpNPC = &gObj[index];

	int px = x;
	int py = y;

	lpNPC->PosNum = -1;
	lpNPC->X = px;
	lpNPC->Y = py;
	lpNPC->TX = px;
	lpNPC->TY = py;
	lpNPC->OldX = px;
	lpNPC->OldY = py;
	lpNPC->StartX = px;
	lpNPC->StartY = py;
	lpNPC->Dir = 1;
	lpNPC->Map = map;

	if(gObjSetMonster(index,465) == 0)
	{
		gObjDel(index);
		return;
	}

	this->m_NpcIndex = index;

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(660),MapName);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(665),x,y);

	this->m_RemainTime = time*60;

	LogAdd(LOG_EVENT,"[QUICKLY EVENT] Start");
}

void CEventQuickly::ClickNPC(LPOBJ lpObj) // OK 
{

	gObjDel(this->m_NpcIndex);
	this->Clear();

	GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(663),lpObj->Name);

	if (gServerInfo.m_QuicklyEventAutoReward1 > 0 || gServerInfo.m_QuicklyEventAutoReward2 > 0 || gServerInfo.m_QuicklyEventAutoReward3 > 0)
	{	
		GDSetCoinSend(lpObj->Index, gServerInfo.m_QuicklyEventAutoReward1, gServerInfo.m_QuicklyEventAutoReward2, gServerInfo.m_QuicklyEventAutoReward3,"QuicklyEvent");
	}

	LogAdd(LOG_EVENT,"[QUICKLY EVENT] Finish (Winner:%s)",lpObj->Name);
}