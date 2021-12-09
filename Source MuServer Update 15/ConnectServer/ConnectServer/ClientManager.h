// ClientManager.h: interface for the CClientManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "SocketManager.h"

#define MAX_CLIENT 10000
#define MAX_ONLINE_TIME 300000

#define CLIENT_RANGE(x) (((x)<0)?0:((x)>=MAX_CLIENT)?0:1)

enum eClientState
{
	CLIENT_OFFLINE = 0,
	CLIENT_ONLINE = 1,
};

class CClientManager
{
public:
	CClientManager();
	virtual ~CClientManager();
	bool CheckState();
	bool CheckAlloc();
	bool CheckOnlineTime();
	void AddClient(int index,char* ip,SOCKET socket);
	void DelClient();
public:
	int m_index;
	eClientState m_state;
	char m_IpAddr[16];
	SOCKET m_socket;
	IO_RECV_CONTEXT* m_IoRecvContext;
	IO_SEND_CONTEXT* m_IoSendContext;
	DWORD m_OnlineTime;
	DWORD m_PacketTime;
};

extern CClientManager gClientManager[MAX_CLIENT];
