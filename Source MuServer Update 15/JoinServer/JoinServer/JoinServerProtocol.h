#pragma once

#define SET_NUMBERHB(x) ((BYTE)((DWORD)(x)>>(DWORD)8))
#define SET_NUMBERLB(x) ((BYTE)((DWORD)(x)&0xFF))
#define SET_NUMBERHW(x) ((WORD)((DWORD)(x)>>(DWORD)16))
#define SET_NUMBERLW(x) ((WORD)((DWORD)(x)&0xFFFF))
#define SET_NUMBERHDW(x) ((DWORD)((QWORD)(x)>>(QWORD)32))
#define SET_NUMBERLDW(x) ((DWORD)((QWORD)(x)&0xFFFFFFFF))

#define MAKE_NUMBERW(x,y) ((WORD)(((BYTE)((y)&0xFF))|((BYTE)((x)&0xFF)<<8)))
#define MAKE_NUMBERDW(x,y) ((DWORD)(((WORD)((y)&0xFFFF))|((WORD)((x)&0xFFFF)<<16)))
#define MAKE_NUMBERQW(x,y) ((QWORD)(((DWORD)((y)&0xFFFFFFFF))|((DWORD)((x)&0xFFFFFFFF)<<32)))

//**********************************************//
//************ Packet Base *********************//
//**********************************************//

struct PBMSG_HEAD
{
	void set(BYTE head,BYTE size) // OK
	{
		this->type = 0xC1;
		this->size = size;
		this->head = head;
	}

	void setE(BYTE head,BYTE size) // OK
	{
		this->type = 0xC3;
		this->size = size;
		this->head = head;
	}

	BYTE type;
	BYTE size;
	BYTE head;
};

struct PSBMSG_HEAD
{
	void set(BYTE head,BYTE subh,BYTE size) // OK
	{
		this->type = 0xC1;
		this->size = size;
		this->head = head;
		this->subh = subh;
	}

	void setE(BYTE head,BYTE subh,BYTE size) // OK
	{
		this->type = 0xC3;
		this->size = size;
		this->head = head;
		this->subh = subh;
	}

	BYTE type;
	BYTE size;
	BYTE head;
	BYTE subh;
};

struct PWMSG_HEAD
{
	void set(BYTE head,WORD size) // OK
	{
		this->type = 0xC2;
		this->size[0] = SET_NUMBERHB(size);
		this->size[1] = SET_NUMBERLB(size);
		this->head = head;
	}

	void setE(BYTE head,WORD size) // OK
	{
		this->type = 0xC4;
		this->size[0] = SET_NUMBERHB(size);
		this->size[1] = SET_NUMBERLB(size);
		this->head = head;
	}

	BYTE type;
	BYTE size[2];
	BYTE head;
};

struct PSWMSG_HEAD
{
	void set(BYTE head,BYTE subh,WORD size) // OK
	{
		this->type = 0xC2;
		this->size[0] = SET_NUMBERHB(size);
		this->size[1] = SET_NUMBERLB(size);
		this->head = head;
		this->subh = subh;
	}

	void setE(BYTE head,BYTE subh,WORD size) // OK
	{
		this->type = 0xC4;
		this->size[0] = SET_NUMBERHB(size);
		this->size[1] = SET_NUMBERLB(size);
		this->head = head;
		this->subh = subh;
	}

	BYTE type;
	BYTE size[2];
	BYTE head;
	BYTE subh;
};

//**********************************************//
//********** GameServer -> JoinServer **********//
//**********************************************//

struct SDHP_SERVER_INFO_RECV
{
	PBMSG_HEAD header; // C1:00
	BYTE type;
	WORD ServerPort;
	char ServerName[50];
	WORD ServerCode;
};

struct SDHP_CONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:01
	WORD index;
	char account[11];
	char password[11];
	char IpAddress[16];
};

struct SDHP_DISCONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	char IpAddress[16];
};

struct SDHP_MAP_SERVER_MOVE_RECV
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

struct SDHP_MAP_SERVER_MOVE_AUTH_RECV
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

struct SDHP_ACCOUNT_LEVEL_RECV
{
	PBMSG_HEAD header; // C1:05
	WORD index;
	char account[11];
};

struct SDHP_MAP_SERVER_MOVE_CANCEL_RECV
{
	PBMSG_HEAD header; // C1:10
	WORD index;
	char account[11];
};

struct SDHP_ACCOUNT_LEVEL_SAVE_RECV
{
	PBMSG_HEAD header; // C1:11
	WORD index;
	char account[11];
	WORD AccountLevel;
	DWORD AccountExpireTime;
};

struct SDHP_SERVER_USER_INFO_RECV
{
	PBMSG_HEAD header; // C1:20
	WORD CurUserCount;
	WORD MaxUserCount;
};

struct SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV
{
	PBMSG_HEAD header; // C1:30
	char account[11];
};

struct SDHP_LOCK_SAVE_RECV
{
	PBMSG_HEAD header; // C1:11
	WORD index;
	char account[11];
	DWORD Lock;
};

//**********************************************//
//********** JoinServer -> GameServer **********//
//**********************************************//

struct SDHP_SERVER_INFO_SEND
{
	PBMSG_HEAD header; // C1:00
	BYTE result;
	DWORD ItemCount;
};

struct SDHP_CONNECT_ACCOUNT_SEND
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

struct SDHP_DISCONNECT_ACCOUNT_SEND
{
	PBMSG_HEAD header; // C1:02
	WORD index;
	char account[11];
	BYTE result;
};

struct SDHP_MAP_SERVER_MOVE_SEND
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

struct SDHP_MAP_SERVER_MOVE_AUTH_SEND
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

struct SDHP_ACCOUNT_LEVEL_SEND
{
	PBMSG_HEAD header; // C1:05
	WORD index;
	char account[11];
	WORD AccountLevel;
	char AccountExpireDate[20];
};

struct SDHP_ACCOUNT_ALREADY_CONNECTED_SEND
{
	PBMSG_HEAD header; // C1:30
	WORD index;
	char account[11];
};

//**********************************************//
//**********************************************//
//**********************************************//

void JoinServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size);
void GJServerInfoRecv(SDHP_SERVER_INFO_RECV* lpMsg,int index);
void GJConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg,int index);
void GJDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg,int index);
void GJMapServerMoveRecv(SDHP_MAP_SERVER_MOVE_RECV* lpMsg,int index);
void GJMapServerMoveAuthRecv(SDHP_MAP_SERVER_MOVE_AUTH_RECV* lpMsg,int index);
void GJAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index);
void GJMapServerMoveCancelRecv(SDHP_MAP_SERVER_MOVE_CANCEL_RECV* lpMsg,int index);
void GJAccountLevelSaveRecv(SDHP_ACCOUNT_LEVEL_SAVE_RECV* lpMsg,int index);
void GJServerUserInfoRecv(SDHP_SERVER_USER_INFO_RECV* lpMsg,int index);
void GJExternalDisconnectAccountRecv(SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV* lpMsg,int index);
void JGExternalDisconnectAccountSend(int GameServerCode,int UserIndex,char* account);
void JGAccountAlreadyConnectedSend(int GameServerCode,int UserIndex,char* account);
void GJAccountLevelRecv2(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index);
void GJAccountLockSaveRecv(SDHP_LOCK_SAVE_RECV* lpMsg,int index);
