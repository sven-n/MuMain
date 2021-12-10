#include "stdafx.h"
#include "CSProtocol.h"
#include "GameMain.h"
#include "ServerInfo.h"
#include "Util.h"

void CSDataSend(BYTE* lpMsg,int size)
{
	switch(lpMsg[0])
	{
		case 0xC1:
			{
				BYTE send[8192];

				PSBMSG_HEAD pMsg;

				pMsg.set(0xE1,lpMsg[2],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[4],&lpMsg[3],(size-4));

				gDataServerConnection.DataSend(send,size);
			}
			break;
		case 0xC2:
			{
				BYTE send[8192];

				PSWMSG_HEAD pMsg;

				pMsg.set(0xE1,lpMsg[3],(size=(size+1)));

				memcpy(&send[0],&pMsg,sizeof(pMsg));

				memcpy(&send[5],&lpMsg[4],(size-5));

				gDataServerConnection.DataSend(send,size);
			}
			break;
	}
}

void CSDataRecv(BYTE head,BYTE* lpMsg,int size) // OK
{
	switch(lpMsg[0])
	{
		case 0xC1:
			{
				BYTE recv[8192];

				PBMSG_HEAD pMsg;

				pMsg.set(lpMsg[3],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[3],&lpMsg[4],(size-3));

				ChatServerProtocolCore(pMsg.head,recv,size);
			}
			break;
		case 0xC2:
			{
				BYTE recv[8192];

				PWMSG_HEAD pMsg;

				pMsg.set(lpMsg[4],(size=(size-1)));

				memcpy(&recv[0],&pMsg,sizeof(pMsg));

				memcpy(&recv[4],&lpMsg[5],(size-4));

				ChatServerProtocolCore(pMsg.head,recv,size);
			}
			break;
	}
}

void ChatServerProtocolCore(BYTE head,BYTE* lpMsg,int size) // OK
{
	BYTE* aRecv = lpMsg;

	switch(head)
	{
		case 0x60:
			FriendListResult(aRecv);
			break;
		case 0x61:
			WaitFriendListResult((FHP_WAITFRIENDLIST_COUNT *)aRecv);
			break;
		case 0x62:
			FriendStateRecv((FHP_FRIEND_STATE *)aRecv);
			break;
		case 0x63:
			FriendAddResult((FHP_FRIEND_ADD_RESULT *)aRecv);
			break;
		case 0x64:
			WaitFriendAddResult((FHP_WAITFRIEND_ADD_RESULT *)aRecv);
			break;
		case 0x65:
			FriendDelResult((FHP_FRIEND_DEL_RESULT *)aRecv);
			break;
		case 0x66:
			FriendChatRoomCreateResult((FHP_FRIEND_CHATROOM_CREATE_RESULT *)aRecv);
			break;
		case 0x70:
			FriendMemoSendResult((FHP_FRIEND_MEMO_SEND_RESULT *)aRecv);
			break;
		case 0x71:
			FriendMemoList((FHP_FRIEND_MEMO_LIST *)aRecv);
			break;
		case 0x72:
			FriendMemoRead((FHP_FRIEND_MEMO_RECV *)aRecv);
			break;
		case 0x73:
			FriendMemoDelResult((FHP_FRIEND_MEMO_DEL_RESULT *)aRecv);
			break;
		case 0x74:
			FriendRoomInvitationRecv((FHP_FRIEND_INVITATION_RET *)aRecv);
			break;
	}
}

void FriendListRequest(int aIndex)
{
	if ( gObjIsConnectedGP(aIndex) == FALSE )
	{
		return;
	}

	FHP_FRIENDLIST_REQ pMsg;

	pMsg.h.set(0x60, sizeof(FHP_FRIENDLIST_REQ));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	pMsg.Number = aIndex;
	pMsg.pServer = (BYTE)gServerInfo.m_ServerCode;

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendListResult(LPBYTE lpMsg)
{
	FHP_FRIENDLIST_COUNT * lpCount;
	FHP_FRIENDLIST * lpList;
	int lOfs = sizeof(FHP_FRIENDLIST_COUNT);
	PMSG_FRIEND_LIST_COUNT pCount;
	PMSG_FRIEND_LIST pList;
	int pOfs = sizeof(PMSG_FRIEND_LIST_COUNT);

	lpCount = (FHP_FRIENDLIST_COUNT *)lpMsg;

	BYTE TmpSendBuf[2000];
	
	if ( !gObjIsNameValid(lpCount->Number, lpCount->Name))
	{
		return;
	}

	pCount.Count = lpCount->Count;
	pCount.MemoCount = lpCount->MailCount;
	pCount.MailTotal = 50;

	if ( pCount.Count != 0 )
	{
		for ( int n=0;n<pCount.Count;n++)
		{
			lpList = (FHP_FRIENDLIST *)&lpMsg[lOfs];

			memcpy(pList.Name,lpList->Name,sizeof(pList.Name));

			pList.Server = lpList->Server;

			memcpy(&TmpSendBuf[pOfs], &pList, sizeof(PMSG_FRIEND_LIST));

			lOfs+=sizeof(FHP_FRIENDLIST);
			pOfs+=sizeof(PMSG_FRIEND_LIST);
		}
	}

	pCount.h.set(0xC0, pOfs);
	memcpy(TmpSendBuf, &pCount, sizeof(PMSG_FRIEND_LIST_COUNT));

	DataSend(lpCount->Number, TmpSendBuf, pOfs);
}

void WaitFriendListResult(FHP_WAITFRIENDLIST_COUNT * lpMsg)
{
	PMSG_FRIEND_ADD_SIN_REQ pMsg;

	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	pMsg.h.set(0xC2, sizeof(pMsg));
	memcpy(pMsg.Name, lpMsg->FriendName, sizeof(pMsg.Name));

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendStateClientRecv(PMSG_FRIEND_STATE_C * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_STATE_C pMsg;

	pMsg.h.set(0x62, sizeof(pMsg));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	pMsg.State = lpMsg->ChatVeto;

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendStateRecv(FHP_FRIEND_STATE * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_STATE pMsg;

	pMsg.h.set(0xC4, sizeof(pMsg));
	memcpy(pMsg.Name, lpMsg->FriendName, sizeof(pMsg.Name));
	pMsg.State = lpMsg->State;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendAddRequest(PMSG_FRIEND_ADD_REQ * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	//Season 4.5 Addon start
	LPOBJ find = gObjFind(lpMsg->Name);

	int aTargetIndex = ((find==0)?-1:find->Index);
 
    if(aTargetIndex >= 0 && aTargetIndex <= MAX_OBJECT-1 && gObj[aTargetIndex].Type != 1  )
    {
		CloseClient(aIndex);
		return;
	}
	//Season 4.5 Addon end

	FHP_FRIEND_ADD_REQ pMsg;

	pMsg.h.set(0x63, sizeof(pMsg));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	memset(pMsg.FriendName, 0 , sizeof(pMsg.FriendName));
	memcpy(pMsg.FriendName, lpMsg->Name, sizeof(lpMsg->Name));
	pMsg.Number = aIndex;

	char szName[11];

	memset(szName,0,sizeof(szName));

	memcpy(szName,lpMsg->Name,sizeof(lpMsg->Name));

	if ( !strcmp(gObj[aIndex].Name, szName))
	{
		PMSG_FRIEND_ADD_RESULT pResult;

		pResult.h.set(0xC1, sizeof(pResult));
		memcpy(pResult.Name, lpMsg->Name, sizeof(pMsg.Name));
		pResult.Result = 5;
		pResult.State = -1;

		DataSend(aIndex, (LPBYTE)&pResult, sizeof(pResult));
	}
	else
	{
		CSDataSend((BYTE*)&pMsg, pMsg.h.size);
	}
}

void FriendAddResult(FHP_FRIEND_ADD_RESULT * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_ADD_RESULT pMsg;

	pMsg.h.set(0xC1, sizeof(pMsg));
	memcpy(pMsg.Name, lpMsg->FriendName, sizeof(pMsg.Name));
	pMsg.Result = lpMsg->Result;
	pMsg.State = lpMsg->Server;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void WaitFriendAddRequest(PMSG_FRIEND_ADD_SIN_RESULT * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_WAITFRIEND_ADD_REQ pMsg;

	pMsg.h.set(0x64, sizeof(pMsg));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	memset(pMsg.FriendName, 0 , sizeof(pMsg.FriendName));
	memcpy(pMsg.FriendName, lpMsg->Name, sizeof(lpMsg->Name));
	pMsg.Result = lpMsg->Result;
	pMsg.Number = aIndex;

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void WaitFriendAddResult(FHP_WAITFRIEND_ADD_RESULT * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_ADD_RESULT pMsg;

	pMsg.h.set(0xC1, sizeof(pMsg));
	memcpy(pMsg.Name, lpMsg->FriendName, sizeof(pMsg.Name));
	pMsg.Result = lpMsg->Result;
	pMsg.State = lpMsg->pServer;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendDelRequest(PMSG_FRIEND_DEL_REQ * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_ADD_REQ pMsg;

	pMsg.h.set(0x65, sizeof(pMsg));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	memset(pMsg.FriendName, 0 , sizeof(pMsg.FriendName));
	memcpy(pMsg.FriendName, lpMsg->Name, sizeof(lpMsg->Name));
	pMsg.Number = aIndex;

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendDelResult(FHP_FRIEND_DEL_RESULT * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_DEL_RESULT pMsg;

	pMsg.h.set(0xC3, sizeof(pMsg));
	memcpy(pMsg.Name, lpMsg->FriendName, sizeof(pMsg.Name));
	pMsg.Result = lpMsg->Result;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendChatRoomCreateReq(PMSG_FRIEND_ROOMCREATE_REQ * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_CHATROOM_CREATE_REQ pMsg;

	pMsg.h.set(0x66, sizeof(pMsg));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	pMsg.Number = aIndex;
	memset(pMsg.fName, 0 , sizeof(pMsg.fName));
	memcpy(pMsg.fName, lpMsg->Name, sizeof(lpMsg->Name));
	
	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendChatRoomCreateResult(FHP_FRIEND_CHATROOM_CREATE_RESULT * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_ROOMCREATE_RESULT pMsg;

	pMsg.h.set(0xCA, sizeof(pMsg));
	memcpy(pMsg.ServerIp, lpMsg->ServerIp, sizeof(pMsg.ServerIp));
	pMsg.RoomNumber = lpMsg->RoomNumber;
	pMsg.Ticket = lpMsg->Ticket;
	pMsg.Type = lpMsg->Type;
	pMsg.Result = lpMsg->Result;
	memcpy(pMsg.Name, lpMsg->FriendName, sizeof(pMsg.Name));

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendMemoSend(PMSG_FRIEND_MEMO * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	if ( gObj[aIndex].Money < 1000 )
	{
		FHP_FRIEND_MEMO_SEND_RESULT pResult;

		pResult.Number = aIndex;
		pResult.Result = 7;
		pResult.WindowGuid = lpMsg->WindowGuid;
		memcpy(pResult.Name, gObj[aIndex].Name, sizeof(pResult.Name));

		FriendMemoSendResult(&pResult);

		return;
	}

	if ( lpMsg->MemoSize < 0 || lpMsg->MemoSize > 1000 )
	{
		FHP_FRIEND_MEMO_SEND_RESULT pResult;

		pResult.Number = aIndex;
		pResult.Result = 0;
		pResult.WindowGuid = lpMsg->WindowGuid;
		memcpy(pResult.Name, gObj[aIndex].Name, sizeof(pResult.Name));
		FriendMemoSendResult(&pResult);
		return;
	}

	char szName[11];

	memset(szName,0,sizeof(szName));

	memcpy(szName,lpMsg->Name,sizeof(lpMsg->Name));

	if ( strlen(szName) < 1 )
	{
		FHP_FRIEND_MEMO_SEND_RESULT pResult;

		pResult.Number = aIndex;
		pResult.Result = 0;
		pResult.WindowGuid = lpMsg->WindowGuid;
		memcpy(pResult.Name, gObj[aIndex].Name, sizeof(pResult.Name));
		FriendMemoSendResult(&pResult);
		return;
	}

	FHP_FRIEND_MEMO_SEND pMsg;
	int bsize = lpMsg->MemoSize + sizeof(pMsg) - sizeof(pMsg.Memo);

	pMsg.h.set(0x70, bsize);
	pMsg.WindowGuid = lpMsg->WindowGuid;
	pMsg.MemoSize = lpMsg->MemoSize;
	pMsg.Number = aIndex;
	pMsg.Dir = lpMsg->Dir;
	pMsg.Action = lpMsg->Action;

	memcpy(pMsg.Subject, lpMsg->Subject, sizeof(pMsg.Subject));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));
	memcpy(pMsg.Photo, gObj[aIndex].CharSet, sizeof(pMsg.Photo));
	memset(pMsg.ToName, 0 , sizeof(pMsg.ToName));
	memcpy(pMsg.ToName, lpMsg->Name, sizeof(lpMsg->Name));
	memcpy(pMsg.Memo, lpMsg->Memo, lpMsg->MemoSize);

	CSDataSend((BYTE*)&pMsg, bsize);
}

void FriendMemoSendResult(FHP_FRIEND_MEMO_SEND_RESULT * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_MEMO_RESULT pMsg;

	pMsg.h.set(0xC5, sizeof(pMsg));
	pMsg.Result = lpMsg->Result;
	pMsg.WindowGuid = lpMsg->WindowGuid;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));

	if ( pMsg.Result == 1 )
	{
		if ( gObj[lpMsg->Number].Money >= ((DWORD)1000) )
		{
			gObj[lpMsg->Number].Money -= 1000;
			GCMoneySend(gObj[lpMsg->Number].Index, gObj[lpMsg->Number].Money);
		}
	}
}

void FriendMemoListReq(int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_MEMO_LIST_REQ pMsg;

	pMsg.h.set(0x71, sizeof(pMsg));
	pMsg.Number = aIndex;
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendMemoList(FHP_FRIEND_MEMO_LIST * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->RecvName))
	{
		return;
	}

	PMSG_FRIEND_MEMO_LIST pMsg;

	pMsg.h.set(0xC6, sizeof(pMsg));
	memcpy(pMsg.Date, lpMsg->Date, sizeof(pMsg.Date));
	memcpy(pMsg.Name, lpMsg->SendName, sizeof(pMsg.Name));
	memcpy(pMsg.Subject, lpMsg->Subject, sizeof(lpMsg->Subject));
	pMsg.Number = lpMsg->MemoIndex;
	pMsg.read = lpMsg->read;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendMemoReadReq(PMSG_FRIEND_READ_MEMO_REQ * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_MEMO_RECV_REQ pMsg;

	pMsg.h.set(0x72, sizeof(pMsg));
	pMsg.MemoIndex = lpMsg->MemoIndex;
	pMsg.Number = aIndex;
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendMemoRead(FHP_FRIEND_MEMO_RECV * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_READ_MEMO pMsg;

	int nsize = sizeof(pMsg)-sizeof(pMsg.Memo);

	if ( lpMsg->MemoSize > sizeof(pMsg.Memo) )
	{
		return;
	}

	nsize += lpMsg->MemoSize;

	pMsg.h.set(0xC7, nsize);
	pMsg.MemoIndex = lpMsg->MemoIndex;
	pMsg.MemoSize = lpMsg->MemoSize;
	pMsg.Dir = lpMsg->Dir;
	pMsg.Action = lpMsg->Action;
	memcpy(pMsg.Photo, lpMsg->Photo,sizeof(pMsg.Photo));
	memcpy(pMsg.Memo, lpMsg->Memo, lpMsg->MemoSize);

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, nsize);
}

void FriendMemoDelReq(PMSG_FRIEND_MEMO_DEL_REQ * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_MEMO_DEL_REQ pMsg;

	pMsg.h.set(0x73, sizeof(pMsg));
	pMsg.MemoIndex = lpMsg->MemoIndex;
	pMsg.Number = aIndex;
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendMemoDelResult(FHP_FRIEND_MEMO_DEL_RESULT * lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_FRIEND_MEMO_DEL_RESULT pMsg;

	pMsg.h.set(0xC8, sizeof(pMsg));
	pMsg.MemoIndex = lpMsg->MemoIndex;
	pMsg.Result = lpMsg->Result;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void FriendRoomInvitationReq(PMSG_ROOM_INVITATION * lpMsg, int aIndex)
{
	if ( !gObjIsConnectedGP(aIndex))
	{
		return;
	}

	FHP_FRIEND_INVITATION_REQ pMsg;

	pMsg.h.set(0x74, sizeof(pMsg));
	pMsg.Number = aIndex;
	pMsg.RoomNumber = lpMsg->RoomNumber;
	pMsg.WindowGuid = lpMsg->WindowGuid;
	memset(pMsg.FriendName, 0 , sizeof(pMsg.FriendName));
	memcpy(pMsg.FriendName, lpMsg->Name, sizeof(lpMsg->Name));
	memcpy(pMsg.Name, gObj[aIndex].Name, sizeof(pMsg.Name));

	CSDataSend((BYTE*)&pMsg, pMsg.h.size);
}

void FriendRoomInvitationRecv(FHP_FRIEND_INVITATION_RET* lpMsg)
{
	if ( !gObjIsNameValid(lpMsg->Number, lpMsg->Name))
	{
		return;
	}

	PMSG_ROOM_INVITATION_RESULT pMsg;

	pMsg.h.set(0xCB, sizeof(pMsg));
	pMsg.Result = lpMsg->Result;
	pMsg.WindowGuid = lpMsg->WindowGuid;

	DataSend(lpMsg->Number, (LPBYTE)&pMsg, sizeof(pMsg));
}

void GEMailMessageSend(char* name,char* target,char* subject,BYTE dir,BYTE action,BYTE* photo,char* text) // OK
{
	int size = strlen(text);

	FHP_FRIEND_MEMO_SEND pMsg;

	pMsg.h.set(0x70,(sizeof(pMsg)-(sizeof(pMsg.Memo)-size)));

	pMsg.Number = -1;

	pMsg.WindowGuid = 0;

	memcpy(pMsg.Name,name,sizeof(pMsg.Name));

	memcpy(pMsg.ToName,target,sizeof(pMsg.ToName));

	memcpy(pMsg.Subject,subject,sizeof(pMsg.Subject));

	pMsg.Dir = dir;

	pMsg.Action = action;

	pMsg.MemoSize = size;

	memcpy(pMsg.Photo,photo,sizeof(pMsg.Photo));

	memcpy(pMsg.Memo,text,size);

	CSDataSend((BYTE*)&pMsg,(sizeof(pMsg)-(sizeof(pMsg.Memo)-size)));
}
