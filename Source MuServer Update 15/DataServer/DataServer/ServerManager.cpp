// ServerManager.cpp: implementation of the CServerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerManager.h"
#include "CharacterManager.h"
#include "GuildMatching.h"
#include "PartyMatching.h"
#include "Util.h"

CServerManager gServerManager[MAX_SERVER];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerManager::CServerManager() // OK
{
	this->m_index = -1;

	this->m_state = SERVER_OFFLINE;

	this->m_socket = INVALID_SOCKET;

	this->m_IoRecvContext = 0;

	this->m_IoSendContext = 0;

	this->m_OnlineTime = 0;

	this->m_PacketTime = 0;
}

CServerManager::~CServerManager() // OK
{

}

bool CServerManager::CheckState() // OK
{
	if(SERVER_RANGE(this->m_index) == 0 || this->m_state == SERVER_OFFLINE || this->m_socket == INVALID_SOCKET)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool CServerManager::CheckAlloc() // OK
{
	if(this->m_IoRecvContext == 0 || this->m_IoSendContext == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void CServerManager::AddServer(int index,char* ip,SOCKET socket) // OK
{
	this->m_index = index;

	this->m_state = SERVER_ONLINE;

	strcpy_s(this->m_IpAddr,ip);

	this->m_socket = socket;

	gServerCount = ((this->CheckAlloc()==0)?(((++gServerCount)>=MAX_SERVER)?0:gServerCount):gServerCount);

	this->m_IoRecvContext = ((this->m_IoRecvContext==0)?(new IO_RECV_CONTEXT):this->m_IoRecvContext);

	this->m_IoSendContext = ((this->m_IoSendContext==0)?(new IO_SEND_CONTEXT):this->m_IoSendContext);

	memset(&this->m_IoRecvContext->overlapped,0,sizeof(this->m_IoRecvContext->overlapped));

	this->m_IoRecvContext->wsabuf.buf = (char*)this->m_IoRecvContext->IoMainBuffer.buff;
	this->m_IoRecvContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;
	this->m_IoRecvContext->IoType = IO_RECV;
	this->m_IoRecvContext->IoSize = 0;
	this->m_IoRecvContext->IoMainBuffer.size = 0;

	memset(&this->m_IoSendContext->overlapped,0,sizeof(this->m_IoSendContext->overlapped));

	this->m_IoSendContext->wsabuf.buf = (char*)this->m_IoSendContext->IoMainBuffer.buff;
	this->m_IoSendContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;
	this->m_IoSendContext->IoType = IO_SEND;
	this->m_IoSendContext->IoSize = 0;
	this->m_IoSendContext->IoMainBuffer.size = 0;
	this->m_IoSendContext->IoSideBuffer.size = 0;

	memset(this->m_ServerName,0,sizeof(this->m_ServerName));

	this->m_ServerPort = 0xFFFF;

	this->m_ServerCode = 0xFFFF;

	this->m_OnlineTime = GetTickCount();

	this->m_PacketTime = 0;

	LogAdd(LOG_BLACK,"[ServerManager][%d] AddServer (%s)",this->m_index,this->m_IpAddr);
}

void CServerManager::DelServer() // OK
{
	LogAdd(LOG_BLACK,"[ServerManager][%d] DelServer (%s)",this->m_index,this->m_IpAddr);

	WORD ServerCode = this->m_ServerCode;

	this->m_index = -1;

	this->m_state = SERVER_OFFLINE;

	memset(this->m_IpAddr,0,sizeof(this->m_IpAddr));

	this->m_socket = INVALID_SOCKET;

	memset(this->m_ServerName,0,sizeof(this->m_ServerName));

	this->m_ServerPort = 0xFFFF;

	this->m_ServerCode = 0xFFFF;

	this->m_OnlineTime = GetTickCount();

	this->m_PacketTime = 0;

	gCharacterManager.ClearServerCharacterInfo(ServerCode);

	gGuildMatching.ClearGuildMatchingInfo(ServerCode);

	gGuildMatching.ClearGuildMatchingJoinInfo(ServerCode);

	gPartyMatching.ClearPartyMatchingInfo(ServerCode);

	gPartyMatching.ClearPartyMatchingJoinInfo(ServerCode);
}

void CServerManager::SetServerInfo(char* name,WORD port,WORD code) // OK
{
	strcpy_s(this->m_ServerName,name);

	this->m_ServerPort = port;

	this->m_ServerCode = code;

	LogAdd(LOG_BLACK,"[ServerManager][%d] ServerInfo (%s) (%d) (%d)",this->m_index,this->m_ServerName,this->m_ServerPort,this->m_ServerCode);
}
