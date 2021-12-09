#include "stdafx.h"
#include "Util.h"
#include "Protect.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "ThemidaSDK.h"

int gServerCount = 0;

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

void JoinServerLiveProc() // OK
{
	PROTECT_START

	SDHP_JOIN_SERVER_LIVE_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.QueueSize = gSocketManager.GetQueueSize();

	gSocketManagerUdp.DataSend((BYTE*)&pMsg,pMsg.header.size);

	PROTECT_FINAL
}

bool CheckTextSyntax(char* text,int size) // OK
{
	for(int n=0;n < size;n++)
	{
		if(text[n] == 0x20 || text[n] == 0x22 || text[n] == 0x27)
		{
			return 0;
		}
	}

	return 1;
}

LONG CheckAccountCaseSensitive(int value) // OK
{
	if(CaseSensitive == 0)
	{
		return tolower(value);
	}
	else
	{
		return value;
	}
}

int GetFreeServerIndex() // OK
{
	int index = -1;
	int count = gServerCount;

	if(SearchFreeServerIndex(&index,0,MAX_SERVER,10000) != 0)
	{
		return index;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[count].CheckState() == 0)
		{
			return count;
		}
		else
		{
			count = (((++count)>=MAX_SERVER)?0:count);
		}
	}

	return -1;
}

int SearchFreeServerIndex(int* index,int MinIndex,int MaxIndex,DWORD MinTime) // OK
{
	DWORD CurOnlineTime = 0;
	DWORD MaxOnlineTime = 0;

	for(int n=MinIndex;n < MaxIndex;n++)
	{
		if(gServerManager[n].CheckState() == 0 && gServerManager[n].CheckAlloc() != 0)
		{
			if((CurOnlineTime=(GetTickCount()-gServerManager[n].m_OnlineTime)) > MinTime && CurOnlineTime > MaxOnlineTime)
			{
				(*index) = n;
				MaxOnlineTime = CurOnlineTime;
			}
		}
	}

	return (((*index)==-1)?0:1);
}

CServerManager* FindServerByCode(WORD ServerCode) // OK
{
	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0 && gServerManager[n].m_ServerCode == ServerCode)
		{
			return &gServerManager[n];
		}
	}

	return 0;
}

DWORD MakeAccountKey(char* account) // OK
{
	int size = strlen(account);

	DWORD key = 0;

	for(int n=0;n < size;n++)
	{
		key += account[n]+17;
	}

	return ((key+((10-size)*17))%256);
}
