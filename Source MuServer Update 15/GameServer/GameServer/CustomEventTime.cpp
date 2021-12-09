#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "CustomEventTime.h"
#include "MemScript.h"
#include "Notice.h"

CCustomEventTime gCustomEventTime;


void CCustomEventTime::GCReqEventTime(int Index,PMSG_CUSTOM_EVENTTIME_RECV* lpMsg)
{

#if (GAMESERVER_CLIENTE_UPDATE >= 8)

	if(gServerInfo.m_CustomEventTimeSwitch == 0)
	{
		return;
	}

	if(gObjIsConnected(Index) == false)
	{
		return;
	}

	BYTE send[4096];

	PMSG_CUSTOM_EVENTTIME_SEND pMsg;

	pMsg.header.set(0xF3,0xE8,0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	CUSTOM_EVENTTIME_DATA info;

	for(int n=0;n < 14;n++)
	{
	    info.index = n;

		if (n == 0)
		info.time = gServerDisplayer.EventBc;

		if (n == 1)
		info.time = gServerDisplayer.EventDs;

		if (n == 2)
		info.time = gServerDisplayer.EventCc;

		if (n == 3)
		info.time = gServerDisplayer.EventIt;

		if (n == 4)
		info.time = gServerDisplayer.EventCustomLottery;

		if (n == 5)
		info.time = gServerDisplayer.EventCustomQuiz;

		if (n == 6)
		info.time = gServerDisplayer.EventCustomBonus;

		if (n == 7)
		info.time = gServerDisplayer.EventMoss;

		if (n == 8)
		info.time = gServerDisplayer.EventDrop;

		if (n == 9)
		info.time = gServerDisplayer.EventKing;

		if (n == 10)
		info.time = gServerDisplayer.EventTvT;

		if (n == 11)
		info.time = -1;

		if (n == 12)
		info.time = -1;

		if (n == 13)
		info.time = -1;

		pMsg.count++;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);
	}

	for (int n=0; n < 14; n++)
	{
	    info.index = n+14;

		info.time = gServerDisplayer.EventInvasion[n];

		pMsg.count++;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);
	}

	for (int n=0; n < 14; n++)
	{
	    info.index = n+28;

		info.time = gServerDisplayer.EventCustomArena[n];

		pMsg.count++;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(Index,send,size);

#endif
	return;
}