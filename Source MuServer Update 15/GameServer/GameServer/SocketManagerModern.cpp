#include "stdafx.h"
#if(NEW_PROTOCOL_SYSTEM==1)
#include "SocketManagerModern.h"
#include "Attack.h"
#include "ConsoleDebug.h"
#include "PacketManager.h"
#include "Protocol.h"
#include "SkillManager.h"
#include "User.h"
#include "Util.h"

CSocketManagerModern gSocketManagerModern;

bool CSocketManagerModern::StartServer(uint16_t nPort)
{
	if (!this->CheckPortUse(nPort)) //mudar para nPort
	{
		g_ConsoleDebug; //comment to hide the console

		this->connection = new CSocketConnection(nPort);

		this->connection->Start();

		this->RunServerThread();
		return true;
	}
	std::cout << "Port already in use!\n";
	return false;
}

bool CSocketManagerModern::CheckPortUse(unsigned short port)
{
	using namespace boost::asio;
	using ip::tcp;

	io_service svc;
	tcp::acceptor a(svc);

	boost::system::error_code ec;
	a.open(tcp::v4(), ec) || a.bind({ tcp::v4(), port }, ec);

	return ec == error::address_in_use;
}

void CSocketManagerModern::ListenServer()
{
	while (1)
	{
		gSocketManagerModern.connection->Update(-1, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void CSocketManagerModern::DisconenctClient(uint16_t aIndex)
{
	this->connection->DisconnectClient(aIndex);
}

void CSocketManagerModern::PacketSend(uint16_t aIndex, ProtocolHead head,uint8_t* message, uint16_t size) 
{
	if (size > MAX_MAIN_PACKET_SIZE)
	{
		LogAdd(LOG_RED,"[SocketManager] ERROR: Max msg size %d (Msg size:%d)",MAX_MAIN_PACKET_SIZE,size);
		return;
	}

	PacketMsg lpMsg; 

	lpMsg.header.id = head;

	lpMsg.body.resize(lpMsg.body.size() + size);

	std::memcpy(lpMsg.body.data(), message, size);

	lpMsg.header.size = lpMsg.size();

	this->connection->ProtocolSend(aIndex, lpMsg);
}

void CSocketManagerModern::DataReceived(uint16_t aIndex,olc::net::message<ProtocolHead> msg)
{
	switch (msg.header.id)
	{
		case ProtocolHead::CLIENT_LIVE_CLIENT:
		{
			PMSG_TESTE_RECV lpMsg;
			msg >> lpMsg;

			std::cout << "[" << aIndex << "]: [" << lpMsg.TickCount << "][" << lpMsg.PhysiSpeed << "][" << lpMsg.MagicSpeed << "]:Data Recv\n";

			auto lpObj = &gObj[aIndex];

			lpObj->ConnectTickCount = GetTickCount();

			//std::cout << "DataSend [" << GetTickCount() << "]\n";

			PMSG_TESTE_SEND TesteSend;

			TesteSend.TickCount = GetTickCount();

			this->PacketSend(aIndex,ProtocolHead::SERVER_DISCONNECT,(uint8_t*)&TesteSend,sizeof(TesteSend));
			break;
		}
		case ProtocolHead::BOTH_CONNECT_LOGIN: 
			CGConnectAccountRecv((PMSG_CONNECT_ACCOUNT_RECV*)msg.body.data(), aIndex);
			break;
		case ProtocolHead::BOTH_CONNECT_CHARACTER: 
			CGCharacterListRecv(aIndex);
			break;
		case ProtocolHead::BOTH_POSITION: 
			CGPositionRecv((PMSG_POSITION_RECV*)msg.body.data(),aIndex);
			break;
		case ProtocolHead::BOTH_MOVE: 
			CGMoveRecv((PMSG_MOVE_RECV*)msg.body.data(),aIndex);
			break;
		case ProtocolHead::BOTH_ATTACK1: 
			gAttack.CGAttackRecv((PMSG_ATTACK_RECV*)msg.body.data(),aIndex);
			break;
		case ProtocolHead::BOTH_ATTACK2: 
			gSkillManager.CGMultiSkillAttackRecv((PMSG_MULTI_SKILL_ATTACK_RECV*)msg.body.data(),aIndex,0);
			break;
		case ProtocolHead::BOTH_MESSAGE:
			{
				static uint8_t recv[8024];

				for (uint16_t start = 0; start < msg.header.size; start++) {
					std::memcpy(&recv[start], &msg.body[start], 1);
				}

				//uint8_t head = 0xFF;

				//if (recv[0] == 0xC1 || recv[0] == 0xC3) {
				//	head = recv[2];
				//}
				//else if (recv[0] == 0xC2 || recv[0] == 0xC4) {
				//	head = recv[3];
				//}

				int count=0,size=0,DecSize=0,DecEncrypt=0,DecSerial=0;
				static BYTE DecBuff[MAX_MAIN_PACKET_SIZE];
				BYTE header,head;

				if(recv[count] == 0xC1 || recv[count] == 0xC3)
				{
					header = recv[count];
					size = recv[count+1];
					head = recv[count+2];
				}
				else if(recv[count] == 0xC2 || recv[count] == 0xC4)
				{
					header = recv[count];
					size = MAKEWORD(recv[count+2],recv[count+1]);
					head = recv[count+3];
				}

				if(gPacketManager.AddData(&recv[count],size) == 0 || gPacketManager.ExtractPacket(DecBuff) == 0)
				{
					return;
				}

				ProtocolCore(head,DecBuff,size,aIndex, 0, 0);
				
			}
			break;

		
	}
}

//bool CSocketManagerModern::ExtractPacket(BYTE* lpBuff) // OK
//{
//	int size,end;
//
//	switch(lpBuff[0])
//	{
//		case 0xC1:
//			size = lpBuff[1];
//			end = 2;
//			break;
//		case 0xC2:
//			size = MAKEWORD(lpBuff[2],lpBuff[1]);
//			end = 3;
//			break;
//		default:
//			return 0;
//	}
//
//	this->XorData((size-1),end);
//
//	//memcpy(lpBuff,this->m_buff,size);
//
//	return 1;
//}

#endif