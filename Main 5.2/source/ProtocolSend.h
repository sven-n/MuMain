#pragma once
#ifdef NEW_PROTOCOL_SYSTEM
#include <cstdint>
#include "ProtocolAsio.h"
#include "./Utilities/Log/muConsoleDebug.h"

enum class ProtocolHead : uint16_t
{
	CLIENT_ACCEPT,			//Enviado pelo cliente
	CLIENT_LIVE_CLIENT,

	SERVER_CONNECT,			//Enviado pelo servidor
	SERVER_DISCONNECT,

	BOTH_CONNECT_LOGIN,			//Enviado pelo cliente e servidor
	BOTH_CONNECT_CHARACTER,
	BOTH_POSITION,
	BOTH_MOVE,
	BOTH_ATTACK1,
	BOTH_ATTACK2,
	BOTH_ATTACK3,

	BOTH_MESSAGE,
};

struct PMSG_CONNECT_ACCOUNT_SEND
{
	#pragma pack(1)
	char account[10];
	char password[20];
	DWORD TickCount;
	BYTE ClientVersion[5];
	BYTE ClientSerial[16];
	#pragma pack()
};

struct PMSG_SIMPLE_RESULT_SEND
{
	BYTE result;
};

struct PMSG_POSITION_SEND
{
	BYTE x;
	BYTE y;
};

struct PMSG_MOVE_SEND
{
	BYTE x;
	BYTE y;
	BYTE path[8];
};

struct PMSG_CONNECT_CLIENT_RECV
{
	BYTE result;
	BYTE index[2];
	BYTE ClientVersion[5];
};

struct PMSG_SIMPLE_RESULT_RECV
{
	BYTE result;
};

class CustomClient : public olc::net::client_interface<ProtocolHead>
{
public:
	bool bWaitingForConnection = true;

	void PingServer()
	{
		olc::net::message<ProtocolHead> msg;
		msg.header.id = ProtocolHead::CLIENT_LIVE_CLIENT;

		DWORD dwTick = GetTickCount();
		msg << dwTick;
		WORD teste1 = 1;
		WORD teste2 = 2;
		msg << teste1;
		msg << teste2;

		std::cout << "[" << dwTick << "]" << "[" << teste1 << "]" << "[" << teste2 << "]Send data.\n";

		//// Caution with this...
		//std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		//msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		olc::net::message<ProtocolHead> msg;
		msg.header.id = ProtocolHead::CLIENT_ACCEPT;
		Send(msg);
	}

	void DataSend(ProtocolHead head,uint8_t* message, uint16_t size)
	{
		olc::net::message<ProtocolHead> lpMsg; 

		lpMsg.header.id = head;

		lpMsg.body.resize(lpMsg.body.size() + size);

		std::memcpy(lpMsg.body.data(), message, size);

		lpMsg.header.size = lpMsg.size();

		Send(lpMsg);
	}

	void DataSend(uint8_t* lpMsg, uint16_t size) 
	{
		if (IsConnected())
		{
			olc::net::message<ProtocolHead> msg;

			msg.header.id = ProtocolHead::BOTH_MESSAGE;

			msg.body.resize(msg.body.size() + size);

			std::memcpy(msg.body.data(), lpMsg, size);

			msg.header.size = msg.size();

			Send(msg);
		}
	}
};

class CProtocolSend
{
public:
	CProtocolSend();
	bool ConnectServer(char* IP,uint16_t Port);
	void DisconnectServer();
	bool CheckConnected();
	void SendPingTest();
	void RecvMessage();
	void SendPacket(ProtocolHead head,uint8_t* message, uint16_t size) { if(SocketConnect) SocketConnect->DataSend(head,message,size); }
	void SendPacketClassic(uint8_t* message,uint16_t size) { if(SocketConnect) SocketConnect->DataSend(message,size);}
	void SendCheckOnline();
	void SendRequestLogInNew(char* account, char* password);
	void SendRequestCharactersListNew();
	void SendPositionNew(uint8_t PosX, uint8_t PosY);
	void SendCharacterMoveNew(unsigned short Key, float Angle, unsigned char PathNum, unsigned char* PathX, unsigned char* PathY, unsigned char TargetX, unsigned char TargetY);
	void RecvJoinServerNew(PMSG_CONNECT_CLIENT_RECV* pMsg);
	void RecvLoginNew(PMSG_SIMPLE_RESULT_RECV* pMsg);
private:
	CustomClient* SocketConnect;
	HANDLE m_ClientAcceptThread;
	
}; 

extern CProtocolSend gProtocolSend;
#endif