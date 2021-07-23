
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WSCTLC_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WSCTLC_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifndef __WSCTLC_H__
#define __WSCTLC_H__


#ifdef ANTIHACKING_ENABLE
#include "proc.h"
#endif //ANTIHACKING_ENABLE

#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Log/muConsoleDebug.h"

#define MAX_SENDBUF		8192
#define MAX_RECVBUF		8192

typedef struct 
{
	int used;
	int len;
	BYTE buf[MAX_RECVBUF];
} STRUCT_RECVSTACK;

class CPacketQueue;

// This class is exported from the wsctlc.dll
class CWsctlc {

private :
	HWND   m_hWnd;					// 윈도우 메시지를 처리할 윈도우 핸들
	BOOL	m_bGame;	// 게임쪽 커넥션인가?

	int	   m_iMaxSockets;				// 소켓 최대 개수
	SOCKET m_socket;				// 사용할 소켓 

	BYTE   m_SendBuf[MAX_SENDBUF];	//
	int    m_nSendBufLen;			// 버퍼크기

	BYTE   m_RecvBuf[MAX_RECVBUF];
	int    m_nRecvBufLen;

	int		m_LogPrint;
	FILE*   m_logfp;

	CPacketQueue*	m_pPacketQueue;

	BOOL ShutdownConnection(SOCKET sd);
	
public:
	
	CWsctlc(void);
	~CWsctlc();

	SOCKET GetSocket();

	BOOL Startup();
	void Cleanup();
	
	int  Create(HWND hWnd, BOOL bGame = FALSE);
	BOOL Close();
	BOOL Close(SOCKET & socket);
	
	BOOL Connect(char *ip_addr, unsigned short port, DWORD WinMsgNum);

	int  sSend(SOCKET socket, char *buf, int len);
	__forceinline int  sSend(char *buf, int len)
	{
		int nResult;
		
		int nLeft = len;
		int nDx=0;
		if( m_socket == INVALID_SOCKET ) return FALSE;

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
		for( ; ; )
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		while( 1 )
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		{
			nResult = send(m_socket, (char*)buf+nDx, len-nDx, 0);
			if( nResult == SOCKET_ERROR )
			{
#ifdef PBG_LOG_PACKET_WINSOCKERROR
				DebugAngel_Write(PACKET_LOG_FILE, "ssend nResult[%d]\r\n",nResult);
#endif //PBG_LOG_PACKET_WINSOCKERROR

				if( WSAGetLastError() != WSAEWOULDBLOCK )
				{
#ifdef CONSOLE_DEBUG
					g_ConsoleDebug->Write(MCD_ERROR, "[Send Packet Error] WSAGetLastError() != WSAEWOULDBLOCK");
					g_ConsoleDebug->Write(MCD_ERROR, "[Send Packet Error] WSAGetLastError = %d", WSAGetLastError());
#endif // CONSOLE_DEBUG
					g_ErrorReport.Write("[Send Packet Error] WSAGetLastError() != WSAEWOULDBLOCK\r\n");
					Close();
					return FALSE;
				}
				else 
				{
					if( (m_nSendBufLen+len) > MAX_SENDBUF )
					{
#ifdef CONSOLE_DEBUG
						g_ConsoleDebug->Write(MCD_ERROR, "[Send Packet Error] SendBuffer Overflow");
						g_ConsoleDebug->Write(MCD_ERROR, "[Send Packet Error] WSAGetLastError = %d", WSAGetLastError());
#endif // CONSOLE_DEBUG
						g_ErrorReport.Write("[Send Packet Error] SendBuffer Overflow\r\n");
						Close();
						return FALSE;
					}
					memcpy( m_SendBuf+m_nSendBufLen, buf, nLeft);
					m_nSendBufLen += nLeft;

					//LogPrint("send() WSAEWOULDBLOCK : %d", WSAGetLastError());
					return FALSE;
				}
			}
			else 
			{
				if( nResult == 0 ) 
				{
					//LogPrint("send()  result is zero", WSAGetLastError());
					break;
				}
				if( m_LogPrint )
				{
					LogHexPrintS((BYTE*)buf, nResult);
				}
			}
			nDx += nResult;
			nLeft -= nResult;
			if( nLeft <= 0 ) break;
		}	
		return TRUE;
	}
	int  FDWriteSend();
	
	int  nRecv();
	
	BYTE * GetReadMsg();


	void LogPrint( char *szlog, ...);
	void LogHexPrint(BYTE *buf, int size);
	void LogHexPrintS( BYTE *buf, int size);
	void LogPrintOn();
	void LogPrintOff();

};


#endif
