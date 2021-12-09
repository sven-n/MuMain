#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct FHP_FRIENDLIST_REQ
{
	PBMSG_HEAD h; // C1:60
	short Number;
	char Name[11];
	BYTE pServer;
};

struct FHP_FRIEND_STATE_C
{
	PBMSG_HEAD h; // C1:62
	short Number;
	char Name[11];
	BYTE State;
};

struct FHP_FRIEND_ADD_REQ
{
	PBMSG_HEAD h; // C1:[63/65]
	short Number;
	char Name[11];
	char FriendName[11];
};

struct FHP_WAITFRIEND_ADD_REQ
{
	PBMSG_HEAD h; // C1:64
	BYTE Result;
	short Number;
	char Name[11];
	char FriendName[11];
};

struct FHP_FRIEND_CHATROOM_CREATE_REQ
{
	PBMSG_HEAD h; // C1:66
	short Number;
	char Name[11];
	char fName[11];
};

struct FHP_FRIEND_MEMO_SEND
{
	PWMSG_HEAD h; // C1:70
	short Number;
	DWORD WindowGuid;
	char Name[11];
	char ToName[11];
	char Subject[32];
	BYTE Dir;
	BYTE Action;
	short MemoSize;
	BYTE Photo[18];
	char Memo[1000];
};

struct FHP_FRIEND_MEMO_LIST_REQ
{
	PBMSG_HEAD h; // C1:71
	WORD Number;
	char Name[11];
};

struct FHP_FRIEND_MEMO_RECV_REQ
{
	PBMSG_HEAD h; // C1:72
	short Number;
	WORD MemoIndex;
	char Name[11];
};

struct FHP_FRIEND_MEMO_DEL_REQ
{
	PBMSG_HEAD h; // C1:73
	short Number;
	WORD MemoIndex;
	char Name[11];
};

struct FHP_FRIEND_INVITATION_REQ
{
	PBMSG_HEAD h; // C1:74
	short Number;
	char Name[11];
	char FriendName[11];
	WORD RoomNumber;
	DWORD WindowGuid;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct FHP_FRIENDLIST_COUNT
{
	PWMSG_HEAD h; // C1:60
	short Number;
	char Name[11];
	BYTE Count;
	BYTE MailCount;
};

struct FHP_FRIENDLIST
{
	char Name[11];
	BYTE Server;
};

struct FHP_WAITFRIENDLIST_COUNT
{
	PBMSG_HEAD h; // C1:61
	short Number;
	char Name[11];
	char FriendName[11];
};

struct FHP_FRIEND_STATE
{
	PBMSG_HEAD h; // C1:62
	short Number;
	char Name[11];
	char FriendName[11];
	BYTE State;
};

struct FHP_FRIEND_ADD_RESULT
{
	PBMSG_HEAD h; // C1:63
	short Number;
	BYTE Result;
	char Name[11];
	char FriendName[11];
	BYTE Server;
};

struct FHP_WAITFRIEND_ADD_RESULT
{
	PBMSG_HEAD h; // C1:64
	short Number;
	BYTE Result;
	char Name[11];
	char FriendName[11];
	BYTE pServer;
};

struct FHP_FRIEND_DEL_RESULT
{
	PBMSG_HEAD h; // C1:65
	short Number;
	BYTE Result;
	char Name[11];
	char FriendName[11];
};

struct FHP_FRIEND_CHATROOM_CREATE_RESULT
{
	PBMSG_HEAD h; // C1:66
	BYTE Result;
	short Number;
	char Name[11];
	char FriendName[11];
	char ServerIp[16];
	WORD RoomNumber;
	DWORD Ticket;
	BYTE Type;
};

struct FHP_FRIEND_MEMO_SEND_RESULT
{
	PBMSG_HEAD h; // C1:70
	short Number;
	char Name[11];
	BYTE Result;
	DWORD WindowGuid;
};

struct FHP_FRIEND_MEMO_LIST
{
	PWMSG_HEAD h; // C1:71
	WORD Number;
	WORD MemoIndex;
	char SendName[11];
	char RecvName[11];
	char Date[30];
	char Subject[32];
	BYTE read;
};

struct FHP_FRIEND_MEMO_RECV
{
	PWMSG_HEAD h; // C1:72
	short Number;
	char Name[11];
	WORD MemoIndex;
	short MemoSize;
	BYTE Photo[18];
	BYTE Dir;
	BYTE Action;
	char Memo[1000];
};

struct FHP_FRIEND_MEMO_DEL_RESULT
{
	PBMSG_HEAD h; // C1:73
	BYTE Result;
	WORD MemoIndex;
	short Number;
	char Name[11];
};

struct FHP_FRIEND_INVITATION_RET
{
	PBMSG_HEAD h; // C1:74
	BYTE Result;
	short Number;
	char Name[11];
	DWORD WindowGuid;
};

//**********************************************//
//**********************************************//
//**********************************************//

void CSDataSend(int index,BYTE* lpMsg,int size);
void CSDataRecv(int index,BYTE head,BYTE* lpMsg,int size);
void ChatServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size);
void FriendListRequest(FHP_FRIENDLIST_REQ* lpMsg,int index);
void FriendStateClientRecv(FHP_FRIEND_STATE_C* lpMsg,int index);
void FriendAddRequest(FHP_FRIEND_ADD_REQ* lpMsg,int index);
void WaitFriendAddRequest(FHP_WAITFRIEND_ADD_REQ* lpMsg,int index);
void FriendDelRequest(FHP_FRIEND_ADD_REQ* lpMsg,int index);
void FriendMemoSend(FHP_FRIEND_MEMO_SEND* lpMsg,int index);
void FriendMemoListReq(FHP_FRIEND_MEMO_LIST_REQ* lpMsg,int index);
void FriendMemoReadReq(FHP_FRIEND_MEMO_RECV_REQ* lpMsg,int index);
void FriendMemoDelReq(FHP_FRIEND_MEMO_DEL_REQ* lpMsg,int index);
void WaitFriendListResult(int index,DWORD guid,WORD aIndex,char* name);
void FriendStateRecv(char* name,BYTE state);
void FriendMemoList(int index,DWORD guid,WORD aIndex,char* name);
