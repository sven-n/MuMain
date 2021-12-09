// ServerList.h: interface for the CServerList class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "ConnectServerProtocol.h"

#define MAX_JOIN_SERVER_QUEUE_SIZE 100

//**********************************************//
//**********************************************//
//**********************************************//

struct SDHP_GAME_SERVER_LIVE_RECV
{
	PBMSG_HEAD header; // C1:01
	WORD ServerCode;
	BYTE UserTotal;
	WORD UserCount;
	WORD AccountCount;
	WORD PCPointCount;
	WORD MaxUserCount;
};

struct SDHP_JOIN_SERVER_LIVE_RECV
{
	PBMSG_HEAD header; // C1:02
	DWORD QueueSize;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct SERVER_LIST_INFO
{
	WORD ServerCode;
	char ServerName[32];
	char ServerAddress[16];
	WORD ServerPort;
	bool ServerShow;
	bool ServerState;
	UINT ServerStateTime;
	BYTE UserTotal;
	WORD UserCount;
	WORD AccountCount;
	WORD PCPointCount;
	WORD MaxUserCount;
};

class CServerList
{
public:
	CServerList();
	virtual ~CServerList();
	void Load(char* path);
	void MainProc();
	bool CheckJoinServerState();
	long GenerateServerList(BYTE* lpMsg,int* size);
	SERVER_LIST_INFO* GetServerListInfo(int ServerCode);
	void ServerProtocolCore(BYTE head,BYTE* lpMsg,int size);
	void GCGameServerLiveRecv(SDHP_GAME_SERVER_LIVE_RECV* lpMsg);
	void JCJoinServerLiveRecv(SDHP_JOIN_SERVER_LIVE_RECV* lpMsg);
private:
	bool m_JoinServerState;
	DWORD m_JoinServerStateTime;
	DWORD m_JoinServerQueueSize;
	std::map<int,SERVER_LIST_INFO> m_ServerListInfo;
};

extern CServerList gServerList;
