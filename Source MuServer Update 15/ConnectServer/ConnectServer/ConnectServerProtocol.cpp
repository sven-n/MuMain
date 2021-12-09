#include "stdafx.h"
#include "ConnectServerProtocol.h"
#include "ClientManager.h"
#include "Log.h"
#include "Protect.h"
#include "ServerList.h"
#include "SocketManager.h"
#include "Util.h"

void ConnectServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	gClientManager[index].m_PacketTime = GetTickCount();

	switch(head)
	{
		case 0xF4:
			switch(lpMsg[3])
			{
				case 0x03:
					CCServerInfoRecv((PMSG_SERVER_INFO_RECV*)lpMsg,index);
					break;
				case 0x06:
					CCServerListRecv((PMSG_SERVER_LIST_RECV*)lpMsg,index);
					break;
			}
			break;
	}

	PROTECT_FINAL
}

void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg,int index) // OK
{
	if(gServerList.CheckJoinServerState() == 0)
	{
		return;
	}

	SERVER_LIST_INFO* lpServerListInfo = gServerList.GetServerListInfo(lpMsg->ServerCode);

	if(lpServerListInfo == 0)
	{
		return;
	}

	if(lpServerListInfo->ServerShow == 0 || lpServerListInfo->ServerState == 0)
	{
		return;
	}

	PMSG_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0xF4,0x03,sizeof(pMsg));

	memcpy(pMsg.ServerAddress,lpServerListInfo->ServerAddress,sizeof(pMsg.ServerAddress));

	pMsg.ServerPort = lpServerListInfo->ServerPort;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}

void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg,int index) // OK
{
	BYTE send[2048];

	PMSG_SERVER_LIST_SEND pMsg;

	pMsg.header.set(0xF4,0x06,0);

	int size = sizeof(pMsg);

	int count = gServerList.GenerateServerList(send,&size);

	pMsg.count[0] = SET_NUMBERHB(count);

	pMsg.count[1] = SET_NUMBERLB(count);

	pMsg.header.size[0] = SET_NUMBERHB(size);

	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);
}

void CCServerInitSend(int index,int result) // OK
{
	PMSG_SERVER_INIT_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	pMsg.result = result;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}
