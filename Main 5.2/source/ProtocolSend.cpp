#include "stdafx.h"
#ifdef NEW_PROTOCOL_SYSTEM
#include "ProtocolSend.h"
#include "GuildCache.h"
#include "UIMng.h"
#include "WSclient.h"
#include "wsclientinline.h"

CProtocolSend gProtocolSend;

CProtocolSend::CProtocolSend()
{
	this->m_ClientAcceptThread = 0;
}

bool CProtocolSend::ConnectServer(char* IP,uint16_t Port)
{
	this->SocketConnect = new CustomClient;

	if (this->SocketConnect->Connect(IP,Port))
	{
		//if(this->m_ClientAcceptThread == 0)
			//this->m_ClientAcceptThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)this->RecvMessage, this, 0, 0);
		return true;
	}
	return false;
}

void CProtocolSend::DisconnectServer()
{
	if(this->SocketConnect)
	{
		g_bGameServerConnected = FALSE;
		this->SocketConnect->Disconnect();
		g_ConsoleDebug->Write(MCD_NORMAL, " > ProtocolSend Disconnet");
	}
}

bool CProtocolSend::CheckConnected()
{
	if(this->SocketConnect && this->SocketConnect->IsConnected())
		return true;
	return false;
}

void CProtocolSend::SendPingTest()
{
	this->SocketConnect->PingServer();
}

void CProtocolSend::RecvMessage()
{
	//while (true)
	//{
		if (gProtocolSend.SocketConnect && gProtocolSend.SocketConnect->IsConnected())
		{
			while(true)
			{
				if (!gProtocolSend.SocketConnect->Incoming().empty())
				{
					auto msg = gProtocolSend.SocketConnect->Incoming().pop_front().msg;

					switch(msg.header.id)
					{
						case ProtocolHead::SERVER_CONNECT:
						{
							PMSG_CONNECT_CLIENT_RECV pMsg;
							msg >> pMsg;
							gProtocolSend.RecvJoinServerNew(&pMsg);
							break;
						}
						case ProtocolHead::SERVER_DISCONNECT:
						{
							// Server has responded to a ping request	
							DWORD clientID;
							msg >> clientID;
							std::cout << "Hello from [" << clientID << "]\n";
							break;
						}
						case ProtocolHead::BOTH_CONNECT_LOGIN:
						{
							//PMSG_SIMPLE_RESULT_RECV pMsg; //exemplo
							//msg >> pMsg;
							gProtocolSend.RecvLoginNew((PMSG_SIMPLE_RESULT_RECV*)msg.body.data());
							break;
						}
						case ProtocolHead::BOTH_CONNECT_CHARACTER:
							ReceiveCharacterList((BYTE*)msg.body.data());
							break;
						case ProtocolHead::BOTH_POSITION:
							ReceiveMovePosition((BYTE*)msg.body.data());
							break;
						case ProtocolHead::BOTH_MOVE:
							ReceiveMoveCharacter((BYTE*)msg.body.data());
							break;

						case(ProtocolHead::BOTH_MESSAGE):
						{
							uint8_t recv[8024];

							std::memcpy(recv,&msg.body[0],msg.header.size);

							BYTE header,head;
							int size = 0;

							if(recv[0] == 0xC1 || recv[0] == 0xC3)
							{
								header = recv[0];
								size = recv[1];
								head = recv[2];
							}
							else if(recv[0] == 0xC2 || recv[0] == 0xC4)
							{
								header = recv[0];
								size = MAKEWORD(recv[2],recv[1]);
								head = recv[3];
							}

							//BYTE subhead = ((recv[0] == 0xC1 || recv[0] == 0xC3) ? recv[3] : recv[4]);

							//if((head != 0xF3 || subhead != 0xF1) && head != 0x0E && head != 0x18 && head != 0xD4 && subhead != 0xE1)
							//{
							//	if(size < 129)
							//	{
							//		char String[200];
							//		char hex_chars[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

							//		for(int i = 0; i < size; ++i)
							//		{
							//			sprintf(&String[2 * i],"%02X",recv[i]);
							//		}

							//		g_ConsoleDebug->Write(MCD_NORMAL,"PACKET: %s",String);
							//	}
							//}

							//g_ConsoleDebug->Write(MCD_NORMAL, "[BOTH MESSAGE] %02x %02x %02x %02x %02x %02x",head,recv[0],recv[1],recv[2],recv[3],recv[4]);

							TranslateProtocol(head,recv,size,0);
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	//}
}

void CProtocolSend::SendCheckOnline()
{
	if (!g_bGameServerConnected)
	{
		return;
	}

	this->SendPingTest();

	g_ConsoleDebug->Write(MCD_SEND, "SendCheck");
}

void CProtocolSend::SendRequestLogInNew(char* account, char* password)
{
	PMSG_CONNECT_ACCOUNT_SEND pMsg = { 0 };

	//char Account[MAX_ID_SIZE+1] = { 0 };
	//char Password[MAX_PASSWORD_SIZE+1] = { 0 };

	LogIn = 1;

	CurrentProtocolState = REQUEST_LOG_IN;

	strncpy(pMsg.account, account, MAX_ID_SIZE);
	strncpy(pMsg.password, password, MAX_PASSWORD_SIZE);

	std::cout << "Send Login [" << pMsg.account << pMsg.password << "]\n";

	BuxConvert((BYTE*)pMsg.account, MAX_ID_SIZE);
	BuxConvert((BYTE*)pMsg.password, MAX_PASSWORD_SIZE);

	//memcpy(pMsg.account,&Account,MAX_ID_SIZE);
	//memcpy(pMsg.password,&Password,MAX_PASSWORD_SIZE);

	pMsg.TickCount = GetTickCount();

	for (int i = 0; i < SIZE_PROTOCOLVERSION; i++)
		pMsg.ClientVersion[i] = (BYTE)(Version[i] - (i + 1));
	for (int i = 0; i < SIZE_PROTOCOLSERIAL; i++)
		pMsg.ClientSerial[i] = Serial[i];

	g_pChatListBox->AddText("", GlobalText[472], SEASON3B::TYPE_SYSTEM_MESSAGE);
	g_pChatListBox->AddText("", GlobalText[473], SEASON3B::TYPE_SYSTEM_MESSAGE);

	SendPacket(ProtocolHead::BOTH_CONNECT_LOGIN,(uint8_t*)&pMsg,sizeof(pMsg));
}

void CProtocolSend::SendRequestCharactersListNew()
{
	PMSG_SIMPLE_RESULT_SEND pMsg = { 0 };

	pMsg.result = 1;

	SendPacket(ProtocolHead::BOTH_CONNECT_CHARACTER,(uint8_t*)&pMsg,sizeof(pMsg));
}

void CProtocolSend::SendPositionNew(uint8_t PosX,uint8_t PosY)
{
	PMSG_POSITION_SEND pMsg = { 0 };

	pMsg.x = PosX;

	pMsg.y = PosY;

	SendPacket(ProtocolHead::BOTH_POSITION,(uint8_t*)&pMsg,sizeof(pMsg));
}

void CProtocolSend::SendCharacterMoveNew(unsigned short Key, float Angle, unsigned char PathNum, unsigned char* PathX, unsigned char* PathY, unsigned char TargetX, unsigned char TargetY)
{
	if (PathNum < 1)
		return;

	PMSG_MOVE_SEND pMsg = { 0 };

	if (PathNum >= MAX_PATH_FIND)
	{
		PathNum = MAX_PATH_FIND - 1;
	}

	pMsg.x = PathX[0];
	pMsg.y = PathY[0];

	BYTE Path[8];
	ZeroMemory(Path, 8);
	BYTE Dir = 0;

	for (int i = 1; i < PathNum; i++)
	{
		Dir = 0;
		for (int j = 0; j < 8; j++)
		{
			if (DirTable[j * 2] == (PathX[i] - PathX[i - 1]) && DirTable[j * 2 + 1] == (PathY[i] - PathY[i - 1]))
			{
				Dir = j;
				break;
			}
		}

		if (i % 2 == 1)
		{
			Path[(i + 1) / 2] = Dir << 4;
		}
		else
		{
			Path[(i + 1) / 2] += Dir;
		}
	}
	if (PathNum == 1)
	{
		Path[0] = ((BYTE)((Angle + 22.5f) / 360.f * 8.f + 1.f) % 8) << 4;
	}
	else
	{
		for (int j = 0; j < 8; j++)
		{
			if (DirTable[j * 2] == (TargetX - PathX[PathNum - 1]) && DirTable[j * 2 + 1] == (TargetY - PathY[PathNum - 1]))
			{
				Dir = j;
				break;
			}
		}
		Path[0] = Dir << 4;
	}

	Path[0] += (BYTE)(PathNum - 1);

	memcpy(pMsg.path, Path, sizeof(pMsg.path));

	SendPacket(ProtocolHead::BOTH_MOVE,(uint8_t*)&pMsg,sizeof(pMsg));
}

BOOL Util_CheckOption( char *lpszCommandLine, unsigned char cOption, char *lpszString);

void CProtocolSend::RecvJoinServerNew(PMSG_CONNECT_CLIENT_RECV* pMsg )
{
	HeroKey = ((int)(pMsg->index[0])<<8) + pMsg->index[1];

	std::cout << "Login" << HeroKey << pMsg->ClientVersion[0] << pMsg->ClientVersion[1] << pMsg->ClientVersion[2] << pMsg->ClientVersion[3] << pMsg->ClientVersion[4] << "\n";

	g_ConsoleDebug->Write(MCD_NORMAL, "[Connect to Server] %d = %02x %02x %02x %02x %02x %02x %02x",HeroKey,pMsg->index[0],pMsg->index[1], pMsg->ClientVersion[0], pMsg->ClientVersion[1],pMsg->ClientVersion[2],pMsg->ClientVersion[3],pMsg->ClientVersion[4],pMsg->ClientVersion[5]);

    if ( LogIn!=0 )
    {
        g_csMapServer.SendChangeMapServer();
    }
    else
    {
		CUIMng& rUIMng = CUIMng::Instance();
		
        switch(pMsg->result)
        {
			case 0x01:
				rUIMng.ShowWin(&rUIMng.m_LoginWin);
				CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
				break;
			
			default:
				g_ErrorReport.Write( "Connectting error. ");
				g_ErrorReport.WriteCurrentTime();
				rUIMng.PopUpMsgWin(MESSAGE_SERVER_LOST);
				break;
        }

        for(int i=0;i<SIZE_PROTOCOLVERSION;i++)
        {
            if(Version[i]-(i+1) != pMsg->ClientVersion[i])
            {
				rUIMng.HideWin(&rUIMng.m_LoginWin);
				rUIMng.PopUpMsgWin(MESSAGE_VERSION);
                g_ErrorReport.Write( "Version dismatch - Join server.\r\n");
			}
		}
	}

	std::cout << "Login2\n";
	
	g_GuildCache.Reset(); 
	
#if defined _DEBUG || defined FOR_WORK
	if ( pMsg->result == 0x01)
	{
		char lpszTemp[256];
		if ( Util_CheckOption( GetCommandLine(), 'i', lpszTemp))
		{
			g_ErrorReport.Write( "> Try to Login \"%s\"\r\n", m_ID);
			//SendRequestLogIn( m_ID, lpszTemp);
			gProtocolSend.SendRequestLogInNew(m_ID, lpszTemp);
		}
	}
#endif
}

void CProtocolSend::RecvLoginNew(PMSG_SIMPLE_RESULT_RECV* pMsg)
{
	switch (pMsg->result)
	{
		case 0x20:
			CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
			LogIn = 2;
			CheckHack();
			break;
		case 0x00:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PASSWORD);
			break;
		case 0x01:
			CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
			LogIn = 2;
			CheckHack();
			break;
		case 0x02:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID);
			break;
		case 0x03:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID_CONNECTED);
			break;
		case 0x04:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_SERVER_BUSY);
			break;
		case 0x05:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID_BLOCK);
			break;
		case 0x06:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_VERSION);
			g_ErrorReport.Write("Version dismatch. - Login\r\n");
			break;
		case 0x07:
		default:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CONNECT);
			break;
		case 0x08:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ERROR);
			break;
		case 0x09:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO);
			break;
		case 0x0A:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_USER_TIME1);
			break;
		case 0x0B:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_USER_TIME2);
			break;
		case 0x0C:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PC_TIME1);
			break;
		case 0x0D:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PC_TIME2);
			break;
		case 0x11:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ONLY_OVER_15);
			break;
		case 0x40:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL);
			break;
		case 0xC0:
		case 0xD0:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_POINT_DATE);
			break;
		case 0xC1:
		case 0xD1:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_POINT_HOUR);
			break;
		case 0xC2:
		case 0xD2:
			CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_INVALID_IP);
			break;
		}
}
#endif