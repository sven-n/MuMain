// Connection.h: interface for the CConnection class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CriticalSection.h"

#define MAX_BUFF_SIZE 524288

class CConnection
{
public:
	CConnection();
	virtual ~CConnection();
	void Init(HWND hwnd,void* function);
	bool Connect(char* IpAddress,WORD port,DWORD WinMsg);
	void Disconnect();
	bool CheckState();
	bool DataRecv();
	bool DataSend(BYTE* lpMsg,int size);
	bool DataSendEx();
private:
	HWND m_hwnd;
	SOCKET m_socket;
	BYTE m_RecvBuff[MAX_BUFF_SIZE];
	int m_RecvSize;
	BYTE m_SendBuff[MAX_BUFF_SIZE];
	int m_SendSize;
	void (*wsProtocolCore)(BYTE,BYTE*,int);
	CCriticalSection m_critical;
};
