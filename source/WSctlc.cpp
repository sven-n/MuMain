///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "wsctlc.h"
#include "wsctlc_addon.h"

typedef struct
{
	BYTE c;
	BYTE size;
	BYTE headcode1;
} WSCTLC_PBMSG_HEAD, *WSCTLC_LPPBMSG_HEAD;

typedef struct
{
	BYTE c;
	BYTE sizeH;
	BYTE sizeL;
	BYTE headcode1;
} WSCTLC_PWMSG_HEAD, *WSCTLC_LPPWMSG_HEAD;


CWsctlc::CWsctlc()
{ 
	m_hWnd = NULL;
	m_bGame = FALSE;
	m_iMaxSockets = 0;
	m_SendBuf[0] = '\0';
	m_nSendBufLen = 0;
	m_RecvBuf[0] = '\0';
	m_nRecvBufLen = 0;

	m_pPacketQueue = new CPacketQueue;
	m_LogPrint = 0;
	m_logfp = NULL;
#ifdef PBG_LOG_PACKET_WINSOCKERROR
	remove(PACKET_LOG_FILE);
#endif //PBG_LOG_PACKET_WINSOCKERROR
	return;
}

CWsctlc::~CWsctlc()
{
	delete m_pPacketQueue;
	LogPrintOff();
}

BOOL CWsctlc::Startup()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2,2);

	err = WSAStartup( wVersionRequested, &wsaData);
	if( err != 0)
	{
		g_ErrorReport.Write( "Winsock DLL Initialize error.\r\n");
		MessageBox(NULL,"WINSOCK DLL","Error",MB_OK);
		return FALSE;
	}

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
		WSACleanup( );
		g_ErrorReport.Write( "Winsock version low.\r\n");
		MessageBox(NULL,"WINSOCK","Error",MB_OK);
		return FALSE;
	}
	m_socket = NULL;
	m_iMaxSockets = wsaData.iMaxSockets;	
	LogPrintOn();
	return TRUE;
}
 

BOOL CWsctlc::ShutdownConnection(SOCKET sd)
{
    // Disallow any further data sends.  This will tell the other side
    // that we want to go away now.  If we skip this step, we don't
    // shut the connection down nicely.
    if (shutdown(sd, SD_SEND) == SOCKET_ERROR) {
        return false;
    }

    // Receive any extra data still sitting on the socket.  After all
    // data is received, this call will block until the remote host
    // acknowledges the TCP control packet sent by the shutdown above.
    // Then we'll get a 0 back from recv, signalling that the remote
    // host has closed its side of the connection.
    char acReadBuffer[1024];
    while (1) {
        int nNewBytes = recv(sd, acReadBuffer, 1024, 0);
        if (nNewBytes == SOCKET_ERROR) {
            return false;
        }
        else if (nNewBytes != 0) {
          //  cerr << endl << "FYI, received " << nNewBytes <<
            //        " unexpected bytes during shutdown." << endl;
        }
        else {
            // Okay, we're done!
            break;
        }
    }

    // Close the socket.
    if (closesocket(sd) == SOCKET_ERROR) {
        return false;
    }
	
    return true;
}

void CWsctlc::Cleanup()
{
	WSACleanup();
}

extern BOOL g_bGameServerConnected;
extern BYTE g_byNextFuncCrcCheck;

int CWsctlc::Create(HWND hWnd, BOOL bGame)
{	
	m_socket = socket( PF_INET, SOCK_STREAM, 0);
	m_bGame = bGame;
	if ( m_bGame)
	{
		g_bGameServerConnected = FALSE;
	}

	if( m_socket == INVALID_SOCKET ) 
	{
		char lpszMessage[128];
		wsprintf(lpszMessage, "WSAGetLastError %d", WSAGetLastError());
		g_ErrorReport.Write( lpszMessage);
		g_ErrorReport.Write( "\r\n");
		MessageBox(NULL,lpszMessage, "Error", MB_OK);
		return FALSE;
	}
	m_hWnd = hWnd;	
	return TRUE;
}

BOOL CWsctlc::Close()
{
	if ( m_bGame)
	{
		g_bGameServerConnected = FALSE;
	}
	
	LINGER linger;
	linger.l_onoff	= 1;
	linger.l_linger	= 0;

	int iRetVal	= setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	if (iRetVal == SOCKET_ERROR)
	{
		WSAGetLastError();
	}
	ZeroMemory(m_RecvBuf, sizeof(MAX_RECVBUF));
	ZeroMemory(m_SendBuf, sizeof(MAX_SENDBUF));

	m_nSendBufLen = 0;
	m_nRecvBufLen = 0;

	// Clear Packet Queue
	while(!m_pPacketQueue->IsEmpty()) 
	{
		m_pPacketQueue->PopPacket();
	}
	g_ErrorReport.Write("[Socket Closed][Clear PacketQueue]\r\n");

	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
	return TRUE;
}

BOOL CWsctlc::Close(SOCKET & socket)
{
	if ( m_bGame)
	{
		g_bGameServerConnected = FALSE;
	}

	closesocket(socket);
	socket = INVALID_SOCKET;
	return TRUE;
}

SOCKET CWsctlc::GetSocket()
{
	return m_socket;
}

int CWsctlc::Connect(char *ip_addr, unsigned short port, DWORD WinMsgNum)
{
	sockaddr_in		addr;
	int nResult;
	struct hostent    *host = NULL;

	if( m_hWnd == NULL ) {
		MessageBox(NULL, "Connect Error", "Error", MB_OK);
		return FALSE;
	}
    addr.sin_family			= PF_INET;
    addr.sin_port			= htons( port );
    addr.sin_addr.s_addr	= inet_addr(ip_addr); 

	if( addr.sin_addr.s_addr == INADDR_NONE )
	{
		host = gethostbyname(ip_addr);
		if( host == NULL )
		{
			return 2;
		}
		CopyMemory(&addr.sin_addr, host->h_addr_list[0], host->h_length);
	}
   
	if ( addr.sin_addr.S_un.S_un_b.s_b1 == 127 && addr.sin_addr.S_un.S_un_b.s_b2 == 0 &&
		addr.sin_addr.S_un.S_un_b.s_b3 == 0 && addr.sin_addr.S_un.S_un_b.s_b4 == 1)
	{	// local host
		return ( FALSE);
	}

	nResult = connect( m_socket, (LPSOCKADDR)&addr, sizeof(addr) );
	if( nResult == SOCKET_ERROR) 
	{
#ifdef _DEBUG		
		LogPrint("Connect error (%d)", WSAGetLastError());
#endif // _DEBUG
		if(WSAGetLastError() != WSAEWOULDBLOCK) 
		{
			closesocket(m_socket);
			return FALSE;
		}
    }

    nResult = WSAAsyncSelect( m_socket, m_hWnd, WinMsgNum, FD_READ | FD_WRITE | FD_CLOSE);
    if( nResult == SOCKET_ERROR) 
	{
		closesocket(m_socket);		
		//cLogProc.Add("Client WSAAsyncSelect error %d", WSAGetLastError());
		return FALSE;
    }
	return 1;
}

int CWsctlc::sSend(SOCKET socket, char *buf, int len)
{	
	int nResult;
	
	
	int nLeft = len;
	int nDx=0;
	
	while( 1 ) 
	{
		nResult = send(socket, (char*)buf+nDx, len-nDx, 0);
		if( nResult == SOCKET_ERROR )
		{
			if( WSAGetLastError() != WSAEWOULDBLOCK )
			{
				g_ConsoleDebug->Write(MCD_ERROR, "[Send Packet Error] WSAGetLastError() != WSAEWOULDBLOCK");
				g_ErrorReport.Write("[Send Packet Error] WSAGetLastError() != WSAEWOULDBLOCK\r\n");
				Close();
				return FALSE;
			}
			else 
			{
				if( (m_nSendBufLen+len) > MAX_SENDBUF )
				{

					g_ConsoleDebug->Write(MCD_ERROR, "Send Packet Error] SendBuffer Overflow");

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
		else {
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

int CWsctlc::FDWriteSend()
{
	int nResult;
	int nDx	  = 0;
	
	while( m_nSendBufLen > 0 ) 
	{
		nResult = send(m_socket, (char*)m_SendBuf+nDx, m_nSendBufLen-nDx, 0);

		if( nResult == SOCKET_ERROR )
		{
			if( WSAGetLastError() != WSAEWOULDBLOCK )
			{
				g_ErrorReport.Write("FDWriteSend Error 1.\r\n");
				Close();
				return FALSE;
			}
			else 
			{
				break;
			}
		}
		else {
			if( nResult <= 0 ) 
			{
				g_ErrorReport.Write("FDWriteSend Error 2.\r\n");
				Close();
				return FALSE;
			}
			if( m_LogPrint )
			{
				LogHexPrintS((BYTE*)m_SendBuf, nResult);
			}
		}
		nDx += nResult;
		m_nSendBufLen -= nResult;		
	}
	return TRUE;
}

void CWsctlc::LogPrintOn()
{
#ifdef _DEBUG
	m_LogPrint = 1;
	m_logfp = fopen("wsctlc.log", "wt");
#endif
}

void CWsctlc::LogPrintOff()
{
#ifndef _DEBUG
	return;
#endif
	if( m_LogPrint )
	{
		m_LogPrint = 0;
		if( m_logfp != NULL ) fclose(m_logfp);
	}
}

void CWsctlc::LogHexPrint( BYTE *buf, int size)
{	
#ifndef _DEBUG
	return;
#endif
	if( m_LogPrint == 0 ) return;
	
	if( buf[0] == 0xC1 )
	{
		if( buf[2] == 0x26 || buf[2] == 0x27 || buf[2] == 0x28 || buf[2] == 0x2a )
		{
			fprintf(m_logfp, "R 0x%02x %d\n", buf[2], buf[3]);
			return;
		}		
		fprintf(m_logfp, "R 0x%02x %d\n", buf[2], buf[3]);
	}
	else fprintf(m_logfp, "R 0x%02x %d\n", buf[3], buf[4]);
	
	//for( int n=0; n<size; n++) fprintf(m_logfp, "%02x ", buf[n]);
	//fprintf(m_logfp, "\n");
}

void CWsctlc::LogHexPrintS( BYTE *buf, int size)
{	
#ifndef _DEBUG
	return;
#endif
	if( m_LogPrint == 0 ) return;
	

	if( buf[0] == 0xC1 )
	{
		if( buf[2] == 0x26 || buf[2] == 0x27 || buf[2] == 0x28 || buf[2] == 0x2a )
		{
			fprintf(m_logfp, "S 0x%02x %d\n", buf[2], buf[3]);
			return;
		}		
		fprintf(m_logfp, "S 0x%02x %d\n", buf[2], buf[3]);
	} 
	// 임시로 막음 else fprintf(m_logfp, "S 0x%02x %d\n", buf[3], buf[4]);

	//fprintf(m_logfp, "S ");
	//for( int n=0; n<size; n++) fprintf(m_logfp, "%02x ", buf[n]);
	//fprintf(m_logfp, "\n");
}


void CWsctlc::LogPrint( char *szlog, ...)
{
#ifndef _DEBUG
	return;
#endif
	if( m_LogPrint == 0 ) return;

	char szBuffer[256]="";
	va_list		pArguments;
	
	va_start(pArguments, szlog);
    vsprintf(szBuffer, szlog, pArguments);
    va_end(pArguments);
	fprintf(m_logfp, "%s\n", szBuffer);	
}


int CWsctlc::nRecv()
{	
	int nResult;

	if(m_nRecvBufLen >= MAX_RECVBUF) 
	{
		g_ErrorReport.Write("Receive Packet Buffer Overflow.\r\n");
		return 1;
	}

	nResult = recv( m_socket, (char*)m_RecvBuf+m_nRecvBufLen, MAX_RECVBUF-m_nRecvBufLen, 0);

	if( nResult == 0 )
	{
		return 1;
	}
	if( nResult == SOCKET_ERROR )
	{
		if( WSAGetLastError() == WSAEWOULDBLOCK )
		{
			return 1;
		}
		else {
#ifdef _DEBUG
			LogPrint("recv() %d", WSAGetLastError());
#endif
		}
		return 1;
	}
	m_nRecvBufLen += nResult;

	if( m_nRecvBufLen < 3 ) return 3;

	int lOfs=0;
	int size=0;
	
	while(1)
	{
		if( m_RecvBuf[lOfs] == 0xC1 || m_RecvBuf[lOfs] == 0xC3 )
		{
			WSCTLC_LPPBMSG_HEAD lphead=(WSCTLC_LPPBMSG_HEAD)(m_RecvBuf+lOfs);
			size = (int)lphead->size;
		}
		else if( m_RecvBuf[lOfs] == 0xC2 || m_RecvBuf[lOfs] == 0xC4 )
		{
			WSCTLC_LPPWMSG_HEAD lphead=(WSCTLC_LPPWMSG_HEAD)(m_RecvBuf+lOfs);
			size = (((int)(lphead->sizeH))<<8)+lphead->sizeL;
		}
		else {
#ifdef _DEBUG
			LogPrint("Packet Error.(%s %d)", __FILE__, __LINE__);
#endif
			m_nRecvBufLen = 0;
			return FALSE;
		}


		if( size <= 0 ) 
		{
#ifdef _DEBUG
			LogPrint("size %d", size);
#endif
			return FALSE;
		}
		else if( size <= m_nRecvBufLen )
		{
			m_pPacketQueue->PushPacket(m_RecvBuf+lOfs, size);
			if( m_LogPrint )
			{
				LogHexPrint((BYTE*)(m_RecvBuf+lOfs), size);
			}
			lOfs += size;
			m_nRecvBufLen -= size;
			if( m_nRecvBufLen <= 0 )
			{
				break;
			}
		}			
		else 
		{
			if( lOfs > 0 ) 
			{
				if( m_nRecvBufLen < 1 ) 
				{
#ifdef _DEBUG
					LogPrint("m_nRecvBufLen..");
#endif
					break;
				}
				else
				{
					memcpy(m_RecvBuf, m_RecvBuf+lOfs, m_nRecvBufLen);
				}
			}
			break;
		}
	}

	m_pPacketQueue->ClearGarbage();
	
	return 0;
}

BYTE * CWsctlc::GetReadMsg()
{
	if(!m_pPacketQueue->IsEmpty())
	{
		CPacket* pPacket = m_pPacketQueue->FrontPacket();
		m_pPacketQueue->PopPacket();
		return pPacket->GetBuffer();
	}
	return NULL;
}


