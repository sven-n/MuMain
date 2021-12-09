#pragma once
#include "SocketConnection.h"

#if(NEW_PROTOCOL_SYSTEM==1)
#define MAX_MAIN_PACKET_SIZE 8192

struct PMSG_TESTE_RECV
{
	DWORD TickCount;
	WORD PhysiSpeed;
	WORD MagicSpeed;
};

struct PMSG_TESTE_SEND
{
	DWORD TickCount;
};

class CSocketManagerModern
{
public:
	~CSocketManagerModern() {
		mThread.detach();
		//mThread();
		//connection->Stop();
		// Tidy up the context thread
		//if (mThread.joinable()) mThread.join();
	}

private:
		void DataSend(uint16_t aIndex, olc::net::message<ProtocolHead> msg) {
		this->connection->ProtocolSend(aIndex, msg);
	}

public:
	bool StartServer(uint16_t nPort);
	bool CheckPortUse(unsigned short port);
	void static ListenServer();
	void DisconenctClient(uint16_t aIndex);



	//template<typename StructType>
	void PacketSend(uint16_t aIndex, ProtocolHead head, uint8_t* message, uint16_t size);
	void DataReceived(uint16_t aIndex,olc::net::message<ProtocolHead> msg);

private:
	bool RunServerThread()
	{
		try
		{
			mThread = std::thread([this] { ListenServer(); });
			//mThread.join();
		}
		catch (...)
		{
			return false;
		}
		return true;
	}

private:
	CSocketConnection* connection;
	std::thread mThread;
	typedef typename olc::net::message<ProtocolHead> PacketMsg;
};

extern CSocketManagerModern gSocketManagerModern;

//namespace NewMsg = ;
#endif

