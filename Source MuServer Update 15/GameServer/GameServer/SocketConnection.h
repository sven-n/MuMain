#pragma once
#if(NEW_PROTOCOL_SYSTEM==1)
#include "olcPGEX_Network.h"
#include "SocketManagerEnum.h"

class CSocketConnection : public olc::net::server_interface<ProtocolHead>
{
public:
	CSocketConnection(uint16_t nPort) : olc::net::server_interface<ProtocolHead>(nPort)
	{}

//protected:
	//CSocketConnection() = default;

protected:
	bool OnClientConnect(std::shared_ptr<olc::net::connection<ProtocolHead>> client) override;
	void OnClientValidated(std::shared_ptr<olc::net::connection<ProtocolHead>> client) override;
	void OnClientDisconnect(std::shared_ptr<olc::net::connection<ProtocolHead>> client) override;
	void OnMessage(std::shared_ptr<olc::net::connection<ProtocolHead>> client, olc::net::message<ProtocolHead>& msg) override;

public:
	void ProtocolSend(uint16_t aIndex,olc::net::message<ProtocolHead> msg);
	void DisconnectClient(uint16_t aIndex);
}; 
#endif
