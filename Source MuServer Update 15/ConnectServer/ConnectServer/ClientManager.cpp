// ClientManager.cpp: implementation of the CClientManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientManager.h"
#include "ConnectServerProtocol.h"
#include "IpManager.h"
#include "Util.h"

CClientManager gClientManager[MAX_CLIENT];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientManager::CClientManager() // OK
{
	this->m_index = -1;

	this->m_state = CLIENT_OFFLINE;

	this->m_socket = INVALID_SOCKET;

	this->m_IoRecvContext = 0;

	this->m_IoSendContext = 0;

	this->m_OnlineTime = 0;

	this->m_PacketTime = 0;
}

CClientManager::~CClientManager() // OK
{

}

bool CClientManager::CheckState() // OK
{
	if(CLIENT_RANGE(this->m_index) == 0 || this->m_state == CLIENT_OFFLINE || this->m_socket == INVALID_SOCKET)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool CClientManager::CheckAlloc() // OK
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

bool CClientManager::CheckOnlineTime() // OK
{
	if((GetTickCount()-this->m_OnlineTime) > MAX_ONLINE_TIME)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void CClientManager::AddClient(int index,char* ip,SOCKET socket) // OK
{
	this->m_index = index;

	this->m_state = CLIENT_ONLINE;

	strcpy_s(this->m_IpAddr,ip);

	this->m_socket = socket;

	gClientCount = ((this->CheckAlloc()==0)?(((++gClientCount)>=MAX_CLIENT)?0:gClientCount):gClientCount);

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

	this->m_OnlineTime = GetTickCount();

	this->m_PacketTime = 0;

	gIpManager.InsertIpAddress(this->m_IpAddr);

	CCServerInitSend(this->m_index,1);
}

void CClientManager::DelClient() // OK
{
	gIpManager.RemoveIpAddress(this->m_IpAddr);

	this->m_index = -1;

	this->m_state = CLIENT_OFFLINE;

	memset(this->m_IpAddr,0,sizeof(this->m_IpAddr));

	this->m_socket = INVALID_SOCKET;

	this->m_OnlineTime = GetTickCount();

	this->m_PacketTime = 0;
}
