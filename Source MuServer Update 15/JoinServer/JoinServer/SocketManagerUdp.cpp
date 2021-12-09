// SocketManagerUdp.cpp: implementation of the CSocketManagerUdp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketManagerUdp.h"
#include "Util.h"

CSocketManagerUdp gSocketManagerUdp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketManagerUdp::CSocketManagerUdp() // OK
{
	this->m_socket = INVALID_SOCKET;

	this->m_ServerRecvThread = 0;
}

CSocketManagerUdp::~CSocketManagerUdp() // OK
{
	this->Clean();
}

bool CSocketManagerUdp::Start(WORD port) // OK
{
	if((this->m_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_IP)) == INVALID_SOCKET)
	{
		LogAdd(LOG_RED,"[SocketManagerUdp] WSASocket() failed with error: %d",WSAGetLastError());
		this->Clean();
		return 0;
	}

	this->m_SocketAddr.sin_family = AF_INET;
	this->m_SocketAddr.sin_addr.s_addr = htonl(0);
	this->m_SocketAddr.sin_port = htons(port);

	if(bind(this->m_socket,(sockaddr*)&this->m_SocketAddr,sizeof(this->m_SocketAddr)) == SOCKET_ERROR)
	{
		LogAdd(LOG_RED,"[SocketManagerUdp] bind() failed with error: %d",WSAGetLastError());
		this->Clean();
		return 0;
	}

	if((this->m_ServerRecvThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)this->ServerRecvThread,this,0,0)) == 0)
	{
		LogAdd(LOG_RED,"[SocketManagerUdp] CreateThread() failed with error: %d",GetLastError());
		this->Clean();
		return 0;
	}

	memset(this->m_RecvBuff,0,sizeof(this->m_RecvBuff));

	this->m_RecvSize = 0;

	return 1;
}

bool CSocketManagerUdp::Connect(char* IpAddress,WORD port) // OK
{
	if((this->m_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_IP)) == INVALID_SOCKET)
	{
		LogAdd(LOG_RED,"[SocketManagerUdp] WSASocket() failed with error: %d",WSAGetLastError());
		this->Clean();
		return 0;
	}

	this->m_SocketAddr.sin_family = AF_INET;
	this->m_SocketAddr.sin_addr.s_addr = inet_addr(IpAddress);
	this->m_SocketAddr.sin_port = htons(port);

	if(this->m_SocketAddr.sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(IpAddress);

		if(host == 0)
		{
			LogAdd(LOG_RED,"[SocketManagerUdp] gethostbyname() failed with error: %d",WSAGetLastError());
			this->Clean();
			return 0;
		}

		memcpy(&this->m_SocketAddr.sin_addr.s_addr,*host->h_addr_list,host->h_length);
	}

	memset(this->m_SendBuff,0,sizeof(this->m_SendBuff));

	this->m_SendSize = 0;

	return 1;
}

void CSocketManagerUdp::Clean() // OK
{
	if(this->m_ServerRecvThread != 0)
	{
		TerminateThread(this->m_ServerRecvThread,0);
		CloseHandle(this->m_ServerRecvThread);
		this->m_ServerRecvThread = 0;
	}

	if(this->m_socket != INVALID_SOCKET)
	{
		closesocket(this->m_socket);
		this->m_socket = INVALID_SOCKET;
	}
}

bool CSocketManagerUdp::DataRecv() // OK
{
	if(this->m_RecvSize < 3)
	{
		return 1;
	}

	BYTE* lpMsg = this->m_RecvBuff;

	int count=0,size=0;
	BYTE header,head;

	while(true)
	{
		if(lpMsg[count] == 0xC1)
		{
			header = lpMsg[count];
			size = lpMsg[count+1];
			head = lpMsg[count+2];
		}
		else if(lpMsg[count] == 0xC2)
		{
			header = lpMsg[count];
			size = MAKEWORD(lpMsg[count+2],lpMsg[count+1]);
			head = lpMsg[count+3];
		}
		else
		{
			LogAdd(LOG_RED,"[SocketManagerUdp] Protocol header error (Header: %x)",lpMsg[count]);
			memset(this->m_RecvBuff,0,sizeof(this->m_RecvBuff));
			this->m_RecvSize = 0;
			return 0;
		}

		if(size < 3 || size > MAX_UDP_PACKET_SIZE)
		{
			LogAdd(LOG_RED,"[SocketManagerUdp] Protocol size error (Header: %x, Size: %d, Head: %x)",header,size,head);
			memset(this->m_RecvBuff,0,sizeof(this->m_RecvBuff));
			this->m_RecvSize = 0;
			return 0;
		}

		if(size <= this->m_RecvSize)
		{
			count += size;

			this->m_RecvSize -= size;

			if(this->m_RecvSize <= 0)
			{
				break;
			}
		}
		else
		{
			if(count > 0 && this->m_RecvSize > 0 && this->m_RecvSize <= (MAX_UDP_PACKET_SIZE-count))
			{
				memmove(lpMsg,&lpMsg[count],this->m_RecvSize);
			}

			break;
		}
	}

	return 1;
}

bool CSocketManagerUdp::DataSend(BYTE* lpMsg,int size) // OK
{
	if(this->m_socket == INVALID_SOCKET)
	{
		return 0;
	}

	if((this->m_SendSize+size) > MAX_UDP_PACKET_SIZE)
	{
		LogAdd(LOG_RED,"[SocketManagerUdp] Max msg size (Size: %d)",size);
		memset(this->m_SendBuff,0,sizeof(this->m_SendBuff));
		this->m_SendSize = 0;
		return 0;
	}

	memcpy(&this->m_SendBuff[this->m_SendSize],lpMsg,size);

	this->m_SendSize += size;

	int result = sendto(this->m_socket,(char*)this->m_SendBuff,this->m_SendSize,0,(sockaddr*)&this->m_SocketAddr,sizeof(this->m_SocketAddr));

	if(result == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
			LogAdd(LOG_RED,"[SocketManagerUdp] sendto() failed with error: %d",WSAGetLastError());
			memset(this->m_SendBuff,0,sizeof(this->m_SendBuff));
			this->m_SendSize = 0;
			return 0;
		}

		return 1;
	}

	this->m_SendSize -= result;

	memmove(this->m_SendBuff,&this->m_SendBuff[result],this->m_SendSize);
	return 1;
}

DWORD WINAPI CSocketManagerUdp::ServerRecvThread(CSocketManagerUdp* lpSocketManagerUdp) // OK
{
	while(true)
	{
		SOCKADDR_IN SocketAddr;
		int SocketAddrSize = sizeof(SocketAddr);

		int result = recvfrom(lpSocketManagerUdp->m_socket,(char*)&lpSocketManagerUdp->m_RecvBuff[lpSocketManagerUdp->m_RecvSize],(MAX_UDP_PACKET_SIZE-lpSocketManagerUdp->m_RecvSize),0,(sockaddr*)&SocketAddr,&SocketAddrSize);

		if(result == SOCKET_ERROR)
		{
			LogAdd(LOG_RED,"[SocketManagerUdp] recvfrom() failed with error: %d",GetLastError());
			memset(lpSocketManagerUdp->m_RecvBuff,0,sizeof(lpSocketManagerUdp->m_RecvBuff));
			lpSocketManagerUdp->m_RecvSize = 0;
			continue;
		}

		lpSocketManagerUdp->m_RecvSize += result;

		lpSocketManagerUdp->DataRecv();
	}

	return 0;
}
