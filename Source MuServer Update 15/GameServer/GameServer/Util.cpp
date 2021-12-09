#include "stdafx.h"
#include "Util.h"
#include "GameMain.h"
#include "HackCheck.h"
#include "ItemManager.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "ThemidaSDK.h"
#include "Viewport.h"
#include "SocketManagerModern.h"

std::mt19937 seed;
std::uniform_int_distribution<int> dist;
short RoadPathTable[MAX_ROAD_PATH_TABLE] = {-1,-1,0,-1,1,-1,1,0,1,1,0,1,-1,1,-1,0};

int SafeGetItem(int index) // OK
{
	return CHECK_ITEM(index);
}

flt GetRoundValue(float value) // OK
{
	float integral;

	if(modf(value,&integral) > 0.5f)
	{
		return ceil(value);
	}

	return floor(value);
}

BYTE GetNewOptionCount(BYTE NewOption) // OK
{
	BYTE count = 0;

	for(int n=0;n < MAX_EXC_OPTION;n++)
	{
		if((NewOption & (1 << n)) != 0)
		{
			count++;
		}
	}

	return count;
}

BYTE GetSocketOptionCount(BYTE SocketOption[5]) // OK
{
	BYTE count = 0;

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		if(SocketOption[n] != 0xFF)
		{
			count++;
		}
	}

	return count;
}

BYTE GetPathPacketDirPos(int px,int py) // OK
{
	if(px <= -1 && py <= -1)
	{
		return 0;
	}
	else if(px <= -1 && py == 0)
	{
		return 7;
	}
	else if(px <= -1 && py >= 1)
	{
		return 6;
	}
	else if(px == 0 && py <= -1)
	{
		return 1;
	}
	else if(px == 0 && py >= 1)
	{
		return 5;
	}
	else if(px >= 1 && py <= -1)
	{
		return 2;
	}
	else if(px >= 1 && py == 0)
	{
		return 3;
	}
	else if(px >= 1 && py >= 1)
	{
		return 4;
	}

	return 0;
}

void PacketArgumentDecrypt(char* out_buff,char* in_buff,int size) // OK
{
	BYTE XorTable[3] = {0xFC,0xCF,0xAB};

	for(int n=0;n < size;n++)
	{
		out_buff[n] = in_buff[n]^XorTable[n%3];
	}
}

void ErrorMessageBox(char* message,...) // OK
{
	VM_START

	char buff[256];

	memset(buff,0,sizeof(buff));

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	MessageBox(0,buff,"Error",MB_OK | MB_ICONERROR);

	VM_END

	ExitProcess(0);
}

void LogAdd(eLogColor color,char* text,...) // OK
{
	tm today;
	time_t ltime;
	time(&ltime);

	if(localtime_s(&today,&ltime) != 0)
	{
		return;
	}

	char time[32];

	if(asctime_s(time,sizeof(time),&today) != 0)
	{
		return;
	}

	char temp[1024];

	va_list arg;
	va_start(arg,text);
	vsprintf_s(temp,text,arg);
	va_end(arg);

	char log[1024];

	wsprintf(log,"%.8s %s",&time[11],temp);

	gServerDisplayer.LogAddText(color,log,strlen(log));
}

void LogAddConnect(eLogColor color,char* text,...) // OK
{
	tm today;
	time_t ltime;
	time(&ltime);

	if(localtime_s(&today,&ltime) != 0)
	{
		return;
	}

	char time[32];

	if(asctime_s(time,sizeof(time),&today) != 0)
	{
		return;
	}

	char temp[1024];

	va_list arg;
	va_start(arg,text);
	vsprintf_s(temp,text,arg);
	va_end(arg);

	char log[1024];

	wsprintf(log,"%.8s %s",&time[11],temp);

	gServerDisplayer.LogAddTextConnect(color,log,strlen(log));
}

bool DataSend(int aIndex,BYTE* lpMsg,DWORD size) // OK
{
	//BYTE head		= ((lpMsg[0]==0xC1 || lpMsg[0]==0xC3)?lpMsg[2]:lpMsg[3]);
	//BYTE subhead	= ((lpMsg[0]==0xC1 || lpMsg[0]==0xC3)?lpMsg[3]:lpMsg[4]);

	//if((head != 0xF3 || subhead != 0xF1) && (head != 0xF3 || subhead != 0xE2) && head != 0x0D && head != 0x0E && head != 0x0F && head != 0x18 && head != PROTOCOL_CODE1)
	//{
	//	char String[8092];
	//	char hex_chars[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	//	for( int i = 0; i < (int)size; ++i )
	//	{
	//		sprintf(&String[2*i], "%02X", lpMsg[i]);
	//	}

	//	if(strlen(String) <= 1024)
	//		LogAdd(LOG_RED,"PacketOUT: ID: %d T: %#X, H: %#04X, SH: %#04X, SIZE: %03d, PACKET: %s",aIndex,lpMsg[0],head,subhead,size,String);
	//}

	#if (NEW_PROTOCOL_SYSTEM==1)
		gSocketManagerModern.PacketSend(aIndex, ProtocolHead::BOTH_MESSAGE, lpMsg,size);
		return 1;
	#else
		return gSocketManager.DataSend(aIndex,lpMsg,size);
	#endif
}

void DataSendAll(BYTE* lpMsg,int size) // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) != 0)
		{
			DataSend(n,lpMsg,size);
		}
	}
}

bool DataSendSocket(SOCKET socket,BYTE* lpMsg,DWORD size) // OK
{
	if(socket == INVALID_SOCKET)
	{
		return 0;
	}

	#if(ENCRYPT_STATE==1)

	EncryptData(lpMsg,size);

	#endif

	int count=0,result=0;

	while(size > 0)
	{
		if((result=send(socket,(char*)&lpMsg[count],size,0)) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSAEWOULDBLOCK)
			{
				return 0;
			}
		}
		else
		{
			count += result;
			size -= result;
		}
	}

	return 1;
}

void MsgSendV2(LPOBJ lpObj,BYTE* lpMsg,int size) // OK
{
	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer2[n].state != VIEWPORT_NONE && lpObj->VpPlayer2[n].type == OBJECT_USER && OBJECT_BOTS)
		{
			DataSend(lpObj->VpPlayer2[n].index,lpMsg,size);
		}
	}
}

void SendMonsterV2Msg(LPOBJ lpObj, LPBYTE lpMsg, int size)
{
    for( int i=0; i< MAX_VIEWPORT; i++ )
    {
        if( lpObj->VpPlayer2[i].type == OBJECT_USER )
        {
            if( lpObj->VpPlayer2[i].state )
            {
                if( (lpObj->Connected > 1) && (lpObj->Live) )
                {
                    DataSend(lpObj->VpPlayer2[i].index, lpMsg, size);
                }
                else
                {
                    lpObj->VpPlayer2[i].index = -1;
                    lpObj->VpPlayer2[i].state  = VIEWPORT_NONE;
                    lpObj->VPCount2--;
                }
            }  
        }
 
    }
}

void CloseClient(int aIndex) // OK
{
	#if(NEW_PROTOCOL_SYSTEM==1)
		gSocketManagerModern.DisconenctClient(aIndex);
	#else
		gSocketManager.Disconnect(aIndex);
	#endif
}

void PostMessage1(char* name,char* message,char* text) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {0};

	wsprintf(buff,message,text);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_WHISPER_SEND pMsg;

	pMsg.header.set(0x02,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

void PostMessage2(char* name,char* message,char* text) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {'~'};

	wsprintf(&buff[1],message,text);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

void PostMessage3(char* name,char* message,char* text) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {'@'};

	wsprintf(&buff[1],message,text);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

void PostMessage4(char* name,char* message,char* text) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {'$'};

	wsprintf(&buff[1],message,text);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

void PostMessagePK(char* name,char* message,char* text) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {'~'};

	wsprintf(&buff[1],message,name,text);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	memcpy(pMsg.name,"[PK]",sizeof(pMsg.name));

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

void PostMessageUserON(char* name,char* message) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {'@'};

	wsprintf(&buff[1],message,name);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	memcpy(pMsg.name,"[UserON]",sizeof(pMsg.name));

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}

void PostMessageNew(char* name,char* message,char* text) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n],buff);
		}
	}

	#else

	char buff[256] = {0};

	wsprintf(buff,message,name,text);

	int size = strlen(buff);

	size = ((size>MAX_CHAT_MESSAGE_SIZE)?MAX_CHAT_MESSAGE_SIZE:size);

	PMSG_CHAT_WHISPER_SEND pMsg = { 0 };

	pMsg.header.set(0x02,(sizeof(pMsg)-(sizeof(pMsg.message)-(size+1))));

	sprintf_s(pMsg.name,sizeof(pMsg.name),"[POST]");

	memcpy(pMsg.message,buff,size);

	pMsg.message[size] = 0;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	#endif
}


void SetLargeRand() // OK
{
	std::random_device rd;
	seed = std::mt19937(rd());
	dist = std::uniform_int_distribution<int>(0, 2147483647);
}

long GetLargeRand() // OK
{
	return dist(seed);
}
