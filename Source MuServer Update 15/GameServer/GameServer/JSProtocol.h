#pragma once

#include "Protocol.h"

//**********************************************//
//********** JoinServer -> GameServer **********//
//**********************************************//

struct SDHP_JOIN_SERVER_INFO_RECV
{
	PBMSG_HEAD header; // C1:00
	BYTE result;
	DWORD ItemCount;
};

struct SDHP_CONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
	char PersonalCode[14];
	BYTE result;
	BYTE BlockCode;
	WORD AccountLevel;
	char AccountExpireDate[20];
	DWORD Lock;
};

struct SDHP_DISCONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	BYTE result;
};

struct SDHP_MAP_SERVER_MOVE_RECV
{
	PBMSG_HEAD header; // C1:03
	WORD index;
	char account[11];
	char name[11];
	BYTE result;
	WORD GameServerCode;
	WORD NextServerCode;
	WORD map;
	BYTE x;
	BYTE y;
	DWORD AuthCode1;
	DWORD AuthCode2;
	DWORD AuthCode3;
	DWORD AuthCode4;
};

struct SDHP_MAP_SERVER_MOVE_AUTH_RECV
{
	PBMSG_HEAD header; // C1:04
	WORD index;
	char account[11];
	char name[11];
	char PersonalCode[14];
	BYTE result;
	BYTE BlockCode;
	WORD LastServerCode;
	WORD map;
	BYTE x;
	BYTE y;
	WORD AccountLevel;
	char AccountExpireDate[20];
	DWORD Lock;
};

struct SDHP_ACCOUNT_LEVEL_RECV
{
	PBMSG_HEAD header; // C1:05
	WORD index;
	char account[11];
	WORD AccountLevel;
	char AccountExpireDate[20];
};

struct SDHP_ACCOUNT_ALREADY_CONNECTED_RECV
{
	PBMSG_HEAD header; // C1:30
	WORD index;
	char account[11];
};

//**********************************************//
//********** GameServer -> JoinServer **********//
//**********************************************//

struct SDHP_JOIN_SERVER_INFO_SEND
{
	PBMSG_HEAD header; // C1:00
	BYTE type;
	WORD ServerPort;
	char ServerName[50];
	WORD ServerCode;
};

struct SDHP_CONNECT_ACCOUNT_SEND
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
	char password[11];
	char IpAddress[16];
};

struct SDHP_DISCONNECT_ACCOUNT_SEND
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	char IpAddress[16];
};

struct SDHP_MAP_SERVER_MOVE_SEND
{
	PBMSG_HEAD header; // C1:03
	WORD index;
	char account[11];
	char name[11];
	WORD GameServerCode;
	WORD NextServerCode;
	WORD map;
	BYTE x;
	BYTE y;
};

struct SDHP_MAP_SERVER_MOVE_AUTH_SEND
{
	PBMSG_HEAD header; // C1:04
	WORD index;
	char account[11];
	char name[11];
	WORD LastServerCode;
	DWORD AuthCode1;
	DWORD AuthCode2;
	DWORD AuthCode3;
	DWORD AuthCode4;
};

struct SDHP_ACCOUNT_LEVEL_SEND
{
	PBMSG_HEAD header; // C1:05
	WORD index;
	char account[11];
};

struct SDHP_MAP_SERVER_MOVE_CANCEL_SEND
{
	PBMSG_HEAD header; // C1:10
	WORD index;
	char account[11];
};

struct SDHP_ACCOUNT_LEVEL_SAVE_SEND
{
	PBMSG_HEAD header; // C1:11
	WORD index;
	char account[11];
	WORD AccountLevel;
	DWORD AccountExpireTime;
};

struct SDHP_SERVER_USER_INFO_SEND
{
	PBMSG_HEAD header; // C1:20
	WORD CurUserCount;
	WORD MaxUserCount;
};

struct SDHP_LOCK_SAVE_SEND
{
	PBMSG_HEAD header; // C1:11
	WORD index;
	char account[11];
	DWORD Lock;
};

//**********************************************//
//**********************************************//
//**********************************************//

void JoinServerProtocolCore(BYTE head,BYTE* lpMsg,int size);
void JGServerInfoRecv(SDHP_JOIN_SERVER_INFO_RECV* lpMsg);
void JGConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg);
void JGDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg);
void JGMapServerMoveRecv(SDHP_MAP_SERVER_MOVE_RECV* lpMsg);
void JGMapServerMoveAuthRecv(SDHP_MAP_SERVER_MOVE_AUTH_RECV* lpMsg);
void JGAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg);
void JGAccountLevelRecv2(SDHP_ACCOUNT_LEVEL_RECV* lpMsg);
void JGAccountAlreadyConnectedRecv(SDHP_ACCOUNT_ALREADY_CONNECTED_RECV* lpMsg);
void GJServerInfoSend();
void GJConnectAccountSend(int aIndex,char* account,char* password,char* IpAddress);
void GJDisconnectAccountSend(int aIndex,char* account,char* IpAddress);
void GJMapServerMoveSend(int aIndex,WORD NextServerCode,WORD map,BYTE x,BYTE y);
void GJMapServerMoveAuthSend(int aIndex,char* account,char* name,DWORD AuthCode1,DWORD AuthCode2,DWORD AuthCode3,DWORD AuthCode4);
void GJAccountLevelSend(int aIndex);
void GJAccountLevelSend(int aIndex,char* account);
void GJMapServerMoveCancelSend(int aIndex);
void GJAccountLevelSaveSend(int aIndex,int AccountLevel,int AccountExpireTime);
void GJAccountLevelSaveSend(int aIndex,int AccountLevel,int AccountExpireTime,char* account);
void GJServerUserInfoSend();
void GJAccountLockSaveSend(int aIndex,int Lock);
