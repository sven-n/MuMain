#include "stdafx.h"
#include "JSProtocol.h"
#include "CustomAttack.h"
#include "CustomStore.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Log.h"
#include "MapServerManager.h"
#include "Message.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "Util.h"

void JoinServerProtocolCore(BYTE head,BYTE* lpMsg,int size) // OK
{
	switch(head)
	{
		case 0x00:
			JGServerInfoRecv((SDHP_JOIN_SERVER_INFO_RECV*)lpMsg);
			break;
		case 0x01:
			JGConnectAccountRecv((SDHP_CONNECT_ACCOUNT_RECV*)lpMsg);
			break;
		case 0x02:
			JGDisconnectAccountRecv((SDHP_DISCONNECT_ACCOUNT_RECV*)lpMsg);
			break;
		case 0x03:
			JGMapServerMoveRecv((SDHP_MAP_SERVER_MOVE_RECV*)lpMsg);
			break;
		case 0x04:
			JGMapServerMoveAuthRecv((SDHP_MAP_SERVER_MOVE_AUTH_RECV*)lpMsg);
			break;
		case 0x05:
			JGAccountLevelRecv((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg);
			break;
		case 0x06:
			JGAccountLevelRecv2((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg);
			break;
		case 0x30:
			JGAccountAlreadyConnectedRecv((SDHP_ACCOUNT_ALREADY_CONNECTED_RECV*)lpMsg);
			break;
	}
}

void JGServerInfoRecv(SDHP_JOIN_SERVER_INFO_RECV* lpMsg) // OK
{

}

void JGConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg) // OK
{
	if(OBJECT_RANGE(lpMsg->index) == 0)
	{
		return;
	}

	if((gObj[lpMsg->index].LoginMessageSend--) == 0)
	{
		CloseClient(lpMsg->index);
		return;
	}

	if(gObj[lpMsg->index].Connected != OBJECT_CONNECTED)
	{
		GJDisconnectAccountSend(lpMsg->index,lpMsg->account,gObj[lpMsg->index].IpAddr);
		return;
	}

	if(lpMsg->result != 1)
	{
		GCConnectAccountSend(lpMsg->index,((gObj[lpMsg->index].LoginMessageCount>=3)?8:lpMsg->result));
		return;
	}

	if(lpMsg->BlockCode != 0)
	{
		GJDisconnectAccountSend(lpMsg->index,lpMsg->account,gObj[lpMsg->index].IpAddr);
		GCConnectAccountSend(lpMsg->index,5);
		return;
	}

	if(gServerInfo.m_ServerLock > lpMsg->AccountLevel)
	{
		GJDisconnectAccountSend(lpMsg->index,lpMsg->account,gObj[lpMsg->index].IpAddr);
		GCConnectAccountSend(lpMsg->index,9);
		return;
	}

	gObj[lpMsg->index].Connected = OBJECT_LOGGED;

	memcpy(gObj[lpMsg->index].Account,lpMsg->account,sizeof(gObj[lpMsg->index].Account));

	memcpy(gObj[lpMsg->index].PersonalCode,lpMsg->PersonalCode,sizeof(gObj[lpMsg->index].PersonalCode));

	gObj[lpMsg->index].AccountLevel = lpMsg->AccountLevel;

	memcpy(gObj[lpMsg->index].AccountExpireDate,lpMsg->AccountExpireDate,sizeof(gObj[lpMsg->index].AccountExpireDate));

	gObj[lpMsg->index].Lock = lpMsg->Lock;

	GCConnectAccountSend(lpMsg->index,1);

	LogAddConnect(LOG_GREEN,"[Obj][%d] AddAccount (%s)",lpMsg->index,gObj[lpMsg->index].Account);
}

void JGDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) != 0)
	{
		CloseClient(lpMsg->index);
	}
}

void JGMapServerMoveRecv(SDHP_MAP_SERVER_MOVE_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[JGMapServerMoveRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}


	if(lpMsg->result == 0 && gObj[lpMsg->index].MapMoveDisable != 1)
	{
		gNotice.GCNoticeSend(lpMsg->index,0,0,0,0,0,0,gMessage.GetMessage(259));
		GJMapServerMoveSend(lpMsg->index,gServerInfo.m_ServerCode,0,125,125);
		return;
	}

	PMSG_MAP_SERVER_MOVE_SEND pMsg = {0};

	pMsg.header.set(0xB1,0x00,sizeof(pMsg));

	if(lpMsg->result == 0)
	{
		gObj[lpMsg->index].State = OBJECT_PLAYING;
		DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);
		gNotice.GCNoticeSend(lpMsg->index,0,0,0,0,0,0,gMessage.GetMessage(259));
		return;
	}

	if(gMapServerManager.GetMapServerData(lpMsg->NextServerCode,pMsg.IpAddress,&pMsg.ServerPort) == 0)
	{
		gObj[lpMsg->index].State = OBJECT_PLAYING;
		GJMapServerMoveCancelSend(lpMsg->index);
		DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);
		gNotice.GCNoticeSend(lpMsg->index,0,0,0,0,0,0,gMessage.GetMessage(259));

		//gObjMoveGate(lpMsg->index,17);
		return;
	}

	pMsg.ServerCode = gMapServerManager.GetMapServerGroup();

	pMsg.AuthCode1 = lpMsg->AuthCode1;

	pMsg.AuthCode2 = lpMsg->AuthCode2;

	pMsg.AuthCode3 = lpMsg->AuthCode3;

	pMsg.AuthCode4 = lpMsg->AuthCode4;

	DataSend(lpMsg->index,(BYTE*)&pMsg,pMsg.header.size);

	gObjectManager.CharacterGameClose(lpMsg->index);

	gObj[lpMsg->index].MapServerMoveQuit = 1;

	gObj[lpMsg->index].MapServerMoveQuitTickCount = GetTickCount();
}

void JGMapServerMoveAuthRecv(SDHP_MAP_SERVER_MOVE_AUTH_RECV* lpMsg) // OK
{
	if(OBJECT_RANGE(lpMsg->index) == 0)
	{
		return;
	}

	if((gObj[lpMsg->index].LoginMessageSend--) == 0)
	{
		CloseClient(lpMsg->index);
		return;
	}

	if(gObj[lpMsg->index].MapServerMoveRequest == 0)
	{
		CloseClient(lpMsg->index);
		return;
	}

	if(gObj[lpMsg->index].Connected != OBJECT_CONNECTED)
	{
		GJDisconnectAccountSend(lpMsg->index,lpMsg->account,gObj[lpMsg->index].IpAddr);
		return;
	}

	if(lpMsg->result != 1)
	{
		GCMapServerMoveAuthSend(lpMsg->index,((gObj[lpMsg->index].LoginMessageCount>=3)?8:lpMsg->result));
		return;
	}

	if(lpMsg->BlockCode != 0)
	{
		GJDisconnectAccountSend(lpMsg->index,lpMsg->account,gObj[lpMsg->index].IpAddr);
		GCMapServerMoveAuthSend(lpMsg->index,5);
		return;
	}

	if(gServerInfo.m_ServerLock > lpMsg->AccountLevel)
	{
		GJDisconnectAccountSend(lpMsg->index,lpMsg->account,gObj[lpMsg->index].IpAddr);
		GCMapServerMoveAuthSend(lpMsg->index,9);
		return;
	}

	gObj[lpMsg->index].Connected = OBJECT_LOGGED;

	memcpy(gObj[lpMsg->index].Account,lpMsg->account,sizeof(gObj[lpMsg->index].Account));

	memcpy(gObj[lpMsg->index].PersonalCode,lpMsg->PersonalCode,sizeof(gObj[lpMsg->index].PersonalCode));

	gObj[lpMsg->index].AccountLevel = lpMsg->AccountLevel;

	gObj[lpMsg->index].Lock = lpMsg->Lock;

	memcpy(gObj[lpMsg->index].AccountExpireDate,lpMsg->AccountExpireDate,sizeof(gObj[lpMsg->index].AccountExpireDate));

	GCMapServerMoveAuthSend(lpMsg->index,1);

	gObj[lpMsg->index].EnableDelCharacter = 0;

	gObj[lpMsg->index].LastServerCode = lpMsg->LastServerCode;

	gObj[lpMsg->index].DestMap  = lpMsg->map;

	gObj[lpMsg->index].DestX = lpMsg->x;

	gObj[lpMsg->index].DestY = lpMsg->y;

	GDCharacterInfoSend(lpMsg->index,lpMsg->name);

	LogAddConnect(LOG_GREEN,"[Obj][%d] AddAccount (%s)",lpMsg->index,gObj[lpMsg->index].Account);
}

void JGAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[JGAccountLevelRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	if(gServerInfo.m_ServerLock > lpMsg->AccountLevel)
	{
		gObjUserKill(lpMsg->index);
		return;
	}

	if(gObj[lpMsg->index].AccountLevel == lpMsg->AccountLevel && strcmp(gObj[lpMsg->index].AccountExpireDate,lpMsg->AccountExpireDate) == 0)
	{
		gObj[lpMsg->index].AccountLevel = lpMsg->AccountLevel;
		memcpy(gObj[lpMsg->index].AccountExpireDate,lpMsg->AccountExpireDate,sizeof(gObj[lpMsg->index].AccountExpireDate));
	}
	else
	{
		gObj[lpMsg->index].AccountLevel = lpMsg->AccountLevel;
		memcpy(gObj[lpMsg->index].AccountExpireDate,lpMsg->AccountExpireDate,sizeof(gObj[lpMsg->index].AccountExpireDate));
		gNotice.GCNoticeSend(lpMsg->index,1,0,0,0,0,0,gMessage.GetMessage((248+gObj[lpMsg->index].AccountLevel)),gObj[lpMsg->index].AccountExpireDate);
	}
}

void JGAccountLevelRecv2(SDHP_ACCOUNT_LEVEL_RECV* lpMsg) // OK
{
	if(strcmp(lpMsg->account,"null") == 0)
	{
		gNotice.GCNoticeSend(lpMsg->index,0,0,0,0,0,0,gMessage.GetMessage(520));
		return;
	}
	else
	{

		for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
		{
			if(strcmp(gObj[n].Account,lpMsg->account))
			{
				GJAccountLevelSend(gObj[n].Index);
			}
		}
		
		//gNotice.GCNoticeSend(lpMsg->index,1,0,0,0,0,0,gMessage.GetMessage(519));
		gNotice.GCNoticeSend(lpMsg->index,0,0,0,0,0,0,gMessage.GetMessage(248+lpMsg->AccountLevel),lpMsg->AccountExpireDate);
	}
}


void JGAccountAlreadyConnectedRecv(SDHP_ACCOUNT_ALREADY_CONNECTED_RECV* lpMsg) // OK
{
	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[JGAccountAlreadyConnectedRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	if(gServerInfo.m_DisconnectOnlineAccount == 1)
	{
		gObjUserKill(lpMsg->index);
	}

	gCustomAttack.OnAttackAlreadyConnected(&gObj[lpMsg->index]);

	gCustomStore.OnPShopAlreadyConnected(&gObj[lpMsg->index]);

}

void GJServerInfoSend() // OK
{
	SDHP_JOIN_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	pMsg.type = 1;

	pMsg.ServerPort = (WORD)gServerInfo.m_ServerPort;

	strcpy_s(pMsg.ServerName,gServerInfo.m_ServerName);

	pMsg.ServerCode = (WORD)gServerInfo.m_ServerCode;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJConnectAccountSend(int aIndex,char* account,char* password,char* IpAddress) // OK
{
	SDHP_CONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	memcpy(pMsg.password,password,sizeof(pMsg.password));

	memcpy(pMsg.IpAddress,IpAddress,sizeof(pMsg.IpAddress));

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJDisconnectAccountSend(int aIndex,char* account,char* IpAddress) // OK
{
	SDHP_DISCONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	memcpy(pMsg.IpAddress,IpAddress,sizeof(pMsg.IpAddress));

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJMapServerMoveSend(int aIndex,WORD NextServerCode,WORD map,BYTE x,BYTE y) // OK
{
	SDHP_MAP_SERVER_MOVE_SEND pMsg;

	pMsg.header.set(0x03,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.GameServerCode = (WORD)gServerInfo.m_ServerCode;

	pMsg.NextServerCode = NextServerCode;

	pMsg.map = map;

	pMsg.x = x;

	pMsg.y = y;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJMapServerMoveAuthSend(int aIndex,char* account,char* name,DWORD AuthCode1,DWORD AuthCode2,DWORD AuthCode3,DWORD AuthCode4) // OK
{
	SDHP_MAP_SERVER_MOVE_AUTH_SEND pMsg;

	pMsg.header.set(0x04,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	pMsg.LastServerCode = (WORD)gServerInfo.m_ServerCode;

	pMsg.AuthCode1 = AuthCode1;

	pMsg.AuthCode2 = AuthCode2;

	pMsg.AuthCode3 = AuthCode3;

	pMsg.AuthCode4 = AuthCode4;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJAccountLevelSend(int aIndex) // OK
{
	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x05,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJAccountLevelSend(int aIndex, char* account) // OK
{
	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x06,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJMapServerMoveCancelSend(int aIndex) // OK
{
	SDHP_MAP_SERVER_MOVE_CANCEL_SEND pMsg;

	pMsg.header.set(0x10,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJAccountLevelSaveSend(int aIndex,int AccountLevel,int AccountExpireTime) // OK
{
	SDHP_ACCOUNT_LEVEL_SAVE_SEND pMsg;

	pMsg.header.set(0x11,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.AccountLevel = AccountLevel;

	pMsg.AccountExpireTime = AccountExpireTime;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJAccountLevelSaveSend(int aIndex,int AccountLevel,int AccountExpireTime,char* account) // OK
{
	SDHP_ACCOUNT_LEVEL_SAVE_SEND pMsg;

	pMsg.header.set(0x11,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	pMsg.AccountLevel = AccountLevel;

	pMsg.AccountExpireTime = AccountExpireTime;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJAccountLockSaveSend(int aIndex,int Lock) // OK
{
	SDHP_LOCK_SAVE_SEND pMsg;

	pMsg.header.set(0x12,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.Lock = Lock;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void GJServerUserInfoSend() // OK
{
	SDHP_SERVER_USER_INFO_SEND pMsg;

	pMsg.header.set(0x20,sizeof(pMsg));

	pMsg.CurUserCount = (WORD)gObjTotalUser;

	pMsg.MaxUserCount = (WORD)gServerInfo.m_ServerMaxUserNumber;

	gJoinServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}
