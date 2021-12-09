#include "stdafx.h"

#if(NEW_PROTOCOL_SYSTEM==1)

#include "SocketConnection.h"
#include "SocketManagerModern.h"
#include "Protocol.h"
#include "User.h"
#include "Util.h"

bool CSocketConnection::OnClientConnect(std::shared_ptr<olc::net::connection<ProtocolHead>> client)
{
	return true;
}

void CSocketConnection::OnClientValidated(std::shared_ptr<olc::net::connection<ProtocolHead>> client)
{
	uint16_t aIndex = client->GetID();

	if(gObjAddNew(client->GetIP(),aIndex) == -1)
	{
		LogAdd(LOG_RED, "[Obj][%d] Error Connect (%s)", client->GetID(), client->GetIP());
		return;
	}

	GCConnectClientSend(aIndex,1);
}

void CSocketConnection::OnClientDisconnect(std::shared_ptr<olc::net::connection<ProtocolHead>> client)
{
	uint32_t Index = client->GetID();

	if(OBJECT_USER_RANGE(Index) != 0)
	{
		LogAddConnect(LOG_ALERT,"[Obj][%d] DelClientNew (%s)",Index,gObj[Index].IpAddr);

		gObj[Index].Socket = INVALID_SOCKET;

		gObjDel(Index);
	}
}

void CSocketConnection::OnMessage(std::shared_ptr<olc::net::connection<ProtocolHead>> client, olc::net::message<ProtocolHead>& msg)
{
	uint16_t aIndex = client->GetID();

	//if (gObjIsConnectedGP(aIndex) == 0) //ativar futuramente 
	//{
	//	return;
	//}

	if (OBJECT_USER_RANGE(aIndex) == 0)
	{
		return;
	}

	gSocketManagerModern.DataReceived(aIndex,msg);
}

void CSocketConnection::ProtocolSend(uint16_t aIndex, olc::net::message<ProtocolHead> msg)
{
	MessageClientByIndex(msg, aIndex);
}

void CSocketConnection::DisconnectClient(uint16_t aIndex)
{
	DisconnectClientByIndex(aIndex);

	if(OBJECT_USER_RANGE(aIndex) != 0)
	{
		gObj[aIndex].Socket = INVALID_SOCKET;

		gObjDel(aIndex);
	}
}

#endif