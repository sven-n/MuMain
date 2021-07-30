#pragma once
#include <cstdint>
#include "ProtocolAsio.h"
#include "./Utilities/Log/muConsoleDebug.h"

enum class ProtocolHead : uint16_t
{
	CLIENT_ACCEPT,
	CLIENT_LIVE_CLIENT,

	SERVER_CLIENT_CONNECT,
	SERVER_CLIENT_DISCONNECT,

	BOTH_PROTOCOL,
};

struct sPlayerDescription
{
	uint32_t nUniqueID = 0;
	uint32_t nAvatarID = 0;

	uint32_t nHealth = 100;
	uint32_t nAmmo = 20;
	uint32_t nKills = 0;
	uint32_t nDeaths = 0;

	float fRadius = 0.5f;
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
};

class CProtocolSend
{

public:
	bool ConnectServer();
	void SendPing();
	static void RecvMessage();
private:
	CustomClient SocketConnect;
	
}; 

extern CProtocolSend gProtocolSend;
