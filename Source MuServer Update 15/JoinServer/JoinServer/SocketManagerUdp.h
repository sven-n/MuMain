// SocketManagerUdp.h: interface for the CSocketManagerUdp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_UDP_PACKET_SIZE 4096

class CSocketManagerUdp
{
public:
	CSocketManagerUdp();
	virtual ~CSocketManagerUdp();
	bool Start(WORD port);
	bool Connect(char* IpAddress,WORD port);
	void Clean();
	bool DataRecv();
	bool DataSend(BYTE* lpMsg,int size);
	static DWORD WINAPI ServerRecvThread(CSocketManagerUdp* lpSocketManagerUdp);
private:
	SOCKET m_socket;
	SOCKADDR_IN m_SocketAddr;
	HANDLE m_ServerRecvThread;
	BYTE m_RecvBuff[MAX_UDP_PACKET_SIZE];
	int m_RecvSize;
	BYTE m_SendBuff[MAX_UDP_PACKET_SIZE];
	int m_SendSize;
};

extern CSocketManagerUdp gSocketManagerUdp;
