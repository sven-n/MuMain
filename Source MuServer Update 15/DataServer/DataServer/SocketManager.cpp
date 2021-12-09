// SocketManager.cpp: implementation of the CSocketManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketManager.h"
#include "AllowableIpList.h"
#include "DataServerProtocol.h"
#include "Protect.h"
#include "ServerManager.h"
#include "Util.h"

CSocketManager gSocketManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketManager::CSocketManager() // OK
{
	this->m_listen = INVALID_SOCKET;

	this->m_CompletionPort = 0;

	this->m_port = 0;

	this->m_ServerAcceptThread = 0;

	for(int n=0;n < MAX_SERVER_WORKER_THREAD;n++)
	{
		this->m_ServerWorkerThread[n] = 0;
	}

	this->m_ServerWorkerThreadCount = 0;

	this->m_ServerQueueSemaphore = 0;

	this->m_ServerQueueThread = 0;
}

CSocketManager::~CSocketManager() // OK
{
	this->Clean();
}

bool CSocketManager::Start(WORD port) // OK
{
	PROTECT_START

	this->m_port = port;

	if(this->CreateListenSocket() == 0)
	{
		this->Clean();
		return 0;
	}

	if(this->CreateCompletionPort() == 0)
	{
		this->Clean();
		return 0;
	}

	if(this->CreateAcceptThread() == 0)
	{
		this->Clean();
		return 0;
	}

	if(this->CreateWorkerThread() == 0)
	{
		this->Clean();
		return 0;
	}

	if(this->CreateServerQueue() == 0)
	{
		this->Clean();
		return 0;
	}

	PROTECT_FINAL

	LogAdd(LOG_BLACK,"[SocketManager] Server started at port [%d]",this->m_port);
	return 1;
}

void CSocketManager::Clean() // OK
{
	if(this->m_ServerQueueThread != 0)
	{
		TerminateThread(this->m_ServerQueueThread,0);
		CloseHandle(this->m_ServerQueueThread);
		this->m_ServerQueueThread = 0;
	}

	if(this->m_ServerQueueSemaphore != 0)
	{
		CloseHandle(this->m_ServerQueueSemaphore);
		this->m_ServerQueueSemaphore = 0;
	}

	this->m_ServerQueue.ClearQueue();

	for(DWORD n=0;n < MAX_SERVER_WORKER_THREAD;n++)
	{
		if(this->m_ServerWorkerThread[n] != 0)
		{
			TerminateThread(this->m_ServerWorkerThread[n],0);
			CloseHandle(this->m_ServerWorkerThread[n]);
			this->m_ServerWorkerThread[n] = 0;
		}
	}

	if(this->m_ServerAcceptThread != 0)
	{
		TerminateThread(this->m_ServerAcceptThread,0);
		CloseHandle(this->m_ServerAcceptThread);
		this->m_ServerAcceptThread = 0;
	}

	if(this->m_CompletionPort != 0)
	{
		CloseHandle(this->m_CompletionPort);
		this->m_CompletionPort = 0;
	}

	if(this->m_listen != INVALID_SOCKET)
	{
		closesocket(this->m_listen);
		this->m_listen = INVALID_SOCKET;
	}
}

bool CSocketManager::CreateListenSocket() // OK
{
	if((this->m_listen=WSASocket(AF_INET,SOCK_STREAM,0,0,0,WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		LogAdd(LOG_RED,"[SocketManager] WSASocket() failed with error: %d",WSAGetLastError());
		return 0;
	}

	SOCKADDR_IN SocketAddr;

	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(0);
	SocketAddr.sin_port = htons(this->m_port);

	if(bind(this->m_listen,(sockaddr*)&SocketAddr,sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		LogAdd(LOG_RED,"[SocketManager] bind() failed with error: %d",WSAGetLastError());
		return 0;
	}

	if(listen(this->m_listen,5) == SOCKET_ERROR)
	{
		LogAdd(LOG_RED,"[SocketManager] listen() failed with error: %d",WSAGetLastError());
		return 0;
	}

	return 1;
}

bool CSocketManager::CreateCompletionPort() // OK
{
	SOCKET socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_IP);

	if(socket == INVALID_SOCKET) 
	{
		LogAdd(LOG_RED,"[SocketManager] socket() failed with error: %d",WSAGetLastError());
		return 0;
	}

	if((this->m_CompletionPort=CreateIoCompletionPort((HANDLE)socket,0,0,0)) == 0)
	{
		LogAdd(LOG_RED,"[SocketManager] CreateIoCompletionPort() failed with error: %d",GetLastError());
		closesocket(socket);
		return 0;
	}

	closesocket(socket);
	return 1;
}

bool CSocketManager::CreateAcceptThread() // OK
{
	if((this->m_ServerAcceptThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)this->ServerAcceptThread,this,0,0)) == 0)
	{
		LogAdd(LOG_RED,"[SocketManager] CreateThread() failed with error: %d",GetLastError());
		return 0;
	}

	if(SetThreadPriority(this->m_ServerAcceptThread,THREAD_PRIORITY_HIGHEST) == 0)
	{
		LogAdd(LOG_RED,"[SocketManager] SetThreadPriority() failed with error: %d",GetLastError());
		return 0;
	}

	return 1;
}

bool CSocketManager::CreateWorkerThread() // OK
{
	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);

	this->m_ServerWorkerThreadCount = ((SystemInfo.dwNumberOfProcessors>MAX_SERVER_WORKER_THREAD)?MAX_SERVER_WORKER_THREAD:SystemInfo.dwNumberOfProcessors);

	for(DWORD n=0;n < this->m_ServerWorkerThreadCount;n++)
	{
		if((this->m_ServerWorkerThread[n]=CreateThread(0,0,(LPTHREAD_START_ROUTINE)this->ServerWorkerThread,this,0,0)) == 0)
		{
			LogAdd(LOG_RED,"[SocketManager] CreateThread() failed with error: %d",GetLastError());
			return 0;
		}

		if(SetThreadPriority(this->m_ServerWorkerThread[n],THREAD_PRIORITY_HIGHEST) == 0)
		{
			LogAdd(LOG_RED,"[SocketManager] SetThreadPriority() failed with error: %d",GetLastError());
			return 0;
		}
	}

	return 1;
}

bool CSocketManager::CreateServerQueue() // OK
{
	if((this->m_ServerQueueSemaphore=CreateSemaphore(0,0,MAX_QUEUE_SIZE,0)) == 0)
	{
		LogAdd(LOG_RED,"[SocketManager] CreateSemaphore() failed with error: %d",GetLastError());
		return 0;
	}

	if((this->m_ServerQueueThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)this->ServerQueueThread,this,0,0)) == 0)
	{
		LogAdd(LOG_RED,"[SocketManager] CreateThread() failed with error: %d",GetLastError());
		return 0;
	}

	if(SetThreadPriority(this->m_ServerQueueThread,THREAD_PRIORITY_HIGHEST) == 0)
	{
		LogAdd(LOG_RED,"[SocketManager] SetThreadPriority() failed with error: %d",GetLastError());
		return 0;
	}

	return 1;
}

bool CSocketManager::DataRecv(int index,IO_MAIN_BUFFER* lpIoBuffer) // OK
{
	if(lpIoBuffer->size < 3)
	{
		return 1;
	}

	BYTE* lpMsg = lpIoBuffer->buff;

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
			LogAdd(LOG_RED,"[SocketManager] Protocol header error (Index: %d, Header: %x)",index,lpMsg[count]);
			return 0;
		}

		if(size < 3 || size > MAX_MAIN_PACKET_SIZE)
		{
			LogAdd(LOG_RED,"[SocketManager] Protocol size error (Index: %d, Header: %x, Size: %d, Head: %x)",index,header,size,head);
			return 0;
		}

		if(size <= lpIoBuffer->size)
		{
			static QUEUE_INFO QueueInfo;

			QueueInfo.index = index;

			QueueInfo.head = head;

			memcpy(QueueInfo.buff,&lpMsg[count],size);

			QueueInfo.size = size;

			if(this->m_ServerQueue.AddToQueue(&QueueInfo) != 0)
			{
				ReleaseSemaphore(this->m_ServerQueueSemaphore,1,0);
			}

			count += size;

			lpIoBuffer->size -= size;

			if(lpIoBuffer->size <= 0)
			{
				break;
			}
		}
		else
		{
			if(count > 0 && lpIoBuffer->size > 0 && lpIoBuffer->size <= (MAX_MAIN_PACKET_SIZE-count))
			{
				memmove(lpMsg,&lpMsg[count],lpIoBuffer->size);
			}

			break;
		}
	}

	return 1;
}

bool CSocketManager::DataSend(int index,BYTE* lpMsg,int size) // OK
{
	this->m_critical.lock();

	if(SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return 0;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if(lpServerManager->CheckState() == 0)
	{
		this->m_critical.unlock();
		return 0;
	}

	if(size > MAX_MAIN_PACKET_SIZE)
	{
		LogAdd(LOG_RED,"[SocketManager] Max msg size (Type: 1, Index: %d, Size: %d)",index,size);
		this->m_critical.unlock();
		return 0;
	}

	IO_SEND_CONTEXT* lpIoContext = lpServerManager->m_IoSendContext;

	if(lpIoContext->IoSize > 0)
	{
		if((lpIoContext->IoSideBuffer.size+size) > MAX_SIDE_PACKET_SIZE)
		{
			LogAdd(LOG_RED,"[SocketManager] Max msg size (Type: 2, Index: %d, Size: %d)",index,(lpIoContext->IoSideBuffer.size+size));
			this->Disconnect(index);
			this->m_critical.unlock();
			return 0;
		}

		memcpy(&lpIoContext->IoSideBuffer.buff[lpIoContext->IoSideBuffer.size],lpMsg,size);
		lpIoContext->IoSideBuffer.size += size;
		this->m_critical.unlock();
		return 1;
	}

	memcpy(lpIoContext->IoMainBuffer.buff,lpMsg,size);

	lpIoContext->wsabuf.buf = (char*)lpIoContext->IoMainBuffer.buff;

	lpIoContext->wsabuf.len = size;

	lpIoContext->IoType = IO_SEND;

	lpIoContext->IoSize = size;

	lpIoContext->IoMainBuffer.size = 0;

	DWORD SendSize=0,Flags=0;
	
	if(WSASend(lpServerManager->m_socket,&lpIoContext->wsabuf,1,&SendSize,Flags,&lpIoContext->overlapped,0) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			LogAdd(LOG_RED,"[SocketManager] WSASend() failed with error: %d",WSAGetLastError());
			this->Disconnect(index);
			this->m_critical.unlock();
			return 0;
		}
	}

	this->m_critical.unlock();
	return 1;
}

void CSocketManager::Disconnect(int index) // OK
{
	this->m_critical.lock();

	if(SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if(lpServerManager->CheckState() == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if(closesocket(lpServerManager->m_socket) == SOCKET_ERROR && WSAGetLastError() != WSAENOTSOCK)
	{
		LogAdd(LOG_RED,"[SocketManager] closesocket() failed with error: %d",WSAGetLastError());
		this->m_critical.unlock();
		return;
	}

	lpServerManager->DelServer();

	this->m_critical.unlock();
}

void CSocketManager::OnRecv(int index,DWORD IoSize,IO_RECV_CONTEXT* lpIoContext) // OK
{
	this->m_critical.lock();

	if(SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if(IoSize == 0)
	{
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	lpIoContext->IoMainBuffer.size += IoSize;

	if(this->DataRecv(index,&lpIoContext->IoMainBuffer) == 0)
	{
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	lpIoContext->wsabuf.buf = (char*)&lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size];

	lpIoContext->wsabuf.len = MAX_MAIN_PACKET_SIZE-lpIoContext->IoMainBuffer.size;

	lpIoContext->IoType = IO_RECV;

	DWORD RecvSize=0,Flags=0;

	if(WSARecv(lpServerManager->m_socket,&lpIoContext->wsabuf,1,&RecvSize,&Flags,&lpIoContext->overlapped,0) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			LogAdd(LOG_RED,"[SocketManager] WSARecv() failed with error: %d",WSAGetLastError());
			this->Disconnect(index);
			this->m_critical.unlock();
			return;
		}
	}

	this->m_critical.unlock();
}

void CSocketManager::OnSend(int index,DWORD IoSize,IO_SEND_CONTEXT* lpIoContext) // OK
{
	this->m_critical.lock();

	if(SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if(IoSize == 0)
	{
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	lpIoContext->IoMainBuffer.size += IoSize;

	if(lpIoContext->IoMainBuffer.size >= lpIoContext->IoSize)
	{
		if(lpIoContext->IoSideBuffer.size <= 0)
		{
			lpIoContext->IoSize = 0;
			this->m_critical.unlock();
			return;
		}

		if(lpIoContext->IoSideBuffer.size > MAX_MAIN_PACKET_SIZE)
		{
			memcpy(lpIoContext->IoMainBuffer.buff,lpIoContext->IoSideBuffer.buff,MAX_MAIN_PACKET_SIZE);

			lpIoContext->wsabuf.buf = (char*)lpIoContext->IoMainBuffer.buff;

			lpIoContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;

			lpIoContext->IoType = IO_SEND;

			lpIoContext->IoSize = MAX_MAIN_PACKET_SIZE;

			lpIoContext->IoMainBuffer.size = 0;

			memmove(lpIoContext->IoSideBuffer.buff,&lpIoContext->IoSideBuffer.buff[MAX_MAIN_PACKET_SIZE],(lpIoContext->IoSideBuffer.size-MAX_MAIN_PACKET_SIZE));

			lpIoContext->IoSideBuffer.size = lpIoContext->IoSideBuffer.size-MAX_MAIN_PACKET_SIZE;
		}
		else
		{
			memcpy(lpIoContext->IoMainBuffer.buff,lpIoContext->IoSideBuffer.buff,lpIoContext->IoSideBuffer.size);

			lpIoContext->wsabuf.buf = (char*)lpIoContext->IoMainBuffer.buff;

			lpIoContext->wsabuf.len = lpIoContext->IoSideBuffer.size;

			lpIoContext->IoType = IO_SEND;

			lpIoContext->IoSize = lpIoContext->IoSideBuffer.size;

			lpIoContext->IoMainBuffer.size = 0;

			lpIoContext->IoSideBuffer.size = 0;
		}
	}
	else
	{
		lpIoContext->wsabuf.buf = (char*)&lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size];

		lpIoContext->wsabuf.len = lpIoContext->IoSize-lpIoContext->IoMainBuffer.size;

		lpIoContext->IoType = IO_SEND;
	}

	DWORD SendSize=0,Flags=0;

	if(WSASend(lpServerManager->m_socket,&lpIoContext->wsabuf,1,&SendSize,Flags,&lpIoContext->overlapped,0) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			LogAdd(LOG_RED,"[SocketManager] WSASend() failed with error: %d",WSAGetLastError());
			this->Disconnect(index);
			this->m_critical.unlock();
			return;
		}
	}

	this->m_critical.unlock();
}

int CALLBACK CSocketManager::ServerAcceptCondition(IN LPWSABUF lpCallerId,IN LPWSABUF lpCallerData,IN OUT LPQOS lpSQOS,IN OUT LPQOS lpGQOS,IN LPWSABUF lpCalleeId,OUT LPWSABUF lpCalleeData,OUT GROUP FAR* g,CSocketManager* lpSocketManager) // OK
{
	SOCKADDR_IN* SocketAddr = (SOCKADDR_IN*)lpCallerId->buf;

	if(gAllowableIpList.CheckAllowableIp(inet_ntoa(SocketAddr->sin_addr)) == 0)
	{
		return CF_REJECT;
	}
	else
	{
		return CF_ACCEPT;
	}
}

DWORD WINAPI CSocketManager::ServerAcceptThread(CSocketManager* lpSocketManager) // OK
{
	SOCKADDR_IN SocketAddr;
	int SocketAddrSize = sizeof(SocketAddr);

	while(true)
	{
		SOCKET socket = WSAAccept(lpSocketManager->m_listen,(sockaddr*)&SocketAddr,&SocketAddrSize,(LPCONDITIONPROC)&lpSocketManager->ServerAcceptCondition,(DWORD)lpSocketManager);

		if(socket == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
		{
			lpSocketManager->m_critical.lock();
			LogAdd(LOG_RED,"[SocketManager] WSAAccept() failed with error: %d",WSAGetLastError());
			lpSocketManager->m_critical.unlock();
			continue;
		}

		lpSocketManager->m_critical.lock();

		int index = -1;

		if((index=GetFreeServerIndex()) == -1)
		{
			closesocket(socket);
			lpSocketManager->m_critical.unlock();
			continue;
		}

		if(CreateIoCompletionPort((HANDLE)socket,lpSocketManager->m_CompletionPort,index,0) == 0)
		{
			LogAdd(LOG_RED,"[SocketManager] CreateIoCompletionPort() failed with error: %d",GetLastError());
			closesocket(socket);
			lpSocketManager->m_critical.unlock();
			continue;
		}

		PROTECT_START

		CServerManager* lpServerManager = &gServerManager[index];

		lpServerManager->AddServer(index,inet_ntoa(SocketAddr.sin_addr),socket);

		PROTECT_FINAL

		DWORD RecvSize=0,Flags=0;

		if(WSARecv(socket,&lpServerManager->m_IoRecvContext->wsabuf,1,&RecvSize,&Flags,&lpServerManager->m_IoRecvContext->overlapped,0) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING)
			{
				LogAdd(LOG_RED,"[SocketManager] WSARecv() failed with error: %d",WSAGetLastError());
				lpSocketManager->Disconnect(index);
				lpSocketManager->m_critical.unlock();
				continue;
			}
		}

		lpSocketManager->m_critical.unlock();
	}

	return 0;
}

DWORD WINAPI CSocketManager::ServerWorkerThread(CSocketManager* lpSocketManager) // OK
{
	DWORD IoSize;
	DWORD index;
	LPOVERLAPPED lpOverlapped;

	while(true)
	{
		if(GetQueuedCompletionStatus(lpSocketManager->m_CompletionPort,&IoSize,&index,&lpOverlapped,INFINITE) == 0)
		{
			if(lpOverlapped == 0 || (GetLastError() != ERROR_NETNAME_DELETED && GetLastError() != ERROR_CONNECTION_ABORTED && GetLastError() != ERROR_OPERATION_ABORTED && GetLastError() != ERROR_SEM_TIMEOUT))
			{
				lpSocketManager->m_critical.lock();
				LogAdd(LOG_RED,"[SocketManager] GetQueuedCompletionStatus() failed with error: %d",GetLastError());
				lpSocketManager->m_critical.unlock();
				return 0;
			}
		}

		lpSocketManager->m_critical.lock();

		if(IoSize == 0 && index == 0 && lpOverlapped == 0)
		{
			lpSocketManager->m_critical.unlock();
			return 0;
		}

		IO_CONTEXT* lpIoContext = (IO_CONTEXT*)lpOverlapped;

		switch(lpIoContext->IoType)
		{
			case IO_RECV:
				lpSocketManager->OnRecv(index,IoSize,(IO_RECV_CONTEXT*)lpIoContext);
				break;
			case IO_SEND:
				lpSocketManager->OnSend(index,IoSize,(IO_SEND_CONTEXT*)lpIoContext);
				break;
		}

		lpSocketManager->m_critical.unlock();
	}

	return 0;
}

DWORD WINAPI CSocketManager::ServerQueueThread(CSocketManager* lpSocketManager) // OK
{
	while(true)
	{
		if(WaitForSingleObject(lpSocketManager->m_ServerQueueSemaphore,INFINITE) == WAIT_FAILED)
		{
			LogAdd(LOG_RED,"[SocketManager] WaitForSingleObject() failed with error: %d",GetLastError());
			break;
		}

		static QUEUE_INFO QueueInfo;

		if(lpSocketManager->m_ServerQueue.GetFromQueue(&QueueInfo) != 0)
		{
			if(SERVER_RANGE(QueueInfo.index) != 0 && gServerManager[QueueInfo.index].CheckState() != 0)
			{
				DataServerProtocolCore(QueueInfo.index,QueueInfo.head,QueueInfo.buff,QueueInfo.size);
			}
		}
	}

	return 0;
}

DWORD CSocketManager::GetQueueSize() // OK
{
	return this->m_ServerQueue.GetQueueSize();
}
