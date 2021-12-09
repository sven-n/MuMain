#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_FRIEND_ADD_REQ
{
	PBMSG_HEAD h;
	char Name[10];	// 3
};

struct PMSG_FRIEND_STATE_C
{
	PBMSG_HEAD h;	//	
	BYTE ChatVeto;	//	3
};

struct PMSG_FRIEND_DEL_REQ
{
	PBMSG_HEAD h;	//	
	char Name[10];	//	3
};

struct PMSG_FRIEND_ROOMCREATE_REQ
{
	PBMSG_HEAD h;	//	
	char Name[10];	//	3
};

struct PMSG_FRIEND_MEMO
{
	PWMSG_HEAD h;	//	
	DWORD WindowGuid;	//	4
	char Name[10];	//	8
	#if(GAMESERVER_UPDATE>=603)
	char Subject[60];	//	12
	#else
	char Subject[32];	//	12
	#endif
	BYTE Action;	//	32
	BYTE Dir;	//	33
	short MemoSize;	//	34
	char Memo[1000];	//	36
};

struct PMSG_FRIEND_READ_MEMO_REQ
{
	PBMSG_HEAD h;	//	
	WORD MemoIndex;	//	4
};

struct PMSG_FRIEND_MEMO_DEL_REQ
{
	PBMSG_HEAD h;	//	
	WORD MemoIndex;	//	4
};

struct PMSG_ROOM_INVITATION
{
	PBMSG_HEAD h;	//	
	char Name[10];	//	3
	WORD RoomNumber;	//	E
	DWORD WindowGuid;	//	10
};

struct PMSG_FRIEND_ADD_SIN_RESULT
{
	PBMSG_HEAD h;	//	
	BYTE Result;	//	3
	char Name[10];	//	4
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_FRIEND_ADD_SIN_REQ
{
	PBMSG_HEAD h;
	char Name[10];	// 3
};

struct PMSG_FRIEND_LIST_COUNT
{
	PWMSG_HEAD h;
	BYTE MemoCount;	// 4
	BYTE MailTotal;	// 5
	BYTE Count;	// 6
};

struct PMSG_FRIEND_LIST
{
	char Name[10];
	BYTE Server;
};

struct PMSG_FRIEND_ADD_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;	// 3
	char Name[10];	// 4
	BYTE State;	// E
};

struct PMSG_FRIEND_DEL_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;	// 3
	char Name[10];	// 4
};

struct PMSG_FRIEND_ROOMCREATE_RESULT
{
	PBMSG_HEAD h;
	BYTE ServerIp[15];	// 3
	WORD RoomNumber;	// 12
	DWORD Ticket;	// 14
	BYTE Type;	// 18
	char Name[10];	// 19
	BYTE Result;	// 23
};

struct PMSG_FRIEND_MEMO_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;	// 3
	DWORD WindowGuid;	// 4
};

struct PMSG_FRIEND_MEMO_LIST
{
	PBMSG_HEAD h;
	WORD Number;	// 4
	char Name[10];	// 6
	char Date[30];	// 10
	#if(GAMESERVER_UPDATE>=603)
	char Subject[60];	// 2E
	#else
	char Subject[32];	// 2E
	#endif
	BYTE read;	// 4E
};

struct PMSG_FRIEND_READ_MEMO
{
	PWMSG_HEAD h;
	WORD MemoIndex;	// 4
	short MemoSize;	// 6
	BYTE Photo[18];	// 8
	BYTE Action;	// 1A
	BYTE Dir;	//1B
	char Memo[1000];	// 1C
};

struct PMSG_FRIEND_MEMO_DEL_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;	// 3
	WORD MemoIndex;	// 4
};

struct PMSG_ROOM_INVITATION_RESULT
{
	PBMSG_HEAD h;
	BYTE Result;	// 3
	DWORD WindowGuid;	// 4
};

struct PMSG_FRIEND_STATE
{
	PBMSG_HEAD h;	// C1:C4
	char Name[10];	// 3
	BYTE State;	// D
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
//**********************************************//
//**********************************************//

void CSDataSend(BYTE* lpMsg,int size);
void CSDataRecv(BYTE head,BYTE* lpMsg,int size);
void ChatServerProtocolCore(BYTE head,BYTE* lpMsg,int size);
void FriendListRequest(int aIndex);
void FriendListResult(LPBYTE lpMsg);
void WaitFriendListResult( FHP_WAITFRIENDLIST_COUNT* lpMsg);
void FriendStateClientRecv( PMSG_FRIEND_STATE_C* lpMsg, int aIndex);
void FriendStateRecv( FHP_FRIEND_STATE* lpMsg);
void FriendAddRequest( PMSG_FRIEND_ADD_REQ * lpMsg, int aIndex);
void FriendAddResult( FHP_FRIEND_ADD_RESULT* lpMsg);
void WaitFriendAddRequest( PMSG_FRIEND_ADD_SIN_RESULT* lpMsg, int aIndex);
void WaitFriendAddResult( FHP_WAITFRIEND_ADD_RESULT* lpMsg);
void FriendDelRequest( PMSG_FRIEND_DEL_REQ* lpMsg, int aIndex);
void FriendDelResult( FHP_FRIEND_DEL_RESULT* lpMsg);
void FriendChatRoomCreateReq( PMSG_FRIEND_ROOMCREATE_REQ* lpMsg, int aIndex);
void FriendChatRoomCreateResult( FHP_FRIEND_CHATROOM_CREATE_RESULT* lpMsg);
void FriendMemoSend( PMSG_FRIEND_MEMO* lpMsg, int aIndex);
void FriendMemoSendResult( FHP_FRIEND_MEMO_SEND_RESULT* lpMsg);
void FriendMemoListReq(int aIndex);
void FriendMemoList( FHP_FRIEND_MEMO_LIST* lpMsg);
void FriendMemoReadReq( PMSG_FRIEND_READ_MEMO_REQ* lpMsg, int aIndex);
void FriendMemoRead( FHP_FRIEND_MEMO_RECV* lpMsg);
void FriendMemoDelReq( PMSG_FRIEND_MEMO_DEL_REQ* lpMsg, int aIndex);
void FriendMemoDelResult( FHP_FRIEND_MEMO_DEL_RESULT* lpMsg);
void FriendRoomInvitationReq( PMSG_ROOM_INVITATION* lpMsg, int aIndex);
void FriendRoomInvitationRecv( FHP_FRIEND_INVITATION_RET* lpMsg);
void GEMailMessageSend(char* name,char* target,char* subject,BYTE dir,BYTE action,BYTE* photo,char* text);
