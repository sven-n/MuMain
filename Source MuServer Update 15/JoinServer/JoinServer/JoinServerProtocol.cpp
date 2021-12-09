#include "stdafx.h"
#include "JoinServerProtocol.h"
#include "..\\..\\Util\\MD5.h"
#include "AccountManager.h"
#include "Log.h"
#include "Protect.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"
#include "Util.h"

void JoinServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	gServerManager[index].m_PacketTime = GetTickCount();

	switch(head)
	{
		case 0x00:
			GJServerInfoRecv((SDHP_SERVER_INFO_RECV*)lpMsg,index);
			break;
		case 0x01:
			GJConnectAccountRecv((SDHP_CONNECT_ACCOUNT_RECV*)lpMsg,index);
			break;
		case 0x02:
			GJDisconnectAccountRecv((SDHP_DISCONNECT_ACCOUNT_RECV*)lpMsg,index);
			break;
		case 0x03:
			GJMapServerMoveRecv((SDHP_MAP_SERVER_MOVE_RECV*)lpMsg,index);
			break;
		case 0x04:
			GJMapServerMoveAuthRecv((SDHP_MAP_SERVER_MOVE_AUTH_RECV*)lpMsg,index);
			break;
		case 0x05:
			GJAccountLevelRecv((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg,index);
			break;
		case 0x06:
			GJAccountLevelRecv2((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg,index);
			break;
		case 0x10:
			GJMapServerMoveCancelRecv((SDHP_MAP_SERVER_MOVE_CANCEL_RECV*)lpMsg,index);
			break;
		case 0x11:
			GJAccountLevelSaveRecv((SDHP_ACCOUNT_LEVEL_SAVE_RECV*)lpMsg,index);
			break;
		case 0x12:
			GJAccountLockSaveRecv((SDHP_LOCK_SAVE_RECV*)lpMsg,index);
			break;
		case 0x20:
			GJServerUserInfoRecv((SDHP_SERVER_USER_INFO_RECV*)lpMsg,index);
			break;
		case 0x30:
			GJExternalDisconnectAccountRecv((SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV*)lpMsg,index);
			break;
	}

	PROTECT_FINAL
}

void GJServerInfoRecv(SDHP_SERVER_INFO_RECV* lpMsg,int index) // OK
{
	SDHP_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	pMsg.result = 1;

	pMsg.ItemCount = 0;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	gServerManager[index].SetServerInfo(lpMsg->ServerName,lpMsg->ServerPort,lpMsg->ServerCode);
}

void GJConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg,int index) // OK
{
	SDHP_CONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 1;

	if(CheckTextSyntax(lpMsg->account,sizeof(lpMsg->account)) == 0)
	{
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

    if(gAccountManager.GetAccountCount() >= 
#if    PROTECT_STATE
        gJoinServerMaxAccount[gProtect.m_AuthInfo.PackageType][gProtect.m_AuthInfo.PlanType]
#else
        MAX_ACCOUNT
#endif
        )
	//if(gAccountManager.GetAccountCount() >= gJoinServerMaxAccount[gProtect.m_AuthInfo.PackageType][gProtect.m_AuthInfo.PlanType])
	{
		pMsg.result = 4;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(MD5Encryption == 0)
	{
		if(gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s' COLLATE Latin1_General_BIN",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.result = 2;
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		char password[11] = {0};

		gQueryManager.GetAsString("memb__pwd",password,sizeof(password));

		if(strcmp(lpMsg->password,password) != 0 && strcmp(lpMsg->password,GlobalPassword) != 0)
		{
			gQueryManager.Close();
			pMsg.result = 0;
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gQueryManager.Close();

		if(strcmp(lpMsg->password,GlobalPassword) == 0)
		{
			LogAdd(LOG_RED,"Account %s logged with global password!",lpMsg->account);
		}
	}
	else
	{
		if(gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.result = 2;
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		BYTE password[16] = {0};

		gQueryManager.GetAsBinary("memb__pwd",password,sizeof(password));

		MD5 MD5Hash;

		if(MD5Hash.MD5_CheckValue(lpMsg->password,(char*)password,MakeAccountKey(lpMsg->account)) == 0 && strcmp(lpMsg->password,GlobalPassword) != 0)
		{
			gQueryManager.Close();
			pMsg.result = 0;
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gQueryManager.Close();

		if(strcmp(lpMsg->password,GlobalPassword) == 0)
		{
			LogAdd(LOG_RED,"Account %s logged with global password!",lpMsg->account);
		}
	}

	if(gQueryManager.ExecQuery("EXEC WZ_DesblocAccount '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gQueryManager.ExecQuery("SELECT sno__numb,bloc_code FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gQueryManager.GetAsString("sno__numb",pMsg.PersonalCode,sizeof(pMsg.PersonalCode));

	pMsg.BlockCode = (BYTE)gQueryManager.GetAsInteger("bloc_code");

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

	gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("SELECT top 1 Lock FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.Lock = 0;
	}
	else
	{
		pMsg.Lock = gQueryManager.GetAsInteger("Lock");
		gQueryManager.Close();
	}

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) != 0)
	{
		pMsg.result = 3;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		JGAccountAlreadyConnectedSend(AccountInfo.GameServerCode,AccountInfo.UserIndex,AccountInfo.Account);
		return;
	}

	gQueryManager.ExecQuery("EXEC WZ_CONNECT_MEMB '%s','%s','%s'",lpMsg->account,gServerManager[index].m_ServerName,lpMsg->IpAddress);

	gQueryManager.Close();

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	strcpy_s(AccountInfo.Account,lpMsg->account);

	strcpy_s(AccountInfo.IpAddress,lpMsg->IpAddress);

	AccountInfo.UserIndex = lpMsg->index;
	AccountInfo.GameServerCode = gServerManager[index].m_ServerCode;
	AccountInfo.MapServerMove = 0;
	AccountInfo.MapServerMoveTime = 0;
	AccountInfo.LastServerCode = -1;
	AccountInfo.NextServerCode = -1;
	AccountInfo.Map = 0;
	AccountInfo.X = 0;
	AccountInfo.Y = 0;
	AccountInfo.AuthCode1 = 0;
	AccountInfo.AuthCode2 = 0;
	AccountInfo.AuthCode3 = 0;
	AccountInfo.AuthCode4 = 0;

	gAccountManager.InsertAccountInfo(AccountInfo);

	gLog.Output(LOG_ACCOUNT,"[AccountInfo] Account connected (Account: %s, IpAddress: %s, GameServerCode: %d)",AccountInfo.Account,AccountInfo.IpAddress,AccountInfo.GameServerCode);
}

void GJDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg,int index) // OK
{
	SDHP_DISCONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.UserIndex != lpMsg->index)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.GameServerCode != gServerManager[index].m_ServerCode)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.MapServerMove != 0 && (GetTickCount()-AccountInfo.MapServerMoveTime) < 30000)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gQueryManager.ExecQuery("EXEC WZ_DISCONNECT_MEMB '%s'",lpMsg->account);

	gQueryManager.Close();

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	gAccountManager.RemoveAccountInfo(AccountInfo);

	gLog.Output(LOG_ACCOUNT,"[AccountInfo] Account disconnected (Account: %s, IpAddress: %s, GameServerCode: %d)",AccountInfo.Account,AccountInfo.IpAddress,AccountInfo.GameServerCode);
}

void GJMapServerMoveRecv(SDHP_MAP_SERVER_MOVE_RECV* lpMsg,int index) // OK
{
	SDHP_MAP_SERVER_MOVE_SEND pMsg;

	pMsg.header.set(0x03,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.UserIndex != lpMsg->index)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.GameServerCode != gServerManager[index].m_ServerCode)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.MapServerMove != 0)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CServerManager* lpServerManager = FindServerByCode(lpMsg->NextServerCode);

	if(lpServerManager == 0)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpServerManager->m_CurUserCount >= lpServerManager->m_MaxUserCount)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.GameServerCode = lpMsg->GameServerCode;

	pMsg.NextServerCode = lpMsg->NextServerCode;

	pMsg.map = lpMsg->map;

	pMsg.x = lpMsg->x;

	pMsg.y = lpMsg->y;

	pMsg.AuthCode1 = GetTickCount();

	pMsg.AuthCode2 = GetTickCount()%10000;

	pMsg.AuthCode3 = GetTickCount()%777;

	pMsg.AuthCode4 = GetTickCount()%8911;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	AccountInfo.MapServerMove = 1;
	AccountInfo.MapServerMoveTime = GetTickCount();
	AccountInfo.LastServerCode = pMsg.GameServerCode;
	AccountInfo.NextServerCode = pMsg.NextServerCode;
	AccountInfo.Map = pMsg.map;
	AccountInfo.X = pMsg.x;
	AccountInfo.Y = pMsg.y;
	AccountInfo.AuthCode1 = pMsg.AuthCode1;
	AccountInfo.AuthCode2 = pMsg.AuthCode2;
	AccountInfo.AuthCode3 = pMsg.AuthCode3;
	AccountInfo.AuthCode4 = pMsg.AuthCode4;

	gAccountManager.InsertAccountInfo(AccountInfo);
}

void GJMapServerMoveAuthRecv(SDHP_MAP_SERVER_MOVE_AUTH_RECV* lpMsg,int index) // OK
{
	SDHP_MAP_SERVER_MOVE_AUTH_SEND pMsg;

	pMsg.header.set(0x04,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		pMsg.result = 4;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.MapServerMove == 0)
	{
		pMsg.result = 4;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpMsg->AuthCode1 != AccountInfo.AuthCode1 || lpMsg->AuthCode2 != AccountInfo.AuthCode2 || lpMsg->AuthCode3 != AccountInfo.AuthCode3 || lpMsg->AuthCode4 != AccountInfo.AuthCode4)
	{
		pMsg.result = 4;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gQueryManager.ExecQuery("SELECT sno__numb,bloc_code FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gQueryManager.GetAsString("sno__numb",pMsg.PersonalCode,sizeof(pMsg.PersonalCode));

	pMsg.BlockCode = (BYTE)gQueryManager.GetAsInteger("bloc_code");

	gQueryManager.Close();

	pMsg.LastServerCode = AccountInfo.LastServerCode;

	pMsg.map = AccountInfo.Map;

	pMsg.x = AccountInfo.X;

	pMsg.y = AccountInfo.Y;

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

	gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("SELECT top 1 Lock FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.Lock = 0;
	}
	else
	{
		pMsg.Lock = gQueryManager.GetAsInteger("Lock");
		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	AccountInfo.UserIndex = lpMsg->index;
	AccountInfo.GameServerCode = gServerManager[index].m_ServerCode;
	AccountInfo.MapServerMove = 0;
	AccountInfo.MapServerMoveTime = 0;
	AccountInfo.LastServerCode = -1;
	AccountInfo.NextServerCode = -1;
	AccountInfo.Map = 0;
	AccountInfo.X = 0;
	AccountInfo.Y = 0;
	AccountInfo.AuthCode1 = 0;
	AccountInfo.AuthCode2 = 0;
	AccountInfo.AuthCode3 = 0;
	AccountInfo.AuthCode4 = 0;

	gAccountManager.InsertAccountInfo(AccountInfo);
}

void GJAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index) // OK
{
	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		return;
	}

	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x05,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.AccountLevel = 0;
	}
	else
	{
		pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

		gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}


void GJAccountLevelRecv2(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index) // OK
{

	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x06,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.AccountLevel = 0;

		memcpy(pMsg.account,"null",sizeof(pMsg.account));
	}
	else
	{
		pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

		gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}

void GJMapServerMoveCancelRecv(SDHP_MAP_SERVER_MOVE_CANCEL_RECV* lpMsg,int index) // OK
{
	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		return;
	}

	AccountInfo.MapServerMove = 0;
	AccountInfo.MapServerMoveTime = 0;
	AccountInfo.LastServerCode = -1;
	AccountInfo.NextServerCode = -1;
	AccountInfo.Map = 0;
	AccountInfo.X = 0;
	AccountInfo.Y = 0;
	AccountInfo.AuthCode1 = 0;
	AccountInfo.AuthCode2 = 0;
	AccountInfo.AuthCode3 = 0;
	AccountInfo.AuthCode4 = 0;

	gAccountManager.InsertAccountInfo(AccountInfo);
}

void GJAccountLevelSaveRecv(SDHP_ACCOUNT_LEVEL_SAVE_RECV* lpMsg,int index) // OK
{
	gQueryManager.ExecQuery("EXEC WZ_SetAccountLevel '%s','%d','%d'",lpMsg->account,lpMsg->AccountLevel,lpMsg->AccountExpireTime);
	gQueryManager.Fetch();
	gQueryManager.Close();
}

void GJAccountLockSaveRecv(SDHP_LOCK_SAVE_RECV* lpMsg,int index) // OK
{
	gQueryManager.ExecQuery("UPDATE memb_info SET Lock = %d WHERE memb___id = '%s'",lpMsg->Lock,lpMsg->account);
	gQueryManager.Fetch();
	gQueryManager.Close();
}

void GJServerUserInfoRecv(SDHP_SERVER_USER_INFO_RECV* lpMsg,int index) // OK
{
	gServerManager[index].m_CurUserCount = lpMsg->CurUserCount;

	gServerManager[index].m_MaxUserCount = lpMsg->MaxUserCount;
}

void GJExternalDisconnectAccountRecv(SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV* lpMsg,int index) // OK
{
	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		return;
	}

	JGExternalDisconnectAccountSend(AccountInfo.GameServerCode,AccountInfo.UserIndex,AccountInfo.Account);
}

void JGExternalDisconnectAccountSend(int GameServerCode,int UserIndex,char* account) // OK
{
	CServerManager* lpServerManager = FindServerByCode(GameServerCode);

	if(lpServerManager == 0)
	{
		return;
	}

	SDHP_DISCONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.index = UserIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	pMsg.result = 0;

	gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
}

void JGAccountAlreadyConnectedSend(int GameServerCode,int UserIndex,char* account) // OK
{
	CServerManager* lpServerManager = FindServerByCode(GameServerCode);

	if(lpServerManager == 0)
	{
		return;
	}

	SDHP_ACCOUNT_ALREADY_CONNECTED_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.index = UserIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
}
