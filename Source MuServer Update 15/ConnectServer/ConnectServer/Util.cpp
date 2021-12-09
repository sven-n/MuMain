#include "stdafx.h"
#include "Util.h"
#include "ClientManager.h"
#include "SocketManager.h"
#include "ThemidaSDK.h"

int gClientCount = 0;

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

void ConnectServerTimeoutProc() // OK
{
	for(int n=0;n < MAX_CLIENT;n++)
	{
		if(gClientManager[n].CheckState() != 0)
		{
			if(gClientManager[n].CheckOnlineTime() == 0)
			{
				gSocketManager.Disconnect(n);
			}
		}
	}
}

int GetFreeClientIndex() // OK
{
	int index = -1;
	int count = gClientCount;

	if(SearchFreeClientIndex(&index,0,MAX_CLIENT,10000) != 0)
	{
		return index;
	}

	for(int n=0;n < MAX_CLIENT;n++)
	{
		if(gClientManager[count].CheckState() == 0)
		{
			return count;
		}
		else
		{
			count = (((++count)>=MAX_CLIENT)?0:count);
		}
	}

	return -1;
}

int SearchFreeClientIndex(int* index,int MinIndex,int MaxIndex,DWORD MinTime) // OK
{
	DWORD CurOnlineTime = 0;
	DWORD MaxOnlineTime = 0;

	for(int n=MinIndex;n < MaxIndex;n++)
	{
		if(gClientManager[n].CheckState() == 0 && gClientManager[n].CheckAlloc() != 0)
		{
			if((CurOnlineTime=(GetTickCount()-gClientManager[n].m_OnlineTime)) > MinTime && CurOnlineTime > MaxOnlineTime)
			{
				(*index) = n;
				MaxOnlineTime = CurOnlineTime;
			}
		}
	}

	return (((*index)==-1)?0:1);
}
