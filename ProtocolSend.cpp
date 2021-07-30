#include "stdafx.h"
#include "ProtocolSend.h"


CProtocolSend gProtocolSend;

int teste1 = 0;

bool CProtocolSend::ConnectServer()
{
	
	if (this->SocketConnect.Connect("127.0.0.1", 55905))
	{
		teste1 = 1;
		//RecvMessage();
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)this->RecvMessage, 0, 0, 0);
		return true;
	}

	//this->SendPing();
	//this->SendPing();
	 
	//while (true)
	//{ 
		//c.PingServer();
		//Sleep(500);
	//}

	
	
}

void CProtocolSend::SendPing()
{
	this->SocketConnect.PingServer();
}

void CProtocolSend::RecvMessage()
{
	std::cout << "Hello from [" << 1 << "]\n";
	while (true)
	{
		//if (teste1 != 0)
		//{

			if (gProtocolSend.SocketConnect.IsConnected())
			{
				if (!gProtocolSend.SocketConnect.Incoming().empty())
				{
					auto msg = gProtocolSend.SocketConnect.Incoming().pop_front().msg;

					switch (msg.header.id)
					{
					case ProtocolHead::SERVER_CLIENT_DISCONNECT:
					{
						// Server has responded to a ping request	
						uint32_t clientID;
						msg >> clientID;
						std::cout << "Hello from [" << clientID << "]\n";
					}
					//break;
					}
				}
			//}
		}
		Sleep(20);
	}
}