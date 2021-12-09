// ServerManager.h: interface for the CServerManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "SocketManager.h"

#define MAX_SERVER 20

#define SERVER_RANGE(x) (((x)<0)?0:((x)>=MAX_SERVER)?0:1)

enum eServerState
{
	SERVER_OFFLINE = 0,
	SERVER_ONLINE = 1,
};

class CServerManager
{
public:
	CServerManager();
	virtual ~CServerManager();
	bool CheckState();
	bool CheckAlloc();
	void AddServer(int index,char* ip,SOCKET socket);
	void DelServer();
	void SetServerInfo(char* name,WORD port,WORD code);
public:
	int m_index;
	eServerState m_state;
	char m_IpAddr[16];
	SOCKET m_socket;
	IO_RECV_CONTEXT* m_IoRecvContext;
	IO_SEND_CONTEXT* m_IoSendContext;
	char m_ServerName[50];
	WORD m_ServerPort;
	WORD m_ServerCode;
	DWORD m_OnlineTime;
	DWORD m_PacketTime;
};

extern CServerManager gServerManager[MAX_SERVER];
